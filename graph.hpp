
#pragma once

#include "node.hpp" // node
#include <set> // set
#include <vector> // vector

namespace modgraph {


class graph {
  using subgraph= std::set<int>;

  std::vector<subgraph> subgraphs_; ///< Partition of nodes into subgraphs.
  std::vector<node> nodes_; ///< Collection of all nodes.

  /// Linear size across volume in which nodes will be distributed.
  /// - For simple graph and even for large sections of complicated graph,
  ///   natural symmetry lies in plane.
  /// - So making linear size of volume be size of each node (implicitly one)
  ///   times square root of number of nodes will allow for repulsion between
  ///   nodes to push them into planar arrangement.
  /// - Attractive force between one node and every other becomes significant
  ///   only above scale defined by `size`.
  double lin_size_;

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

  /// Write text files for neato.
  void write_neato() const;

  /// Establish all interconnections among nodes.
  void connect();

  /// Partition into subgraphs.
  /// - I might be able to eliminate this after asymptote fully replaces neato.
  void partition();

  /// Choose initial, random location for every node.
  void init_loc();

  /// Calculate net force on each node.
  /// @param   On return, maximum force between any two nodes.
  /// @return  Net force on each node.
  std::vector<Eigen::Vector3d> forces(Eigen::Vector3d &max) const;

  /// Arrange nodes three-dimensionally.
  void arrange_3d();

public:
  /// Construct graphs for modulus m.
  /// @param m  Modulus of graphs.
  graph(int m);
};


} // namespace modgraph

// EOF
