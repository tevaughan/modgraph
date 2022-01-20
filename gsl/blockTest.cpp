/// @file       gsl/blockTest.cpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Tests for gsl::block.

#include "block.hpp"
#include <catch.hpp>


TEST_CASE("gsl::block works properly.", "[block]") {
  auto b= gsl::block::alloc(100);
  REQUIRE(b->size() == 100);
  b->free();
}


// EOF
