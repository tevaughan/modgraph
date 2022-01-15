
/// @file       gsl-funcs.cpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Definition of pos_map(), f(), df(), and fdf().

#include "gsl-funcs.hpp"
#include "minimizer.hpp" // minimizer
#include <iostream> // cerr, endl

using Eigen::Matrix3Xd;
using std::cerr;
using std::cout;
using std::endl;


/// Map gsl_vector of positions into 3xN-matrix.
/// - `pos_map` is used by f() on each GSL-iteration to read positions.
/// - `pos_map` is used by driver to copy positions out of GSL when done.
/// @param x  Pointer to gsl_vector that contains coordinates for N nodes.
/// @return   Map that presents data as 3xN-matrix.
Eigen::Map<Eigen::MatrixXd const> pos_map(gsl_vector const *x) {
  if(x->size != x->size / 3 * 3) {
    std::cerr << "pos_Map: ERROR: size not multiple of three" << std::endl;
    throw "invalid size";
  }
  return Eigen::Map<Eigen::MatrixXd const>(x->data, 3, x->size / 3);
}


extern "C" {
double f(gsl_vector const *x, void *pmin) {
  if(!x || !pmin) throw "null pointer";
  auto const positions= pos_map(x);
  auto &min= *(modgraph::minimizer *)pmin;
  min.net_force_and_pot(positions);
  return min.potential();
}


void fdf(gsl_vector const *x, void *pmin, double *pot, gsl_vector *grd) {
  if(!pot || !grd) throw "null pointer";
  *pot= f(x, pmin);
  auto &min= *(modgraph::minimizer *)pmin;
  for(unsigned i= 0; i < grd->size; ++i) {
    // Force is *negative* gradient of potential.
    gsl_vector_set(grd, i, -min.net_force_component(i));
  }
}


// Code is not set up to calculate the gradient independently from potential.
// That could be future optimization.  At present, df() just calls fdf() and
// ignores potential.
void df(gsl_vector const *x, void *pmin, gsl_vector *grd) {
  double pot; // Ignored result.
  fdf(x, pmin, &pot, grd);
}
}


namespace modgraph {


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


}

// EOF
