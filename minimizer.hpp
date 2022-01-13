/// @file       minimizer.hpp
/// @brief      Declaration of modgraph::minimizer.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include "node.hpp" // node
#include <eigen3/Eigen/Dense> // Matrix
#include <gsl/gsl_multimin.h> // gsl_vector_view, gsl_vector_const_view
#include <vector> // vector

#if 0
#define NM_SIMPLEX
#endif

namespace modgraph {


using Eigen::Matrix3Xd;
using Eigen::MatrixXd;
using Eigen::Vector3d;


/// Facility for force-minimization via GSL.
class minimizer {
  /// Relationships among nodes for modulus equal to nodes_.size().
  std::vector<node> const &nodes_;

  /// 3NxN matrix storing force felt by each node from each other node.
  /// - forces_ is initialized by net_force_and_pot().
  MatrixXd forces_;

  /// 3Nx1 matrix storing net force felt by each node.
  /// - net_forces_ is initialized by net_force_and_pot().
  MatrixXd net_forces_;

  /// Scalar potential whose gradient produces forces.
  /// - potential_ is calculated by net_force_and_pot().
  double potential_;

  /// Compute force felt by Node i from Node j, and update potential_.
  /// - force_and_pot() is called by net_force_and_pot().
  /// @param i  Offset of one node.
  /// @param j  Offset of other node.
  /// @return   Force felt by Node i from Node j.
  Vector3d force_and_pot(unsigned i, unsigned j, Matrix3Xd const &positions);

  /// Compute net force felt by each node from every other node, and compute
  /// overall potential of system.
  /// - `calc_net_force_and_pot()` takes argument (and does not directly use
  ///   positions_) because gsl keeps its own record of positions during
  ///   minimization.
  /// @param positions  3xN matrix for position of each of N particles.
  void net_force_and_pot(Matrix3Xd const &positions);

  /// Scale of attraction between pair of nodes connected by a directed edge.
  /// - `edge_attract_` should be larger than unity.
  /// - `edge_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double edge_attract_= 1.5;

  /// Scale of attraction between pair of nodes whose sum is modulus or half of
  /// modulus.
  /// - `sum_attract_` should be larger than unity.
  /// - `sum_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double sum_attract_= 15.0;

  /// Scale of attraction between (a) each of zero and one and (b) every other
  /// node.
  /// - `all_attract_` should be larger than unity.
  /// - `all_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double all_attract_= 150.0;

  /// Potential that GSL will minimize.
  /// - Data in `x` have same structure as data in positions_.
  /// - However, gsl maintains its own copy of them during minimization.
  /// @param x  Pointer to working position-components of every particle.
  /// @param p  Pointer to instance of graph.
  /// @return   Potential to be minimized.
  static double f(gsl_vector const *x, void *p);

#ifndef NM_SIMPLEX
  /// Calculate gradient of potential.
  /// - Data in `x` have same structure as data in positions_.
  /// - However, gsl maintains its own copy of them during minimization.
  /// @param x  Pointer to working position-components of every particle.
  /// @param p  Pointer to instance of graph.
  /// @param g  Pointer to (output) components of gradient.
  static void df(gsl_vector const *x, void *p, gsl_vector *g);

  /// Calculate potential to be minimized and gradient of potential.
  /// - Data in `x` have same structure as data in positions_.
  /// - However, gsl maintains its own copy of them during minimization.
  /// @param x  Pointer to working position-components of every particle.
  /// @param p  Pointer to instance of graph.
  /// @param f  Pointer to (output) potential to be minimized.
  /// @param g  Pointer to (output) components of gradient.
  static void fdf(gsl_vector const *x, void *p, double *f, gsl_vector *g);
#endif

  // Minimize potential via simplex method not requiring forces.
  // - This is called by minimize().
  /// @param positions  3xN matrix for position of each of N nodes.
  void minimize_nm_simplex(Matrix3Xd &positions);

  // Minimize potential via gradient-method requiring forces.
  // - This is called by minimize().
  /// @param positions  3xN matrix for position of each of N nodes.
  void minimize_gradient(Matrix3Xd &positions);

  Vector3d attraction(double k, Vector3d const &u, double r);

  /// Calculate inverse-square-distance repulsive force between felt by one
  /// node from other, and increment global potential.
  /// @param u  Unit-vector from one node to other.
  /// @param r  Distance between nodes.
  /// @return  Force felt by one node.
  Vector3d repulsion(Vector3d const &u, double r);

  /// Calculate spring-attraction force felt by Node i from Node j because of
  /// directed graph-edge between them.
  /// @param i  Offset of one node.
  /// @param j  Offset of other node.
  /// @param u  Unit-vector from Node i toward Node j.
  /// @param r  Distance between Node i and Node j.
  /// @return  Force felt by Node i.
  Vector3d edge_attraction(int i, int j, Vector3d const &u, double r);

  /// Calculate spring-attraction force felt by Node i from Node j either
  /// because (i+j)%M==f, where f is factor of modulus M, or because
  /// (i+j)%M==M-f.
  /// @param i  Offset of one node.
  /// @param j  Offset of other node.
  /// @param u  Unit-vector from Node i toward Node j.
  /// @param r  Distance between Node i and Node j.
  /// @return  Force felt by Node i.
  Vector3d sum_attraction(int i, int j, Vector3d const &u, double r);

  Vector3d all_attraction(int i, int j, Vector3d const &u, double r);

public:
  /// Initialize reference to relationships among nodes.
  /// @param nodes  Relationships among nodes.
  minimizer(std::vector<node> const &nodes): nodes_(nodes) {}

  /// Copy initial `positions` into gsl; drive gsl's minimizer; and
  /// then copy final values from gsl back into `positions`.
  /// @param positions  3xN matrix for position of each of N nodes.
  void go(Matrix3Xd &positions);

  /// Scale of attraction between pair of nodes connected by a directed edge.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  /// @return  Scale of attraction between pair of nodes connected by
  ///          a directed edge.
  double edge_attract() const { return edge_attract_; }

  /// Scale of attraction between pair of nodes whose sum is modulus or half of
  /// modulus.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  /// @return  Scale of attraction between pair of nodes whose sum is modulus
  ///          or half of modulus.
  double sum_attract() const { return sum_attract_; }

  /// Scale of attraction between (a) each of zero and one and (b) every other
  /// node.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  /// @return  Scale of attraction between (a) each of zero and one and (b)
  ///          every other node.
  double all_attract() const { return all_attract_; }
};


} // namespace modgraph

// EOF