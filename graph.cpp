
/// @file       graph.cpp
/// @copyright  2021 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream
#include <iostream> // cout, endl

namespace modgraph {


void graph::write_asy() const {
  int const m= nodes_.size();
  using namespace std;
  ostringstream oss;
  oss << m << ".asy";
  ofstream ofs(oss.str());
  ofs << "settings.outformat = \"pdf\";\n"
      << "settings.prc = false;\n"
      << "unitsize(" << 1 << "cm);\n"
      << "import three;\n"
      << "currentprojection = perspective(1,-2,1);\n";
  for(int i= 0; i < m; ++i) {
    auto const &a= nodes_[i];
    ofs << "label(\"" << i << "\","
        << "(" << a.pos[0] << "," << a.pos[1] << "," << a.pos[2] << ")"
        << ",red,Billboard);\n";
    int const j= a.next;
    if(i != j) {
      auto const &b= nodes_[j];
      auto const ab_u= (b.pos - a.pos).normalized() * 0.25;
      auto const ab= a.pos + ab_u;
      auto const ba= b.pos - ab_u;
      ofs << "draw("
          << "(" << ab[0] << "," << ab[1] << "," << ab[2] << ")"
          << "--"
          << "(" << ba[0] << "," << ba[1] << "," << ba[2] << ")"
          << ",arrow=Arrow3()"
          << ",p=gray(0.6)"
          << ",light=currentlight);\n";
    }
    for(int k= 0; k < m; ++k) {
      if(i == k) continue;
      int const s= (i + k) % m;
      char const *color= "";
      unsigned const fs= factors_.size();
      bool const f= (fs > 0 && s == factors_[fs - 1] ? true : false);
      if(s == 0) {
        color= "blue";
      } else if(f) {
        color= "lightgray";
      } else {
        continue;
      }
      auto const &c= nodes_[k];
      auto const ac_u= (c.pos - a.pos).normalized() * 0.25;
      auto const ac= a.pos + ac_u;
      auto const ca= c.pos - ac_u;
      ofs << "draw("
          << "(" << ac[0] << "," << ac[1] << "," << ac[2] << ")"
          << "--"
          << "(" << ca[0] << "," << ca[1] << "," << ca[2] << ")"
          << "," << color << ");\n";
    }
  }
}


void graph::connect() {
  int const m= nodes_.size();
  for(int cur_off= 0; cur_off < m; ++cur_off) {
    int const nxt_off= (cur_off * cur_off) % m; // offset of next
    nodes_[cur_off].next= nxt_off;
    nodes_[nxt_off].prev.push_back(cur_off);
    int const complement= m - cur_off;
    if(cur_off <= complement && complement != m) {
      nodes_[cur_off].complement= complement;
    } else {
      nodes_[cur_off].complement= -1;
    }
  }
}


void graph::init_loc() {
  int const m= nodes_.size();
  for(int i= 0; i < m; ++i) {
    constexpr double u= 1.0 / RAND_MAX;
    nodes_[i].pos[0]= m * (rand() * u - 0.5);
    nodes_[i].pos[1]= m * (rand() * u - 0.5);
    nodes_[i].pos[2]= m * (rand() * u - 0.5);
  }
}


using Eigen::Vector3d;


Vector3d graph::force(int i) const {
  int const m= nodes_.size();
  Vector3d nf= Vector3d::Zero(); // Net force of node at offset i.
  auto const &ni= nodes_[i];
  for(int j= 0; j < m; ++j) {
    if(i == j) continue;
    auto const &nj= nodes_[j];
    Vector3d const d= nj.pos - ni.pos; // Displacement from i to j.
    double const r= d.norm(); // Distance between i and j.
    double const r2= r * r;
    Vector3d const u= d / r; // Unit-vector from i toward j.
    // a, b, c, and e are attractive forces.
    double const sma= r / sum_modulus_attract_;
    double const a= (i + j == m ? sma * sma : 0.0);
    double const sfa= r / sum_factor_attract_;
    double b= 0;
#if 1
    unsigned const fs= factors_.size();
    if(fs > 0) { b= (((i + j) % m) == factors_[fs - 1] ? sfa * sfa : 0.0); }
#endif
    double const da= r / direct_attract_;
    double const c= (ni.next == j || nj.next == i ? da * da : 0.0);
    double const ua= r / univ_attract_;
    double const e= ua * ua;
    nf+= u * (a + b + c + e - 1.0 / r2);
  }
  return nf;
}


void graph::arrange_3d() {
  init_loc();
  double max_norm= 0.0;
  constexpr double NORM_LIM= 0.001;
  int off= 0;
  int const m= nodes_.size();
  while(true) {
    Vector3d const f= force(off);
    double const norm= f.norm();
    if(norm > max_norm) max_norm= norm;
    // Guarantee that motion will be no larger than NORM_LIM.
    double const scale= (norm <= NORM_LIM ? 1.0 : NORM_LIM / norm);
    // Move only node on each iteration.
    nodes_[off].pos+= f * scale;
    ++off;
    if(off >= m) {
      off= 0; // Loop around back to beginning of node-list.
      std::cout << "max_norm=" << max_norm << std::endl;
      if(max_norm < 10.0 * NORM_LIM) break;
      max_norm= 0.0;
    }
  }
}


void graph::init_factors(int m) {
  int const u= m / 2;
  for(int i= 2; i <= u; ++i) {
    if(m == m / i * i) { factors_.push_back(i); }
  }
}


graph::graph(int m): nodes_(m) {
  if(m < 0) throw "illegal modulus";
  if(univ_attract_ <= 1.0) throw "illegal universal attraction";
  init_factors(m); // Find factors of m.
  connect(); // Establish all interconnections among nodes.
  arrange_3d(); // Find symmetric positions in 3-d.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
