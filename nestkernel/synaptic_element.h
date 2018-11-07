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

/** @BeginDocumentation
  Name: synaptic_element - Contact point element for the dynamic creation
   and deletion of synapses.

  Description:
   This class represents synaptic element of a node (like Axonl boutons or
   dendritic spines) used for structural plasticity.
   The synaptic elements represent connection points between two neurons. They
   grow according to a homeostatic growth rule. The dynamics of the
   number of synaptic elements is driven by the average electrical activity of
   the neuron (indirectly measured through the Calcium concentration of the
   node). The probability of two neurons creating a new synapse between them,
   depends on the number of available synaptic elements of each neuron.

  Parameters:
   z                double  - Current number of synaptic elements. Stored as a
                              double variable but the actual usable number of
                              synaptic elements is an integer truncated from
  this
                              double value. An standard value for the growth of
  a
                              synaptic element is around 0.0001 elements/ms.
   continuous       boolean - Defines if the number of synaptic elements should
                              be treated as a continuous double number or as an
                              integer value. Default is false.
   growth_rate      double  - The maximum amount by which the synaptic elements
  will
                              change between time steps. In elements/ms.
   tau_vacant       double  - Rate at which vacant synaptic elements will decay.
                              Typical is 0.1 which represents a
                              loss of 10% of the vacant synaptic elements each
  time
                              the structural_plasticity_update_interval is
                              reached by the simulation time.
   growth_curve     GrowthCurve* - Rule which defines the dynamics of this
  synaptic element.

  References:
   [1] Butz, Markus, Florentin Wörgötter, and Arjen van Ooyen.
   "Activity-dependent structural plasticity." Brain research reviews 60.2
   (2009): 287-305.

   [2] Butz, Markus, and Arjen van Ooyen. "A simple rule for dendritic spine
   and axonal bouton formation can account for cortical reorganization after
   focal retinal lesions." PLoS Comput Biol 9.10 (2013): e1003259.

  FirstVersion: July 2013

  Author: Mikael Naveau, Sandra Diaz

  SeeAlso: GrowthCurve, SPManager, SPBuilder, Node, ArchivingNode.
*/

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "growth_curve.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * \class SynapticElement
 * Synaptic element of a node (axonal bouton or dendritic spine) for the
 * purposes of structural plasticity.
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
  void update( double t, double t_minus, double Ca_minus, double tau_Ca );

  /**
  * \fn double get_z_value(Archiving_Node const *a, double t) const
  * Get the number of synaptic_element at the time t (in ms)
  * Returns a negative number when synaptic elements must be deleted
  * during the next update
  * @param a node of this synaptic_element
  * @param t Current time (in ms)
  */
  int
  get_z_vacant() const
  {
    return std::floor( z_ ) - z_connected_;
  }
  /*
   * Retrieves the current number of synaptic elements bound to a synapse
   */
  int
  get_z_connected() const
  {
    return z_connected_;
  }
  /*
   * Retrieves the value of tau_vacant
   */
  double
  get_tau_vacant() const
  {
    return tau_vacant_;
  }
  /*
   * Changes the number of bound synaptic elements by n.
   * @param n number of new connections. Can be negative.
   */
  void
  connect( int n )
  {
    z_connected_ += n;
    if ( z_connected_ > floor( z_ ) )
    {
      z_ = z_connected_ + ( z_ - floor( z_ ) );
    }
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
   * Retrieves the current value of the growth rate
   */
  double
  get_growth_rate() const
  {
    return growth_rate_;
  }

  void
  set_z( const double z_new )
  {
    z_ = z_new;
  }
  double
  get_z() const
  {
    return z_;
  }
  /**
   * Reduce the amount of vacant synaptic elements by a factor
   * of tau_vacant_
   */
  void
  decay_z_vacant()
  {
    if ( get_z_vacant() > 0 )
    {
      z_ -= get_z_vacant() * tau_vacant_;
    }
  }

  bool
  continuous() const
  {
    return continuous_;
  }

private:
  // The current number of synaptic elements at t = z_t_
  double z_;
  // Last time stamp when the number of synaptic elements was updated
  double z_t_;
  // Number of synaptic elements bound to a synapse
  int z_connected_;
  // Variable which defines if the number of synaptic elements should be treated
  // as a continous double number or as an integer value
  bool continuous_;
  // The maximum amount by which the synaptic elements will change between time
  // steps.
  double growth_rate_;
  // Rate at which vacant synaptic elements will decay
  double tau_vacant_;
  // Growth curve which defines the dynamics of this synaptic element.
  GrowthCurve* growth_curve_;
};

} // of namespace

#endif
