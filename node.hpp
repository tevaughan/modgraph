
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include <vector>

namespace modgraph {


/// Properties of number n in graph of squares modulo N.
struct node {
  int next; ///< Square modulo N.
  std::vector<int> prev; ///< Inverses of square modulo N.

  /// Construct uninitialized node.
  node(): next(-1) {}
};


} // namespace modgraph

// EOF
