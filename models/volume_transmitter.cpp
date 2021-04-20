/*
 *  volume_transmitter.cpp
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

#include "volume_transmitter.h"

// C++ includes:
#include <numeric>

// Includes from nestkernel:
#include "connector_base.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "spikecounter.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

/* ----------------------------------------------------------------
 * Default constructor defining default parameters
 * ---------------------------------------------------------------- */

nest::volume_transmitter::Parameters_::Parameters_()
  : deliver_interval_( 1 ) // in steps of mindelay
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::volume_transmitter::Parameters_::get( DictionaryDatum& d ) const
{
  def< long >( d, names::deliver_interval, deliver_interval_ );
}

void ::nest::volume_transmitter::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< long >( d, names::deliver_interval, deliver_interval_, node );
}

/* ----------------------------------------------------------------
 * Default and copy constructor for volume transmitter
 * ---------------------------------------------------------------- */

nest::volume_transmitter::volume_transmitter()
  : ArchivingNode()
  , P_()
  , local_device_id_( 0 )
{
}

nest::volume_transmitter::volume_transmitter( const volume_transmitter& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , local_device_id_( n.local_device_id_ )
{
}

void
nest::volume_transmitter::init_state_( const Node& )
{
}

void
nest::volume_transmitter::init_buffers_()
{
  B_.neuromodulatory_spikes_.clear();
  B_.spikecounter_.clear();
  B_.spikecounter_.push_back( spikecounter( 0.0, 0.0 ) ); // insert pseudo last dopa spike at t = 0.0
  ArchivingNode::clear_history();
}

void
nest::volume_transmitter::calibrate()
{
  // +1 as pseudo dopa spike at t_trig is inserted after trigger_update_weight
  B_.spikecounter_.reserve( kernel().connection_manager.get_min_delay() * P_.deliver_interval_ + 1 );
}

void
nest::volume_transmitter::update( const Time&, const long from, const long to )
{
  // spikes that arrive in this time slice are stored in spikecounter_
  double t_spike;
  double multiplicity;
  for ( long lag = from; lag < to; ++lag )
  {
    multiplicity = B_.neuromodulatory_spikes_.get_value( lag );
    if ( multiplicity > 0 )
    {
      t_spike = Time( Time::step( kernel().simulation_manager.get_slice_origin().get_steps() + lag + 1 ) ).get_ms();
      B_.spikecounter_.push_back( spikecounter( t_spike, multiplicity ) );
    }
  }

  // all spikes stored in spikecounter_ are delivered to the target synapses
  if ( ( kernel().simulation_manager.get_slice_origin().get_steps() + to )
      % ( P_.deliver_interval_ * kernel().connection_manager.get_min_delay() )
    == 0 )
  {
    double t_trig = Time( Time::step( kernel().simulation_manager.get_slice_origin().get_steps() + to ) ).get_ms();

    if ( not B_.spikecounter_.empty() )
    {
      kernel().connection_manager.trigger_update_weight( get_node_id(), B_.spikecounter_, t_trig );
    }

    // clear spikecounter
    B_.spikecounter_.clear();

    // as with trigger_update_weight dopamine trace has been updated to t_trig,
    // insert pseudo last dopa spike at t_trig
    B_.spikecounter_.push_back( spikecounter( t_trig, 0.0 ) );
  }
}

void
nest::volume_transmitter::handle( SpikeEvent& e )
{
  B_.neuromodulatory_spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    static_cast< double >( e.get_multiplicity() ) );
}
