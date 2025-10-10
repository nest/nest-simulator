/*
 *  urbanczik_archiving_node.h
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

#ifndef URBANCZIK_ARCHIVING_NODE_H
#define URBANCZIK_ARCHIVING_NODE_H

// C++ includes:
#include <deque>

// Includes from nestkernel:
#include "archiving_node.h"
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "synaptic_element.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * An archiving node which additionally archives parameters
 * and buffers needed for the Urbanczik-Senn plasticity rule
 */
template < class urbanczik_parameters >
class UrbanczikArchivingNode : public ArchivingNode
{

public:
  UrbanczikArchivingNode();

  UrbanczikArchivingNode( const UrbanczikArchivingNode& );

  bool
  supports_urbanczik_archiving() const override
  {
    return true;
  }

  /**
   * Sets pointer start (finish) to the first (last) entry in urbanczik_history_[comp]
   * whose time argument is between t1 and t2
   */
  void get_urbanczik_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish,
    int comp ) override;

  /**
   * Returns membrane capacitance
   */
  double get_C_m( int comp ) override;

  /**
   * Returns leak conductance g_L
   */
  double get_g_L( int comp ) override;

  /**
   * Returns time constant tau_L
   */
  double get_tau_L( int comp ) override;

  /**
   * Returns time constant tau_syn_ex
   */
  double get_tau_syn_ex( int comp ) override;

  /**
   * Returns time constant tau_syn_in
   */
  double get_tau_syn_in( int comp ) override;

protected:
  /**
   * Writes the history for compartment comp into the buffers.
   */
  void write_urbanczik_history( Time const& t_sp, double V_W, int n_spikes, int comp );

  urbanczik_parameters* urbanczik_params;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

private:
  std::deque< histentry_extended > urbanczik_history_[ urbanczik_parameters::NCOMP - 1 ];
};

} // namespace nest

#endif /* #ifndef URBANCZIK_ARCHIVING_NODE_H */
