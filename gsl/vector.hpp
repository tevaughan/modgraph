/// @file       gsl/vector.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition of gsl::vector_interface, gsl::vector.

#pragma once

// Use inline-definition of each accessor-function.
// - Define this before including `gsl_vector.h`.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include <gsl/gsl_vector.h>

namespace gsl {


// Forward declaration.
template<int S> class vector;


/// When positive, template-argument to class vector is number of elements
/// stored inside instance of vector.
/// - Certain negative size-codes are defined, each with different meaning.
/// - Each negative code does indicate that size of vector is determined for
///   new instance at run-time.
enum size_code {
  DYNAMIC= -1, ///< Instance refers to data allocated on construction.
  VIEW= -2, ///< Instance refers to existing block of data.
  CONST_VIEW= -3, ///< Instance refers to existing block of immutable data.
};


/// Interface for every kind of vector.
/// @tparam D  Type of descendant of `vector_interface<D>`.
template<typename D> class vector_interface {
  gsl_vector *p() { return static_cast<D *>(this)->pv(); }
  gsl_vector const *p() const { return static_cast<D const *>(this)->pv(); }

public:
  size_t size() const { return p()->size; }
  size_t stride() const { return p()->stride; }
  double *data() { return p()->data; }
  double const *data() const { return p()->data; }
  gsl_block const *block() const { return p()->block; }
  int owner() const { return p()->owner; }

  /// Read element with bounds-checking.
  double get(size_t i) const { return gsl_vector_get(p(), i); }

  /// Write element with bounds-checking.
  void set(size_t i, double x) { gsl_vector_set(p(), i, x); }

  /// Read element without bounds-checking.
  double const &operator[](size_t i) const { return data()[i * stride()]; }

  /// Write element without bounds-checking.
  double &operator[](size_t i) { return data()[i * stride()]; }

  /// Retrieve pointer with bounds-checking.
  double *ptr(size_t i) { return gsl_vector_ptr(p(), i); }

  /// Retrieve pointer with bounds-checking.
  double const *ptr(size_t i) const { return gsl_vector_const_ptr(p(), i); }

  void set_all(double x) { gsl_vector_set_all(p(), x); }
  void set_zero() { gsl_vector_set_zero(p()); }
  int set_basis(size_t i) { return gsl_vector_set_basis(p(), i); }
  int fwrite(FILE *f) const { return gsl_vector_fwrite(f, p()); }
  int fread(FILE *f) { return gsl_vector_fread(f, p()); };

  int fprintf(FILE *file, char const *fmt) const {
    return gsl_vector_fprintf(file, p(), fmt);
  }

  int fscanf(FILE *f) { return gsl_vector_fscanf(f, p()); }

  vector<VIEW> subvector(size_t offset, size_t n, size_t stride= 1);

  vector<CONST_VIEW> const_subvector(
      size_t offset, size_t n, size_t stride= 1) const;

  static vector<VIEW> view_array(double *base, size_t n, size_t stride= 1);

  static vector<CONST_VIEW> const_view_array(
      double const *b, size_t n, size_t s= 1);

  int memcpy(vector_interface const &src) {
    return gsl_vector_memcpy(p(), src.p());
  }

  int swap(vector_interface &w) { return gsl_vector_swap(p(), w.p()); }

  int swap_elements(size_t i, size_t j) {
    return gsl_vector_swap_elements(p(), i, j);
  }

  int reverse() { return gsl_vector_reverse(p()); }

  int add(vector_interface const &b) { return gsl_vector_add(p(), b.p()); }
  int sub(vector_interface const &b) { return gsl_vector_sub(p(), b.p()); }
  int mul(vector_interface const &b) { return gsl_vector_mul(p(), b.p()); }
  int div(vector_interface const &b) { return gsl_vector_div(p(), b.p()); }

  vector_interface &operator+=(vector_interface const &b) {
    add(b);
    return *this;
  }

  vector_interface &operator-=(vector_interface const &b) {
    sub(b);
    return *this;
  }

  vector_interface &operator*=(vector_interface const &b) {
    mul(b);
    return *this;
  }

  vector_interface &operator/=(vector_interface const &b) {
    div(b);
    return *this;
  }

  int scale(double x) { return gsl_vector_scale(p(), x); }

  vector_interface &operator*=(double x) {
    scale(x);
    return *this;
  }

  int add_constant(double x) { return gsl_vector_add_constant(p(), x); }

  vector_interface &operator+=(double x) {
    add_constant(x);
    return *this;
  }

  double sum() const { return gsl_vector_sum(p()); }

  static int axpby(double alpha,
      vector_interface const &x,
      double beta,
      vector_interface &y) {
    return gsl_vector_axpby(alpha, x.p(), beta, y.p());
  }

  double max() const { return gsl_vector_max(p()); }
  double min() const { return gsl_vector_min(p()); }

  void minmax(double &min_out, double &max_out) const {
    gsl_vector_minmax(p(), &min_out, &max_out);
  }

  size_t max_index() const { return gsl_vector_max_index(p()); }
  size_t min_index() const { return gsl_vector_min_index(p()); }

  void minmax_index(size_t &imin, size_t &imax) {
    gsl_vector_minmax_index(p(), &imin, &imax);
  }

  bool isnull() const { return gsl_vector_isnull(p()); }
  bool ispos() const { return gsl_vector_ispos(p()); }
  bool isneg() const { return gsl_vector_isneg(p()); }
  bool isnonneg() const { return gsl_vector_isnonneg(p()); }

  static bool equal(vector_interface const &u, vector_interface const &v) {
    return gsl_vector_equal(u.p(), v.p());
  }

  friend bool operator==(
      vector_interface const &u, vector_interface const &v) {
    return vector_interface::equal(u, v);
  }
};


template<int S> class vector: public vector_interface<vector<S>> {
  static_assert(S > 0);
  template<typename T> friend class vector_interface;

  double d_[S]; ///< Storage for data.
  gsl_vector_view view_; ///< GSL's view of data within instance of vector.

  gsl_vector *pv() { return &view_.vector; }
  gsl_vector const *pv() const { return &view_.vector; }

public:
  vector(): view_(gsl_vector_view_array(d_, S)) {}
};


template<> class vector<DYNAMIC>: public vector_interface<vector<DYNAMIC>> {
  template<typename T> friend class vector_interface;

public:
  enum class alloc_type { ALLOC, CALLOC };

private:
  alloc_type alloc_type_= alloc_type::ALLOC;
  gsl_vector *pv_= nullptr;

  gsl_vector *pv() { return pv_; }
  gsl_vector const *pv() const { return pv_; }

  void free() {
    if(pv_) gsl_vector_free(pv_);
    pv_= nullptr;
  }

  gsl_vector *allocate(size_t n) {
    free();
    if(alloc_type_ == alloc_type::ALLOC) return gsl_vector_alloc(n);
    return gsl_vector_calloc(n);
  }

public:
  vector(size_t n, alloc_type a= alloc_type::ALLOC):
      alloc_type_(a), pv_(allocate(n)) {}

  /// Deep copy on construction.
  vector(vector const &src):
      alloc_type_(src.alloc_type_), pv_(allocate(src.pv_->size)) {
    gsl_vector_memcpy(pv_, src.pv_);
  }

  /// Move on construction.
  vector(vector &&src): alloc_type_(src.alloc_type_), pv_(src.pv_) {
    src.alloc_type_= alloc_type::ALLOC;
    src.pv_= nullptr;
  }

  /// Deep copy on assignment.
  vector &operator=(vector const &src) {
    alloc_type_= src.alloc_type_;
    pv_= allocate(src.pv_->size);
    gsl_vector_memcpy(pv_, src.pv_);
    return *this;
  }

  /// Move on assignment.
  vector &operator=(vector &&src) {
    alloc_type_= src.alloc_type_;
    pv_= src.pv_;
    src.alloc_type_= alloc_type::ALLOC;
    src.pv_= nullptr;
    return *this;
  }

  virtual ~vector() { free(); }
};


template<> class vector<VIEW>: public vector_interface<vector<VIEW>> {
  template<typename T> friend class vector_interface;

  /// GSL's view of data outside instance of vector.
  gsl_vector_view view_;

  gsl_vector *pv() { return &view_.vector; }
  gsl_vector const *pv() const { return &view_.vector; }

  /// Called by subvector() and view_array().
  vector(gsl_vector_view const &v): view_(v) {}

public:
  vector(double *d, size_t n, size_t stride= 1):
      view_(gsl_vector_view_array_with_stride(d, stride, n)) {}

  template<unsigned N>
  vector(double (&d)[N], size_t stride= 1):
      view_(gsl_vector_view_array_with_stride(d, stride, N)) {}

  template<int S>
  vector(vector<S> const &v): view_(v.subvector(0, v.size())) {}
};


template<>
class vector<CONST_VIEW>: public vector_interface<vector<CONST_VIEW>> {
  template<typename T> friend class vector_interface;

