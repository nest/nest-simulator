/*
 *  propagator_stability.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PROPAGATOR_STABILITY_H
#define PROPAGATOR_STABILITY_H

// C++ includes:
#include <cmath>
#include <tuple>

// Includes from libnestutil:
#include "numerics.h"


/**
 * Propagator class for handling similar tau_m and tau_syn_* time constants.
 *
 * Constants are calculated in the constructor, while
 * propagator_31 and propagator_32 are calculated in `evaluate( h )`.
 *
 * For details, please see doc/userdoc/model_details/IAF_neurons_singularity.ipynb.
 */
class Propagator
{
public:
  /**
   * Empty constructor needed for initialization of buffers.
   */
  Propagator();

  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau_m Membrane time constant in ms
   * @param c_m Membrane capacitance in pF
   */
  Propagator( double tau_syn, double tau_m, double c_m );

protected:
  /**
   * Calculate propagator 32 and return value along with exponentials.
   *
   * Exponentials are returned so they can be used directly when calculating P31.
   *
   * @param h time step
   *
   * @returns tuple with propagator 32, exp( -h / tau_syn_ ), expm1( -h / tau_m_ + h / tau_syn_ )
   * and exp( -h / tau_m_ ), all as doubles
   */
  std::tuple< double, double, double, double > evaluate_P32_( double h ) const;

  double tau_syn_; //!< Time constant of synaptic current in ms
  double tau_m_;   //!< Membrane time constant in ms
  double c_m_;     //!< Membrane capacitance in pF

  double alpha_; //!< 1/(c*tau*tau) * (tau_syn - tau)
  double beta_;  //!< tau_syn * tau/(tau - tau_syn)
  double gamma_; //!< beta_/c
};


/**
 * Propagator class for handling similar tau_m and tau_syn_* time constants for
 * models with exponential postsynaptic currents.
 *
 * propagator_32 is calculated in `evaluate( h )` and returned as a double.
 */
class PropagatorExp : public Propagator
{
public:
  /**
   * Empty constructor needed for initialization of buffers.
   */
  PropagatorExp();

  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau_m Membrane time constant in ms
   * @param c_m Membrane capacitance in pF
   */
  PropagatorExp( double tau_syn, double tau_m, double c_m );

  /**
   * Calculate propagator 32.
   *
   * @param h time step
   *
   * @returns propagator 32 as a double
   */
  double evaluate( double h ) const;
};

/**
 * Propagator class for handling similar tau_m and tau_syn_* time constants for
 * models with postsynaptic currents modeled as an alpha current.
 *
 * propagator_31 and propagator_32 are calculated in `evaluate( h )` and returned
 * as a tuple, where P31 is the first variable.
 */
class PropagatorAlpha : public Propagator
{
public:
  /**
   * Empty constructor needed for initialization of buffers.
   */
  PropagatorAlpha();

  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau Membrane time constant in ms
   * @param Membrane capacitance in pF
   */
  PropagatorAlpha( double tau_syn, double tau_m, double c_m );

  /**
   * Calculate propagators 31 and 32.
   *
   * @param h time step
   *
   * @returns tuple containing propagators 31 and 32
   */
  std::tuple< double, double > evaluate( double h ) const;
};

inline std::tuple< double, double, double, double >
Propagator::evaluate_P32_( double h ) const
{
  const double exp_h_tau_syn = std::exp( -h / tau_syn_ );
  const double expm1_h_tau = numerics::expm1( -h / tau_m_ + h / tau_syn_ );

  double P32 = gamma_ * exp_h_tau_syn * expm1_h_tau;

  double exp_h_tau = 0.0;
  if ( std::abs( tau_m_ - tau_syn_ ) < 0.1 )
  {
    exp_h_tau = std::exp( -h / tau_m_ );

    const double P32_singular = h / c_m_ * exp_h_tau;
    if ( tau_m_ == tau_syn_ )
    {
      P32 = P32_singular;
    }
    else
    {
      const double P32_linear = alpha_ * h * h * exp_h_tau / 2.;
      const double dev_P32 = std::abs( P32 - P32_singular );

      if ( dev_P32 > 2 * std::abs( P32_linear ) )
      {
        P32 = P32_singular;
      }
    }
  }

  return std::make_tuple( P32, exp_h_tau_syn, expm1_h_tau, exp_h_tau );
}

#endif
