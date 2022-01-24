
/// @file       graph.hpp
/// @brief      Declaration of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#pragma once

#include "minimizer.hpp" // minimizer

namespace modgraph {


/// Three-dimenional position for each node in directed graph of squares under
/// modular arithmetic.
class graph {
  /// Write text-file for asymptote.
  void write_asy() const;

  /// Largest distance of any node from origin.
  /// @return  Largest distance of any node from origin.
  double biggest_radius() const {
    return minimizer_.positions().colwise().norm().maxCoeff();
  }

public:
  /// Construct graphs for modulus m.
  /// @param m  Modulus of graphs.
  graph(int m);

  int const modulus; ///< Modulus for graph of squares.

  /// Number of node pointed to by Node `i`.
  /// @param i  Number of node.
  /// @return  Number of node pointed to by Node `i`.
  int next(int i) const { return (i * i) % modulus; }

private:
  minimizer minimizer_; ///< Facility for force-minimization via GSL.
};


} // namespace modgraph

// EOF
