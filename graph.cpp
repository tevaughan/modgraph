
/// @file       graph.cpp
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


Vector3d graph::force(unsigned i, unsigned j) const {
  Vector3d f= Vector3d::Zero();
  if(i == j) return f;
  auto const d= positions_.col(j) - positions_.col(i);
  double const r= d.norm();
  // Unit-vector from Node i toward Node j.
  auto const u= d / r;
  // Repulsion by inverse-square law.
  f+= -u / (r * r);
  // Attraction along edge by spring-law.
  unsigned const inext= nodes_[i].next;
  unsigned const jnext= nodes_[j].next;
  if(inext == j || jnext == i) { f+= u * (r / direct_attract_); }
  unsigned const M= nodes_.size(); // Modulus.
  // Attraction of complements by spring-law.
  unsigned const sr= (i + j) % M;
  if(sr == 0 || sr == M - sr) { f+= u * (r / sum_modulus_attract_); }
  // Attraction of each node to zero by spring-law.
  if(i == 0 || j == 0) { f+= u * (r / univ_attract_); }
  return f;
}


void graph::init_forces() {
  unsigned const M= nodes_.size(); // Modulus.
  forces_= MatrixXd::Zero(3 * M, M);
  for(unsigned i= 0; i < M; ++i) {
    for(unsigned j= i + 1; j < M; ++j) {
      Vector3d const f= force(i, j);
      forces_.block(i * 3, j, 3, 1)= f;
      forces_.block(j * 3, i, 3, 1)= -f;
    }
  }
  // OK to call force(unsigned) after this point.
  net_forces_= forces_.rowwise().sum();
  max_force_mag_= 0.0;
  for(unsigned i= 0; i < M; ++i) {
    auto const f= force(i);
    double const mag= f.norm();
    if(mag > max_force_mag_) {
      max_force_mag_= mag;
      max_force_off_= i;
    }
  }
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


double graph::f_min(gsl_vector const *x, void *p) {
  auto const positions= pos_map(x);
  auto const &g= *(graph *)p;
  int const m= g.nodes_.size(); // Modulus.
  double v= 0.0; // Value to be minimized.
  for(int i= 0; i < positions.cols(); ++i) {
    auto const pi= positions.col(i); // Position of ith node.
    Vector3d di= Vector3d::Zero(); // Change in position of ith node.
    for(int j= 0; j < positions.cols(); ++j) {
      if(i == j) continue;
      auto const pj= positions.col(j); // Position of jth node.
      auto const rvec= pj - pi; // Displacement from i to j.
      double const r= rvec.norm(); // Magnitude of displacement.
      auto const u= rvec / r;
      di+= -u / (r * r); // Universal repulsion by 1/r^2.
      if(g.nodes_[i].next == j) {
        di+= u * 0.5; // Attraction along edge.
      }
      if((i + j) % m == 0) {
        di+= u * 0.1; // Attraction to complement.
      }
      if(i == 0 || j == 0) {
        di+= u * 0.02; // Attraction to zero.
      }
    }
    v+= di.norm();
  }
  return v;
}


#ifndef NM_SIMPLEX
constexpr double h= 0.001;


void graph::df(gsl_vector const *x, void *p, gsl_vector *g) {
  double const f0= f_min(x, p); // Current value of quantity to minimize.
  static VectorXd xi(x->size);
  for(unsigned i= 0; i < g->size; ++i) {
    xi= Map<VectorXd>(x->data, x->size); // Copy components.
    xi[i]+= h; // Modify ith component.
    gsl_vector_const_view gxiv= gsl_vector_const_view_array(&xi[0], x->size);
    gsl_vector const *gxi= (gsl_vector const *)&gxiv;
    gsl_vector_set(g, i, (f_min(gxi, p) - f0) / h);
  }
}


void graph::fdf(gsl_vector const *x, void *p, double *f, gsl_vector *g) {
  *f= f_min(x, p);
  static VectorXd xi(x->size);
  for(unsigned i= 0; i < g->size; ++i) {
    xi= Map<VectorXd>(x->data, x->size); // Copy components.
    xi[i]+= h;
    gsl_vector_const_view gxiv= gsl_vector_const_view_array(&xi[0], x->size);
    gsl_vector const *gxi= (gsl_vector const *)&gxiv;
    gsl_vector_set(g, i, (f_min(gxi, p) - *f) / h);
  }
}
#endif


void graph::minimize() {
#if 0
  constexpr int MAX_ITER= 1000000;
  unsigned const NUM_NODES= positions_.cols();
  unsigned const GSL_SIZE= 3 * NUM_NODES;

  /* Starting point */
  gsl_vector_view init= gsl_vector_view_array(&positions_(0, 0), GSL_SIZE);
  gsl_vector *x= (gsl_vector *)&init;
  cout << "x->size=" << x->size << endl;

#ifdef NM_SIMPLEX
  /* Set initial step-sizes. */
  gsl_vector *ss= gsl_vector_alloc(GSL_SIZE);
  gsl_vector_set_all(ss, 10.0);
#endif

  /* Initialize method and iterate */
#ifdef NM_SIMPLEX
  gsl_multimin_function minex_func;
#else
  gsl_multimin_function_fdf minex_func;
#endif
  minex_func.n= GSL_SIZE;
  minex_func.f= f_min;
#ifndef NM_SIMPLEX
  minex_func.df= df;
  minex_func.fdf= fdf;
#endif
  minex_func.params= this;

#ifdef NM_SIMPLEX
  gsl_multimin_fminimizer_type const *T= gsl_multimin_fminimizer_nmsimplex2;
  gsl_multimin_fminimizer *s= gsl_multimin_fminimizer_alloc(T, GSL_SIZE);
  gsl_multimin_fminimizer_set(s, &minex_func, x, ss);
#else
  gsl_multimin_fdfminimizer_type const *T=
      gsl_multimin_fdfminimizer_conjugate_fr;
  gsl_multimin_fdfminimizer *s= gsl_multimin_fdfminimizer_alloc(T, GSL_SIZE);
  gsl_multimin_fdfminimizer_set(s, &minex_func, x, 0.01, 1.0E-04);
#endif

  int status;
  int iter= 0;
  do {
    ++iter;
#ifdef NM_SIMPLEX
    status= gsl_multimin_fminimizer_iterate(s);
#else
    status= gsl_multimin_fdfminimizer_iterate(s);
#endif
    if(status) break;
#ifdef NM_SIMPLEX
    double const size= gsl_multimin_fminimizer_size(s);
    status= gsl_multimin_test_size(size, 0.1);
#else
    status= gsl_multimin_test_gradient(s->gradient, 1.0E-03);
#endif
    if(status == GSL_SUCCESS) { printf("converged to minimum at\n"); }
#ifdef NM_SIMPLEX
    printf("%5d f()=%7.3f size=%.3f\n", iter, s->fval, size);
#else
    printf("%5d f()=%7.3f\n", iter, s->f);
#endif
  } while(status == GSL_CONTINUE && iter < MAX_ITER);

  positions_= pos_map(s->x);
#ifdef NM_SIMPLEX
  gsl_multimin_fminimizer_free(s);
#else
  gsl_multimin_fdfminimizer_free(s);
#endif
#endif

  init_forces();
  constexpr double MAX_STEP= 0.01;
  constexpr double STOPPING_CRITERION= 10.0 * MAX_STEP;
  unsigned const M= nodes_.size();
  while(max_force_mag_ > STOPPING_CRITERION) {
    cout << "max_force_mag_=" << max_force_mag_ << endl;
    double scale= 1.0;
    if(max_force_mag_ > MAX_STEP) { scale= MAX_STEP / max_force_mag_; }
    for(unsigned i= 0; i < M; ++i) {
      positions_.block(0, i, 3, 1)+= scale * force(i);
    }
    init_forces();
  }
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
      << "currentprojection = perspective(1,-2,1);\n";
  for(int i= 0; i < m; ++i) {
    auto const &ap= positions_.col(i);
    ofs << "label(\"" << i << "\","
        << "(" << ap[0] << "," << ap[1] << "," << ap[2] << ")"
        << ",red,Billboard);\n";
#if 1
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
    for(int k= 0; k < m; ++k) {
      if(i == k) continue;
      char const *color= "";
      int const s= (i + k) % m;
      bool flag= false;
#if 0
      for(auto f: factors_) {
        if(s == f || s == m - f) {
          flag= true;
          if(i == 22 && k == 28 && m == 30) cout << "flag=true" << endl;
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
      auto const &cp= positions_.col(k);
      auto const ac_u= (cp - ap).normalized() * 0.25;
      auto const ac= ap + ac_u;
      auto const ca= cp - ac_u;
      ofs << "draw("
          << "(" << ac[0] << "," << ac[1] << "," << ac[2] << ")"
          << "--"
          << "(" << ca[0] << "," << ca[1] << "," << ca[2] << ")"
          << "," << color << ");\n";
    }
#endif
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
  unsigned const N= 3 * m;
  MatrixXd r(3, N); // Return-value.
  for(unsigned i= 0; i < N; ++i) {
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


void graph::init_factors(int m) {
  // Collect only composite factors.
  // - 4 is least possible composite factor.
  // - m/2 is greatest possible composite factor.
  for(int i= 4; i <= m / 2; ++i) {
    if(m % i == 0 && !is_prime(i)) factors_.push_back(i);
  }
}


graph::graph(int m): positions_(init_loc(m)), nodes_(m) {
  if(m < 0) throw "illegal modulus";
  if(univ_attract_ <= 1.0) throw "illegal universal attraction";
  cout << "initializing factors" << endl;
  init_factors(m); // Initialize list of factors of m.
  connect(); // Establish all interconnections among nodes.
  minimize(); // Find final positions.
  write_asy(); // Write text-file for asymptote.
}


} // namespace modgraph

// EOF
