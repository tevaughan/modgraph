/// @file       gsl/VectorTest.cpp
/// @copyright  2022 Thomas E. Vaughan; all rights reserved.
/// @brief      Tests for gsl::Vector.

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "Vector.hpp"

using gsl::makeVector;
using gsl::Vector;


TEST_CASE("Statically allocated vector basically works.", "[static]") {
  Vector<10> v;
  for(int i= 9; i >= 0; --i) { v.set(i, i); }
  for(int i= 0; i < 10; ++i) { REQUIRE(v.get(i) == i); }
  for(int i= 9; i >= 0; --i) { v[i]= 2 * i; }
  for(int i= 0; i < 10; ++i) { REQUIRE(v[i] == 2 * i); }
}


TEST_CASE("Dynamically allocated vector basically works.", "[dynamic]") {
  auto v= makeVector(10);
  for(int i= 9; i >= 0; --i) { v.set(i, i); }
  for(int i= 0; i < 10; ++i) { REQUIRE(v.get(i) == i); }
  for(int i= 9; i >= 0; --i) { v[i]= 2 * i; }
  for(int i= 0; i < 10; ++i) { REQUIRE(v[i] == 2 * i); }
}


TEST_CASE("View basically works.", "[view]") {
  double data[10];

  auto v1= makeVector(data); // ten elements (default stride 1)
  for(int i= 9; i >= 0; --i) { v1.set(i, i); }
  for(int i= 0; i < 10; ++i) { REQUIRE(v1.get(i) == i); }
  for(int i= 9; i >= 0; --i) { v1[i]= 2 * i; }
  for(int i= 0; i < 10; ++i) { REQUIRE(v1[i] == 2 * i); }

  double *d= data; // decayed pointer

  auto v2= makeVector(d, 10); // ten elements (default stride 1)
  for(int i= 9; i >= 0; --i) { v2.set(i, i); }
  for(int i= 0; i < 10; ++i) { REQUIRE(v2.get(i) == i); }
  for(int i= 9; i >= 0; --i) { v2[i]= 2 * i; }
  for(int i= 0; i < 10; ++i) { REQUIRE(v2[i] == 2 * i); }

  auto v3= makeVector(data, 5, 2); // five elements, stride 2
  for(int i= 0; i < 5; ++i) { v3.set(i, i); }
  for(int i= 0; i < 5; ++i) {
    REQUIRE(v3.get(i) == i);
    REQUIRE(v2.get(2 * i) == i);
  }
  for(int i= 0; i < 5; ++i) { v3[i]= 2 * i; }
  for(int i= 0; i < 5; ++i) {
    REQUIRE(v3[i] == 2 * i);
    REQUIRE(v2[2 * i] == 2 * i);
  }

  auto v4= makeVector(d, 5, 2); // five elements, stride 2
  for(int i= 0; i < 5; ++i) { v4.set(i, i); }
  for(int i= 0; i < 5; ++i) {
    REQUIRE(v4.get(i) == i);
    REQUIRE(v2.get(2 * i) == i);
  }
  for(int i= 0; i < 5; ++i) { v4[i]= 2 * i; }
  for(int i= 0; i < 5; ++i) {
    REQUIRE(v4[i] == 2 * i);
    REQUIRE(v2[2 * i] == 2 * i);
  }
}


TEST_CASE("View of immutable data basically works.", "[const-view]") {
  double const data[10]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  auto v1= makeVector(data); // ten elements (default stride 1)
  for(int i= 0; i < 10; ++i) {
    REQUIRE(v1.get(i) == i);
    REQUIRE(v1[i] == i);
  }

  double const *d= data; // decayed pointer

  auto v2= makeVector(d, 10); // ten elements (default stride 1)
  for(int i= 0; i < 10; ++i) {
    REQUIRE(v2.get(i) == i);
    REQUIRE(v2[i] == i);
  }

  auto v3= makeVector(data, 5, 2); // five elements, stride 2
  for(int i= 0; i < 5; ++i) {
    REQUIRE(v3.get(i) == 2 * i);
    REQUIRE(v3[i] == 2 * i);
  }

  auto v4= makeVector(d, 5, 2); // five elements, stride 2
  for(int i= 0; i < 5; ++i) {
    REQUIRE(v4.get(i) == 2 * i);
    REQUIRE(v4[i] == 2 * i);
  }
}


// EOF
