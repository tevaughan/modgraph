/// @file       gsl/vec-base.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition for gsl::vec_base.

#pragma once

// Use inline-definition of each accessor-function.
// - Define this before including `gsl_vector.h`.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include "size-code.hpp"
#include <gsl/gsl_vector.h> // gsl_vector_view, gsl_vector_const_view

namespace gsl {


// Forward-declarations.
template<typename D> class vec_iface;
template<int S, typename V> class vector;


/// Base-class name-space for static functions.
struct vec_base {
  /// Construct view of C-array as vector.
  /// @param base  Pointer to first element of vector.
  /// @param n  Number of elements in vector.
  /// @param stride  Stride of vector relative to array.
  /// @return  View of array.
  static vector<VIEW, gsl_vector_view> view_array(
      double *base, size_t n, size_t stride= 1);

  /// Construct view of immutable C-array as vector.
  /// @param base  Pointer to first element of immutable vector.
  /// @param n  Number of elements in vector.
  /// @param stride  Stride of vector relative to array.
  /// @return  View of immutable array.
  static vector<VIEW, gsl_vector_const_view> const_view_array(
      double const *b, size_t n, size_t s= 1);

  /// Linearly combine vector `x` into vector `y` in place.
  /// @tparam X  Type of first source-vector.
  /// @tparam Y  Type of second source-vector and destination-vector.
  /// @param a  Coeffient of `x`.
  /// @param x  First source-vector.
  /// @param b  Coefficient of `y`.
  /// @param y  Second source-vector and destination-vector.
  /// @return  TBD.
  template<typename X, typename Y>
  static int axpby(double a, vec_iface<X> const &x, double b, vec_iface<Y> &y);

  /// Test equality of two vectors.
  /// @tparam U  Type of one descendant of vec_iface.
  /// @tparam V  Type of other descendant of vec_iface.
  /// @param u  Reference to one vector.
  /// @param v  Reference to other vector.
  /// @return  True only if vectors be equal.
  template<typename U, typename V>
  static bool equal(vec_iface<U> const &u, vec_iface<V> const &v);
};


/// Test equality of two vectors.
/// @tparam U  Type of one descendant of vec_iface.
/// @tparam V  Type of other descendant of vec_iface.
/// @param u  Reference to one vector.
/// @param v  Reference to other vector.
/// @return  True only if vectors be equal.
template<typename U, typename V>
bool operator==(vec_iface<U> const &u, vec_iface<V> const &v) {
  return vec_base::equal(u, v);
}


} // namespace gsl

// EOF
