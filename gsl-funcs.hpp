
/// @file       gsl-funcs.hpp
/// @copyright  2022 Thomas E. Vaughan, all rights reserved.
/// @brief      Declaration of f(), df(), and fdf().
///
/// Each of f(), df(), and fdf() has C-linkage and is passed by
/// function-pointer to one of GSL's minimization-routines.
/// - Each of df() and fdf() is used when derivative is needed.
/// - f() just provides value to minimize.

#include <gsl/gsl_vector.h> // gsl_vector


extern "C" {
/// Potential that GSL will minimize.
/// - Data in `x` have same structure as data in positions_.
/// - However, gsl maintains its own copy of them during minimization.
/// @param x  Pointer to working position-components of every particle.
/// @param p  Pointer to instance of class minimizer.
/// @return   Potential to be minimized.
double f(gsl_vector const *x, void *p);


/// Calculate gradient of potential.
/// - Data in `x` have same structure as data in positions_.
/// - However, gsl maintains its own copy of them during minimization.
/// @param x  Pointer to working position-components of every particle.
/// @param p  Pointer to instance of class minimizer.
/// @param g  Pointer to (output) components of gradient.
void df(gsl_vector const *x, void *p, gsl_vector *g);


/// Calculate potential to be minimized and gradient of potential.
/// - Data in `x` have same structure as data in positions_.
/// - However, gsl maintains its own copy of them during minimization.
/// @param x  Pointer to working position-components of every particle.
/// @param p  Pointer to instance of class minimizer.
/// @param f  Pointer to (output) potential to be minimized.
/// @param g  Pointer to (output) components of gradient.
void fdf(gsl_vector const *x, void *p, double *f, gsl_vector *g);
}


// EOF
