/// @file       gsl/vector.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
///
/// @brief      Definition for each of
///             gsl::size_code,
///             gsl::vect_base,
///             gsl::vect_itfc, and
///             gsl::vector.

#pragma once

// Use inline-definition of each accessor-function.
// - Define this before including `gsl_vector.h`.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include <algorithm> // swap
#include <gsl/gsl_vector.h> // gsl_vector

/// Namespace for C++-interface to small subset of GSL's functionality,
/// initially just minimization, which requires using gsl_vector.
namespace gsl {


// Forward declaration for descendant of vect_base.
template<typename D> class vect_itfc;

/// When positive, template-argument to class vector is number of elements
/// stored inside instance of vector.
/// - Certain negative size-codes are defined, each with different meaning.
/// - Each non-positive code does indicate that size of vector is determined
///   for new instance at run-time.
enum size_code {
  DYNAMIC= 0, ///< Instance refers to data allocated on construction.
  VIEW= -1, ///< Instance refers to existing block of data.
};


/// Forward declaration for generic template for CRTP-descendant from
/// vect_itfc.
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


/// Base-class name-space for static functions.
struct vect_base {
  /// Construct view of C-array as vector.
  /// @param base  Pointer to first element of vector.
  /// @param n  Number of elements in vector.
  /// @param stride  Stride of vector relative to array.
  /// @return  View of array.
  static vector<VIEW> view_array(double *base, size_t n, size_t stride= 1);

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
  /// @param alpha  Coeffient of `x`.
  /// @param x  First source-vector.
  /// @param beta  Coefficient of `y`.
  /// @param y  Second source-vector and destination-vector.
  /// @return  TBD.
  template<typename X, typename Y>
  static int axpby(
      double alpha, vect_itfc<X> const &x, double beta, vect_itfc<Y> &y);

