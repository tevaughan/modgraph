/// @file       gsl/Vector.hpp
/// @copyright  2022 Thomas E. Vaughan; all rights reserved.
///
/// @brief      Definition of
///               gsl::VecBase,
///               gsl::Vector, and
///               gsl::makeVector().

#pragma once

// Use inline-definition of each accessor-function.
#ifndef HAVE_INLINE
#define HAVE_INLINE
#endif

#include <gsl/gsl_vector.h> // gsl_vector

/// Namespace for C++-types, each of which corresponds to pure-C type in GSL.
namespace gsl {


enum {
  /// For Vector, template-argument indicating that size of Vector is
  /// determined at run-time, not compile-time, and that dynamic allocation of
  /// elements occurs on construction.
  DYNAMIC= -1,
  /// For Vector, template-argument indicating that size of Vector is
  /// determined at run-time, not compile-time, and that Vector refers to
  /// existing block of data.
  VIEW= -2,
  /// For Vector, template-argument indicating that size of Vector is
  /// determined at run-time, not compile-time, and that Vector refers to
  /// existing block of immutable data.
  CONST_VIEW= -3,
};


template<typename D> struct ConstAccessors {
  auto &v() const { return static_cast<D const *>(this)->vec(); }
  double get(size_t i) const { return gsl_vector_get(&v(), i); }
  double const &operator[](size_t i) const { return v().data[i * v().stride]; }
};


template<typename D, typename T> struct Accessors: public ConstAccessors<D> {};


template<typename D>
struct Accessors<D, gsl_vector>: public ConstAccessors<D> {
  auto &v() { return static_cast<D *>(this)->vec(); }
  void set(size_t i, double x) { gsl_vector_set(&v(), i, x); }
  double &operator[](size_t i) { return v().data[i * v().stride]; }
};


/// Template for base of vector-type incorporating view.
/// - Whether view be const or not, const-accessors are available.
/// - ConstView incorporates either const-view or non-const-view.
/// - ConstView exposes only const-accessors, regardless of view's type.
/// - Each kind of descendant View inherits const-accessors.
/// - Specialization of View adds non-const accessors.
/// @tparam T  Either gsl_vector_view or gsl_vector_const_view.
template<typename T> class ConstView {
protected:
  T view_; ///< View enabling compatibility with gsl_vector.

public:
  /// Copy parameters of view.
  /// @param v  Parameters of view.
  ConstView(T const &v): view_(v) {}

  /// Reference to immutable gsl_vector.
  /// @return  Reference to immutable gsl_vector.
  gsl_vector const &vec() const { return view_.vector; }
};


/// Generic template for intermediate base of vector-type incorporating view.
/// - Specialization View<gsl_vector_view> adds non-const accessors.
template<typename T> struct View: public ConstView<T> {
  using ConstView<T>::ConstView; // Inherit constructor.
};


/// Specialization that adds non-const accessors.
template<> struct View<gsl_vector_view>: public ConstView<gsl_vector_view> {
  using ConstView<gsl_vector_view>::ConstView; // Inherit constructor.
  using ConstView<gsl_vector_view>::vec;

  /// Reference to gsl_vector.
  /// @return  Reference to gsl_vector.
  gsl_vector &vec() { return view_.vector; }
};


/// Generic template for base of type that corresponds to gsl_vector.
/// Specializations:
/// - VecBase<DYNAMIC> for vector with run-time size.
/// - VecBase<VIEW> for vector with external storage.
/// - VecBase<CONST_VIEW> for vector with external, immutable storage.
/// @tparam S  Compile-time number of elements in array.
template<int S>
class VecBase:
    public View<gsl_vector_view>,
    public Accessors<VecBase<S>, gsl_vector> {
  /// Make sure that size is at least unity.
  static_assert(S > 0);

  double data_[S]; ///< Storage for data.

public:
  /// Initialize view of internal vector.
  VecBase(): View<gsl_vector_view>(gsl_vector_view_array(data_, S)) {}
};


/// Specialization for basic vector with run-time size.
template<>
class VecBase<DYNAMIC>: public Accessors<VecBase<DYNAMIC>, gsl_vector> {
  gsl_vector *v_; ///< Pointer to dynamically allocated vector.

public:
  /// Allocate storage for vector.
  /// @param n  Number of elements in vector.
  VecBase(size_t n): v_(gsl_vector_alloc(n)) {}

  /// Deallocate storage for vector.
  virtual ~VecBase() {
    gsl_vector_free(v_);
    v_= nullptr;
  }

  /// Convert to reference to gsl_vector.
  /// @return  Reference to gsl_vector.
  gsl_vector &vec() { return *v_; }

