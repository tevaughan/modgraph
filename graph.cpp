
/// @file       graph.cpp
/// @brief      Definition of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include "asy-adapter.hpp" // asy_adapter and descendants
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream

namespace modgraph {


using Eigen::MatrixXd;


/// File-name for modulus `m`.
/// @param m  Modulus.
/// @return  Name of asymptote-file.
auto filename(int m) {
  std::ostringstream oss;
  oss << m << ".asy";
  return oss.str();
}


void graph::write_asy() const {
  std::ofstream ofs(filename(modulus));
  ofs << header() << perspective({0, -2.0 * biggest_radius(), 0});
  for(int i= 0; i < modulus; ++i) {
    auto const &ip= minimizer_.positions().col(i); // Position of Node i.
    ofs << sphere(ip) << label(i, ip);
    int const j= next(i);
    if(i != j) {
      auto const &jp= minimizer_.positions().col(j); // Position of Node j.
      // ij_q is one-quarter of displacement from Node i toward Node j.
      auto const ij_q= (jp - ip).normalized() * 0.25;
      ofs << arrow(ip + ij_q, jp - ij_q);
    }
  }
}


graph::graph(int m): modulus(m), minimizer_(*this) {
  if(m < 0) throw "illegal modulus";
  minimizer_.go(); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
