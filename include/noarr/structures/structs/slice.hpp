#ifndef NOARR_STRUCTURES_SLICE_HPP
#define NOARR_STRUCTURES_SLICE_HPP

#include "../base/contain.hpp"
#include "../base/signature.hpp"
#include "../base/state.hpp"
#include "../base/structs_common.hpp"
#include "../base/utility.hpp"

namespace noarr {

template<char Dim, class T, class StartT>
struct shift_t : contain<T, StartT> {
	using base = contain<T, StartT>;
	using base::base;

	static constexpr char name[] = "shift_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>>;

	constexpr T sub_structure() const noexcept { return base::template get<0>(); }
	constexpr StartT start() const noexcept { return base::template get<1>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> {
		template<class L, class S>
		struct subtract { using type = dynamic_arg_length; };
		template<class S>
		struct subtract<unknown_arg_length, S> { using type = unknown_arg_length; };
		template<std::size_t L, std::size_t S>
		struct subtract<static_arg_length<L>, std::integral_constant<std::size_t, S>> { using type = static_arg_length<L-S>; };
		using type = function_sig<Dim, typename subtract<ArgLength, StartT>::type, RetSig>;
	};
	template<class... RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot shift a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t len = sizeof...(RetSigs) - start;

		template<class Indices = std::make_index_sequence<len>>
		struct pack_helper;
		template<std::size_t... Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<class State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		auto tmp_state = state.template remove<index_in<Dim>, length_in<Dim>>();
		if constexpr(State::template contains<index_in<Dim>>)
			if constexpr(State::template contains<length_in<Dim>>)
				return tmp_state.template with<index_in<Dim>, length_in<Dim>>(state.template get<index_in<Dim>>() + start(), state.template get<length_in<Dim>>() + start());
			else
				return tmp_state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() + start());
		else
			if constexpr(State::template contains<length_in<Dim>>)
				return tmp_state.template with<length_in<Dim>>(state.template get<length_in<Dim>>() + start());
			else
				return tmp_state;
	}

