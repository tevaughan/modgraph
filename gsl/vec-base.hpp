/// @file       gsl/vec-base.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Declaration for gsl::vec_base and for functions for vectors.

#pragma once

#include "view-aux.hpp" // view_aux

namespace gsl {


template<typename D> struct vec_iface; // Forward-declaration.


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
  /// Specification of view in terms of element-type.
  /// @tparam T  Each element's type, either `double` or `double const`.
  template<typename T> using view= typename view_aux<T>::vector_type;

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
