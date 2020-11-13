/*
 *  structural_plasticity_node.h
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

#ifndef STRUCTURAL_PLASTICITY_NODE_H
#define STRUCTURAL_PLASTICITY_NODE_H

// C++ includes:
#include <algorithm>
#include <deque>

// Includes from nestkernel:
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"
#include "synaptic_element.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * \class StructuralPlasticityNode
 * Implements functionality related to structural synaptic plasticity.
 */
class StructuralPlasticityNode : public Node
{
  using Node::get_synaptic_elements;

public:
  /**
   * \fn StructuralPlasticityNode()
   * Constructor.
   */
  StructuralPlasticityNode();

  /**
   * \fn StructuralPlasticityNode()
   * Copy Constructor.
   */
  StructuralPlasticityNode( const StructuralPlasticityNode& );
  /**

   * \fn double get_Ca_minus()
   * return the current value of Ca_minus
   */
  double get_Ca_minus() const;

  /**
   * \fn double get_synaptic_elements(Name n)
   * get the number of synaptic element for the current Node
   * the number of synaptic elements is a double value but the number of
   * actual vacant and connected elements is an integer truncated from this
   * value
   */
  double get_synaptic_elements( Name n ) const;

  /**
   * \fn int get_synaptic_elements_vacant(Name n)
   * Get the number of synaptic elements of type n which are available
   * for new synapse creation
   */
  int get_synaptic_elements_vacant( Name n ) const;

  /**
   * \fn int get_synaptic_elements_connected(Name n)
   * get the number of synaptic element of type n which are currently
   * connected
   */
  int get_synaptic_elements_connected( Name n ) const;

  /**
   * \fn std::map<Name, double> get_synaptic_elements()
   * get the number of all synaptic elements for the current Node
   */
  std::map< Name, double > get_synaptic_elements() const;

  /**
   * \fn void update_synaptic_elements()
   * Change the number of synaptic elements in the node depending on the
   * dynamics described by the corresponding growth curve
   */
  void update_synaptic_elements( double t );

  /**
   * \fn void decay_synaptic_elements_vacant()
   * Delete a certain portion of the vacant synaptic elements which are not
   * in use
   */
  void decay_synaptic_elements_vacant();

  /**
   * \fn void connect_synaptic_element()
   * Change the number of connected synaptic elements by n
   */
  void connect_synaptic_element( Name name, int n );

  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d );

  /**
   * retrieve the current value of tau_Ca which defines the exponential decay
   * constant of the intracellular calcium concentration
   */
  double get_tau_Ca() const;

protected:
  /**
   * \fn void set_spiketime(Time const & t_sp, double offset)
   * record spike history
   */
  void set_spiketime( Time const& t_sp, double offset = 0.0 );

  /**
   * \fn void clear_history()
   * clear spike history
   */
  void clear_history();

private:
  /**
   * Time of the last update of the Calcium concentration in ms
   */
  double Ca_t_;

  /**
   * Value of the calcium concentration [Ca2+] at Ca_t_.
   *
   * Intracellular calcium concentration has a linear factor to mean
   * electrical activity of 10^2, this means, for example, that a [Ca2+] of
   * 0.2 is equivalent to a mean activity of 20 Hz.
   */
  double Ca_minus_;

  /**
   * Time constant for exponential decay of the intracellular calcium
   * concentration
   */
  double tau_Ca_;

  /**
   * Increase in calcium concentration [Ca2+] for each spike of the neuron
   */
  double beta_Ca_;

  /**
   * Map of the synaptic elements
   */
  std::map< Name, SynapticElement > synaptic_elements_map_;
};

inline double
StructuralPlasticityNode::get_tau_Ca() const
{
  return tau_Ca_;
}

inline double
StructuralPlasticityNode::get_Ca_minus() const
{
  return Ca_minus_;
}

} // of namespace
#endif