  /// Test equality of two vectors.
  /// @tparam U  Type of one descendant of vect_itfc.
  /// @tparam V  Type of other descendant of vect_itfc.
  /// @param u  Reference to one vector.
  /// @param v  Reference to other vector.
  /// @return  True only if vectors be equal.
  template<typename U, typename V>
  static bool equal(vect_itfc<U> const &u, vect_itfc<V> const &v);
};


/// Test equality of two vectors.
/// @tparam U  Type of one descendant of vect_itfc.
/// @tparam V  Type of other descendant of vect_itfc.
/// @param u  Reference to one vector.
/// @param v  Reference to other vector.
/// @return  True only if vectors be equal.
template<typename U, typename V>
bool operator==(vect_itfc<U> const &u, vect_itfc<V> const &v) {
  return vect_base::equal(u, v);
}


/// Interface for every kind of vector.
/// @tparam D  Type of descendant of `vect_itfc<D>`.
template<typename D> class vect_itfc: public vect_base {
  template<typename OD> friend class vect_itfc;

  /// Pointer to descendant's gsl_vector.
  auto *p() { return static_cast<D *>(this)->pv(); }

  /// Pointer to descendant's immutable gsl_vector.
  auto const *p() const { return static_cast<D const *>(this)->pv(); }

public:
  /// Size of vector.
  /// @return  Size of vector.
  size_t size() const { return p()->size; }

  /// Stride of vector in memory.
  /// @return  Stride of vector.
  size_t stride() const { return p()->stride; }

  /// Pointer to first element in vector.
  /// @return  Pointer to first element.
  double *data() { return p()->data; }

  /// Pointer to first element in immutable vector.
  /// @return  Pointer to first immutable element.
  double const *data() const { return p()->data; }

  /// Pointer to first element of block containing vector's data.
  /// @return  Pointer to first element of block containing vector's data.
  gsl_block const *block() const { return p()->block; }

  /// Nonzero only if instance own its data.
  /// @return  Nonzero only if instance own its data.
  int owner() const { return p()->owner; }

  /// Read element with bounds-checking.
  /// @parameter i  Offset of element.
  /// @return  Value of element.
  double get(size_t i) const { return gsl_vector_get(p(), i); }

  /// Write element with bounds-checking.
  /// @parameter i  Offset of element.
  /// @parameter x  New value for element.
  void set(size_t i, double x) { gsl_vector_set(p(), i, x); }

  /// Read element without bounds-checking.
  /// @parameter i  Offset of element.
  /// @return  Reference to immutable element.
  double const &operator[](size_t i) const { return data()[i * stride()]; }

  /// Write element without bounds-checking.
  /// @parameter i  Offset of element.
  /// @return  Reference to mutable element.
  double &operator[](size_t i) { return data()[i * stride()]; }

  /// Retrieve pointer with bounds-checking.
  /// - This could be useful if stride unknown.
  /// @parameter i  Offset of element.
  /// @return  Pointer to mutable element.
  double *ptr(size_t i) { return gsl_vector_ptr(p(), i); }

  /// Retrieve pointer with bounds-checking.
  /// - This could be useful if stride unknown.
  /// @parameter i  Offset of element.
  /// @return  Pointer to immutable element.
  double const *ptr(size_t i) const { return gsl_vector_const_ptr(p(), i); }

  /// Set every element.
  /// @param x  Value to which each element should be set.
  void set_all(double x) { gsl_vector_set_all(p(), x); }

  /// Set every element to zero.
  void set_zero() { gsl_vector_set_zero(p()); }

  /// Set element at offset `i` to unity and every other element to zero.
  /// @param i  Offset of element set to unity.
  /// @return  TBD.
  int set_basis(size_t i) { return gsl_vector_set_basis(p(), i); }

  /// Write non-portable binary image of vector to file.
  /// @param f  Pointer to structure for buffered interface.
  /// @return  Zero only on success.
  int fwrite(FILE *f) const { return gsl_vector_fwrite(f, p()); }

  /// Read non-portable binary image of vector from file.
  /// @param f  Pointer to structure for buffered interface.
  /// @return  Zero only on success.
  int fread(FILE *f) { return gsl_vector_fread(f, p()); };

  /// Write ASCII-formatted representation of vector to file.
  /// @param file  Pointer to structure for buffered interface.
  /// @param fmt  printf()-style format-string.
  /// @return  Zero only on success.
  int fprintf(FILE *file, char const *fmt) const {
    return gsl_vector_fprintf(file, p(), fmt);
  }

  /// Read ASCII-formatted representation of vector from file.
  /// @param f  Pointer to structure for buffered interface.
  /// @return  Zero only on success.
  int fscanf(FILE *f) { return gsl_vector_fscanf(f, p()); }

  /// View of subvector of current vector.
  /// @param offset  First element in subvector.
  /// @param n  Number of elements in subvector.
  /// @param stride  Stride of subvector relative to current vector.
  vector<VIEW> subvector(size_t offset, size_t n, size_t stride= 1);

  /// View of immutable subvector of current vector.
  /// @param offset  First element in subvector.
  /// @param n  Number of elements in subvector.
  /// @param stride  Stride of subvector relative to current vector.
  vector<VIEW, gsl_vector_const_view> const_subvector(
      size_t offset, size_t n, size_t stride= 1) const;

  /// Copy data from source-vector whose length must be same as this vector.
  /// @tparam T  Type of source-vector.
  /// @param src  Source vector of length same as this.
  /// @return  TBD.
  template<typename T> int memcpy(vect_itfc<T> const &src) {
    return gsl_vector_memcpy(p(), src.p());
  }

  /// Swap contents of this and other vector, each with same length.
  /// @tparam T  Type of other vector.
  /// @param w  Other vector of length same as this.
  /// @return  TBD.
  template<typename T> int swap(vect_itfc<T> &w) {
    return gsl_vector_swap(p(), w.p());
  }

  /// Swap elements within this vector.
  /// @param i  Offset of one element.
  /// @param j  Offset of other element.
  /// @return  TBD.
  int swap_elements(size_t i, size_t j) {
    return gsl_vector_swap_elements(p(), i, j);
  }

  /// Reverse order of elements.
  /// @return  TBD.
  int reverse() { return gsl_vector_reverse(p()); }

  /// Add contents of `b` into this vector in place.
  /// @tparam T  Type of vector to be added into this.
  /// @param b  Vector whose contents should be added into this.
  /// @return  TBD.
  template<typename T> int add(vect_itfc<T> const &b) {
    return gsl_vector_add(p(), b.p());
  }

  /// Subtract contents of `b` from this vector in place.
  /// @tparam T  Type of vector to be subtracted from this.
  /// @param b  Vector whose contents should be subtracted from this.
  /// @return  TBD.
  template<typename T> int sub(vect_itfc<T> const &b) {
    return gsl_vector_sub(p(), b.p());
  }

  /// Multiply contents of `b` into this vector in place.
  /// @tparam T  Type of vector to be multiplied into this.
  /// @param b  Vector whose contents should be multiplied into this.
  /// @return  TBD.
  template<typename T> int mul(vect_itfc<T> const &b) {
    return gsl_vector_mul(p(), b.p());
  }

  /// Divide contents of `b` into this vector in place.
  /// @tparam T  Type of vector to be divided into this.
  /// @param b  Vector whose contents should be divided into this.
  /// @return  TBD.
  template<typename T> int div(vect_itfc<T> const &b) {
    return gsl_vector_div(p(), b.p());
  }

  /// Add contents of `b` into this vector in place.
  /// @tparam T  Type of vector to be added into this.
  /// @param b  Vector whose contents should be added into this.
  /// @return  Reference to this vector after modification.
  template<typename T> vect_itfc &operator+=(vect_itfc<T> const &b) {
    add(b);
    return *this;
  }

  /// Subtract contents of `b` from this vector in place.
  /// @tparam T  Type of vector to be subtracted from this.
  /// @param b  Vector whose contents should be subtracted from this.
  /// @return  Reference to this vector after modification.
  template<typename T> vect_itfc &operator-=(vect_itfc<T> const &b) {
    sub(b);
    return *this;
  }

  /// Multiply contents of `b` into this vector in place.
  /// @tparam T  Type of vector to be multiplied into this.
  /// @param b  Vector whose contents should be multiplied into this.
  /// @return  Reference to this vector after modification.
  template<typename T> vect_itfc &operator*=(vect_itfc<T> const &b) {
    mul(b);
    return *this;
  }

  /// Divide contents of `b` into this vector in place.
  /// @tparam T  Type of vector to be divided into this.
  /// @param b  Vector whose contents should be divided into this.
  /// @return  Reference to this vector after modification.
  template<typename T> vect_itfc &operator/=(vect_itfc<T> const &b) {
    div(b);
    return *this;
  }

  /// Multiply scalar into this vector in place.
  /// @param x  Scalar to multiply into this.
  /// @return  TBD.
  int scale(double x) { return gsl_vector_scale(p(), x); }

  /// Multiply scalar into this vector in place.
  /// @param x  Scalar to multiply into this.
  /// @return  Reference to this vector after modification.
  vect_itfc &operator*=(double x) {
    scale(x);
    return *this;
  }

  /// Add constant into each element of this vector in place.
  /// @param x  Constant to add into this vector.
  /// @return  TBD.
  int add_constant(double x) { return gsl_vector_add_constant(p(), x); }

  /// Add constant into each element of this vector in place.
  /// @param x  Constant to add into this vector.
  /// @return  Reference to this vector after modification.
  vect_itfc &operator+=(double x) {
    add_constant(x);
    return *this;
  }

  /// Sum of elements.
  /// @return  Sum of elements.
  double sum() const { return gsl_vector_sum(p()); }

  /// Greatest value of any element.
  /// @return  Greatest value of any element.
  double max() const { return gsl_vector_max(p()); }

  /// Least value of any element.
  /// @return  Least value of any element.
  double min() const { return gsl_vector_min(p()); }

  /// Greatest value and least value of any element.
  /// @param min_out  On return, least value.
  /// @param max_out  On return, greatest value.
  void minmax(double &min_out, double &max_out) const {
    gsl_vector_minmax(p(), &min_out, &max_out);
  }

  /// Offset of greatest value.
  /// @return  Offset of greatest value.
  size_t max_index() const { return gsl_vector_max_index(p()); }

  /// Offset of least value.
  /// @return  Offset of least value.
  size_t min_index() const { return gsl_vector_min_index(p()); }

  /// Offset of least value and offset of greatest value.
  /// @param imin  On return, offset of least value.
  /// @param imax  On return, offset of greatest value.
  void minmax_index(size_t &imin, size_t &imax) {
    gsl_vector_minmax_index(p(), &imin, &imax);
  }

  /// True only if every element have zero value.
  /// @return  True only if every element be zero.
  bool isnull() const { return gsl_vector_isnull(p()); }

  /// True only if every element be positive.
  /// @return  True only if every element be positive.
  bool ispos() const { return gsl_vector_ispos(p()); }

  /// True only if every element be negative.
  /// @return  True only if every element be negative.
  bool isneg() const { return gsl_vector_isneg(p()); }

  /// True only if every element be non-negative.
  /// @return  True only if every element be non-negative.
  bool isnonneg() const { return gsl_vector_isnonneg(p()); }
};


// Generic template.  See documentation at forward declaration.
template<int S, typename V> class vector: public vect_itfc<vector<S, V>> {
  static_assert(S > 0);

  friend struct vect_base;
  template<typename T> friend class vect_itfc;
  template<int OS, typename OV> friend class vector;

  using vect_base::const_view_array;
  using vect_itfc<vector<S, V>>::memcpy;

  double d_[S]; ///< Storage for data.
  V view_; ///< GSL's view of data within instance of vector.

  /// Function needed by vect_itfc.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return &view_.vector; }

  /// Function needed by vect_itfc.
  /// @return  Pointer to GSL's interface to immutable vector.
  gsl_vector const *pv() const { return &view_.vector; }

public:
  /// Initialize GSL's view of static storage, but do not initialize data.
  vector(): view_(gsl_vector_view_array(d_, S)) {}

  using vect_itfc<vector<S, V>>::size;

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
template<> class vector<DYNAMIC>: public vect_itfc<vector<DYNAMIC>> {
  friend struct vect_base;
  template<typename T> friend class vect_itfc;
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

  /// Function needed by vect_itfc.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return pv_; }

  /// Function needed by vect_itfc.
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
template<typename V> class vector<VIEW, V>: public vect_itfc<vector<VIEW, V>> {
  friend struct vect_base;
  template<typename T> friend class vect_itfc;
  template<int S, typename OV> friend class vector;

  V view_; ///< GSL's view of data outside instance.

  /// Function needed by vect_itfc.
  /// @return  Pointer to GSL's interface to vector.
  gsl_vector *pv() { return &view_.vector; }

  /// Function needed by vect_itfc.
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
  /// @param str  Stride of vector relative to array.
  vector(double *d, size_t n, size_t str= 1):
      view_(gsl_vector_view_array_with_stride(d, str, n)) {}

  /// Initialize view of non-decayed C-array with stride.
  /// @tparam N  Number of elements in array.
  /// @param d  Pointer to first element of vector and of array.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by str.
  /// @param str  Stride of vector relative to array.
  template<unsigned N>
  vector(double (&d)[N], size_t n= 0, size_t str= 1):
      view_(view(d, str, n, N)) {}

  /// Initialize view of other vector with stride.
  /// @tparam S  Size-parameter identifying type of other vector.
  /// @param v  Reference to other vector.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by str.
  /// @param str  Stride relative to other vector.
  template<int S>
  vector(vector<S, V> &v, size_t n= 0, size_t str= 1):
      view_(view(v.pv()->data, str, n, v.size())) {}

  /// Initialize view of C-array with specified size and stride.
  /// @param d  Pointer to first immutable element of vector and of array.
  /// @param n  Number of elements in vector.
  /// @param stride  Stride of vector relative to array.
  vector(double const *d, size_t n, size_t stride= 1):
      view_(gsl_vector_const_view_array_with_stride(d, stride, n)) {}

  /// Initialize view of non-decayed C-array with stride.
  /// @tparam N  Number of elements in immutable array.
  /// @param d  Pointer to first element of vector and of array.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by stride.
  /// @param stride  Stride of vector relative to array.
  template<unsigned N>
  vector(double const (&d)[N], size_t n= 0, size_t stride= 1):
      view_(const_view(d, stride, n, N)) {}

  /// Initialize view of other vector.
  /// @tparam S  Size-parameter identifying type of other vector.
  /// @param v  Reference to other vector.
  /// @param n  Number of elements in vector; default, 0, means to use size of
  ///           other vector divided by stride.
  /// @param stride  Stride relative to other vector.
  template<int S>
  vector(vector<S> const &v, size_t n= 0, size_t stride= 1):
      view_(const_view(v.pv()->data, stride, n, v.size())) {}
};


// Implementation for each of several member-functions.

vector<VIEW> vect_base::view_array(double *base, size_t n, size_t stride) {
  return gsl_vector_view_array_with_stride(base, stride, n);
}

vector<VIEW, gsl_vector_const_view> vect_base::const_view_array(
    double const *b, size_t n, size_t s) {
  return gsl_vector_const_view_array_with_stride(b, s, n);
}

template<typename T, typename U>
int vect_base::axpby(
    double alpha, vect_itfc<T> const &x, double beta, vect_itfc<U> &y) {
  return gsl_vector_axpby(alpha, x.p(), beta, y.p());
}

template<typename T, typename U>
bool vect_base::equal(vect_itfc<T> const &u, vect_itfc<U> const &v) {
  return gsl_vector_equal(u.p(), v.p());
}

template<typename D>
vector<VIEW> vect_itfc<D>::subvector(size_t offset, size_t n, size_t stride) {
  return gsl_vector_subvector_with_stride(p(), offset, stride, n);
}

template<typename D>
vector<VIEW, gsl_vector_const_view> vect_itfc<D>::const_subvector(
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
