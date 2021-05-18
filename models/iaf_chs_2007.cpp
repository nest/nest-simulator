/*
 *  iaf_chs_2007.cpp
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

#include "iaf_chs_2007.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_chs_2007 > nest::iaf_chs_2007::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_chs_2007 >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &iaf_chs_2007::get_V_m_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_chs_2007::Parameters_::Parameters_()
  : tau_epsp_( 8.5 )   // in ms
  , tau_reset_( 15.4 ) // in ms
  , E_L_( 0.0 )        // normalized
  , U_th_( 1.0 )       // normalized
  , U_epsp_( 0.77 )    // normalized
  , U_reset_( 2.31 )   // normalized
  , C_( 1.0 )          // Should not be modified
  , U_noise_( 0.0 )    // normalized
  , noise_()

{
}


nest::iaf_chs_2007::State_::State_()
  : i_syn_ex_( 0.0 )
  , V_syn_( 0.0 )
  , V_spike_( 0.0 )
  , V_m_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_chs_2007::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_reset, U_reset_ );
  def< double >( d, names::V_epsp, U_epsp_ );
  def< double >( d, names::tau_epsp, tau_epsp_ );
  def< double >( d, names::tau_reset, tau_reset_ );
  def< double >( d, names::V_noise, U_noise_ );
  ( *d )[ names::noise ] = DoubleVectorDatum( new std::vector< double >( noise_ ) );
}

void
nest::iaf_chs_2007::Parameters_::set( const DictionaryDatum& d, State_& s, Node* node )
{
  updateValueParam< double >( d, names::V_reset, U_reset_, node );
  updateValueParam< double >( d, names::V_epsp, U_epsp_, node );
  updateValueParam< double >( d, names::tau_epsp, tau_epsp_, node );
  updateValueParam< double >( d, names::tau_reset, tau_reset_, node );
  updateValueParam< double >( d, names::V_noise, U_noise_, node );

  const bool updated_noise = updateValue< std::vector< double > >( d, names::noise, noise_ );
  if ( updated_noise )
  {
    s.position_ = 0;
  }
  /*
  // TODO: How to handle setting U_noise first and noise later and still make
           sure they are consistent?
  if ( U_noise_ > 0 && noise_.empty() )
        throw BadProperty("Noise amplitude larger than zero while noise signal "
                          "is missing.");
  */
  if ( U_epsp_ < 0 )
  {
    throw BadProperty( "EPSP cannot be negative." );
  }

  if ( U_reset_ < 0 ) // sign switched above
  {
    throw BadProperty( "Reset potential cannot be negative." );
  }
  if ( tau_epsp_ <= 0 || tau_reset_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

void
nest::iaf_chs_2007::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, V_m_ ); // Membrane potential
}

void
nest::iaf_chs_2007::State_::set( DictionaryDatum const& d, Node* node )
{
  updateValueParam< double >( d, names::V_m, V_m_, node );
}

nest::iaf_chs_2007::Buffers_::Buffers_( iaf_chs_2007& n )
  : logger_( n )
{
}

nest::iaf_chs_2007::Buffers_::Buffers_( const Buffers_&, iaf_chs_2007& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_chs_2007::iaf_chs_2007()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_chs_2007::iaf_chs_2007( const iaf_chs_2007& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::iaf_chs_2007::init_buffers_()
{
  B_.spikes_ex_.clear(); // includes resize
  B_.currents_.clear();  // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

void
nest::iaf_chs_2007::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  // numbering of state vaiables: i_0 = 0, i_syn_ = 1, V_syn_ = 2, V_spike _= 3,
  // V_m_ = 4

  // these P are independent
  V_.P11ex_ = std::exp( -h / P_.tau_epsp_ );

  V_.P22_ = std::exp( -h / P_.tau_epsp_ );

  V_.P30_ = std::exp( -h / P_.tau_reset_ );

  // these depend on the above. Please do not change the order.
  // TODO: use expm1 here to improve accuracy for small timesteps

  V_.P21ex_ = P_.U_epsp_ * std::exp( 1.0 ) / ( P_.C_ ) * V_.P11ex_ * h / P_.tau_epsp_;

  V_.P20_ = P_.tau_epsp_ / P_.C_ * ( 1.0 - V_.P22_ );
}

void
nest::iaf_chs_2007::update( const Time& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // evolve from timestep 'from' to timestep 'to' with steps of h each
  for ( long lag = from; lag < to; ++lag )
  {
    S_.V_syn_ = S_.V_syn_ * V_.P22_ + S_.i_syn_ex_ * V_.P21ex_;

    // exponential decaying PSCs
    S_.i_syn_ex_ *= V_.P11ex_;

    // the spikes arriving at T+1 have an immediate effect on the state of the
    // neuron
    S_.i_syn_ex_ += B_.spikes_ex_.get_value( lag );

    // exponentially decaying ahp
    S_.V_spike_ *= V_.P30_;

    double noise_term = P_.U_noise_ > 0.0 && not P_.noise_.empty() ? P_.U_noise_ * P_.noise_[ S_.position_++ ] : 0.0;

    S_.V_m_ = S_.V_syn_ + S_.V_spike_ + noise_term;


    if ( S_.V_m_ >= P_.U_th_ ) // threshold crossing
    {
      S_.V_spike_ -= P_.U_reset_;
      S_.V_m_ -= P_.U_reset_;


      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_chs_2007::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() >= 0.0 )
  {
    B_.spikes_ex_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::iaf_chs_2007::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
