#ifndef NOARR_STRUCTURES_ZCURVE_HPP
#define NOARR_STRUCTURES_ZCURVE_HPP

#include "../base/contain.hpp"
#include "../base/signature.hpp"
#include "../base/state.hpp"
#include "../base/structs_common.hpp"
#include "../base/utility.hpp"

namespace noarr {

namespace helpers {

template<char...>
struct zc_uniquity;
template<char Dim, char... Dims>
struct zc_uniquity<Dim, Dims...> {
	static constexpr bool value = (... && (Dims != Dim)) && zc_uniquity<Dims...>::value;
};
template<>
struct zc_uniquity<> : std::true_type {};

template<class, class>
struct zc_merged_len { using type = dynamic_arg_length; };
template<std::size_t L0, std::size_t L1>
struct zc_merged_len<static_arg_length<L0>, static_arg_length<L1>> { using type = static_arg_length<L0 * L1>; };

// std::integral_constant but less annoying
template<std::size_t Value>
struct zc_constexpr { static constexpr auto v = Value; };

template<std::size_t... I, class F>
constexpr void zc_static_for(std::index_sequence<I...>, F f) noexcept {
	(..., f(zc_constexpr<I>()));
}

template<std::size_t... I, class F>
constexpr auto zc_product_static_for(std::index_sequence<I...>, F f) noexcept {
	return (... * f(zc_constexpr<I>()));
}

template<std::size_t Levels, class... SizeTs>
constexpr std::tuple<SizeTs...> zc_general(std::size_t z, SizeTs... sizes) noexcept {
	static_assert((... && std::is_same_v<SizeTs, std::size_t>), "bug");
	using EachDim = std::index_sequence_for<SizeTs...>;
	std::tuple<SizeTs...> size = {sizes...};
	std::tuple<SizeTs...> result = {SizeTs(0)...};
	zc_static_for(std::make_index_sequence<Levels>(), [&](auto k) {
		using level = zc_constexpr<Levels - decltype(k)::v - 1>;
		using small_tile_size = zc_constexpr<(std::size_t) 1 << level::v>;
		zc_static_for(EachDim(), [&](auto ic) {
			using i = decltype(ic);
			std::size_t facet = zc_product_static_for(EachDim(), [&](auto jc) {
				using j = decltype(jc);
				if constexpr(j::v == i::v) {
					return 1;
				} else {
					constexpr std::size_t tile_size = (j::v > i::v ? 2 : 1) * small_tile_size::v;
					return (std::get<j::v>(size) & -tile_size) == std::get<j::v>(result) ? ((std::get<j::v>(size) - 1) & (tile_size - 1)) + 1 : tile_size;
				}
			});
			std::size_t half_volume = facet << level::v;
			if(z >= half_volume) {
				z -= half_volume;
				std::get<i::v>(result) += small_tile_size::v;
			}
		});
	});
	return std::tuple<SizeTs...>(result); // force copy, so that `result` is not aliased due to copy elision
}

template<int Period, std::size_t RepBits = 0, class = void>
struct zc_special_helper {
	using rec = zc_special_helper<2 * Period, (RepBits | RepBits << Period)>;
	static constexpr std::size_t rep_bits = rec::rep_bits;
	static constexpr int num_iter = rec::num_iter + 1;
};
template<int Period, std::size_t RepBits>
struct zc_special_helper<Period, RepBits, std::enable_if_t<Period >= sizeof RepBits * 8>> {
	static constexpr std::size_t rep_bits = RepBits;
	static constexpr int num_iter = 0;
};

template<int NDim, int... I>
constexpr std::size_t zc_special_inner(std::size_t tmp, std::integer_sequence<int, I...>) noexcept {
	(..., (tmp &= zc_special_helper<(NDim << I), ((std::size_t) 1 << (1 << I)) - 1>::rep_bits, tmp |= tmp >> ((NDim - 1) << I)));
	return tmp & (((std::size_t) 1 << (1 << sizeof...(I))) - 1);
}

template<int NDim, int Dim>
constexpr std::size_t zc_special(std::size_t z) noexcept {
	static_assert(0 <= Dim && Dim < NDim, "bug");
	return zc_special_inner<NDim>(z >> (NDim-Dim-1), std::make_integer_sequence<int, zc_special_helper<NDim>::num_iter>());
}

template<class Acc, char...>
struct zc_dims_pop;
template<char... Acc, char Head, char... Tail>
struct zc_dims_pop<std::integer_sequence<char, Acc...>, Head, Tail...> : zc_dims_pop<std::integer_sequence<char, Acc..., Head>, Tail...> {};
template<char... Acc, char Last>
struct zc_dims_pop<std::integer_sequence<char, Acc...>, Last> {
	static constexpr char dim = Last;
	using dims = std::integer_sequence<char, Acc...>;
};

template<std::size_t N>
struct zc_log2 {
	static_assert(N && !(N & 1), "Z curve length bound and alignment must be powers of two");
	static constexpr int value = zc_log2<(N>>1)>::value + 1;
};
template<>
struct zc_log2<1> {
	static constexpr int value = 0;
};

} // namespace helpers

template<int SpecialLevel, int GeneralLevel, char Dim, class T, char... Dims>
struct merge_zcurve_t : contain<T> {
	using base = contain<T>;
	using base::base;

