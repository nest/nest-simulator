/*
 *  iaf_propagator.h
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

#ifndef IAF_PROPAGATOR_H
#define IAF_PROPAGATOR_H

// C++ includes:
#include <cmath>
#include <tuple>

// Includes from libnestutil:
#include "numerics.h"


/**
 * Exact integration voltage propagator for leaky integrate-and-fire models.
 *
 * This base class and its derived classes provide the propagator matrix elements
 * connecting synaptic current to membrane potential. They handle the singularity
 * at `tau_m == tau_syn_*` properly.
 *
 * @see `<NEST_SOURCE>/doc/userdoc/model_details/IAF_Integration_Singularity.ipynb`
 */
class IAFPropagator
{
public:
  /**
   * Empty constructor needed for initialization of buffers.
   */
  IAFPropagator();

  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau_m Membrane time constant in ms
   * @param c_m Membrane capacitance in pF
   */
  IAFPropagator( double tau_syn, double tau_m, double c_m );

protected:
  /**
   * Scale factor for singularity test for P31 computation.
   *
   * Regular case applies if h > NUMERICAL_STABILITY_FACTOR_ * tau_m^2 / |tau_m - tau_syn|
   *
   * See IAF_Integration_Singularity for details.
   */
  static constexpr double NUMERICAL_STABILITY_FACTOR_ = 1e-7;

  /**
   * Compute propagator connecting I_syn to V_m and auxiliary quantities for given time interval.
   *
   * This method computes all quantities common to alpha and exponential synaptic dynamics.
   *
   * @note Fourth return value is real-valued only in the singular limit and `numerics::nan` otherwise
   * to avoid unnecessary computation of exp().
   *
   * @param h time interval [ms]
   * @returns tuple with P_VI, exp( -h / tau_syn_ ), expm1( -h / tau_m_ + h / tau_syn_ ), exp( -h / tau_m_ )
   */
  std::tuple< double, double, double, double > evaluate_P32_( double h ) const;

  double tau_syn_; //!< Time constant of synaptic current in ms
  double tau_m_;   //!< Membrane time constant in ms
  double c_m_;     //!< Membrane capacitance in pF

  double h_min_regular_; //!< Smallest h for which regular P31 case applies

  double beta_;  //!< (tau_syn * tau_m) / (tau_m - tau_syn)
  double gamma_; //!< beta_ / c_m

  double inv_tau_syn_; //!< 1 / tau_syn
  double inv_tau_m_;   //!< 1 / tau_m
  double inv_c_m_;     //!< 1 / c_m
  double inv_beta_;    //!< 1 / beta
};


/**
 * Exact integration voltage propagator for models with exponential psc.
 */
class IAFPropagatorExp : public IAFPropagator
{
public:
  /**
   * Empty constructor needed for initialization of buffers.
   */
  IAFPropagatorExp();

  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau_m Membrane time constant in ms
   * @param c_m Membrane capacitance in pF
   */
  IAFPropagatorExp( double tau_syn, double tau_m, double c_m );

  /**
   * Calculate propagator mapping I_syn to V_m for given time step.
   *
   * @param h time step
   * @returns P_VI
   */
  double evaluate( double h ) const;
};

/**
 * Exact integration voltage propagator for models with alpha psc.
 */
class IAFPropagatorAlpha : public IAFPropagator
{
public:
  /**
   * Empty constructor needed for initialization of buffers.
   */
  IAFPropagatorAlpha();

  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau Membrane time constant in ms
   * @param Membrane capacitance in pF
   */
  IAFPropagatorAlpha( double tau_syn, double tau_m, double c_m );

  /**
   * Calculate propagator mapping I_syn and dI_syn to V_m for given time step.
   *
   * @param h time step
   * @returns tuple with P_VdI and P_VI
   */
  std::tuple< double, double > evaluate( double h ) const;
};


inline double
IAFPropagatorExp::evaluate( double h ) const
{
  double P32;
  std::tie( P32, std::ignore, std::ignore, std::ignore ) = evaluate_P32_( h );

  return P32;
}

#endif
