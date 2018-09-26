/*
 *  gif_psc_exp_multisynapse.cpp
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

#include "gif_psc_exp_multisynapse.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"

#include "compose.hpp"
#include "propagator_stability.h"

namespace nest
{
/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< gif_psc_exp_multisynapse >
  gif_psc_exp_multisynapse::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< gif_psc_exp_multisynapse >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &gif_psc_exp_multisynapse::get_V_m_ );
  insert_( names::E_sfa, &gif_psc_exp_multisynapse::get_E_sfa_ );
  insert_( names::I_stc, &gif_psc_exp_multisynapse::get_I_stc_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::gif_psc_exp_multisynapse::Parameters_::Parameters_()
  : g_L_( 4.0 )        // nS
  , E_L_( -70.0 )      // mV
  , V_reset_( -55.0 )  // mV
  , Delta_V_( 0.5 )    // mV
  , V_T_star_( -35 )   // mV
  , lambda_0_( 0.001 ) // 1/ms
  , t_ref_( 4.0 )      // ms
  , c_m_( 80.0 )       // pF
  , tau_stc_()         // ms
  , q_stc_()           // nA
  , tau_sfa_()         // ms
  , q_sfa_()           // mV
  , tau_syn_( 1, 2.0 ) // ms
  , has_connections_( false )
  , I_e_( 0.0 ) // pA
{
}


nest::gif_psc_exp_multisynapse::State_::State_()
  : I_stim_( 0.0 )
  , V_( -70.0 )
  , sfa_( 0.0 )
  , stc_( 0.0 )
  , sfa_elems_()
  , stc_elems_()
  , i_syn_()
  , r_ref_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::gif_psc_exp_multisynapse::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::g_L, g_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::Delta_V, Delta_V_ );
  def< double >( d, names::V_T_star, V_T_star_ );
  def< double >( d, names::lambda_0, lambda_0_ * 1000.0 ); // convert to 1/s
  def< double >( d, names::t_ref, t_ref_ );

  def< int >( d, names::n_receptors, n_receptors_() );
  def< bool >( d, names::has_connections, has_connections_ );

  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, names::tau_syn, tau_syn_ad );

  ArrayDatum tau_sfa_list_ad( tau_sfa_ );
  def< ArrayDatum >( d, names::tau_sfa, tau_sfa_list_ad );

  ArrayDatum q_sfa_list_ad( q_sfa_ );
  def< ArrayDatum >( d, names::q_sfa, q_sfa_list_ad );

  ArrayDatum tau_stc_list_ad( tau_stc_ );
  def< ArrayDatum >( d, names::tau_stc, tau_stc_list_ad );

  ArrayDatum q_stc_list_ad( q_stc_ );
  def< ArrayDatum >( d, names::q_stc, q_stc_list_ad );
}

void
nest::gif_psc_exp_multisynapse::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::E_L, E_L_ );
  updateValue< double >( d, names::g_L, g_L_ );
  updateValue< double >( d, names::C_m, c_m_ );
  updateValue< double >( d, names::V_reset, V_reset_ );
  updateValue< double >( d, names::Delta_V, Delta_V_ );
  updateValue< double >( d, names::V_T_star, V_T_star_ );

  if ( updateValue< double >( d, names::lambda_0, lambda_0_ ) )
  {
    lambda_0_ /= 1000.0; // convert to 1/ms
  }

  updateValue< double >( d, names::t_ref, t_ref_ );

  updateValue< std::vector< double > >( d, names::tau_sfa, tau_sfa_ );
  updateValue< std::vector< double > >( d, names::q_sfa, q_sfa_ );
  updateValue< std::vector< double > >( d, names::tau_stc, tau_stc_ );
  updateValue< std::vector< double > >( d, names::q_stc, q_stc_ );

  if ( tau_sfa_.size() != q_sfa_.size() )
  {
    throw BadProperty( String::compose(
      "'tau_sfa' and 'q_sfa' need to have the same dimensions.\nSize of "
      "tau_sfa: %1\nSize of q_sfa: %2",
      tau_sfa_.size(),
      q_sfa_.size() ) );
  }

  if ( tau_stc_.size() != q_stc_.size() )
  {
    throw BadProperty( String::compose(
      "'tau_stc' and 'q_stc' need to have the same dimensions.\nSize of "
      "tau_stc: %1\nSize of q_stc: %2",
      tau_stc_.size(),
      q_stc_.size() ) );
  }
  if ( g_L_ <= 0 )
  {
    throw BadProperty( "Membrane conductance must be strictly positive." );
  }
  if ( Delta_V_ <= 0 )
  {
    throw BadProperty( "Delta_V must be strictly positive." );
  }
  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }
  if ( lambda_0_ < 0 )
  {
    throw BadProperty( "lambda_0 must not be negative." );
  }

  for ( size_t i = 0; i < tau_sfa_.size(); i++ )
  {
    if ( tau_sfa_[ i ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }

  for ( size_t i = 0; i < tau_stc_.size(); i++ )
  {
    if ( tau_stc_[ i ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }

  std::vector< double > tau_tmp;
  if ( updateValue< std::vector< double > >( d, names::tau_syn, tau_tmp ) )
  {
    if ( has_connections_ && tau_tmp.size() < tau_syn_.size() )
    {
      throw BadProperty(
        "The neuron has connections, "
        "therefore the number of ports cannot be reduced." );
    }

    for ( size_t i = 0; i < tau_tmp.size(); ++i )
    {
      if ( tau_tmp[ i ] <= 0 )
      {
        throw BadProperty( "All synaptic time constants must be > 0." );
      }
    }

    tau_syn_ = tau_tmp;
  }
}

void
nest::gif_psc_exp_multisynapse::State_::get( DictionaryDatum& d,
  const Parameters_& p ) const
{
  def< double >( d, names::V_m, V_ );     // Membrane potential
  def< double >( d, names::E_sfa, sfa_ ); // Adaptive threshold potential
  def< double >( d, names::I_stc, stc_ ); // Spike-triggered current
}

void
nest::gif_psc_exp_multisynapse::State_::set( const DictionaryDatum& d,
  const Parameters_& p )
{
  updateValue< double >( d, names::V_m, V_ );
}

nest::gif_psc_exp_multisynapse::Buffers_::Buffers_(
  gif_psc_exp_multisynapse& n )
  : logger_( n )
{
}

nest::gif_psc_exp_multisynapse::Buffers_::Buffers_( const Buffers_&,
  gif_psc_exp_multisynapse& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::gif_psc_exp_multisynapse::gif_psc_exp_multisynapse()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::gif_psc_exp_multisynapse::gif_psc_exp_multisynapse(
  const gif_psc_exp_multisynapse& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::gif_psc_exp_multisynapse::init_state_( const Node& proto )
{
  const gif_psc_exp_multisynapse& pr =
    downcast< gif_psc_exp_multisynapse >( proto );
  S_ = pr.S_;
}

void
nest::gif_psc_exp_multisynapse::init_buffers_()
{
  B_.spikes_.clear();   //!< includes resize
  B_.currents_.clear(); //!< includes resize
  B_.logger_.reset();   //!< includes resize
  Archiving_Node::clear_history();
}

void
nest::gif_psc_exp_multisynapse::calibrate()
{
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();
  V_.rng_ = kernel().rng_manager.get_rng( get_thread() );

  const double tau_m = P_.c_m_ / P_.g_L_;

  V_.P33_ = std::exp( -h / tau_m );
  V_.P30_ = -1 / P_.c_m_ * numerics::expm1( -h / tau_m ) * tau_m;
  V_.P31_ = -numerics::expm1( -h / tau_m );

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );


  // initializing adaptation (stc/sfa) variables
  V_.P_sfa_.resize( P_.tau_sfa_.size(), 0.0 );
  V_.P_stc_.resize( P_.tau_stc_.size(), 0.0 );

  for ( size_t i = 0; i < P_.tau_sfa_.size(); i++ )
  {
    V_.P_sfa_[ i ] = std::exp( -h / P_.tau_sfa_[ i ] );
  }
  S_.sfa_elems_.resize( P_.tau_sfa_.size(), 0.0 );

  for ( size_t i = 0; i < P_.tau_stc_.size(); i++ )
  {
    V_.P_stc_[ i ] = std::exp( -h / P_.tau_stc_[ i ] );
  }
  S_.stc_elems_.resize( P_.tau_stc_.size(), 0.0 );

  V_.P11_syn_.resize( P_.n_receptors_() );
  V_.P21_syn_.resize( P_.n_receptors_() );

  S_.i_syn_.resize( P_.n_receptors_() );

  B_.spikes_.resize( P_.n_receptors_() );

  for ( size_t i = 0; i < P_.n_receptors_(); i++ )
  {
    V_.P11_syn_[ i ] = std::exp( -h / P_.tau_syn_[ i ] );
    V_.P21_syn_[ i ] = propagator_32( P_.tau_syn_[ i ], tau_m, P_.c_m_, h );

    B_.spikes_[ i ].resize();
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::gif_psc_exp_multisynapse::update( Time const& origin,
  const long from,
  const long to )
{

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {

    // exponential decaying stc and sfa elements
    S_.stc_ = 0.0;
    for ( size_t i = 0; i < S_.stc_elems_.size(); i++ )
    {
      S_.stc_ += S_.stc_elems_[ i ];
      S_.stc_elems_[ i ] = V_.P_stc_[ i ] * S_.stc_elems_[ i ];
    }

    S_.sfa_ = P_.V_T_star_;
    for ( size_t i = 0; i < S_.sfa_elems_.size(); i++ )
    {
      S_.sfa_ += S_.sfa_elems_[ i ];
      S_.sfa_elems_[ i ] = V_.P_sfa_[ i ] * S_.sfa_elems_[ i ];
    }

    double sum_syn_pot = 0.0;
    for ( size_t i = 0; i < P_.n_receptors_(); i++ )
    {
      // computing effect of synaptic currents on membrane potential
      sum_syn_pot += V_.P21_syn_[ i ] * S_.i_syn_[ i ];
      // exponential decaying PSCs
      S_.i_syn_[ i ] = V_.P11_syn_[ i ] * S_.i_syn_[ i ];
      S_.i_syn_[ i ] += B_.spikes_[ i ].get_value( lag ); // collecting spikes
    }

    if ( S_.r_ref_ == 0 ) // neuron is not in refractory period
    {
      // effect of synaptic currents (sum_syn_pot) is added here
      S_.V_ = V_.P30_ * ( S_.I_stim_ + P_.I_e_ - S_.stc_ ) + V_.P33_ * S_.V_
        + V_.P31_ * P_.E_L_ + sum_syn_pot;

      const double lambda =
        P_.lambda_0_ * std::exp( ( S_.V_ - S_.sfa_ ) / P_.Delta_V_ );

      if ( lambda > 0.0 )
      {
        // Draw random number and compare to prob to have a spike
        // hazard function is computed by 1 - exp(- lambda * dt)
        if ( V_.rng_->drand()
          < -numerics::expm1( -lambda * Time::get_resolution().get_ms() ) )
        {

          for ( size_t i = 0; i < S_.stc_elems_.size(); i++ )
          {
            S_.stc_elems_[ i ] += P_.q_stc_[ i ];
          }

          for ( size_t i = 0; i < S_.sfa_elems_.size(); i++ )
          {
            S_.sfa_elems_[ i ] += P_.q_sfa_[ i ];
          }

          S_.r_ref_ = V_.RefractoryCounts_;

          // And send the spike event
          set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
          SpikeEvent se;
          kernel().event_delivery_manager.send( *this, se, lag );
        }
      }
    }
    else
    {
      --S_.r_ref_;         // neuron is absolute refractory
      S_.V_ = P_.V_reset_; // reset the membrane potential
    }

    // Set new input current
    S_.I_stim_ = B_.currents_.get_value( lag );

    // Voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}


void
gif_psc_exp_multisynapse::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( ( e.get_rport() > 0 )
    && ( ( size_t ) e.get_rport() <= P_.n_receptors_() ) );

  B_.spikes_[ e.get_rport() - 1 ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::gif_psc_exp_multisynapse::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // Add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::gif_psc_exp_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
