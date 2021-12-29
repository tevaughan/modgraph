
#pragma once

#include "node.hpp" // node
#include <set> // set
#include <vector> // vector

namespace modgraph {


class graph {
  using subgraph= std::set<int>;

  std::vector<subgraph> subgraphs_; ///< Partition of nodes into subgraphs.
  std::vector<node> nodes_; ///< Collection of all nodes.

  /// Strength of universal attraction, which is constant force between every
  /// pair of nodes.
  /// - `univ_attract` should not be negative because there is already
  ///   universal repulsion.
  /// - `univ_attract` need be positive only when there exist utterly disjoint
  ///   subgraphs with no attraction whatsoever between nodes of one and nodes
  ///   of other.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   distance and has unit-value between two nodes whenever they be
  ///   separated by unit distance.
  double univ_attract_= 0.0;

  /// Strength of attraction between nodes connected by a directed edge.
  /// - `direct_attract_` should not be negative.
  /// - `direct_attract_` is constant force independent of distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   distance and has unit-value between two nodes whenever they be
  ///   separated by unit distance.
  double direct_attract_= 1.0;

  /// Strength of attraction between pair of nodes whose sum is modulus.
  /// - `sum_modulus_attract_` should not be negative.
  /// - `sum_modulus_attract_` is constant force independent of distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   distance and has unit-value between two nodes whenever they be
  ///   separated by unit distance.
  double sum_modulus_attract_= 0.50;

  /// Strength of attraction between pair of nodes whose sum is factor of
  /// modulus.
  /// - `sum_factor_attract_` should not be negative.
  /// - `sum_factor_attract_` is constant force independent of distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   distance and has unit-value between two nodes whenever they be
  ///   separated by unit distance.
  double sum_factor_attract_= 0.25;

  /// Check previous or next node for connection to subgraph at offset s_off.
  /// - check_node() is called only by traverse().
  /// - `n_off` is previous or next relative to current node in traverse().
  /// @param n_off  Offset of previous or next node.
  /// @param s_off  Offset of subgraph.
  void check_node(int n_off, int s_off);

  /// Mark node at offset `n_off` and every connected node as belonging to
  /// subgraph at offset `s_off`.
  ///
  /// @param n_off  Offset of initial node whose connections are all to be
  ///               marked as belonging to subgraph with offset `s_off`.
  ///
  /// @param s_off  Offset of subgraph to associate with node at `n_off` and
  ///               with every node connected to `n_off`.
  void traverse(int n_off, int s_off);


  /// Partition into subgraphs.
  /// - I might be able to eliminate this after asymptote fully replaces neato.
  void partition();

  void connect(); ///< Establish all interconnections among nodes.
  void init_loc(); ///< Choose initial, random location for every node.
  void arrange_3d(); ///< Arrange nodes three-dimensionally.
  void write_neato() const; ///< Write text-files for neato.
  void write_asy() const; ///< Write text-file for asymptote.

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
