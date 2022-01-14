
/// @file       minimizer.cpp
/// @brief      Definition of modgraph::minimizer.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#include "minimizer.hpp"
#include <iostream> // cerr, cout, endl

namespace modgraph {


using Eigen::Map;
using Eigen::Vector3d;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;


/// Calculate factors of `m`.
/// - Include 0, which represents `m` in modular arithmetic.
/// @param m  Positive integer whose factors are calculated.
/// @return  List of nontrivial factors.
vector<int> calculate_factors(int m) {
  vector<int> f({0, 1});
  for(int i= 2; i <= m / 2; ++i) {
    if((m % i) == 0) f.push_back(i);
  }
  return f;
}


Vector3d minimizer::attract(double k, Vector3d const &u, double r) {
  potential_+= 0.5 * k * r * r;
  return u * k * r;
}


Vector3d minimizer::repulsion(Vector3d const &u, double r) {
  potential_+= 1.0 / r;
  return -u / (r * r);
}


Vector3d minimizer::edge_attract(int i, int j, Vector3d const &u, double r) {
  if(nodes_[i].next == j || nodes_[j].next == i) {
    return attract(1.0 / edge_attract_, u, r);
  }
  return Vector3d::Zero();
}


Vector3d minimizer::sum_attract(int i, int j, Vector3d const &u, double r) {
  Vector3d f= Vector3d::Zero();
  int const m= nodes_.size(); // Modulus.
  int const sum= (i + j) % m;
  static vector<int> const factors= calculate_factors(m);
  double const c= 1.0 / sum_attract_;
  double const b= c / m;
  for(int n: factors) {
    double const a= n * b;
    // If sum of i and j be factor of m, then attract i toward j. Attraction is
    // usually proportional to factor but proportional to m if i or j be zero.
    if(sum == n) f+= attract((n == 0 ? c : a), u, r);
    // If sum of i and j be less than m by factor of m, then attract i toward
    // j. Attraction is proportional to factor.
    if(m - sum == n) f+= attract(a, u, r);
  }
  return f;
}


Vector3d minimizer::factor_attract(int i, int j, Vector3d const &u, double r) {
  Vector3d f= Vector3d::Zero();
  int const m= nodes_.size(); // Modulus.
  static vector<int> const factors= calculate_factors(m);
  double const c= 1.0 / factor_attract_;
  double const b= c / m;
  for(int n: factors) {
    double const a= n * b;
    // If either i or j be factor of m, then attract i toward j. Attraction is
    // usually proportional to factor but proportional to m if i or j be zero.
    if(i == n || j == n) f+= attract((n == 0 ? c : a), u, r);
    // If either i or j be less than m by factor of m, then attract i toward j.
    // Attraction is proportional to factor.
    if(i == m - n || j == m - n) f+= attract(a, u, r);
  }
  return f;
}


Vector3d minimizer::force_and_pot(
    unsigned i, unsigned j, Matrix3Xd const &pos) {
  Vector3d f= Vector3d::Zero();
  if(i == j) return f;
  auto const d= pos.col(j) - pos.col(i);
  double const r= d.norm();
  // Unit-vector from Node i toward Node j.
  auto const u= d / r;
  f+= repulsion(u, r); // Repulsion by inverse-square law.
  f+= edge_attract(i, j, u, r); // Attraction along graph-edge by spring.
  f+= sum_attract(i, j, u, r); // Attraction because of sum of i and j.
  f+= factor_attract(i, j, u, r); // Attraction to 0 and 1.
  return f;
}


void minimizer::net_force_and_pot(Matrix3Xd const &positions) {
  unsigned const M= nodes_.size(); // Modulus.
  forces_= MatrixXd::Zero(3 * M, M);
  potential_= 0.0;
  for(unsigned i= 0; i < M; ++i) {
    for(unsigned j= i + 1; j < M; ++j) {
      Vector3d const f= force_and_pot(i, j, positions);
      forces_.block(i * 3, j, 3, 1)= f;
      forces_.block(j * 3, i, 3, 1)= -f;
    }
  }
  // OK to call force(unsigned) after this point.
  net_forces_= forces_.rowwise().sum();
}


/// Map gsl_vector of positions into 3xN-matrix.
/// - `pos_map` is used by f_min on each iteration of minimization.
/// - `pos_map` is used also by minimize() to copy result back into graph.
/// @param x  Pointer to gsl_vector that contains coordinates for N nodes.
/// @return   Map that presents data as 3xN-matrix.
Map<MatrixXd const> pos_map(gsl_vector const *x) {
  if(x->size != x->size / 3 * 3) {
    cerr << "pos_Map: ERROR: size not multiple of three" << endl;
    throw "invalid size";
  }
  return Map<MatrixXd const>(x->data, 3, x->size / 3);
}


double minimizer::f(gsl_vector const *x, void *p) {
  auto const positions= pos_map(x);
  auto &g= *(minimizer *)p;
  g.net_force_and_pot(positions);
  return g.potential_;
}


#ifndef NM_SIMPLEX
constexpr double h= 0.001;


void minimizer::df(gsl_vector const *x, void *p, gsl_vector *grd) {
  f(x, p); // Update not just potential but also forces at x.
  auto &min= *(minimizer *)p;
  for(unsigned i= 0; i < grd->size; ++i) {
    // Don't fully understand why negative sign is needed here. Maybe has to do
    // with force's being the negative of the gradient of the potential.
    gsl_vector_set(grd, i, -min.net_forces_(i, 0));
  }
}


void minimizer::fdf(
    gsl_vector const *x, void *p, double *pot, gsl_vector *grd) {
  *pot= f(x, p);
  auto &min= *(minimizer *)p;
  for(unsigned i= 0; i < grd->size; ++i) {
    // Don't fully understand why negative sign is needed here. Maybe has to do
    // with force's being the negative of the gradient of the potential.
    gsl_vector_set(grd, i, -min.net_forces_(i, 0));
  }
}
#endif


#ifdef NM_SIMPLEX
void minimizer::minimize_nm_simplex(Matrix3Xd &positions) {
  constexpr int MAX_ITER= 1000000;
  unsigned const NUM_NODES= positions.cols();
  unsigned const GSL_SIZE= 3 * NUM_NODES;

  /* Starting point */
  gsl_vector_view init= gsl_vector_view_array(&positions(0, 0), GSL_SIZE);
  gsl_vector *x= (gsl_vector *)&init;
  cout << "x->size=" << x->size << endl;

  /* Set initial step-sizes. */
  gsl_vector *ss= gsl_vector_alloc(GSL_SIZE);
  gsl_vector_set_all(ss, 10.0);

  /* Initialize method and iterate */
  gsl_multimin_function minex_func;
  minex_func.n= GSL_SIZE;
  minex_func.f= f;
  minex_func.params= this;

  gsl_multimin_fminimizer_type const *T= gsl_multimin_fminimizer_nmsimplex2;
  gsl_multimin_fminimizer *s= gsl_multimin_fminimizer_alloc(T, GSL_SIZE);
  gsl_multimin_fminimizer_set(s, &minex_func, x, ss);

  int status= GSL_CONTINUE;
  int iter= 0;
  do {
    ++iter;
    status= gsl_multimin_fminimizer_iterate(s);
    if(status) {
      if(status == GSL_ENOPROG) {
        cerr << "GSL_ENOPROG returned from gsl_multimin_*iterate()" << endl;
      } else {
        cerr << "gsl_multimin_*iterate() returned " << status << endl;
      }
      break;
    }
    double const size= gsl_multimin_fminimizer_size(s);
    status= gsl_multimin_test_size(size, 0.1);
    if(status == GSL_SUCCESS) { printf("converged to minimum at\n"); }
    printf("%5d f()=%7.3f size=%.3f\n", iter, s->fval, size);
  } while(status == GSL_CONTINUE && iter < MAX_ITER);

  positions= pos_map(s->x);
  gsl_multimin_fminimizer_free(s);
}
#else
void minimizer::minimize_gradient(Matrix3Xd &positions) {
  constexpr int MAX_ITER= 1000000;
  unsigned const NUM_NODES= positions.cols();
  unsigned const GSL_SIZE= 3 * NUM_NODES;

  /* Starting point */
  gsl_vector_view init= gsl_vector_view_array(&positions(0, 0), GSL_SIZE);
  gsl_vector *x= (gsl_vector *)&init;
  cout << "x->size=" << x->size << endl;

  gsl_multimin_function_fdf minex_func;
  minex_func.n= GSL_SIZE;
  minex_func.f= f;
  minex_func.df= df;
  minex_func.fdf= fdf;
  minex_func.params= this;

  gsl_multimin_fdfminimizer_type const *T=
      gsl_multimin_fdfminimizer_conjugate_fr;
  gsl_multimin_fdfminimizer *s= gsl_multimin_fdfminimizer_alloc(T, GSL_SIZE);
  gsl_multimin_fdfminimizer_set(s, &minex_func, x, 1.0, 0.1);

  int status= GSL_CONTINUE;
  int iter= 0;
  do {
    ++iter;
    status= gsl_multimin_fdfminimizer_iterate(s);
    if(status) {
      if(status == GSL_ENOPROG) {
        cerr << "GSL_ENOPROG returned from gsl_multimin_*iterate()" << endl;
      } else {
        cerr << "gsl_multimin_*iterate() returned " << status << endl;
      }
      break;
    }
    status= gsl_multimin_test_gradient(s->gradient, 1.0E-04);
    if(status == GSL_SUCCESS) { printf("converged to minimum at\n"); }
    printf("%5d f()=%8.4f\n", iter, s->f);
  } while(status == GSL_CONTINUE && iter < MAX_ITER);

  positions= pos_map(s->x);
  gsl_multimin_fdfminimizer_free(s);
}
#endif


void minimizer::go(Matrix3Xd &positions) {
#ifdef NM_SIMPLEX
  minimize_nm_simplex(positions);
#else
  minimize_gradient(positions);
#endif
}

} // namespace modgraph

// EOF
