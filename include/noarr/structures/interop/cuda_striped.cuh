#ifndef NOARR_STRUCTURES_CUDA_STRIPED_HPP
#define NOARR_STRUCTURES_CUDA_STRIPED_HPP

#include "../base/contain.hpp"
#include "../base/signature.hpp"
#include "../base/state.hpp"
#include "../base/structs_common.hpp"
#include "../base/utility.hpp"
#include "../extra/struct_traits.hpp"
#include "../structs/scalar.hpp"

namespace noarr {

namespace helpers {

template<std::size_t Value, std::size_t Mul>
constexpr std::size_t pad_to_multiple = (Value + Mul - 1) / Mul * Mul;

struct simple_cg_t : contain<std::size_t, std::size_t> {
	using base = contain<std::size_t, std::size_t>;
	using base::base;

	constexpr std::size_t thread_rank() const noexcept { return base::template get<0>(); }
	constexpr std::size_t num_threads() const noexcept { return base::template get<1>(); }
};

} // namespace helpers

constexpr std::size_t cuda_shm_bank_count = 32;
constexpr std::size_t cuda_shm_bank_width = 4;

// Tag for use in state
struct cuda_stripe_index;

template<std::size_t NumStripes, class ElemType, std::size_t BankCount, std::size_t BankWidth, class T>
struct cuda_striped_t : contain<T> {
	static_assert(is_struct<ElemType>(), "The element type of cuda_striped must be a noarr structure.");

	static constexpr char name[] = "cuda_striped_t";
	using params = struct_params<
		value_param<std::size_t, NumStripes>,
		structure_param<ElemType>,
		value_param<std::size_t, BankCount>,
		value_param<std::size_t, BankWidth>,
		structure_param<T>>;

	constexpr cuda_striped_t() noexcept = default;
	explicit constexpr cuda_striped_t(T sub_structure) noexcept : contain<T>(sub_structure) {}

	constexpr T sub_structure() const noexcept { return contain<T>::template get<0>(); }

private:
	static constexpr std::size_t elem_size = decltype(std::declval<ElemType>().size(state<>()))::value;
	// the stripe width in bytes, forbidding bank conflicts
	static constexpr std::size_t tmp_stripe_padded_width = (BankCount / NumStripes) * BankWidth;
	// if the stripe is too narrow for even a single element, enlarge stripe just enough (at the cost of conflicts)
	static constexpr std::size_t stripe_padded_width = (tmp_stripe_padded_width < elem_size ? helpers::pad_to_multiple<elem_size, BankWidth> : tmp_stripe_padded_width);
	// how many successive elements fit in the stripe width
	static constexpr std::size_t stripe_width_elems = stripe_padded_width / elem_size;
	// stripe width, in bytes, without stripe padding --- i.e. how many successive bytes of the original structure will be successive in the new structure
	static constexpr std::size_t stripe_width = stripe_width_elems * elem_size;
	// the period after which we return to stripe 0 --- i.e. the width of all stripes, including stripe padding, and including possible additional padding at the end
	static constexpr std::size_t total_width = helpers::pad_to_multiple<stripe_padded_width * NumStripes, BankCount * BankWidth>;
public:
	// max possible number of threads accessing the same bank, always nonzero, 1 means no conflicts
	static constexpr std::size_t max_conflict_size = total_width / (BankCount * BankWidth);

	using signature = typename T::signature;

