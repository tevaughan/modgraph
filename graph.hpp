
#pragma once

#include "node.hpp" // node
#include <eigen3/Eigen/Dense> // Matrix
#include <gsl/gsl_multimin.h> // gsl_vector_view, gsl_vector_const_view
#include <iostream> // cout, endl
#include <set> // set
#include <vector> // vector

#if 0
#define AD_HOC
#else
#if 0
#define NM_SIMPLEX
#endif
#endif

namespace modgraph {


using Eigen::Matrix3Xd;
using Eigen::MatrixXd;
using Eigen::Vector3d;
using std::cout;
using std::endl;


class graph {
  /// 3xN matrix for position of each of N particles.
  /// - Only initial (random) and final values are stored here.
  /// - Initial values are copied into gsl before minimization.
  /// - Final values are copied from gsl back here after minimization.
  Matrix3Xd positions_;

  /// 3NxN matrix storing force felt by each node from each other node.
  /// - force_ is initialized by init_forces().
  MatrixXd forces_;

  /// 3Nx1 matrix storing net force felt by each node.
  /// - net_forces_ is initialized by init_forces().
  MatrixXd net_forces_;

  double potential_; ///< Scalar potential whose gradient produces forces.
  double max_force_mag_; ///< Magnitude of greatest net force on any node.
  unsigned max_force_off_; ///< Offset of node experiencing maximum force.

  /// Compute force felt by Node i from Node j, and update potential_.
  /// - force() is called by net_force_and_pot().
  /// @param i  Offset of one node.
  /// @param j  Offset of other node.
  /// @return   Force felt by Node i from Node j.
  Vector3d force_and_pot(unsigned i, unsigned j, Matrix3Xd const &positions);

  /// Net force on Node i.
  /// - This should be called only after init_forces() has been called.
  /// @param i  Offset of node.
  /// @return   Net force on node.
  auto force(unsigned i) const { return net_forces_.block(i * 3, 0, 3, 1); }

  /// Compute net force felt by each node from every other node, and compute
  /// overall potential of system.
  /// - `calc_net_force_and_pot()` takes argument (and does not directly use
  ///   positions_) because gsl keeps its own record of positions during
  ///   minimization.
  /// @param positions  3xN matrix for position of each of N particles.
  void net_force_and_pot(Matrix3Xd const &positions);

  /// Function that GSL will minimize.
  /// - Data in `x` have same structure as data in positions_.
  /// - However, gsl maitains its own copy of them during minimization.
  /// @param x  Pointer to working position-components of every particle.
  /// @param p  Pointer to instance of graph.
  /// @return   Quantity to be minimized.
  static double f_min(gsl_vector const *x, void *p);

#ifndef NM_SIMPLEX
  static void df(gsl_vector const *x, void *p, gsl_vector *g);
  static void fdf(gsl_vector const *x, void *p, double *f, gsl_vector *g);
#endif

  void minimize_ad_hoc();
  void minimize_nm_simplex();
  void minimize_steepest_descent();

  /// Copy initial values in positions_ into gsl; drive gsl's minimizer; and
  /// then copy final values from gsl back into positions_.
  void minimize();

  std::vector<node> nodes_; ///< Collection of all nodes in graph.

  /// Scale of universal attraction between every node and every other.
  /// - `univ_attract` should be larger than unity; the larger, the weaker.
  /// - `univ_attract` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double univ_attract_= 50.0;

  /// Scale of attraction between pair of nodes connected by a directed edge.
  /// - `direct_attract_` should be larger than unity.
  /// - `direct_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double direct_attract_= 2.0;

  /// Scale of attraction between pair of nodes whose sum is modulus.
  /// - `sum_modulus_attract_` should be larger than unity.
  /// - `sum_modulus_attract_` is force proportional to distance.
  /// - Scale for forces is set by universal repulsion, which decays with
  ///   inverse-square distance and has unit-value between two nodes whenever
  ///   they be separated by unit distance.
  double sum_modulus_attract_= 10.0;

  void connect(); ///< Establish all interconnections among nodes.
  void write_asy() const; ///< Write text-file for asymptote.

  /// Generate random locations for initialization of positions_.
  /// @param n  Number of locations.
  /// @return   Collection of random locations.
  static MatrixXd init_loc(unsigned n);

  /// Initialize list of composite factors of modulus.
  /// - It seems best to constrain locations only on basis of composite
  ///   factors, if any exist.
  /// @param m  Modulus.
  void init_factors(int m);

public:
  /// Construct graphs for modulus m.
  /// @param m  Modulus of graphs.
  graph(int m);
};


} // namespace modgraph

// EOF
