#ifndef NOARR_STRUCTURES_SHORTCUTS_HPP
#define NOARR_STRUCTURES_SHORTCUTS_HPP

#include "blocks.hpp"
#include "reorder.hpp"

namespace noarr {

template<char Dim, char DimMajor, char DimMinor, class... OptionalMinorLengthT>
constexpr auto strip_mine(OptionalMinorLengthT... optional_minor_length) {
	return into_blocks<Dim, DimMajor, DimMinor>(optional_minor_length...) ^ hoist<DimMajor>();
}

template<char Idx, std::size_t Len = 2, class State, class F>
constexpr auto update_index(State state, F f) {
	static_assert(Len == 2 && ('A' <= Idx && Idx <= 'Z' || Idx == '_' || 'a' <= Idx && Idx <= 'z'), "Invalid index name, must be one letter, without apostrophes");
	static_assert(State::template contains<index_in<Idx>>, "Requested dimension does not exist. To add a new dimension instead of updating existing one, use .template with<index_in<'...'>>(...)");
	auto new_index = f(state.template get<index_in<Idx>>());
	return state.template remove<index_in<Idx>>().template with<index_in<Idx>>(good_index_t<decltype(new_index)>(new_index));
}

} // namespace noarr

#define NOARR_UPDATE_INDEX(STATE, IDX, NEW_VALUE) (noarr::update_index<*#IDX, sizeof(#IDX)>((STATE), [&](auto IDX){return (NEW_VALUE);}))

#endif // NOARR_STRUCTURES_SHORTCUTS_HPP
