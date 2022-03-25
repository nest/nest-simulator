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
#include <tuple>

/**
 * Propagator class for handling similar tau_m and tau_syn_* time constants.
 *
 * Constants are calculated in the constructor or `update_constants`, while
 * propagator_31 and propagator_32 are calculated in `propagate( h )`.
 *
 * For details, please see doc/userdoc/model_details/IAF_neurons_singularity.ipynb.
 */
class PropagatorExp
{
public:

  PropagatorExp();
  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau Membrane time constant in ms
   * @param Membrane capacitance in pF
   */
  PropagatorExp( double tau_syn, double tau, double c_m );

  /**
   * Calculate propagators 31 and 32.
   *
   * @param h time step
   *
   * @returns propagators struct containing 31 and 32
   */
  double evaluate( double h ) const;

protected:
  double tau_syn_; //!< Time constant of synaptic current in ms
  double tau_;     //!< Membrane time constant in ms
  double c_m_;       //!< Membrane capacitance in pF

  double alpha_;   //!< 1/(c*tau*tau) * (tau_syn - tau)
  double beta_;    //!< tau_syn  * tau/(tau - tau_syn)
  double gamma_;   //!< beta_/c
};

class PropagatorAlpha : public PropagatorExp
{
public:

  PropagatorAlpha();
  /**
   * @param tau_syn Time constant of synaptic current in ms
   * @param tau Membrane time constant in ms
   * @param Membrane capacitance in pF
   */
  PropagatorAlpha( double tau_syn, double tau, double c_m );

  /**
   * Calculate propagators 31 and 32.
   *
   * @param h time step
   *
   * @returns propagators struct containing 31 and 32
   */
  std::tuple< double, double > evaluate( double h ) const;

};

#endif
