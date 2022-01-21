/// @file       gsl/vec-base-test.cpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Tests for gsl::vec_base.

#include "vector.hpp"
#include <catch.hpp>

using gsl::vec_base;
using gsl::vec_iface;


/// See that C-array matches gsl::vec_iface with stride.
/// @tparam T  Type of descendant of vec_iface<T>.
/// @param a  C-array.
/// @param b  Reference to instance of vec_iface<T>.
/// @param s  Stride.
template<typename T>
void check(double const *a, vec_iface<T> const &b, size_t s= 1) {
  for(size_t j= 0, i= 0; j < b.size(); ++j, i+= s) { REQUIRE(a[i] == b[j]); }
}


TEST_CASE("vec_base works properly.", "[vec_base]") {
  double a[]= {1, 1, 2, 3, 5, 8};
  auto b= vec_base::view_array(a);
  check(a, b);
}


// EOF
