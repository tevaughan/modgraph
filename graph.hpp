/// @file       graph.hpp
/// @brief      Declaration of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include "minimizer.hpp" // minimizer

namespace modgraph {


class graph {
  /// 3xN matrix for position of each of N particles.
  /// - Only initial (random) and final values are stored here.
  /// - Initial values are copied into gsl before minimization.
  /// - Final values are copied from gsl back here after minimization.
  Matrix3Xd positions_;

  std::vector<node> nodes_; ///< Collection of all nodes in graph.
  minimizer minimizer_; ///< Facility for force-minimization via GSL.

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
