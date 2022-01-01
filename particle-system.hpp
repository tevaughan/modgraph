
/// @file       particle-system.hpp
/// @brief      Declaration of modgraph::particle_system.
/// @copyright  2021 Thomas E. Vaughan, all rights reserved.

#pragma once

#include <eigen3/Eigen/Core>
#include <functional>

namespace modgraph {


/// System of particles that, in three-dimensional space, interact pair-wise
/// via function that converts relative displacement, between first and second
/// particles of pair, into second particle's contribution to change in
/// position of first particle.
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
class particle_system {
  /// Type of storage for each particle's contribution to other particle's
  /// change of position.
  using prop_matrix= Eigen::MatrixXd;

  /// Each particle's contribution to other particle's change of position.
  /// - `prop_({i*3, ..., i*3 + 2}, j)` is effect felt by particle-`i` from
  ///   particle-`j`.
  prop_matrix prop_;

public:
  /// Type of relative or absolute displacement.
  using dspl= Eigen::Vector3d;

  /// Type of storage for positions, one for each particle.
  using pos= Eigen::Matrix3Xd;

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
  auto prop(unsigned i, unsigned j) { return prop_.block(i * 3, j, 3, 1); }

  /// Call move_ to update contribution of particle j toward displacement felt
  /// by particle i.
  /// @param i  Offset of one particle.
  /// @param j  Offset of other particle.
  void update_prop(unsigned i, unsigned j);

  /// Move particle feeling maximum net displacement, and update table of
  /// effects.
  /// @param offset_for_max  Offset of particle feeling maximum displacement.
  /// @param max_dspl        Displacement felt by particle.
  void move_and_update(unsigned offset_for_max, dspl const &max_dspl);

public:
  /// Initialize positions and function for computing position-updates, and
  /// compute initial position-updates.
  /// @param init  Initial positions, one for each particle.
  /// @param move  Function used to update calculated displacements.
  particle_system(pos const &init, func move);

  /// Position of each particle.
  /// @return  Reference to position of each particle.
  pos const &positions() const { return positions_; }

  /// Move particle feelin maximum net computed displacement; after motion,
  /// recompute relevant contributions to computed displacements.
  /// - If computed displacement be larger in magnitude than `lim`, then move
  ///   particle in direction of computed displacement but only by magnitude
  ///   `lim`.
  /// @param lim  Maximum magnitude of particle's motion.
  /// @return     Magnitude of step taken.
  double step(double lim);
};


} // namespace modgraph

// EOF
