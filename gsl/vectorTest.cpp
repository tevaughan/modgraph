/// @file       gsl/vectorTest.cpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Tests for gsl::vector.

#include "vector.hpp"
#include <catch.hpp>

using gsl::vector;
using gsl::vectorc;
using gsl::vectord;
using gsl::vectorv;


template<int S> void set_get(vector<S> &v) {
  v.set_all(0.0);
  for(size_t i= 0; i < v.size(); ++i) { REQUIRE(v.get(i) == 0.0); }

  for(size_t i= 0; i < v.size(); ++i) { v.set(i, 1.23 + i); }
  for(size_t i= 0; i < v.size(); ++i) { REQUIRE(v.get(i) == 1.23 + i); }

  for(size_t i= 0; i < v.size(); ++i) { v[i]= 2.46 + i; }
  for(size_t i= 0; i < v.size(); ++i) { REQUIRE(v[i] == 2.46 + i); }
}


TEST_CASE("gsl::vector works properly.", "[vector]") {
  vector<3> v1;
  set_get(v1);

  vectord v2(10);
  set_get(v2);

  vectorv v3= v2.subvector(1, 4, 2);
  REQUIRE(v3[0] == 2.46 + 1);
  REQUIRE(v3[1] == 2.46 + 3);
  REQUIRE(v3[2] == 2.46 + 5);
  REQUIRE(v3[3] == 2.46 + 7);
  set_get(v3);

  vectorc v4= v3;
  REQUIRE(v3[0] == 2.46 + 0);
  REQUIRE(v3[1] == 2.46 + 1);
  REQUIRE(v3[2] == 2.46 + 2);
  REQUIRE(v3[3] == 2.46 + 3);
}

// EOF
