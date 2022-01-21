/// @file       gsl/vector.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition for gsl::vector.

#pragma once

#include "vec-iface.hpp" // vec_iface
#include <algorithm> // swap
#include <type_traits> // is_const_v, is_same_v, enable_if_t

using std::enable_if_t;
using std::is_const_v;
using std::is_same_v;

/// Namespace for C++-interface to small subset of GSL's functionality,
/// initially just minimization, which requires using gsl_vector.
namespace gsl {


/// Generic template. for CRTP-descendant from vec_iface.
/// - First template-argument must be positive for generic template; argument
///   indicates number of elements stored in instance.
/// - However, each specialization has non-positive template-argument.
/// @tparam S  When positive, number of elements stored in instance (no
///            `malloc()`); otherwise, size_code indicating that elements are
///            stored outside instance.
/// @tparam V  Type of view used for positive `S` and for `S == VIEW`. By
///            default, `gsl_vector_view`; for `S == VIEW`, `V` may be
///            specified as `gsl_vector_const_view`.  The parameter `V` is
///            ignored when `S == DYNAMIC`.
template<int S, typename V= gsl_vector_view>
class vector: public vec_iface<vector<S, V>> {
  static_assert(S > 0);
  using vec_base::arr_view;

  double d_[S]; ///< Storage for data.
  V view_; ///< GSL's view of data within instance of vector.

public:
  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return &view_.vector; }

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return &view_.vector; }

  /// Initialize GSL's view of static storage, but do not initialize data.
  vector(): view_(gsl_vector_view_array(d_, S)) {}

  /// Initialize GSL's view, and initialize vector by deep copy.
  /// @param d  Data to copy for initialization.
  vector(double const (&d)[S]): vector() { memcpy(*this, arr_view(d)); }

  /// Initialize GSL's view, and initialize vector by deep copy.
  /// @param v  Data to copy for initialization.
  vector(vector const &v): vector() { memcpy(*this, v); }

  /// Initialize GSL's view, and initialize vector by deep copy.
  /// @param v  Data to copy for initialization.
  vector &operator=(vector const &v) {
    view_= gsl_vector_view_array(d_, S);
    memcpy(*this, v);
  }
};


/// Specialization for vector with dynamic allocation of memory on
/// construction.
template<> class vector<DYNAMIC>: public vec_iface<vector<DYNAMIC>> {
public:
  /// Identifier for each of two possible allocation-methods.
  enum class alloc_type {
    ALLOC, ///< Just allocate without initialization.
    CALLOC ///< Initialize each element to zero after allocation.
  };

private:
  /// Identifier for one of two possible allocation-methods.
  /// - By default, allocate without initialization.
  alloc_type alloc_type_= alloc_type::ALLOC;

  gsl_vector *pv_= nullptr; ///< Pointer to allocated descriptor for vector.

  /// Deallocate vector and its descriptor.
  void free() {
    if(pv_) gsl_vector_free(pv_);
    pv_= nullptr;
  }

  /// Allocate vector and its descriptor.
  /// @param n  Number of elements in vector.
  /// @return  Pointer to vector's descriptor.
  gsl_vector *allocate(size_t n) {
    free();
    if(alloc_type_ == alloc_type::ALLOC) return gsl_vector_alloc(n);
    return gsl_vector_calloc(n);
  }

public:
  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return pv_; }

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return pv_; }

  /// Allocate vector and its descriptor.
  /// @param n  Number of elements in vector.
  /// @param a  Method to use for allocation.
  vector(size_t n, alloc_type a= alloc_type::ALLOC): alloc_type_(a) {
    pv_= allocate(n);
  }

  /// Allocate vector and its descriptor, and perform deep copy on
  /// construction.
  /// @tparam S  Size-parameter indicating type of source.
  /// @tparam V  Type of view.
  /// @param src  Vector to copy.
  template<int S, typename V>
  vector(vector<S, V> const &src): alloc_type_(alloc_type::ALLOC) {
    pv_= allocate(src.pv()->size);
    memcpy(*this, src);
  }

  /// Move on construction.
  /// - Note that this is not a templated constructor because moving works only
  ///   from other vector<DYNAMIC>.
  /// @param src  Vector to move.
  vector(vector &&src): alloc_type_(src.alloc_type_), pv_(src.pv_) {
    src.alloc_type_= alloc_type::ALLOC;
    src.pv_= nullptr;
  }

  /// Deallocate existing vector and its descriptor; allocate new vector and
  /// its descriptor; and perform deep copy on assignment.
  /// @tparam S  Size-parameter indicating type of source.
  /// @tparam V  Type of view.
  /// @param src  Vector to copy.
  /// @return  Reference to instance after modification.
  template<int S, typename V> vector &operator=(vector<S, V> const &src) {
    alloc_type_= alloc_type::ALLOC;
    pv_= allocate(src.pv()->size);
    memcpy(*this, src);
    return *this;
  }

  /// Move on assignment.
  /// - This instance's original descriptor and data should be deallocated
  ///   after move, when src's destructor is called.
  /// - Note that this is not a templated function because moving works only
  ///   from other vector<DYNAMIC>.
  /// @param src  Vector to exchange state with.
  /// @return  Reference to instance after modification.
  vector &operator=(vector &&src) {
    std::swap(alloc_type_, src.alloc_type_);
    std::swap(pv_, src.pv_);
    return *this;
  }

  /// Deallocate vector and its descriptor.
  virtual ~vector() { free(); }
};


