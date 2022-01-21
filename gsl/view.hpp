/// @file       gsl/view.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition for gsl::vec_base.

#pragma once

// Use inline-definition of each accessor-function.
// - Define this before including `gsl_vector.h`.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include "size-code.hpp" // VIEW
#include <gsl/gsl_vector.h> // gsl_vector_view, gsl_vector_const_view

namespace gsl {


template<int S, typename V> class vector; // Forward-declaration.


/// Generic structure used to translate type of element into type of view.
/// - Template-type parameter `T` must be either `double` or `double const`.
/// - No other choice is presently supported.
/// @tparam T  Type of each element in view.
template<typename T> struct view_aux;


/// Specialization corresponding to `gsl_vector_view`.
template<> struct view_aux<double> {
  /// View associated with element-type `double`.
  using type= vector<VIEW, gsl_vector_view>;
};


/// Specialization corresponding to `gsl_vector_const_view`.
template<> struct view_aux<double const> {
  /// View associated with element-type `double const`.
  using type= vector<VIEW, gsl_vector_const_view>;
};


/// Specification of view in terms of element-type.
/// @tparam T  Each element's type, either `double` or `double const`.
template<typename T> using view= typename view_aux<T>::type;


} // namespace gsl

// EOF
