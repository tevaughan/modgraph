/// @file       gsl/size-code.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition for gsl::size_code.

#pragma once

namespace gsl {


/// Non-positive size-codes, each of which is legal to use as
/// integer-template-argument to class vector.
/// - When positive, integer-template-argument to class vector is number of
///   elements stored inside instance of vector.
/// - Two non-positive size-codes are defined, each with different meaning.
/// - Each non-positive code indicates that size of vector is determined not at
///   compile-time but at run-time.
enum size_code {
  /// Vector's data are contained within instance in memory allocated when
  /// instance is constructed.
  DYNAMIC= 0,
  /// Vector's data are stored outside instance, and memory is *not* allocated
  /// when instance is constructed.
  VIEW= -1,
};


} // namespace gsl

// EOF
