
/// @file       particle-system.hpp
/// @brief      Definition of modgraph::particle_system.
/// @copyright  2021 Thomas E. Vaughan, all rights reserved.

#pragma once

#include <eigen3/Eigen/Core>
#include <functional>

namespace modgraph {


/// Generic system of particles that, in three-dimensional space, interact
/// pair-wise via function that converts relative displacement, between first
/// and second particles of pair, into second particle's contribution to change
/// in position of first particle.
/// - Function must behave so that changes of position eventually reach zero.
/// - In each step, only one particle moves.
/// - Particle that moves is one with greatest computed net-change of position.
/// - Absolute displacement of motion is computed change but clamped in
///   magnitude at specified upper limit.
/// - `particle_system` supposes that (a) contribution of one particle to
///   change in position of other is equal in magnitude and opposite in
///   direction to (b) contribution of other particle to change in position of
///   one.
/// - At end of each step, all computed changes related to particle that moved
///   are updated for next step.
/// @tparam N  Number of particles.
template<unsigned N> class particle_system {
  /// Type of storage for each particle's contribution to other particle's
  /// change of position.
  using prop_matrix= Eigen::Matrix<double, 3 * N, N>;

  /// Each particle's contribution to other particle's change of position.
  /// - `prop_({i*3, ..., i*3 + 2}, j)` is effect felt by particle-`i` from
  ///   particle-`j`.
  prop_matrix prop_= prop_matrix::Zero;

public:
  /// Type of relative or absolute displacement.
  using dspl= Eigen::Vector3d;

  /// Type of storage for positions, one for each particle.
  using pos= Eigen::Matrix<double, 3, N>;

  /// Type of function that converts relative-displacement vector, from one
  /// particle to other, into other's contribution toward absolute displacement
  /// felt by one.
  using func= std::function<dspl /*abs*/ (dspl const & /*rel*/)>;

private:
  pos positions_; ///< Position of each particle.

  /// Function converting relative-displacement vector, from one particle to
  /// other, into other's contribution toward absolute displacement felt by
  /// one.
  /// - One's contribution toward displacement felt by other is taken to be
  ///   negative of other's contribution toward displacement felt by one.
  func move_;

  /// Contribution of particle j toward displacement felt by particle i.
  /// @param i  Offset of one particle.
  /// @param j  Offset of other particle.
  auto &prop(unsigned i, unsigned j) { return prop_.block(i * 3, j, 3, 1); }

public:
  /// Initialize positions and function for computing position-updates, and
  /// compute initial position-updates.
  /// @param init  Initial positions, one for each particle.
  /// @param move  Function used to update calculated displacements.
  particle_system(pos const &init, func move);

  /// Position of each particle.
  /// @return  Reference to position of each particle.
  pos &positions() const { return positions_; }

  /// Move particle that has maximum computed displacement; after motion,
  /// recompute relevant contributions to computed displacements.
  /// - If computed displacement be larger in magnitude than `lim`, then move
  ///   particle in direction of computed displacement but only by magnitude
  ///   `lim`.
  /// @param lim  Maximum magnitude of particle's motion.
  /// @return     Magnitude of step taken.
  double step(double lim);
};


template<unsigned N>
particle_system<N>::particle_system(pos const &init, func move):
    positions_(init), move_(move) {
  for(unsigned i= 0; i < N; ++i) {
    for(unsigned j= i + 1; j < N; ++i) {
      dspl const d= move_(positions_[j] - positions_[i]);
      prop(i, j)= /* + */ d;
      prop(j, i)= /*  */ -d;
    }
  }
}


template<unsigned N> double particle_system<N>::step(double lim) {
  // Find particle that feels maximum net effect from other particles.
  dspl max_dspl= dspl::Zero();
  double max_norm= 0.0;
  unsigned offset_for_max= 0;
  Eigen::Matrix<double, 3 * N, 1> p= prop_.rowwise().sum();
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
  // Move particle that feels maximum net effect from other particles.
  positions_[offset_for_max]+= max_dspl;
  // Update relevant effects of one particle on other.
  unsigned const j= offset_for_max;
  for(unsigned i= 0; i < N; ++i) {
    if(i == j) continue;
    dspl const d= move_(positions_[j] - positions_[i]);
    prop(i, j)= /* + */ d;
    prop(j, i)= /*  */ -d;
  }
  return max_norm;
}


} // namespace modgraph

// EOF
