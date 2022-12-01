#ifndef NOARR_STRUCTURES_OMP_HPP
#define NOARR_STRUCTURES_OMP_HPP

#include "../interop/traverser_iter.hpp"

namespace noarr {

template<class Traverser, class F>
inline void omp_for_each(const Traverser &t, const F &f) noexcept {
	#pragma omp parallel
	#pragma omp for
	for(auto t_inner : t)
		t_inner.for_each(f);
}

} // namespace noarr

#endif // NOARR_STRUCTURES_OMP_HPP