/// Specialization for vector that refers to mutable, external data.
template<typename V> class vector<VIEW, V>: public vec_iface<vector<VIEW, V>> {
  V view_; ///< GSL's view of data outside instance.

public:
  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return &view_.vector; }

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return &view_.vector; }

  /// Constructor called by subvector() and view_array().
  /// @param v  View to copy.
  vector(gsl_vector_view v): view_(v) {}

  /// Constructor called by const_subvector() and const_view_array().
  vector(gsl_vector_const_view v): view_(v) {}

  /// Initialize view of C-array.
  /// @tparam T  Type of each element.
  /// @param d  Pointer to first element of view and of array.
  /// @param n  Number of elements in vector.
  /// @param s  Stride of vector relative to array.
  template<typename T>
  vector(T *d, size_t n, size_t s= 1): view_(ptr_view(d, n, s).view_) {}

  /// Initialize view of non-decayed C-array.
  /// @tparam T  Type of each element.
  /// @tparam N  Number of elements in array.
  /// @param d  Pointer to first element of view and of array.
  /// @param n  Number of elements in view; 0 means `N/s`.
  /// @param s  Stride of vector relative to array.
  template<typename T, int N>
  vector(T (&d)[N], size_t n= 0, size_t s= 1):
      view_(arr_view(d, n, s).view_) {}

  /// Initialize view of vector.
  /// @tparam S  Size (or size-code) of vector.
  /// @tparam T  Raw type of view.
  /// @param v  Reference to vector.
  /// @param n  Number of elements in view; 0 means `v.size()/s`.
  /// @param s  Stride relative to vector.
  template<int S, typename T, typename= enable_if_t<is_same_v<V, T>>>
  vector(vec_iface<vector<S, T>> &v, size_t n= 0, size_t s= 1):
      view_(make_view(v, n, s).view_) {}

  /// Initialize view of vector.
  /// @tparam T  Type of vector.
  /// @param v  Reference to vector.
  /// @param n  Number of elements in view; 0 means `v.size()/s`.
  /// @param s  Stride relative to vector.
  template<typename T>
  vector(vec_iface<T> const &v, size_t n= 0, size_t s= 1):
      view_(make_view(v, n, s).view_) {}
};


/// Short-hand for vector with ownership of dynamically allocated storage.
using vectord= vector<DYNAMIC>;

/// Short-hand for vector without ownership of mutable storage.
using vectorv= vec_base::view<double>;

/// Short-hand for vector without ownership of immutable storage.
using vectorcv= vec_base::view<double const>;


// Implementation for each of several member-functions.


// BEGIN functions defined in 'vec-base.hpp'.
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
// END functions defined in 'vec-base.hpp'.


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
