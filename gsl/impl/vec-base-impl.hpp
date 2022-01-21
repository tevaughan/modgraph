/// @file       gsl/impl/vec-base-impl.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition for gsl::vec_base and for functions for vectors.

#pragma once

#include "../vec-base.hpp"
#include <type_traits> // is_const_v

namespace gsl {


using std::is_const_v;


template<typename T>
vec_base::view<T> vec_base::ptr_view(T *base, size_t n, size_t stride) {
  if constexpr(is_const_v<T>) {
    return gsl_vector_const_view_array_with_stride(base, stride, n);
  } else {
    return gsl_vector_view_array_with_stride(base, stride, n);
  }
}


template<typename T, int N>
vec_base::view<T> vec_base::arr_view(T (&b)[N], size_t n, size_t s) {
  size_t const num= (n ? n : N / s);
  if constexpr(is_const_v<T>) {
    return gsl_vector_const_view_array_with_stride(b, s, num);
  } else {
    return gsl_vector_view_array_with_stride(b, s, num);
  }
}


template<typename T> auto make_view(vec_iface<T> &b, size_t n, size_t s) {
  size_t const num= (n ? n : b.size() / s);
  if constexpr(is_const_v<decltype(*b.data())>) {
    auto v= gsl_vector_const_view_array_with_stride(b.data(), s, num);
    return vectorcv(v);
  } else {
    auto v= gsl_vector_view_array_with_stride(b.data(), s, num);
    return vectorv(v);
  }
}


template<typename T>
auto make_view(vec_iface<T> const &b, size_t n, size_t s) {
  size_t const num= (n ? n : b.size() / s);
  auto v= gsl_vector_const_view_array_with_stride(b.data(), s, num);
  return vectorcv(v);
}


template<typename T, typename U>
int axpby(double alpha, vec_iface<T> const &x, double beta, vec_iface<U> &y) {
  return gsl_vector_axpby(alpha, x.p(), beta, y.p());
}


template<typename T, typename U>
bool equal(vec_iface<T> const &u, vec_iface<U> const &v) {
  return gsl_vector_equal(u.p(), v.p());
}


template<typename D, typename S>
int memcpy(vec_iface<D> &dst, vec_iface<S> const &src) {
  return gsl_vector_memcpy(dst.p(), src.p());
}


template<typename V, typename W> int swap(vec_iface<V> &v, vec_iface<W> &w) {
  return gsl_vector_swap(v.p(), w.p());
}


template<typename U, typename V>
bool operator==(vec_iface<U> const &u, vec_iface<V> const &v) {
  return equal(u, v);
}


} // namespace gsl

// EOF
