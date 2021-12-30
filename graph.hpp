
#pragma once

#include "node.hpp" // node
#include <set> // set
#include <vector> // vector

namespace modgraph {


class graph {
  std::vector<node> nodes_; ///< Collection of all nodes.
  std::vector<int> factors_; ///< All composite factors of modulus.

  /// Scale of universal attraction between every node and the origin.
  /// - `univ_attract` should be larger than unity; the larger, the weaker.
  /// - `univ_attract` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double univ_attract_= 20.0;

  /// Scale of attraction between pair of nodes connected by a directed edge.
  /// - `direct_attract_` should be larger than unity.
  /// - `direct_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double direct_attract_= 2.5;

  /// Scale of attraction between pair of nodes whose sum is modulus.
  /// - `sum_modulus_attract_` should be larger than unity.
  /// - `sum_modulus_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double sum_modulus_attract_= 5.0;

  /// Scale of attraction between pair of nodes whose sum is factor of modulus.
  /// - `sum_factor_attract_` should be larger than unity.
  /// - `sum_factor_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double sum_factor_attract_= 10.0;

  void connect(); ///< Establish all interconnections among nodes.
  void init_loc(); ///< Choose initial, random location for every node.
  void arrange_3d(); ///< Arrange nodes three-dimensionally.
  void write_asy() const; ///< Write text-file for asymptote.

  /// Initialize list of composite factors of modulus.
  /// - It seems best to constrain locations only on basis of composite
  ///   factors, if any exist.
  /// @param m  Modulus.
  void init_factors(int m);

  /// Calculate net force on node.
  /// @param off  Offset of node.
  /// @return     Net force on node.
  Eigen::Vector3d force(int off) const;

public:
  /// Construct graphs for modulus m.
  /// @param m  Modulus of graphs.
  graph(int m);
};


} // namespace modgraph

// EOF
