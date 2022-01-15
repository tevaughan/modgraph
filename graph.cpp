
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
  std::ofstream ofs(filename(modulus_));
  ofs << header() << perspective({0, -2.0 * biggest_radius(), 0});
  for(int i= 0; i < modulus_; ++i) {
    auto const &ip= positions_.col(i); // Position of Node i.
    ofs << sphere(ip) << label(i, ip);
    int const j= next(i);
    if(i != j) {
      auto const &jp= positions_.col(j); // Position of Node j.
      // ij_q is one-quarter of displacement from Node i toward Node j.
      auto const ij_q= (jp - ip).normalized() * 0.25;
      ofs << arrow(ip + ij_q, jp - ij_q);
    }
  }
}


MatrixXd graph::init_loc(unsigned m) {
  MatrixXd r(3, m); // Return-value.
  for(unsigned i= 0; i < m; ++i) {
    constexpr double u= 1.0 / RAND_MAX;
    r(0, i)= m * (rand() * u - 0.5);
    r(1, i)= m * (rand() * u - 0.5);
    r(2, i)= m * (rand() * u - 0.5);
  }
  return r;
}


graph::graph(int m): positions_(init_loc(m)), modulus_(m), minimizer_(*this) {
  if(m < 0) throw "illegal modulus";
  minimizer_.go(positions_); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
