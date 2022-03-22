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

/**
 * Propagator variables
 */
struct propagators
{
  double P31;
  double P32;
};

class propagator
{
public:

  propagator();
  propagator( double tau_syn, double tau, double c );

  void update_constants( double tau_syn, double tau, double c );
  propogate propagate( double h ) const;

private:
  double alpha_;   //!< 1/(c*tau*tau) * (tau_syn - tau)
  double beta_;    //!< tau_syn  * tau/(tau - tau_syn)
  double gamma_;   //!< beta_/c

  double tau_syn_; //!< Time constant of synaptic current in ms
  double tau_;     //!< Membrane time constant in ms
  double c_;       //!< Membrane capacitance in pF
};

#endif