	template<class State>
	constexpr auto size(State state) const noexcept {
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(QDim == Dim) {
			static_assert(!State::template contains<index_in<Dim>>, "Index already set");
			if constexpr(State::template contains<length_in<Dim>>) {
				return state.template get<length_in<Dim>>();
			} else {
				return sub_structure().template length<Dim>(state.template remove<index_in<Dim>, length_in<Dim>>()) - start();
			}
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, class State>
	constexpr auto strict_state_at(State state) const noexcept {
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<char Dim, class StartT>
struct shift_proto : contain<StartT> {
	using base = contain<StartT>;
	using base::base;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return shift_t<Dim, Struct, StartT>(s, base::template get<0>()); }
};

/**
 * @brief shifts an index (or indices) given by dimension name(s) in a structure
 * 
 * @tparam Dim: the dimension names
 * @param start: parameters for shifting the indices
 */
template<char... Dim, class... StartT>
constexpr auto shift(StartT... start) noexcept { return (... ^ shift_proto<Dim, good_index_t<StartT>>(start)); }

template<>
constexpr auto shift<>() noexcept { return neutral_proto(); }

template<char Dim, class T, class StartT, class LenT>
struct slice_t : contain<T, StartT, LenT> {
	using base = contain<T, StartT, LenT>;
	using base::base;

	static constexpr char name[] = "slice_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>,
		type_param<LenT>>;

	constexpr T sub_structure() const noexcept { return base::template get<0>(); }
	constexpr StartT start() const noexcept { return base::template get<1>(); }
	constexpr LenT len() const noexcept { return base::template get<2>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> { using type = function_sig<Dim, arg_length_from_t<LenT>, RetSig>; };
	template<class... RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot slice a tuple dimension dynamically");
		static_assert(LenT::value || true, "Cannot slice a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t len = LenT::value;

		template<class Indices = std::make_index_sequence<len>>
		struct pack_helper;
		template<std::size_t... Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<class State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>)
			return state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() + start());
		else
			return state;
	}

	template<class State>
	constexpr auto size(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		if constexpr(QDim == Dim) {
			static_assert(!State::template contains<index_in<Dim>>, "Index already set");
			return len();
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, class State>
	constexpr auto strict_state_at(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set slice length");
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<char Dim, class StartT, class LenT>
struct slice_proto : contain<StartT, LenT> {
	using base = contain<StartT, LenT>;
	using base::base;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return slice_t<Dim, Struct, StartT, LenT>(s, base::template get<0>(), base::template get<1>()); }
};

template<char Dim, class StartT, class LenT>
constexpr auto slice(StartT start, LenT len) noexcept { return slice_proto<Dim, good_index_t<StartT>, good_index_t<LenT>>(start, len); }

// TODO add tests
template<char Dim, class T, class StartT, class EndT>
struct span_t : contain<T, StartT, EndT> {
	using base = contain<T, StartT, EndT>;
	using base::base;

	static constexpr char name[] = "span_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>,
		type_param<EndT>>;

	constexpr T sub_structure() const noexcept { return base::template get<0>(); }
	constexpr StartT start() const noexcept { return base::template get<1>(); }
	constexpr EndT end() const noexcept { return base::template get<2>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> { using type = function_sig<Dim, arg_length_from_t<EndT>, RetSig>; };
	template<class... RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot span a tuple dimension dynamically");
		static_assert(EndT::value || true, "Cannot span a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t end = EndT::value;

		template<class Indices = std::make_index_sequence<end - start>>
		struct pack_helper;
		template<std::size_t... Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<class State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>)
			return state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() + start());
		else
			return state;
	}

	template<class State>
	constexpr auto size(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		if constexpr(QDim == Dim) {
			static_assert(!State::template contains<index_in<Dim>>, "Index already set");
			return end() - start();
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, class State>
	constexpr auto strict_state_at(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set span length");
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<char Dim, class StartT, class EndT>
struct span_proto : contain<StartT, EndT> {
	using base = contain<StartT, EndT>;
	using base::base;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return span_t<Dim, Struct, StartT, EndT>(s, base::template get<0>(), base::template get<1>()); }
};

template<char Dim, class StartT, class EndT>
constexpr auto span(StartT start, EndT end) noexcept { return span_proto<Dim, good_index_t<StartT>, good_index_t<EndT>>(start, end); }

template<char Dim, class T, class StartT, class StrideT>
struct step_t : contain<T, StartT, StrideT> {
	using base = contain<T, StartT, StrideT>;
	using base::base;

	static constexpr char name[] = "step_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>,
		type_param<StartT>,
		type_param<StrideT>>;

	constexpr T sub_structure() const noexcept { return base::template get<0>(); }
	constexpr StartT start() const noexcept { return base::template get<1>(); }
	constexpr StrideT stride() const noexcept { return base::template get<2>(); }

private:
	template<class Original>
	struct dim_replacement;
	template<class ArgLength, class RetSig>
	struct dim_replacement<function_sig<Dim, ArgLength, RetSig>> { using type = function_sig<Dim, arg_length_from_t<StrideT>, RetSig>; };
	template<class... RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static_assert(StartT::value || true, "Cannot slice a tuple dimension dynamically");
		static_assert(StrideT::value || true, "Cannot slice a tuple dimension dynamically");
		static constexpr std::size_t start = StartT::value;
		static constexpr std::size_t stride = StrideT::value;
		static constexpr std::size_t sub_length = sizeof...(RetSigs);

		template<class Indices = std::make_index_sequence<(sub_length + stride - start - 1) / stride>>
		struct pack_helper;
		template<std::size_t... Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<Indices*stride+start>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<class State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>)
			return state.template with<index_in<Dim>>(state.template get<index_in<Dim>>() * stride() + start());
		else
			return state;
	}

	template<class State>
	constexpr auto size(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		if constexpr(QDim == Dim) {
			static_assert(!State::template contains<index_in<Dim>>, "Index already set");
			auto sub_length = sub_structure().template length<Dim>(state);
			return (sub_length + stride() - start() - make_const<1>()) / stride();
		} else {
			return sub_structure().template length<QDim>(sub_state(state));
		}
	}

	template<class Sub, class State>
	constexpr auto strict_state_at(State state) const noexcept {
		static_assert(!State::template contains<length_in<Dim>>, "Cannot set length after step");
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<char Dim, class StartT, class StrideT>
struct step_proto : contain<StartT, StrideT> {
	using base = contain<StartT, StrideT>;
	using base::base;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return step_t<Dim, Struct, StartT, StrideT>(s, base::template get<0>(), base::template get<1>()); }
};

template<char Dim, class StartT, class StrideT>
constexpr auto step(StartT start, StrideT stride) noexcept { return step_proto<Dim, good_index_t<StartT>, good_index_t<StrideT>>(start, stride); }

template<class StartT, class StrideT>
struct auto_step_proto : contain<StartT, StrideT> {
	using base = contain<StartT, StrideT>;
	using base::base;

	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept {
		static_assert(!Struct::signature::dependent, "Add a dimension name as the first parameter to step, or use a structure with a dynamic topmost dimension");
		constexpr char dim = Struct::signature::dim;
		return step_t<dim, Struct, StartT, StrideT>(s, base::template get<0>(), base::template get<1>());
	}
};

template<class StartT, class StrideT>
constexpr auto step(StartT start, StrideT stride) noexcept { return auto_step_proto<good_index_t<StartT>, good_index_t<StrideT>>(start, stride); }

template<char Dim, class T>
struct reverse_t : contain<T> {
	using base = contain<T>;
	using base::base;

	static constexpr char name[] = "reverse_t";
	using params = struct_params<
		dim_param<Dim>,
		structure_param<T>>;

	constexpr T sub_structure() const noexcept { return base::template get<0>(); }

private:
	template<class Original>
	struct dim_replacement {
		using type = Original;
	};
	template<class... RetSigs>
	struct dim_replacement<dep_function_sig<Dim, RetSigs...>> {
		using original = dep_function_sig<Dim, RetSigs...>;
		static constexpr std::size_t len = sizeof...(RetSigs);

		template<class Indices = std::make_index_sequence<len>>
		struct pack_helper;
		template<std::size_t... Indices>
		struct pack_helper<std::index_sequence<Indices...>> { using type = dep_function_sig<Dim, typename original::template ret_sig<len-1-Indices>...>; };

		using type = typename pack_helper<>::type;
	};
public:
	using signature = typename T::signature::template replace<dim_replacement, Dim>;

	template<class State>
	constexpr auto sub_state(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(State::template contains<index_in<Dim>>) {
			auto tmp_state = state.template remove<index_in<Dim>>();
			return tmp_state.template with<index_in<Dim>>(sub_structure().template length<Dim>(tmp_state) - make_const<1>() - state.template get<index_in<Dim>>());
		} else {
			return state;
		}
	}

	template<class State>
	constexpr auto size(State state) const noexcept {
		return sub_structure().size(sub_state(state));
	}

	template<class Sub, class State>
	constexpr auto strict_offset_of(State state) const noexcept {
		return offset_of<Sub>(sub_structure(), sub_state(state));
	}

	template<char QDim, class State>
	constexpr auto length(State state) const noexcept {
		using namespace constexpr_arithmetic;
		if constexpr(QDim == Dim) {
			static_assert(!State::template contains<index_in<Dim>>, "Index already set");
		}
		return sub_structure().template length<QDim>(state);
	}

	template<class Sub, class State>
	constexpr auto strict_state_at(State state) const noexcept {
		return state_at<Sub>(sub_structure(), sub_state(state));
	}
};

template<char Dim>
struct reverse_proto {
	static constexpr bool proto_preserves_layout = true;

	template<class Struct>
	constexpr auto instantiate_and_construct(Struct s) const noexcept { return reverse_t<Dim, Struct>(s); }
};

/**
 * @brief reverses an index (or indices) given by dimension name(s) in a structure
 *
 * @tparam Dim: the dimension names
 */
template<char... Dim>
constexpr auto reverse() noexcept { return (... ^ reverse_proto<Dim>()); }

template<>
constexpr auto reverse<>() noexcept { return neutral_proto(); }

} // namespace noarr

#endif // NOARR_STRUCTURES_SLICE_HPP
