
/// @file       graph.cpp
/// @copyright  2021 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include <cmath> // sqrt()
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream

namespace modgraph {


void graph::check_node(int n_off, int s_off) {
  node &n= nodes_[n_off];
  if(n.subg == -1) {
    traverse(n_off, s_off);
  } else if(n.subg != s_off) {
    throw "conflict between subgraphs";
  }
}


void graph::traverse(int n_off, int s_off) {
  node &n= nodes_[n_off];
  n.subg= s_off;
  subgraphs_[s_off].insert(n_off);
  // Handle next node.
  int const nxt_off= n.next;
  check_node(nxt_off, s_off);
  // Handle previous nodes.
  for(int const prv_off: n.prev) { check_node(prv_off, s_off); }
}


void graph::write_neato() const {
  unsigned const m= nodes_.size();
  using namespace std;
  for(unsigned s_off= 0; s_off < subgraphs_.size(); ++s_off) {
    ostringstream oss;
    oss << m << "." << s_off << ".neato";
    ofstream ofs(oss.str());
    ofs << "digraph G {\n";
    ofs << "   overlap=scale\n";
    for(int n_off: subgraphs_[s_off]) {
      int const next= nodes_[n_off].next;
      ofs << "   " << n_off << " -> " << next << "\n";
      if(nodes_[n_off].complement > -1) {
        ofs << "   " << n_off << " -> " << nodes_[n_off].complement
            << " [dir=none]\n";
      }
    }
    ofs << "}\n";
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


void graph::partition() {
  int const m= nodes_.size();
  for(int n_off= 0; n_off < m; ++n_off) {
    if(nodes_[n_off].subg == -1) {
      // Need new subgraph.
      int const s_off= subgraphs_.size();
      subgraphs_.push_back(subgraph());
      traverse(n_off, s_off);
    }
  }
}


void graph::init_loc() {
  int const m= nodes_.size();
  for(int i= 0; i < m; ++i) {
    constexpr double u= 1.0 / RAND_MAX;
    nodes_[i].pos[0]= lin_size_ * (rand() * u - 0.5);
    nodes_[i].pos[1]= lin_size_ * (rand() * u - 0.5);
    nodes_[i].pos[2]= lin_size_ * (rand() * u - 0.5);
  }
}


using Eigen::Vector3d;
using std::vector;


vector<Vector3d> graph::forces(Vector3d &max) const {
  max= Vector3d::Zero();
  int const m= nodes_.size();
  vector<Vector3d> r(m); // Return-value.
  for(int i= 0; i < m; ++i) {
    r[i]= Vector3d::Zero();
    for(int j= 0; j < m; ++j) {
      if(i == j) continue;
      Vector3d const d= nodes_[j].pos - nodes_[i].pos;
      double const n= d.norm();
      Vector3d const u= d / n;
      double const s= n / lin_size_; // Scale of general, attractive force.
      double const t= 1.0 / n; // Scale of general, repulsive force.
      double const a= (i + j == m ? 1 : 0); // Specific, attractive force.
      double b= 0.0;
      if(nodes_[i].next == j || nodes_[j].next == i) b= 1.0;
      r[i]+= u * (s * s + a + b - t * t);
    }
  }
  return r;
}


void graph::arrange_3d() {
  init_loc();
}


graph::graph(int m): nodes_(m), lin_size_(sqrt(m)) {
  if(m < 0) throw "illegal modulus";
  connect(); // Establish all interconnections among nodes.
  partition(); // Partition into subgraphs.
  write_neato(); // Write text files.
}


} // namespace modgraph

// EOF
