/// @file       gsl/vector.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition for gsl::vector.

#pragma once

#include <algorithm> // swap
#include "vec-iface.hpp" // vec_iface

/// Namespace for C++-interface to small subset of GSL's functionality,
/// initially just minimization, which requires using gsl_vector.
namespace gsl {


// Forward declaration for descendant of vec_base.
template<typename D> class vec_iface;


/// Forward declaration for generic template for CRTP-descendant from
/// vec_iface.
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
template<int S, typename V= gsl_vector_view> class vector;


// Generic template.  See documentation at forward declaration.
template<int S, typename V> class vector: public vec_iface<vector<S, V>> {
  static_assert(S > 0);

  friend struct vec_base;
  template<typename T> friend class vec_iface;
  template<int OS, typename OV> friend class vector;

  using vec_base::const_view_array;
  using vec_iface<vector<S, V>>::memcpy;

  double d_[S]; ///< Storage for data.
  V view_; ///< GSL's view of data within instance of vector.

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return &view_.vector; }

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return &view_.vector; }

public:
  /// Initialize GSL's view of static storage, but do not initialize data.
  vector(): view_(gsl_vector_view_array(d_, S)) {}

  using vec_iface<vector<S, V>>::size;

  /// Initialize GSL's view, and initialize vector by deep copy.
  /// @param d  Data to copy for initialization.
  vector(double const (&d)[S]): vector() { memcpy(const_view_array(d, S)); }

  /// Initialize GSL's view, and initialize vector by deep copy.
  /// @param v  Data to copy for initialization.
  vector(vector const &v): vector() { memcpy(v); }

  /// Initialize GSL's view, and initialize vector by deep copy.
  /// @param v  Data to copy for initialization.
  vector &operator=(vector const &v) {
    view_= gsl_vector_view_array(d_, S);
    memcpy(v);
  }
};


/// Specialization for vector with dynamic allocation of memory on
/// construction.
template<> class vector<DYNAMIC>: public vec_iface<vector<DYNAMIC>> {
  friend struct vec_base;
  template<typename T> friend class vec_iface;
  template<int S, typename V> friend class vector;

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

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return pv_; }

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return pv_; }

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
    memcpy(src);
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
    memcpy(src);
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
  friend struct vec_base;
  template<typename T> friend class vec_iface;
  template<int S, typename OV> friend class vector;

  V view_; ///< GSL's view of data outside instance.

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return &view_.vector; }

  /// Function needed by vec_iface.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return &view_.vector; }

  /// Constructor called by subvector() and view_array().
  /// @param v  View to copy.
  vector(gsl_vector_view const &v): view_(v) {}

  /// Constructor called by const_subvector() and const_view_array().
  vector(gsl_vector_const_view const &v): view_(v) {}

  /// Construct view from pointer, stride, commanded size of view, and maximum
  /// number of bytes across underlying buffer.
  /// - When num == 0, cause number of elements in view to be max/str;
  ///   otherwise, use num.
  /// @param dat  Pointer to first element.
  /// @param str  Stride across underlying buffer.
  /// @param num  Commanded number of elements in view.
  /// @param max  Number of elements in underlying buffer.
  static V view(double *dat, size_t str, size_t num, size_t max) {
    return gsl_vector_view_array_with_stride(dat, str, num ? num : max / str);
  }

  /// Construct view from pointer to immutable data, stride, commanded size of
  /// view, and maximum number of bytes across underlying buffer.
  /// - When num == 0, cause number of elements in view to be max/str;
  ///   otherwise, use num.
  /// @param d  Pointer to first element.
  /// @param s  Stride across underlying buffer.
  /// @param n  Commanded number of elements in view.
  /// @param m  Number of elements in underlying buffer.
  static V const_view(double const *d, size_t s, size_t n, size_t m) {
    return gsl_vector_const_view_array_with_stride(d, s, n ? n : m / s);
  }

public:
  /// Initialize view of C-array with specified size and stride.
  /// @param d  Pointer to first element of vector and of array.
  /// @param n  Number of elements in vector.
  /// @param s  Stride of vector relative to array.
  vector(double *d, size_t n, size_t s= 1):
      view_(gsl_vector_view_array_with_stride(d, s, n)) {}

  /// Initialize view of non-decayed C-array with stride.
  /// @tparam N  Number of elements in array.
  /// @param d  Pointer to first element of vector and of array.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by str.
  /// @param s  Stride of vector relative to array.
  template<unsigned N>
  vector(double (&d)[N], size_t n= 0, size_t s= 1): view_(view(d, s, n, N)) {}

  /// Initialize view of other vector with stride.
  /// @tparam S  Size-parameter identifying type of other vector.
  /// @param v  Reference to other vector.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by str.
  /// @param s  Stride relative to other vector.
  template<int S>
  vector(vector<S, V> &v, size_t n= 0, size_t s= 1):
      view_(view(v.pv()->data, s, n, v.size())) {}

  /// Initialize view of C-array with specified size and stride.
  /// @param d  Pointer to first immutable element of vector and of array.
  /// @param n  Number of elements in vector.
  /// @param s  Stride of vector relative to array.
  vector(double const *d, size_t n, size_t s= 1):
      view_(gsl_vector_const_view_array_with_stride(d, s, n)) {}

  /// Initialize view of non-decayed C-array with stride.
  /// @tparam N  Number of elements in immutable array.
  /// @param d  Pointer to first element of vector and of array.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by stride.
  /// @param s  Stride of vector relative to array.
  template<unsigned N>
  vector(double const (&d)[N], size_t n= 0, size_t s= 1):
      view_(const_view(d, s, n, N)) {}

  /// Initialize view of other vector.
  /// @tparam S  Size-parameter identifying type of other vector.
  /// @param v  Reference to other vector.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by stride.
  /// @param s  Stride relative to other vector.
  template<int S>
  vector(vector<S> const &v, size_t n= 0, size_t s= 1):
      view_(const_view(v.pv()->data, s, n, v.size())) {}
};


// Implementation for each of several member-functions.

vector<VIEW> vec_base::view_array(double *base, size_t n, size_t stride) {
  return gsl_vector_view_array_with_stride(base, stride, n);
}

vector<VIEW, gsl_vector_const_view> vec_base::const_view_array(
    double const *b, size_t n, size_t s) {
  return gsl_vector_const_view_array_with_stride(b, s, n);
}

template<typename T, typename U>
int vec_base::axpby(
    double alpha, vec_iface<T> const &x, double beta, vec_iface<U> &y) {
  return gsl_vector_axpby(alpha, x.p(), beta, y.p());
}

template<typename T, typename U>
bool vec_base::equal(vec_iface<T> const &u, vec_iface<U> const &v) {
  return gsl_vector_equal(u.p(), v.p());
}

template<typename D>
vector<VIEW> vec_iface<D>::subvector(size_t offset, size_t n, size_t stride) {
  return gsl_vector_subvector_with_stride(p(), offset, stride, n);
}

template<typename D>
vector<VIEW, gsl_vector_const_view> vec_iface<D>::const_subvector(
    size_t offset, size_t n, size_t stride) const {
  return gsl_vector_const_subvector_with_stride(p(), offset, stride, n);
}


/// Short-hand for vector with storage dynamically allocated on construction.
using vectord= vector<DYNAMIC>;

/// Short-hand for vector with mutable storage outside instance.
using vectorv= vector<VIEW>;

/// Short-hand for vector with immutable storage outside instance.
using vectorcv= vector<VIEW, gsl_vector_const_view>;


} // namespace gsl

// EOF