  /// GSL's view of data outside instance of vector.
  gsl_vector_const_view view_;

  gsl_vector const *pv() const { return &view_.vector; }

  /// Called by const_subvector() and const_view_array().
  vector(gsl_vector_const_view const &v): view_(v) {}

public:
  vector(double const *d, size_t n, size_t stride= 1):
      view_(gsl_vector_const_view_array_with_stride(d, stride, n)) {}

  template<unsigned N>
  vector(double const (&d)[N], size_t stride= 1):
      view_(gsl_vector_const_view_array_with_stride(d, stride, N)) {}

  template<int S>
  vector(vector<S> const &v): view_(v.const_subvector(0, v.size()).view_) {}
};


template<typename D>
vector<VIEW> vector_interface<D>::subvector(
    size_t offset, size_t n, size_t stride) {
  return gsl_vector_subvector_with_stride(p(), offset, stride, n);
}

template<typename D>
vector<CONST_VIEW> vector_interface<D>::const_subvector(
    size_t offset, size_t n, size_t stride) const {
  return gsl_vector_const_subvector_with_stride(p(), offset, stride, n);
}

template<typename D>
vector<VIEW> vector_interface<D>::view_array(
    double *base, size_t n, size_t stride) {
  return gsl_vector_view_array_with_stride(base, stride, n);
}

template<typename D>
vector<CONST_VIEW> vector_interface<D>::const_view_array(
    double const *b, size_t n, size_t s) {
  return gsl_vector_const_view_array_with_stride(b, s, n);
}


using vectord= vector<DYNAMIC>;
using vectorv= vector<VIEW>;
using vectorc= vector<CONST_VIEW>;


} // namespace gsl

// EOF
