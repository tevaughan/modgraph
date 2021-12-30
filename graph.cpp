
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
      char const *color= "";
      int const s= (i + k) % m;
      bool flag= false;
#if 0
      for(auto f: factors_) {
        if(s == f || s == m - f) {
          flag= true;
          break;
        }
      }
#endif
      if(s == 0) {
        color= "blue";
      } else if(flag) {
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
  auto const &ni= nodes_[i];
  // Initialize to zero the net force on the node at offset i.
  Vector3d nf= Vector3d::Zero();
  // First add in the attraction of the node to the origin.
  // TODO: The next three lines can be abstracted from here and from the top of
  // the loop; abstracted into a sufficiently general function.
  Vector3d const d0= -ni.pos; // Displacement from i to origin.
  double const r0= d0.norm(); // Distance between i and origin.
  Vector3d const u0= d0 / r0; // Unit-vector from i toward origin.
  double const ua= r0 / univ_attract_;
  nf+= u0 * ua;
  for(int j= 0; j < m; ++j) {
    if(i == j) continue;
    auto const &nj= nodes_[j];
    Vector3d const d= nj.pos - ni.pos; // Displacement from i to j.
    double const r= d.norm(); // Distance between i and j.
    double const r2= r * r;
    Vector3d const u= d / r; // Unit-vector from i toward j.
    // a, b, c, and e are attractive forces.
    double const sma= r / sum_modulus_attract_;
    double const a= (i + j == m ? sma : 0.0);
    double const sfa= r / sum_factor_attract_;
    double b= 0;
#if 1
    int const s= (i + j) % m;
    for(auto f: factors_) {
      if(s == f || s == m - f) { b+= sfa; }
    }
#endif
    double const da= r / direct_attract_;
    double const c= (ni.next == j || nj.next == i ? da : 0.0);
    nf+= u * (a + b + c + -1.0 / r2);
  }
  return nf;
}


void graph::arrange_3d() {
  init_loc();
  constexpr double NORM_LIM= 0.01;
  int const m= nodes_.size();
  while(true) {
    std::vector<Vector3d> fs;
    double max_norm= 0.0;
    for(int off= 0; off < m; ++off) {
      Vector3d const f= force(off);
      fs.push_back(f);
      double const norm= f.norm();
      if(norm > max_norm) max_norm= norm;
    }
    std::cout << "max_norm=" << max_norm << std::endl;
    if(max_norm < 10.0 * NORM_LIM) break;
    // Guarantee that motion will be no larger than NORM_LIM.
    double const scale= (max_norm <= NORM_LIM ? 1.0 : NORM_LIM / max_norm);
    for(int off= 0; off < m; ++off) { nodes_[off].pos+= fs[off] * scale; }
  }
}


bool is_prime(int n) {
  if(n < 2) return false;
  for(int i= 2; i < n; ++i) {
    if(n % i == 0) return false;
  }
  return true;
}


void graph::init_factors(int m) {
  // Collect only composite factors.
  // - 4 is least possible composite factor.
  // - m/2 is greatest possible composite factor.
  for(int i= 4; i <= m / 2; ++i) {
    if(m % i == 0 && !is_prime(i)) factors_.push_back(i);
  }
}


graph::graph(int m): nodes_(m) {
  if(m < 0) throw "illegal modulus";
  if(univ_attract_ <= 1.0) throw "illegal universal attraction";
  init_factors(m); // Initialize list of factors of m.
  sum_factor_attract_*= factors_.size();
  connect(); // Establish all interconnections among nodes.
  arrange_3d(); // Find symmetric positions in 3-d.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
