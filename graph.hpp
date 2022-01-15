/// @file       graph.hpp
/// @brief      Declaration of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include "minimizer.hpp" // minimizer

namespace modgraph {


/// Three-dimenional position for each node in directed graph of squares under
/// modular arithmetic.
class graph {
  /// 3xN matrix for position of each of N particles.
  /// - Only initial (random) and final values are stored here.
  /// - Initial values are copied into gsl before minimization.
  /// - Final values are copied from gsl back here after minimization.
  Eigen::Matrix3Xd positions_;

  int modulus_; ///< Modulus for graph of squares.
  minimizer minimizer_; ///< Facility for force-minimization via GSL.

  /// Write text-file for asymptote.
  void write_asy() const;

  /// Generate random locations for initialization of positions_.
  /// @param n  Number of locations.
  /// @return   Collection of random locations.
  static Eigen::MatrixXd init_loc(unsigned n);

  /// Initialize list of composite factors of modulus.
  /// - It seems best to constrain locations only on basis of composite
  ///   factors, if any exist.
  /// @param m  Modulus.
  void init_factors(int m);

  /// Largest distance of any node from origin.
  /// @return  Largest distance of any node from origin.
  double biggest_radius() const {
    return positions_.colwise().norm().maxCoeff();
  }

public:
  /// Construct graphs for modulus m.
  /// @param m  Modulus of graphs.
  graph(int m);

  int modulus() const { return modulus_; }

  /// Number of node pointed to by Node `i`.
  /// @param i  Number of node.
  /// @return  Number of node pointed to by Node `i`.
  int next(int i) const { return (i * i) % modulus_; }
};


} // namespace modgraph

// EOF
