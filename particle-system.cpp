
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
    for(unsigned j= i + 1; j < N; ++i) update_prop(i, j);
  }
}


void particle_system::update_prop(unsigned i, unsigned j) {
  dspl const r= positions_.col(j) - positions_.col(i);
  dspl const d= move_(r);
  prop(i, j)= /* + */ d;
  prop(j, i)= /*  */ -d;
}


void particle_system::move_and_update(unsigned j, dspl const &max_dspl) {
  // Move particle that feels maximum net effect from other particles.
  positions_.col(j)+= max_dspl;
  // Update relevant effects of one particle on other.
  unsigned const N= positions_.cols();
  for(unsigned i= 0; i < N; ++i) {
    if(i != j) update_prop(i, j);
  }
}


using dspl= particle_system::dspl;


/// Attributes of maximum displacement felt by particle from all other
/// particles.
/// - On construction, instance is in a null state.
/// - After maximum displacement has been found by calls to max_net::update(),
///   max_net::clamp() may be called to rescale the instance as desired
///   according to constraint on motion in simulation.
class max_net {
  /// Maximum net displacement felt by any particle from all others.
  dspl displacement_= dspl::Zero();

  /// Norm of maximum net displacement felt.
  double norm_= 0.0;

  /// Offset of particle feeling maximum net displacement.
  unsigned offset_= 0;

public:
  /// Maximum net displacement felt by any particle from all others.
  /// @return  Reference to maximum net displacement.
  dspl const &displacement() const { return displacement_; }

  /// Norm of maximum net displacement felt.
  /// @return  Norm of maximum net displacement.
  double norm() const { return norm_; }

  /// Offset of particle feeling maximum net displacement.
  /// @return  Offset of particle feeling maximum net displacement.
  unsigned offset() const { return offset_; }

  /// Update current parameters based on new candidate-parameters.
  /// @param d    Candidate maximum net displacement felt.
  /// @param off  Offset of candidate particle feeling net displacement.
  void update(dspl const &d, unsigned off);

  /// If norm of displacement exceed limit `lim`, then rescale displacement so
  /// that its norm is `lim`.
  /// @param lim  Limit on magnitude of displacement.
  void clamp(double lim);
};


void max_net::update(dspl const &d, unsigned off) {
  double n= d.norm();
  if(n > norm_) {
    displacement_= d;
    norm_= n;
    offset_= off;
  }
}


void max_net::clamp(double lim) {
  if(norm_ > lim) {
    dspl const u= displacement_ / norm_; // Unit-vector.
    norm_= lim;
    displacement_= u * lim;
  }
}


double particle_system::step(double lim) {
  max_net max;
  // Calculate net displacement felt by each particle from all others.
  Eigen::VectorXd const p= prop_.rowwise().sum();
  // Find maximum net displacement felt by any particle from all others.
  unsigned const N= positions_.cols();
  for(unsigned i= 0; i < N; ++i) {
    dspl const v= p.block(3 * i, 0, 3, 1); // Net change felt by i.
    max.update(v, i);
  }
  max.clamp(lim);
  move_and_update(max.offset(), max.displacement());
  return max.norm();
}


} // namespace modgraph

// EOF