  /// Convert to reference to immutable gsl_vector.
  /// @return  Reference to immutable gsl_vector.
  gsl_vector const &vec() const { return *v_; }
};


/// Specialization for basic vector that has run-time size and that refers to
/// storage outside itself.
template<>
class VecBase<VIEW>:
    public View<gsl_vector_view>,
    public Accessors<VecBase<VIEW>, gsl_vector> {
public:
  /// Initialize view of external vector.
  /// @param data  Pointer to first element.
  /// @param n  Number of elements in view.
  /// @param str  Memory-increments between consecutive elements in view.
  VecBase(double *data, size_t n, size_t str= 1):
      View<gsl_vector_view>(gsl_vector_view_array_with_stride(data, str, n)) {}

  /// Initialize view of external vector.
  /// @tparam N  Number of elements in vector.
  /// @param data  Pointer to first element.
  /// @param n  Number of elements in view.
  /// @param str  Memory-increments between consecutive elements in view.
  template<size_t N>
  VecBase(double (&data)[N], size_t n= N, size_t str= 1):
      View<gsl_vector_view>(gsl_vector_view_array_with_stride(data, str, n)) {}
};


/// Specialization for basic vector that has run-time size and that refers to
/// immutable storage outside itself.
template<>
class VecBase<CONST_VIEW>:
    public View<gsl_vector_const_view>,
    public Accessors<VecBase<CONST_VIEW>, gsl_vector const> {
public:
  /// Initialize view of external, immutable vector.
  /// @param data  Pointer to first element.
  /// @param n  Number of elements in view.
  /// @param str  Memory-increments between consecutive elements in view.
  VecBase(double const *data, size_t n, size_t str= 1):
      View<gsl_vector_const_view>(
          gsl_vector_const_view_array_with_stride(data, str, n)) {}

  /// Initialize view of external, immutable vector.
  /// @tparam N  Number of elements in vector.
  /// @param data  Pointer to first element.
  /// @param n  Number of elements in view.
  /// @param str  Memory-increments between consecutive elements in view.
  template<size_t N>
  VecBase(double const (&data)[N], size_t n= N, size_t str= 1):
      View<gsl_vector_const_view>(
          gsl_vector_const_view_array_with_stride(data, str, n)) {}

  /// Initialize immutable view of external vector.
  /// @tparam N  Number of elements in vector.
  /// @param data  Pointer to first element.
  /// @param n  Number of elements in view.
  /// @param str  Memory-increments between consecutive elements in view.
  template<size_t N>
  VecBase(double (&data)[N], size_t n= N, size_t str= 1):
      View<gsl_vector_const_view>(
          gsl_vector_const_view_array_with_stride(data, str, n)) {}
};


/// Type that corresponds to gsl_vector.
/// @tparam S  Number of elements or indication of dynamic allocation.
template<int S> struct Vector: public VecBase<S> {
  using VecBase<S>::VecBase; // Inherit constructors.
};


/// Construct vector of dynamically allocated data.
/// @param n  Number of elements.
/// @return  Constructed vector.
auto makeVector(size_t n) { return Vector<DYNAMIC>(n); }


/// Construct vector referring to external data.
/// @param data  Pointer to first element.
/// @param n  Number of elements in view.
/// @param stride  Memory-increments between consecutive elements of view.
/// @return  Constructed view.
auto makeVector(double *data, size_t n, size_t stride= 1) {
  return Vector<VIEW>(data, n, stride);
}


/// Construct vector referring to external array.
/// @tparam N  Number of elements in array.
/// @param data  Pointer to first element.
/// @param n  Number of elements in view.
/// @param stride  Memory-increments between consecutive elements of view.
/// @return  Constructed view.
template<size_t N>
auto makeVector(double (&data)[N], size_t n= N, size_t stride= 1) {
  return Vector<VIEW>(data, n, stride);
}


/// Construct vector referring to external, immutable array.
/// @param data  Pointer to first element in array.
/// @param n  Number of elements in view.
/// @param stride  Memory-increments between consecutive elements of view.
/// @return  Constructed view.
auto makeVector(double const *data, size_t n, size_t stride= 1) {
  return Vector<CONST_VIEW>(data, n, stride);
}


/// Construct vector referring to external array.
/// @tparam N  Number of elements in array.
/// @param data  Pointer to first element of array.
/// @param n  Number of elements in view.
/// @param stride  Memory-increments between consecutive elements of view.
/// @return  Constructed view.
template<size_t N>
auto makeVector(double const (&data)[N], size_t n= N, size_t stride= 1) {
  return Vector<CONST_VIEW>(data, n, stride);
}


} // namespace gsl

/// EOF