	template<class State>
	constexpr auto size(State state) const noexcept {
		using namespace constexpr_arithmetic;
		// substructure size
		auto sub_size = sub_structure().size(state.template remove<cuda_stripe_index>());
		// total elements in each stripe = total elements in sub-structure
		auto sub_elements = sub_size / make_const<elem_size>();
		// stripe length = ceil(total elements in stripe / total elements in stripe width)
		auto stripe_len = (sub_elements + make_const<stripe_width_elems - 1>()) / make_const<stripe_width_elems>();
		// total size = stripe length (i.e. total length) * total width
		return stripe_len * make_const<total_width>();
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		using namespace constexpr_arithmetic;
		auto sub_offset = offset_of<Sub>(sub_structure(), state.template remove<cuda_stripe_index>());
		auto offset_major = sub_offset / make_const<stripe_width>();
		if constexpr(std::is_same_v<Sub, ElemType> && stripe_width_elems == 1) {
			// Optimization: offset_minor should be zero.
			return offset_inner(state, offset_major);
		} else {
			auto offset_minor = sub_offset % make_const<stripe_width>();
			return offset_inner(state, offset_major) + offset_minor;
		}
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		return sub_structure().template length<QDim>(state.template remove<cuda_stripe_index>());
	}

	template<class Sub, class State>
	constexpr void strict_state_at(State) const noexcept {
		static_assert(always_false<cuda_striped_t>, "A cuda_striped_t cannot be used in this context");
	}

	static __device__ inline std::size_t current_stripe_index() noexcept {
		return threadIdx.x % NumStripes;
	}

	static __device__ inline std::size_t num_stripes() noexcept {
		return NumStripes;
	}

	static __device__ inline helpers::simple_cg_t current_stripe_cg() noexcept {
		std::size_t stripe_index = threadIdx.x % NumStripes;
		std::size_t num_threads_in_stripe = (blockDim.x + NumStripes - stripe_index - 1) / NumStripes;
		std::size_t thread_rank_in_stripe = threadIdx.x / NumStripes;
		return helpers::simple_cg_t{thread_rank_in_stripe, num_threads_in_stripe};
	}

private:
	template<class State, class Idx>
	constexpr auto offset_inner(State state, Idx index_of_period) const noexcept {
		using namespace constexpr_arithmetic;
		auto offset_of_period = index_of_period * make_const<total_width>();
		if constexpr(State::template contains<cuda_stripe_index>) {
			auto offset_of_stripe = state.template get<cuda_stripe_index>() * make_const<stripe_padded_width>();
			return offset_of_period + offset_of_stripe;
		} else {
			std::size_t offset_of_stripe = (threadIdx.x % NumStripes) * stripe_padded_width;
			return offset_of_period + offset_of_stripe;
		}
	}
};

template<std::size_t NumStripes, class ElemType, std::size_t BankCount, std::size_t BankWidth>
struct cuda_striped_proto {
	static_assert(is_struct<ElemType>(), "The element type of cuda_striped must be a noarr structure. Omit the type to imply scalar<...>, or specify scalar<...> (or any other noarr structure) explicitly.");
	static constexpr bool proto_preserves_layout = false;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return cuda_striped_t<NumStripes, ElemType, BankCount, BankWidth, Struct>(s); }
};

template<std::size_t NumStripes, class ElemType, std::size_t BankCount = noarr::cuda_shm_bank_count, std::size_t BankWidth = noarr::cuda_shm_bank_width>
constexpr auto cuda_striped() noexcept { return cuda_striped_proto<NumStripes, ElemType, BankCount, BankWidth>(); }



template<std::size_t NumStripes, std::size_t BankCount, std::size_t BankWidth, class T>
using cuda_scalar_striped_t = cuda_striped_t<NumStripes, scalar<scalar_t<T>>, BankCount, BankWidth, T>;

template<std::size_t NumStripes, std::size_t BankCount, std::size_t BankWidth>
struct cuda_scalar_striped_proto {
	static constexpr bool proto_preserves_layout = false;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return cuda_scalar_striped_t<NumStripes, BankCount, BankWidth, Struct>(s); }
};

template<std::size_t NumStripes, std::size_t BankCount = noarr::cuda_shm_bank_count, std::size_t BankWidth = noarr::cuda_shm_bank_width>
constexpr auto cuda_striped() noexcept { return cuda_scalar_striped_proto<NumStripes, BankCount, BankWidth>(); }

} // namespace noarr

#endif // NOARR_STRUCTURES_CUDA_STRIPED_HPP
