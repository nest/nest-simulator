/*
 *  synaptic_element.h
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

/**
 * \file synaptic_element.h
 * Definition of SynapticElement which is capable of
 * managing a synaptic element of a node.
 * \author Mikael Naveau
 * \date July 2013
 */

#ifndef SYNAPTIC_ELEMENT_H
#define SYNAPTIC_ELEMENT_H

#include "dictdatum.h"
#include "dictutils.h"
#include "histentry.h"
#include "growth_curve.h"
#include <cmath>

namespace nest
{

class SynapticElement;
class Archiving_Node;
class GrowthCurveGaussian;
class GrowthCurveLinear;

/**
 * \class SynapticElement
 * Synaptic element of a node (like Axon or dendrite) for the purposes
 * of synaptic plasticity.
 * The synaptic elements represent connection points between two neurons that
 * grow according to a homeostatic growth rule. Basically, the dynamics of the
 * number of synaptic elements is driven by the average electrical activity of
 * the neuron (indirectly measured through the Calcium concentration of the
 * node). The probability of two neurons creating a new synapse between them,
 * depends on the number of available synaptic elements of each neuron.
 */
class SynapticElement
{

public:
  /**
  * \fn SynapticElement()
  * Constructor.
  */
  SynapticElement();

  /**
  * \fn SynapticElement(const SynapticElement& se)
  * Copy Constructor.
  * @param se SynapticElement
  */
  SynapticElement( const SynapticElement& se );

  /**
  * \fn SynapticElement(const SynapticElement& se)
  * copy assignment operator.
  * @param other SynapticElement
  */
  SynapticElement& operator=( const SynapticElement& other );

  /**
  * \fn SynapticElement()
  * Destructor.
  */
  ~SynapticElement()
  {
    delete growth_curve_;
  }

  /**
  * \fn void get(DictionaryDatum&) const
  * Store current values in a dictionary.
  * @param d to write data
  */
  void get( DictionaryDatum& d ) const;

  /**
  * \fn void set(const DictionaryDatum&)
  * Set values from a dictionary.
  * @param d to take data from
  */
  void set( const DictionaryDatum& d );


  /*
   * Updates the number of available synaptic elements according to the mean
   * calcium concentration of the neuron at time t.
   * @param t Current time (in ms)
   * @param t_minus Time of last update
   * @param Ca_minus Calcium concentration at time t_minus
   * @param tau_Ca change in the calcium concentration on each spike
   */
  void update( double_t t, double_t t_minus, double_t Ca_minus, double_t tau_Ca );

  /**
  * \fn double_t get_z_value(Archiving_Node const *a, double_t t) const
  * Get the number of synaptic_element at the time t (in ms)
  * @param a node of this synaptic_element
  * @param t Current time (in ms)
  */
  int_t
  get_z_vacant() const
  {
    return std::floor( z_ ) - z_connected_;
  }
  /*
   * Retrieves the current number of synaptic elements bound to a synapse
   */
  int_t
  get_z_connected() const
  {
    return z_connected_;
  }
  /*
   * Changes the number of bound synaptic elements by n.
   * @param n number of new connections. Can be negative.
   */
  void
  connect( int_t n )
  {
    z_connected_ += n;
  }

  /*
   * Used to define the dynamics of the synaptic elements using a Growth Curve
   */
  void
  set_growth_curve( GrowthCurve& g )
  {
    if ( growth_curve_ != &g )
    {
      delete growth_curve_;
      growth_curve_ = &g;
    }
  }

  /*
   * Retrieves the current value of the growth rate, this is
   */
  double_t
  get_growth_rate() const
  {
    return growth_rate_;
  }

  void
  set_z( const double_t z_new )
  {
    z_ = z_new;
  }
  double_t
  get_z() const
  {
    return z_;
  }

  bool
  continuous() const
  {
    return continuous_;
  }

private:
  // The current number of synaptic elements at t = z_t_
  double_t z_;
  // Last time stamp when the number of synaptic elements was updated
  double_t z_t_;
  // Number of synaptic elements bound to a synapse
  int_t z_connected_;
  // Variable which defines if the number of synaptic elements should be treated
  // as a continous double number or as an integer value
  bool continuous_;
  // The maximum amount by which the synaptic elements will change between time
  // steps.
  double_t growth_rate_;
  // Rate at which vacant synaptic elements will decay
  double_t tau_vacant_;
  // Growth curve which defines the dynamics of this synaptic element.
  GrowthCurve* growth_curve_;
};

} // of namespace

#endif
