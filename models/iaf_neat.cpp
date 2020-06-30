/*
 *  iaf_neat.cpp
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

#include "iaf_neat.h"

// C++ includes:
// #include <limits>

// Includes from libnestutil:
// #include "dict_util.h"
// #include "numerics.h"

// Includes from nestkernel:
#include "node.h"
#include "nest_time.h"
// #include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
// #include "doubledatum.h"
// #include "integerdatum.h"

#include "ionchannels_neat.h"

namespace nest
{


template <>
void
DynamicRecordablesMap< iaf_neat >::create( iaf_neat& host)
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_neat::iaf_neat()
  : Archiving_Node()
  , m_c_tree_()
  , syn_receptors_( 0 )
  , logger_( *this )
  , V_th_( -55.0 )
{
  recordablesMap_.create( *this );
}

nest::iaf_neat::iaf_neat( const iaf_neat& n )
  : Archiving_Node( n )
  , m_c_tree_( n.m_c_tree_ )
  , syn_receptors_( n.syn_receptors_ )
  , logger_( *this )
  , V_th_( n.V_th_ )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::iaf_neat::init_state_( const Node& proto )
{
}

void
nest::iaf_neat::init_buffers_()
{
  logger_.reset();
  Archiving_Node::clear_history();
}

void
iaf_neat::add_compartment( const long compartment_idx, const long parent_compartment_idx, const DictionaryDatum& compartment_params )
{
  const double C_m = getValue< double >( compartment_params, "C_m" );
  const double g_c = getValue< double >( compartment_params, "g_c" );
  const double g_L = getValue< double >( compartment_params, "g_L" );
  const double E_L = getValue< double >( compartment_params, "E_L" );

  m_c_tree_.add_node( compartment_idx, parent_compartment_idx, C_m, g_c, g_L, E_L );

  recordablesMap_.insert( "V_m_" + std::to_string( compartment_idx ),
                          DataAccessFunctor< iaf_neat >( *this, compartment_idx ) );
}

size_t
iaf_neat::add_receptor( const long compartment_idx, const std::string& type )
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

  CompNode* const node = m_c_tree_.find_node( compartment_idx );
  node->m_syns.push_back( syn );

  return syn_idx;
}

void
nest::iaf_neat::calibrate()
{
  logger_.init();

  CompNode* const root = m_c_tree_.get_root();

  std::shared_ptr< IonChannel > fake_potassium( new FakePotassium( 15. * root->m_gl ) );
  root->m_chans.push_back( fake_potassium );

  std::shared_ptr< IonChannel > fake_sodium( new FakeSodium( 40. * root->m_gl ) );
  root->m_chans.push_back( fake_sodium );

  m_c_tree_.init();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::iaf_neat::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    const double v_0_prev = m_c_tree_.get_root()->m_v;

    m_c_tree_.construct_matrix( lag );
    m_c_tree_.solve_matrix();

    // threshold crossing
    if ( m_c_tree_.get_root()->m_v >= V_th_ && v_0_prev < V_th_ )
    {
      m_c_tree_.get_root()->m_chans[0]->add_spike();
      m_c_tree_.get_root()->m_chans[1]->add_spike();

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_neat::handle( SpikeEvent& e )
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
nest::iaf_neat::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  CompNode* const node = m_c_tree_.find_node( e.get_rport() );
  node->m_currents.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::iaf_neat::handle( DataLoggingRequest& e )
{
  logger_.handle( e );
}

} // namespace
