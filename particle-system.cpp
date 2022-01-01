
/// @file       particle-system.cpp
/// @brief      Definition of modgraph::particle_system.
/// @copyright  2021 Thomas E. Vaughan, all rights reserved.

#include "particle-system.hpp"

namespace modgraph {


particle_system::particle_system(pos const &init, func move):
    prop_(Eigen::MatrixXd::Zero(init.rows() * 3, init.cols())),
    positions_(init),
    move_(move) {
  unsigned const N= init.cols();
  for(unsigned i= 0; i < N; ++i) {
    for(unsigned j= i + 1; j < N; ++i) {
      dspl const r= positions_.col(j) - positions_.col(i);
      dspl const d= move_(r);
      prop(i, j)= /* + */ d;
      prop(j, i)= /*  */ -d;
    }
  }
}


void particle_system::move_and_update(unsigned j, dspl const &max_dspl) {
  // Move particle that feels maximum net effect from other particles.
  positions_.col(j)+= max_dspl;
  // Update relevant effects of one particle on other.
  unsigned const N= positions_.cols();
  for(unsigned i= 0; i < N; ++i) {
    if(i == j) continue;
    dspl const r= positions_.col(j) - positions_.col(i);
    dspl const d= move_(r);
    prop(i, j)= /* + */ d;
    prop(j, i)= /*  */ -d;
  }
}


double particle_system::step(double lim) {
  dspl max_dspl= dspl::Zero();
  double max_norm= 0.0;
  unsigned offset_for_max= 0;
  Eigen::VectorXd const p= prop_.rowwise().sum();
  unsigned const N= positions_.cols();
  for(unsigned i= 0; i < N; ++i) {
    dspl const v= p.block(3 * i, 0, 3, 1);
    double const m= v.norm();
    if(m > max_norm) {
      max_dspl= v;
      max_norm= m;
      offset_for_max= i;
    }
  }
  if(max_norm > lim) {
    max_norm= lim;
    dspl const u= max_dspl / max_norm;
    max_dspl= u * lim;
  }
  move_and_update(offset_for_max, max_dspl);
  return max_norm;
}


} // namespace modgraph

// EOF
