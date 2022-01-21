/// @file       gsl/impl/vec-iface-impl.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition of gsl::vec_iface.

#pragma once

#include "../vector.hpp"

namespace gsl {


template<typename D>
vectorv vec_iface<D>::subvector(size_t offset, size_t n, size_t stride) {
  return gsl_vector_subvector_with_stride(p(), offset, stride, n);
}


template<typename D>
vectorcv vec_iface<D>::subvector(
    size_t offset, size_t n, size_t stride) const {
  return gsl_vector_const_subvector_with_stride(p(), offset, stride, n);
}


} // namespace gsl

// EOF
