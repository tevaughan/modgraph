/// @file       gsl/vec-base.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Declaration for gsl::vec_base and for functions for vectors.

#pragma once

// Use inline-definition of each accessor-function.
// - Define this before including `gsl_vector.h`.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include "size-code.hpp" // VIEW
#include <gsl/gsl_vector.h> // gsl_vector_view, gsl_vector_const_view

namespace gsl {


// Forward-declarations.
template<typename D> struct vec_iface;
template<int S, typename V> class vector;


/// Base-class name-space for static functions.
/// - An instance of vec_base does not have any fields.
/// - Also, vec_base has no function to access field of descendant.
/// - But there are some static functions that can be collected here.
/// - Static Function belongs here if arguments do not distinguish it from
///   similar function for matrix (in case GSL's matrix-capability ever be
///   added to library).
/// - Static function whose name is generic but whose arguments involve
///   vector-types can reside at root of namespace gsl and need not reside
///   under vec_base.
struct vec_base {
  /// Generic structure used to translate type of element into type of view.
  /// - Template-type parameter `T` must be either `double` or `double const`.
  /// - No other choice is presently supported.
  /// @tparam T  Type of each element in view.
  template<typename T> struct view_aux;

  /// Specialization corresponding to `gsl_vector_view`.
  template<> struct view_aux<double> {
    /// Raw-GSL view associated with element-type `double`.
    using raw_type= gsl_vector_view;

    /// View associated with element-type `double`.
    using type= vector<VIEW, raw_type>;
  };

  /// Specialization corresponding to `gsl_vector_const_view`.
  template<> struct view_aux<double const> {
    /// Raw-GSL view associated with element-type `double const`.
    using raw_type= gsl_vector_const_view;

    /// View associated with element-type `double const`.
    using type= vector<VIEW, raw_type>;
  };

  /// Specification of view in terms of element-type.
  /// @tparam T  Each element's type, either `double` or `double const`.
  template<typename T> using view= typename view_aux<T>::type;

  /// Specification of raw-GSL view in terms of element-type.
  /// @tparam T  Each element's type, either `double` or `double const`.
  template<typename T> using raw_view= typename view_aux<T>::raw_type;

  /// Construct view of C-array.
  /// @tparam T  Type of each element.
  /// @param b  Pointer to first element of vector.
  /// @param n  Number of elements in vector.
  /// @param s  Stride of vector relative to array.
  /// @return  View of array.
  template<typename T> static view<T> ptr_view(T *b, size_t n, size_t s= 1);

  /// Construct view of non-decayed C-array.
  /// @tparam T  Type of each element.
  /// @tparam N  Number of elements in array.
  /// @param b  Pointer to first element of view and of array.
  /// @param n  Number of elements in view; 0 means `N/s`.
  /// @param s  Stride of view relative to array.
  /// @return  View of array.
  template<typename T, int N>
  static view<T> arr_view(T (&b)[N], size_t n= 0, size_t s= 1);
};


/// Construct view of vector.
/// - This returns mutable or immutable view depending on type of element.
/// @tparam T  Type of vector.
/// @param b  Reference to vector.
/// @param n  Number of elements in view; 0 means `b.size()/s`.
/// @param s  Stride of view relative to vector.
/// @return  View of vector.
template<typename T> auto make_view(vec_iface<T> &b, size_t n= 0, size_t s= 1);


/// Construct view of vector.
/// - This returns immutable view, regardless of type of element.
/// @tparam T  Type of vector.
/// @param b  Reference to vector.
/// @param n  Number of elements in view; 0 means `b.size()/s`.
/// @param s  Stride of view relative to vector.
/// @return  View of vector.
template<typename T>
auto make_view(vec_iface<T> const &b, size_t n= 0, size_t s= 1);


/// Linearly combine vector `x` into vector `y` in place.
/// @tparam X  Type of first source-vector.
/// @tparam Y  Type of second source-vector and destination-vector.
/// @param a  Coeffient of `x`.
/// @param x  First source-vector.
/// @param b  Coefficient of `y`.
/// @param y  Second source-vector and destination-vector.
/// @return  TBD.
template<typename X, typename Y>
int axpby(double a, vec_iface<X> const &x, double b, vec_iface<Y> &y);


/// Test equality of two vectors.
/// @tparam U  Type of one descendant of vec_iface.
/// @tparam V  Type of other descendant of vec_iface.
/// @param u  Reference to one vector.
/// @param v  Reference to other vector.
/// @return  True only if vectors be equal.
template<typename U, typename V>
bool equal(vec_iface<U> const &u, vec_iface<V> const &v);


/// Copy data from source, whose length must be same as that of destination.
/// @tparam D  Type of descendant destination.
/// @tparam S  Type of descendant source.
/// @param dst  Destination.
/// @param src  Source.
/// @return  TBD.
template<typename D, typename S>
int memcpy(vec_iface<D> &dst, vec_iface<S> const &src);


/// Swap contents of one and other vector, each with same length.
/// @tparam V  Type of one vector.
/// @tparam W  Type of other vector.
/// @param v  One vector.
/// @param w  Other vector.
/// @return  TBD.
template<typename V, typename W> int swap(vec_iface<V> &v, vec_iface<W> &w);


/// Test equality of two vectors.
/// @tparam U  Type of one descendant of vec_iface.
/// @tparam V  Type of other descendant of vec_iface.
/// @param u  Reference to one vector.
/// @param v  Reference to other vector.
/// @return  True only if vectors be equal.
template<typename U, typename V>
bool operator==(vec_iface<U> const &u, vec_iface<V> const &v);


} // namespace gsl

// EOF
