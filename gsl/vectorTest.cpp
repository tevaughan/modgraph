/// @file       gsl/vectorTest.cpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Tests for gsl::vector.

#include "vector.hpp"
#include <catch.hpp>

using gsl::vector;


TEST_CASE("gsl::vector works properly.", "[vector]") {
  vector *v= vector::alloc(3);
  for(int i= 0; i < 3; ++i) { v->set(i, 1.23 + i); }
  for(int i= 0; i < 3; ++i) { REQUIRE(v->get(i) == 1.23 + i); }
}

// EOF
