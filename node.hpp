
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include <vector>
#include <eigen3/Eigen/Core> // Vector3d

namespace modgraph {


/// Properties of number n in graph of squares modulo N.
struct node {
  int next; ///< Square modulo N.
  std::vector<int> prev; ///< Inverses of square modulo N.
  int subg; ///< Subgraph to which node belongs.
  Eigen::Vector3d pos; ///< Position in three-dimensional space.

  /// If n <= N - n, then `complement` = N - n; otherwise `complement` = -1.
  int complement;

  /// Construct uninitialized node.
  node(): next(-1), subg(-1) {}
};


} // namespace modgraph

// EOF
