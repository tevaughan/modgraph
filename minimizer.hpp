
/// @file       minimizer.hpp
///
/// @brief      Definition of modgraph::node_pair; declaration of
///             modgraph::minimizer.
///
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include <eigen3/Eigen/Dense> // Matrix
#include <gsl/gsl_multimin.h> // gsl_vector_view, gsl_vector_const_view
#include <iostream> // cerr, endl
#include <vector> // vector

namespace modgraph {


class graph;


/// Information relevant to calculation of force between pair of nodes.
class node_pair {
  int const i_; ///< Offset of one node.
  int const j_; ///< Offset of other node.
  double const r_; ///< Distance between Node i and Node j.
  Eigen::Vector3d const u_; ///< Unit-vector from Node i to Node j.

public:
  /// Initialize offset of one node and other, distance between nodes, and
  /// unit-vector from one toward other.
  /// @param i  Offset of one node.
  /// @param j  Offset of other node.
  /// @param d  Displacement from one node to other.
  node_pair(int const i, int const j, Eigen::Vector3d const &d):
      i_(i), j_(j), r_(d.norm()), u_(d / r_) {}

  /// Offset of one node.
  /// @return  Offset of one node.
  int i() const { return i_; }

  /// Offset of other node.
  /// @return  Offset of other node.
  int j() const { return j_; }

  /// Distance between nodes.
  /// @return  Distance between nodes.
  double r() const { return r_; }

  /// Unit-vector from one node toward other.
  /// @return  Unit-vector from one node toward other.
  Eigen::Vector3d const &u() const { return u_; }
};


/// Facility for force-minimization via GSL of nodes in directed graph of
/// squares under modular arithmetic.
class minimizer {
  /// Reference to graph whose nodes are to be positioned by minimization.
  graph &graph_;

  /// 3NxN matrix storing force felt by each node from each other node.
  /// - forces_ is initialized by net_force_and_pot().
  Eigen::MatrixXd forces_;

  /// 3Nx1 matrix storing net force felt by each node.
  /// - net_forces_ is initialized by net_force_and_pot().
  Eigen::MatrixXd net_forces_;

  /// Scalar potential whose gradient produces forces.
  /// - potential_ is calculated by net_force_and_pot().
  double potential_;

  /// Compute force felt by Node i from Node j, and update potential_.
  /// - force_and_pot() is called by net_force_and_pot().
  /// @param np  Information needed to calculate force on one node from other.
  /// @return  Force felt by Node i from Node j.
  Eigen::Vector3d force_and_pot(node_pair const &np);

  /// Scale of attraction of every Node `i` to each Nodes `j` whenever either
  /// `i` maps to `j`, or `j` maps to `i`; that is, whenever Node `i` and Node
  /// `j` are connected by a directed edge.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  /// @return  Scale of attraction of every Node `i` to each Node `j` whenever
  ///          they are connected by a directed edge.
  double edge_attract_= 1.5;

  /// Relative scale of attraction of every Node `i` to each Node `j` whenever
  /// `(i + j) % m` is either factor `f` of modulus `m` or `m - f`.
  /// - Attraction is usually proportional to `f` but proportional to `m`
  ///   (greatest) if `(i + j) % m` be zero.
  /// - Absolute scale for forces is set by universal repulsion, which decays
  ///   with inverse-square distance and has unit-value between two nodes
  ///   whenever they be separated by unit distance.
  /// @return  Relative scale of attraction of every Node `i` to each Node `j`
  ///          whenever `(i + j) % m` is either `f` or `m - f`.
  double sum_attract_= 15.0;

  /// Relative scale of attraction of every Node `i` to each Node `j` whenever
  /// `j` is either factor `f` of modulus `m` or `m - f`.
  /// - Attraction is usually proportional to `f` but proportional to `m`
  ///   (greatest) if `j` be zero.
  /// - Absolute scale for forces is set by universal repulsion, which decays
  ///   with inverse-square distance and has unit-value between two nodes
  ///   whenever they be separated by unit distance.
  /// @return  Relative scale of attraction of every Node `i` to each Node `j`
  ///          whenever `j` is either `f` or `m` - `f`.
  double factor_attract_= 150.0;

  // Minimize potential via simplex method not requiring forces.
  // - This is called by minimize().
  /// @param positions  3xN matrix for position of each of N nodes.
  void minimize_nm_simplex(Eigen::Matrix3Xd &positions);

  // Minimize potential via gradient-method requiring forces.
  // - This is called by minimize().
  /// @param positions  3xN matrix for position of each of N nodes.
  void minimize_gradient(Eigen::Matrix3Xd &positions);

