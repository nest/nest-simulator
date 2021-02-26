/*
 *  cm_main.cpp
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
#include "cm_main.h"


namespace nest
{


template <>
void
DynamicRecordablesMap< cm_main >::create( cm_main& host)
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::cm_main::cm_main()
  : ArchivingNode()
  , c_tree_()
  , syn_receptors_( 0 )
  , logger_( *this )
  , V_th_( -55.0 )
{
  recordablesMap_.create( *this );
}

nest::cm_main::cm_main( const cm_main& n )
  : ArchivingNode( n )
  , c_tree_( n.c_tree_ )
  , syn_receptors_( n.syn_receptors_ )
  , logger_( *this )
  , V_th_( n.V_th_ )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::cm_main::init_state_( const Node& proto )
{
}

void
nest::cm_main::init_buffers_()
{
  logger_.reset();
  ArchivingNode::clear_history();
}

void
cm_main::add_compartment( const long compartment_idx, const long parent_compartment_idx, const DictionaryDatum& compartment_params )
{
  c_tree_.add_compartment( compartment_idx, parent_compartment_idx, compartment_params);

  // to enable recording the voltage of the current compartment
  recordablesMap_.insert( "V_m_" + std::to_string(compartment_idx),
                          DataAccessFunctor< cm_main >( *this, compartment_idx ) );
}

size_t
cm_main::add_receptor( const long compartment_idx, const std::string& type )
{
  std::shared_ptr< Synapse > syn;
  if ( type == "AMPA" )
  {
    syn = std::shared_ptr< Synapse >( new AMPASyn() );
  }
  else if ( type == "GABA" )
  {
    syn = std::shared_ptr< Synapse >( new GABASyn() );
  }
  else if ( type == "NMDA" )
  {
    syn = std::shared_ptr< Synapse >( new NMDASyn() );
  }
  else if ( type == "AMPA+NMDA" )
  {
    syn = std::shared_ptr< Synapse >( new AMPA_NMDASyn() );
  }
  else
  {
    assert( false );
  }

  const size_t syn_idx = syn_receptors_.size();
  syn_receptors_.push_back( syn );

  Compartment* compartment = c_tree_.get_compartment( compartment_idx );
  compartment->syns.push_back( syn );

  return syn_idx;
}

void
nest::cm_main::calibrate()
{
  logger_.init();
  c_tree_.init();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::cm_main::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    const double v_0_prev = c_tree_.get_root()->v_comp;

    c_tree_.construct_matrix( lag );
    c_tree_.solve_matrix();

    // threshold crossing
    if ( c_tree_.get_root()->v_comp >= V_th_ && v_0_prev < V_th_ )
    {
      c_tree_.get_root()->etype.add_spike();

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::cm_main::handle( SpikeEvent& e )
{
  if ( e.get_weight() < 0 )
  {
    throw BadProperty(
      "Synaptic weights must be positive." );
  }

  assert( e.get_delay_steps() > 0 );
  assert( ( e.get_rport() >= 0 ) && ( ( size_t ) e.get_rport() < syn_receptors_.size() ) );

  syn_receptors_[ e.get_rport() ]->handle(e);
}

void
nest::cm_main::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  Compartment* compartment = c_tree_.get_compartment( e.get_rport() );
  compartment->currents.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::cm_main::handle( DataLoggingRequest& e )
{
  logger_.handle( e );
}

} // namespace
