
/// @file       graph.cpp
/// @brief      Definition of modgraph::graph.
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.

#include "graph.hpp"
#include <cstdlib> // rand(), RAND_MAX
#include <fstream> // ofstream
#include <functional> // bind
#include <iostream> // cout, endl

namespace modgraph {


using Eigen::Map;
using Eigen::Vector3d;
using Eigen::VectorXd;
using std::cerr;
using std::cout;
using std::endl;


Vector3d graph::force_and_pot(unsigned i, unsigned j, Matrix3Xd const &pos) {
  Vector3d f= Vector3d::Zero();
  if(i == j) return f;
  auto const d= pos.col(j) - pos.col(i);
  double const r= d.norm();
  // Unit-vector from Node i toward Node j.
  auto const u= d / r;
  // Repulsion by inverse-square law.
  {
    f+= -u / (r * r);
    potential_+= 1.0 / r;
  }
  // Attraction along edge by spring-law.
  unsigned const inext= nodes_[i].next;
  unsigned const jnext= nodes_[j].next;
  if(inext == j || jnext == i) {
    f+= u * (r / direct_attract_);
    potential_+= 0.5 * r * r / direct_attract_;
  }
  unsigned const M= nodes_.size(); // Modulus.
  // Attraction of complements by spring-law.
  unsigned const sr= (i + j) % M;
  if(sr == 0 || sr == M - sr) {
    f+= u * (r / sum_attract_);
    potential_+= 0.5 * r * r / sum_attract_;
  }
  // Attraction of each node to zero by spring-law.
  if(i == 0 || j == 0) {
    f+= u * (r / univ_attract_);
    potential_+= 0.5 * r * r / univ_attract_;
  }
  return f;
}


void graph::net_force_and_pot(Matrix3Xd const &positions) {
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


double graph::f(gsl_vector const *x, void *p) {
  auto const positions= pos_map(x);
  auto &g= *(graph *)p;
  g.net_force_and_pot(positions);
  return g.potential_;
}


#ifndef NM_SIMPLEX
constexpr double h= 0.001;


void graph::df(gsl_vector const *x, void *p, gsl_vector *grd) {
  f(x, p); // Update not just potential but also forces at x.
  auto &grph= *(graph *)p;
  for(unsigned i= 0; i < grd->size; ++i) {
    // Don't fully understand why negative sign is needed here. Maybe has to do
    // with force's being the negative of the gradient of the potential.
    gsl_vector_set(grd, i, -grph.net_forces_(i, 0));
  }
}


void graph::fdf(gsl_vector const *x, void *p, double *pot, gsl_vector *grd) {
  *pot= f(x, p);
  auto &grph= *(graph *)p;
  for(unsigned i= 0; i < grd->size; ++i) {
    // Don't fully understand why negative sign is needed here. Maybe has to do
    // with force's being the negative of the gradient of the potential.
    gsl_vector_set(grd, i, -grph.net_forces_(i, 0));
  }
}
#endif


void graph::minimize_nm_simplex() {
  constexpr int MAX_ITER= 1000000;
  unsigned const NUM_NODES= positions_.cols();
  unsigned const GSL_SIZE= 3 * NUM_NODES;

  /* Starting point */
  gsl_vector_view init= gsl_vector_view_array(&positions_(0, 0), GSL_SIZE);
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

  positions_= pos_map(s->x);
  gsl_multimin_fminimizer_free(s);
}


void graph::minimize_gradient() {
  constexpr int MAX_ITER= 1000000;
  unsigned const NUM_NODES= positions_.cols();
  unsigned const GSL_SIZE= 3 * NUM_NODES;

  /* Starting point */
  gsl_vector_view init= gsl_vector_view_array(&positions_(0, 0), GSL_SIZE);
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
    status= gsl_multimin_test_gradient(s->gradient, 1.0E-03);
    if(status == GSL_SUCCESS) { printf("converged to minimum at\n"); }
    printf("%5d f()=%7.3f\n", iter, s->f);
  } while(status == GSL_CONTINUE && iter < MAX_ITER);

  positions_= pos_map(s->x);
  gsl_multimin_fdfminimizer_free(s);
}


void graph::minimize() {
#ifdef NM_SIMPLEX
  minimize_nm_simplex();
#else
  minimize_gradient();
#endif
}


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
      << "currentprojection = perspective(0,-2,0);\n";
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
    int const complement= m - cur_off;
    if(cur_off <= complement && complement != m) {
      nodes_[cur_off].complement= complement;
    } else {
      nodes_[cur_off].complement= -1;
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


bool is_prime(int n) {
  if(n < 2) return false;
  for(int i= 2; i < n; ++i) {
    if(n % i == 0) return false;
  }
  return true;
}


graph::graph(int m): positions_(init_loc(m)), nodes_(m) {
  if(m < 0) throw "illegal modulus";
  if(univ_attract_ <= 1.0) throw "illegal universal attraction";
  cout << "initializing factors" << endl;
  connect(); // Establish all interconnections among nodes.
  minimize(); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