  /// Calculate spring-force of attraction felt by one node from another, and
  /// increment global potential.
  /// @param k  Spring-constant.
  /// @param np  Information needed to calculate force on one node from other.
  /// @return  Force felt by one node from other.
  Eigen::Vector3d attract(double k, node_pair const &np);

  /// Calculate inverse-square-distance repulsive force between felt by one
  /// node from other, and increment global potential.
  /// @param np  Information needed to calculate force on one node from other.
  /// @return  Force felt by one node from other.
  Eigen::Vector3d repel(node_pair const &np);

  /// Calculate spring-attraction force felt by Node i from Node j because of
  /// directed graph-edge between them.
  /// @param np  Information needed to calculate force on one node from other.
  /// @return  Force felt by Node i.
  Eigen::Vector3d edge_attract(node_pair const &np);

  /// Calculate spring-attraction force felt by Node i from Node j either
  /// because (i+j)%M==f, where f is factor of modulus M, or because
  /// (i+j)%M==M-f.
  /// @param np  Information needed to calculate force on one node from other.
  /// @return  Force felt by Node i.
  Eigen::Vector3d sum_attract(node_pair const &np);

  /// Calculate spring-attraction force felt by Node i from Node j when either
  /// i or j be factor of modulus or when either i or j be smaller than modulus
  /// by factor of modulus.
  /// - Attraction is usually proportional to factor but proportional to
  ///   modulus itself (strongest) if i or j be zero.
  /// @param np  Information needed to calculate force on one node from other.
  /// @return  Force felt by Node i.
  Eigen::Vector3d factor_attract(node_pair const &np);

public:
  /// Initialize moduls for graph of squares.
  /// @param g  Reference to graph whose nodes are to be positioned.
  minimizer(graph &g): graph_(g) {}

  /// Compute net force felt by each node from every other node, and compute
  /// overall potential of system.
  /// - Argument is reference to gsl's own working copy of positions.
  /// @param positions  3xN matrix for position of each of N particles.
  void net_force_and_pot(Eigen::Matrix3Xd const &positions);

  /// Copy initial `positions` into gsl; drive gsl's minimizer; and
  /// then copy final values from gsl back into `positions`.
  /// @param positions  3xN matrix for position of each of N nodes.
  void go(Eigen::Matrix3Xd &positions);

  /// Scalar potential whose gradient produces forces.
  /// - potential_ is calculated by net_force_and_pot().
  /// @return  Scalar potential whose gradient produces forces.
  double potential() const { return potential_; }

  /// (i % 3)th component of net-force on (i / 3)th node.
  /// @return  (i % 3)th component of net-force on (i / 3)th node.
  double net_force_component(int i) const { return net_forces_(i, 0); }

  /// Scale of attraction of every Node `i` to each Nodes `j` whenever either
  /// `i` maps to `j`, or `j` maps to `i`; that is, whenever Node `i` and Node
  /// `j` are connected by a directed edge.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  /// @return  Scale of attraction of every Node `i` to each Node `j` whenever
  ///          they are connected by a directed edge.
  double edge_attract() const { return edge_attract_; }

  /// Relative scale of attraction of every Node `i` to each Node `j` whenever
  /// `(i + j) % m` is either factor `f` of modulus `m` or `m - f`.
  /// - Attraction is usually proportional to `f` but proportional to `m`
  ///   (greatest) if `(i + j) % m` be zero.
  /// - Absolute scale for forces is set by universal repulsion, which decays
  ///   with inverse-square distance and has unit-value between two nodes
  ///   whenever they be separated by unit distance.
  /// @return  Relative scale of attraction of every Node `i` to each Node `j`
  ///          whenever `(i + j) % m` is either `f` or `m - f`.
  double sum_attract() const { return sum_attract_; }

  /// Relative scale of attraction of every Node `i` to each Node `j` whenever
  /// `j` is either factor `f` of modulus `m` or `m - f`.
  /// - Attraction is usually proportional to `f` but proportional to `m`
  ///   (greatest) if `j` be zero.
  /// - Absolute scale for forces is set by universal repulsion, which decays
  ///   with inverse-square distance and has unit-value between two nodes
  ///   whenever they be separated by unit distance.
  /// @return  Relative scale of attraction of every Node `i` to each Node `j`
  ///          whenever `j` is either `f` or `m` - `f`.
  double factor_attract() const { return factor_attract_; }
};


} // namespace modgraph

// EOF