	static constexpr char name[] = "merge_zcurve_t";
	using params = struct_params<
		value_param<int, SpecialLevel>,
		value_param<int, GeneralLevel>,
		dim_param<Dim>,
		structure_param<T>,
		dim_param<Dims>...>;

	constexpr T sub_structure() const noexcept { return base::template get<0>(); }

	static_assert(SpecialLevel <= GeneralLevel && GeneralLevel < 8*sizeof(std::size_t), "Invalid parameters");
	static_assert(sizeof...(Dims), "No dimensions to merge");
	static_assert(sizeof...(Dims) <= 8*sizeof(std::size_t), "Too many dimensions to merge");
	static_assert(helpers::zc_uniquity<Dims...>::value, "Cannot merge a dimension with itself");
	static_assert((... || (Dim == Dims)) || !T::signature::template any_accept<Dim>, "Dimension of this name already exists");
private:
	template<int Remaining, class ArgLenAcc>
	struct dim_replacer {
		template<class Original>
		struct replacement {
			static_assert(!Original::dependent, "Cannot merge a tuple index");
			static_assert(Original::arg_length::is_known, "The dimension lengths must be set before merging");

			static constexpr int remaining = Remaining - 1;
			using arg_len_acc = typename helpers::zc_merged_len<ArgLenAcc, typename Original::arg_length>::type;

			using type = typename Original::ret_sig::template replace<dim_replacer<remaining, arg_len_acc>::template replacement, Dims...>;
		};
	};
	template<class ArgLenAcc>
	struct dim_replacer<1, ArgLenAcc> {
		template<class Original>
		struct replacement {
			static_assert(!Original::dependent, "Cannot merge a tuple index");
			static_assert(Original::arg_length::is_known, "The dimension lengths must be set before merging");

			using merged_len = typename helpers::zc_merged_len<ArgLenAcc, typename Original::arg_length>::type;

			using type = function_sig<Dim, merged_len, typename Original::ret_sig>;
		};
	};
	using outer_dim_replacer = dim_replacer<sizeof...(Dims), static_arg_length<1>>;
public:
	using signature = typename T::signature::template replace<outer_dim_replacer::template replacement, Dims...>;

	using is = std::make_index_sequence<sizeof...(Dims)>;

	template<class State, std::size_t... DimsI>
	constexpr auto sub_state(State state, std::index_sequence<DimsI...>) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set z-curve length");
		auto clean_state = state.template remove<index_in<Dim>, index_in<Dims>..., length_in<Dims>...>();
		if constexpr(State::template contains<index_in<Dim>>) {
			auto index = state.template get<index_in<Dim>>();
			auto index_general = index >> SpecialLevel*sizeof...(Dims);
			auto index_special = index & ((1 << SpecialLevel*sizeof...(Dims)) - 1);
			auto indices = helpers::zc_general<GeneralLevel-SpecialLevel>(index_general, (sub_structure().template length<Dims>(clean_state) >> SpecialLevel)...);
			return clean_state.template with<index_in<Dims>...>(((std::get<DimsI>(indices) << SpecialLevel) + helpers::zc_special<sizeof...(Dims), DimsI>(index_special))...);
		} else {
			return clean_state;
		}
	}

	template<class State>
	constexpr auto size(State state) const noexcept {
		return sub_structure().size(sub_state(state, is()));
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(State::template contains<index_in<Dim>>, "Index has not been set");
		return offset_of<Sub>(sub_structure(), sub_state(state, is()));
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		static_assert(!State::template contains<index_in<QDim>>, "This dimension is already fixed, it cannot be used from outside");
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set z-curve length");
		if constexpr(QDim == Dim) {
			auto clean_state = state.template remove<index_in<Dim>, length_in<Dim>>();
			return (... * sub_structure().template length<Dims>(clean_state));
		} else {
			return sub_structure().template length<QDim>(sub_state(state, is()));
		}
	}

	template<class Sub, class State>
	constexpr auto strict_state_at(State state) const noexcept {
		return state_at<Sub>(sub_structure(), sub_state(state, is()));
	}
};

template<int SpecialLevel, int GeneralLevel, char Dim, char... Dims>
struct merge_zcurve_proto {
	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return merge_zcurve_t<SpecialLevel, GeneralLevel, Dim, Struct, Dims...>(s); }
};

template<char... AllDims>
struct merge_zcurve {
private:
	using dims_pop = helpers::zc_dims_pop<std::integer_sequence<char>, AllDims...>;
	struct error { static_assert(always_false<merge_zcurve<AllDims...>>, "Do not instantiate this type directly, use merge_zcurve<'original dims', 'new dim'>::maxlen_alignment<len, alignment>()"); };

public:
	template<class = error>
	merge_zcurve(error = {});

	template<std::size_t MaxLen, std::size_t Alignment>
	static constexpr auto maxlen_alignment() noexcept {
		return maxlen_alignment<helpers::zc_log2<Alignment>::value, helpers::zc_log2<MaxLen>::value, dims_pop::dim>(typename dims_pop::dims());
	}

private:
	template<int SpecialLevel, int GeneralLevel, char Dim, char... Dims>
	static constexpr merge_zcurve_proto<SpecialLevel, GeneralLevel, Dim, Dims...> maxlen_alignment(std::integer_sequence<char, Dims...>) noexcept {
		return {};
	}
};

} // namespace noarr

#endif // NOARR_STRUCTURES_ZCURVE_HPP
