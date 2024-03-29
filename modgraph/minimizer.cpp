/// @file       minimizer.cpp
/// @brief      Definition of modgraph::minimizer.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#if 0
#define NM_SIMPLEX
#endif

#include "minimizer.hpp"
#include "graph.hpp"

using Eigen::Matrix3Xd;
using Eigen::MatrixXd;
using Eigen::Vector3d;
using std::vector;


namespace modgraph {


/// Calculate factors of `m`.
/// - Include 0, which represents `m` in modular arithmetic.
/// - Do not include 1 as a factor.
/// @param m  Positive integer whose factors are calculated.
/// @return  List of nontrivial factors.
vector<int> calculate_factors(int m) {
  vector<int> f({0});
  for(int i= 2; i <= m / 2; ++i) {
    if((m % i) == 0) f.push_back(i);
  }
  return f;
}


Vector3d minimizer::attract(double k, node_pair const &np) {
  potential_+= 0.5 * k * np.r() * np.r();
  return np.u() * k * np.r();
}


Vector3d minimizer::repel(node_pair const &np) {
  potential_+= 1.0 / np.r();
  return -np.u() / (np.r() * np.r());
}


Vector3d minimizer::edge_attract(node_pair const &np) {
  auto const &i= np.i();
  auto const &j= np.j();
  if(graph_.next(i) == j || graph_.next(j) == i) {
    return attract(1.0 / edge_attract_, np);
  }
  return Vector3d::Zero();
}


Vector3d minimizer::sum_attract(node_pair const &np) {
  Vector3d f= Vector3d::Zero();
  int const m= graph_.modulus;
  int const sum= (np.i() + np.j()) % m;
  static vector<int> const factors= calculate_factors(m);
  double const c= 1.0L / sum_attract_;
  double const b= c / m;
  for(int n: factors) {
    double const a= n * b;
    // If sum of i and j be factor of m, then attract i toward j. Attraction is
    // usually proportional to factor but proportional to m if i or j be zero.
    if(sum == n) f+= attract((n == 0 ? c : a), np);
    // If sum of i and j be less than m by factor of m, then attract i toward
    // j. Attraction is proportional to factor.
    if(m - sum == n) f+= attract(a, np);
  }
  return f;
}


Vector3d minimizer::factor_attract(node_pair const &np) {
  Vector3d f= Vector3d::Zero();
  int const m= graph_.modulus;
  static vector<int> const factors= calculate_factors(m);
  double const c= 1.0L / factor_attract_;
  double const b= c / m;
  auto const &i= np.i();
  auto const &j= np.j();
  for(int n: factors) {
    double const a= n * b;
    // If either i or j be factor of m, then attract i toward j. Attraction is
    // usually proportional to factor but proportional to m if i or j be zero.
    if(i == n || j == n) f+= attract((n == 0 ? c : a), np);
    // If either i or j be less than m by factor of m, then attract i toward j.
    // Attraction is proportional to factor.
    if(i == m - n || j == m - n) f+= attract(a, np);
  }
  return f;
}


Vector3d minimizer::force_and_pot(node_pair const &np) {
  Vector3d f= Vector3d::Zero();
  f+= repel(np); // Repel by inverse-square law.
  f+= edge_attract(np); // Attract along graph-edge by spring.
  f+= sum_attract(np); // Attract because of sum of i and j.
  f+= factor_attract(np); // Attract 0 to 1 and vice-versa.
  return f;
}


void minimizer::net_force_and_pot(Matrix3Xd const &pos) {
  int const m= graph_.modulus;
  forces_= Eigen::MatrixXd::Zero(3 * m, m);
  potential_= 0.0;
  for(int i= 0; i < m; ++i) {
    for(int j= i + 1; j < m; ++j) {
      Vector3d const f= force_and_pot({i, j, pos.col(j) - pos.col(i)});
      forces_.block(i * 3, j, 3, 1)= f;
      forces_.block(j * 3, i, 3, 1)= -f;
    }
  }
  // OK to call force(unsigned) after this point.
  net_forces_= forces_.rowwise().sum();
}


void minimizer::go() {
#ifdef NM_SIMPLEX
  minimize_nm_simplex(positions_);
#else
  minimize_gradient(positions_);
#endif
}


MatrixXd minimizer::init_loc(unsigned m) {
  MatrixXd r(3, m); // Return-value.
  for(unsigned i= 0; i < m; ++i) {
    constexpr double u= 1.0 / RAND_MAX;
    r(0, i)= m * (rand() * u - 0.5);
    r(1, i)= m * (rand() * u - 0.5);
    r(2, i)= m * (rand() * u - 0.5);
  }
  return r;
}


minimizer::minimizer(graph &g): positions_(init_loc(g.modulus)), graph_(g) {}


} // namespace modgraph

// EOF
