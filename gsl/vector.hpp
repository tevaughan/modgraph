/// @file       gsl/vector.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition of gsl::vector.

#pragma once

// Use inline-definition of each accessor-function.
// - Define this before including `gsl_vector.h`.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include <gsl/gsl_vector.h>

namespace gsl {


class vector: protected gsl_vector {
  vector()= delete;
  vector(vector const&)= delete;
  vector &operator=(vector const &)= delete;

public:
  size_t size() const { return gsl_vector::size; }
  size_t stride() const { return gsl_vector::stride; }
  double *data() { return gsl_vector::data; }
  double const *data() const { return gsl_vector::data; }
  gsl_block const *block() const { return gsl_vector::block; }
  int owner() const { return gsl_vector::owner; }

  static vector *alloc(size_t n) {
    return static_cast<vector *>(gsl_vector_alloc(n));
  }

  static vector *calloc(size_t n) {
    return static_cast<vector *>(gsl_vector_calloc(n));
  }

  void free() { gsl_vector_free(this); }

  /// Read element with bounds-checking.
  double get(size_t i) const { return gsl_vector_get(this, i); }

  /// Write element with bounds-checking.
  void set(size_t i, double x) { gsl_vector_set(this, i, x); }

  /// Read element without bounds-checking.
  double const &operator[](size_t i) const { return data()[i * stride()]; }

  /// Write element without bounds-checking.
  double &operator[](size_t i) { return data()[i * stride()]; }

  /// Retrieve pointer with bounds-checking.
  double *ptr(size_t i) { return gsl_vector_ptr(this, i); }

  /// Retrieve pointer with bounds-checking.
  double const *ptr(size_t i) const { return gsl_vector_const_ptr(this, i); }

  void set_all(double x) { gsl_vector_set_all(this, x); }
  void set_zero() { gsl_vector_set_zero(this); }
  int set_basis(size_t i) { return gsl_vector_set_basis(this, i); }
  int fwrite(FILE *f) const { return gsl_vector_fwrite(f, this); }
  int fread(FILE *f) { return gsl_vector_fread(f, this); };

  int fprintf(FILE *file, char const *fmt) const {
    return gsl_vector_fprintf(file, this, fmt);
  }

  int fscanf(FILE *f) { return gsl_vector_fscanf(f, this); }

  class view: protected gsl_vector_view {
    view()= delete;
    view(view const &)= default;
    view &operator=(view const &)= delete;
    friend class vector;
    view(gsl_vector_view const &v): gsl_vector_view(v) {}

  public:
    gsl::vector &vector() {
      return static_cast<gsl::vector &>(gsl_vector_view::vector);
    }

    gsl::vector const &vector() const {
      return static_cast<gsl::vector const &>(gsl_vector_view::vector);
    }
  };

  class const_view: protected gsl_vector_const_view {
    const_view()= delete;
    const_view(const_view const &)= delete;
    const_view &operator=(const_view const &)= delete;
    friend class vector;
    const_view(gsl_vector_const_view const &v): gsl_vector_const_view(v) {}

  public:
    gsl::vector const &vector() const {
      return static_cast<gsl::vector const &>(gsl_vector_const_view::vector);
    }
  };

  view subvector(size_t offset, size_t n, size_t stride= 1) {
    return gsl_vector_subvector_with_stride(this, offset, n, stride);
  }

  const_view const_subvector(size_t offset, size_t n, size_t stride= 1) {
    return gsl_vector_const_subvector_with_stride(this, offset, n, stride);
  }

  static view view_array(double *base, size_t n, size_t stride= 1) {
    return gsl_vector_view_array_with_stride(base, n, stride);
  }

  static const_view const_view_array(double const *b, size_t n, size_t s= 1) {
    return gsl_vector_const_view_array_with_stride(b, n, s);
  }

  int memcpy(vector const &src) { return gsl_vector_memcpy(this, &src); }
  int swap(vector &w) { return gsl_vector_swap(this, &w); }

  int swap_elements(size_t i, size_t j) {
    return gsl_vector_swap_elements(this, i, j);
  }

  int reverse() { return gsl_vector_reverse(this); }

  int add(vector const &b) { return gsl_vector_add(this, &b); }
  int sub(vector const &b) { return gsl_vector_sub(this, &b); }
  int mul(vector const &b) { return gsl_vector_mul(this, &b); }
  int div(vector const &b) { return gsl_vector_div(this, &b); }

  vector &operator+=(vector const &b) {
    add(b);
    return *this;
  }

  vector &operator-=(vector const &b) {
    sub(b);
    return *this;
  }

  vector &operator*=(vector const &b) {
    mul(b);
    return *this;
  }

  vector &operator/=(vector const &b) {
    div(b);
    return *this;
  }

  int scale(double x) { return gsl_vector_scale(this, x); }

  vector &operator*=(double x) {
    scale(x);
    return *this;
  }

  int add_constant(double x) { return gsl_vector_add_constant(this, x); }

  vector &operator+=(double x) {
    add_constant(x);
    return *this;
  }

  double sum() const { return gsl_vector_sum(this); }

  static int axpby(double alpha, vector const &x, double beta, vector &y) {
    return gsl_vector_axpby(alpha, &x, beta, &y);
  }

  double max() const { return gsl_vector_max(this); }
  double min() const { return gsl_vector_min(this); }

  void minmax(double &min_out, double &max_out) const {
    gsl_vector_minmax(this, &min_out, &max_out);
  }

  size_t max_index() const { return gsl_vector_max_index(this); }
  size_t min_index() const { return gsl_vector_min_index(this); }

  void minmax_index(size_t &imin, size_t &imax) {
    gsl_vector_minmax_index(this, &imin, &imax);
  }

  bool isnull() const { return gsl_vector_isnull(this); }
  bool ispos() const { return gsl_vector_ispos(this); }
  bool isneg() const { return gsl_vector_isneg(this); }
  bool isnonneg() const { return gsl_vector_isnonneg(this); }

  static bool equal(vector const &u, vector const &v) {
    return gsl_vector_equal(&u, &v);
  }

  friend bool operator==(vector const &u, vector const &v) {
    return vector::equal(u, v);
  }
};


} // namespace gsl

// EOF
