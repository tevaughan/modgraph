
/// @file       graph.cpp
/// @brief      Definition of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include "asy-adapter.hpp" // asy_adapter and descendants
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream
#include <iostream> // cout, endl

namespace modgraph {


/// File-name for modulus `m`.
/// @param m  Modulus.
/// @return  Name of asymptote-file.
std::string filename(int m) {
  std::ostringstream oss;
  oss << m << ".asy";
  return oss.str();
}


/// Cube-root of number `n`.
/// @param n  Number whose cube-root is to be found.
/// @return  Cube-root of `n`.
double cube_root(double n) {
  constexpr double e= 1.0 / 3.0; // Exponent for cube-root.
  return pow(n, e);
}


void graph::write_asy() const {
  int const m= nodes_.size();
  std::ofstream ofs(filename(m));
  ofs << header();
  double const ycam= -cube_root(minimizer_.all_attract() * m);
  ofs << perspective({0, ycam, 0});
  for(int i= 0; i < m; ++i) {
    auto const &ip= positions_.col(i); // Position of Node i.
    ofs << sphere(ip) << label(i, ip);
    int const j= nodes_[i].next;
    if(i != j) {
      auto const &jp= positions_.col(j); // Position of Node j.
      // ij_q is one-quarter of displacement from Node i toward Node j.
      auto const ij_q= (jp - ip).normalized() * 0.25;
      ofs << arrow(ip + ij_q, jp - ij_q);
    }
  }
}


void graph::connect() {
  int const m= nodes_.size();
  for(int cur_off= 0; cur_off < m; ++cur_off) {
    int const nxt_off= (cur_off * cur_off) % m; // offset of next
    nodes_[cur_off].next= nxt_off;
    nodes_[nxt_off].prev.push_back(cur_off);
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


graph::graph(int m): positions_(init_loc(m)), nodes_(m), minimizer_(nodes_) {
  if(m < 0) throw "illegal modulus";
  std::cout << "initializing factors" << std::endl;
  connect(); // Establish all interconnections among nodes.
  minimizer_.go(positions_); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
