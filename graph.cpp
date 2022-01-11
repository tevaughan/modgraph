
/// @file       graph.cpp
/// @brief      Definition of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream
#include <iostream> // cout, endl

namespace modgraph {


using std::cout;
using std::endl;


void graph::write_asy() const {
  int const m= nodes_.size();
  using namespace std;
  ostringstream oss;
  oss << m << ".asy";
  ofstream ofs(oss.str());
  double const ycam= -pow(minimizer_.all_attract() * nodes_.size(), 1.0 / 3.0);
  ofs << "settings.outformat = \"pdf\";\n"
      << "settings.prc = false;\n"
      << "unitsize(" << 1 << "cm);\n"
      << "import three;\n"
      << "currentprojection = perspective(0," << ycam << ",0);\n";
  for(int i= 0; i < m; ++i) {
    auto const &ap= positions_.col(i);
    ofs << "draw(shift"
        << "(" << ap[0] << "," << ap[1] << "," << ap[2] << ")"
        << "*scale3(0.25)*unitsphere, white+opacity(0.5));\n";
    // Billboard or Embedded
    ofs << "label(\"" << i << "\","
        << "(" << ap[0] << "," << ap[1] << "," << ap[2] << ")"
        << ",black,Billboard);\n";
    auto const &an= nodes_[i];
    int const j= an.next;
    if(i != j) {
      auto const &bp= positions_.col(j);
      auto const ab_u= (bp - ap).normalized() * 0.25;
      auto const ab= ap + ab_u;
      auto const ba= bp - ab_u;
      ofs << "draw("
          << "(" << ab[0] << "," << ab[1] << "," << ab[2] << ")"
          << "--"
          << "(" << ba[0] << "," << ba[1] << "," << ba[2] << ")"
          << ",arrow=Arrow3()"
          << ",p=gray(0.6)"
          << ",light=currentlight);\n";
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
  cout << "initializing factors" << endl;
  connect(); // Establish all interconnections among nodes.
  minimizer_.go(positions_); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
