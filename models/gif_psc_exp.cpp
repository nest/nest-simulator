/*
 *  gif_psc_exp.cpp
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

#include "gif_psc_exp.h"

// C++ includes:
#include <limits>

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"

#include "numerics.h"
#include "compose.hpp"
#include "propagator_stability.h"

namespace nest
{
/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< gif_psc_exp > gif_psc_exp::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< gif_psc_exp >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &gif_psc_exp::get_V_m_ );
  insert_( names::E_sfa, &gif_psc_exp::get_E_sfa_ );
  insert_( names::input_currents_ex, &gif_psc_exp::get_input_currents_ex_ );
  insert_( names::input_currents_in, &gif_psc_exp::get_input_currents_in_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::gif_psc_exp::Parameters_::Parameters_()
  : g_L_( 4.0 )         // nS
  , E_L_( -70.0 )       // mV
  , c_m_( 80.0 )        // pF
  , V_reset_( -55.0 )   // mV
  , delta_u_( 1.5 )     // mV
  , v_t_star_( -35 )    // mV
  , lambda0_( 10000.0 ) // Hz
  , I_e_( 0.0 )         // pA
  , t_ref_( 4.0 )       // ms
  , tau_ex_( 2.0 )      // in ms
  , tau_in_( 2.0 )      // in ms
{
  tau_sfa_.clear();
  q_sfa_.clear();
  tau_stc_.clear();
  q_stc_.clear();
}

nest::gif_psc_exp::State_::State_()
  : y0_( 0.0 )
  , y3_( -70.0 )
  , q_( 0.0 )
  , r_ref_( 0 )
  , i_syn_ex_( 0.0 )
  , i_syn_in_( 0.0 )
  , initialized_( false )
  , add_stc_sfa_( false )
{
  q_sfa_elems_.clear();
  q_stc_elems_.clear();
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::gif_psc_exp::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::g_L, g_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::delta_u, delta_u_ );
  def< double >( d, names::v_t_star, v_t_star_ );
  def< double >( d, "lambda0", lambda0_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );

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
nest::gif_psc_exp::Parameters_::set( const DictionaryDatum& d )
{

  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::E_L, E_L_ );
  updateValue< double >( d, names::g_L, g_L_ );
  updateValue< double >( d, names::C_m, c_m_ );
  updateValue< double >( d, names::V_reset, V_reset_ );
  updateValue< double >( d, names::delta_u, delta_u_ );
  updateValue< double >( d, names::v_t_star, v_t_star_ );
  updateValue< double >( d, "lambda0", lambda0_ );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::tau_syn_ex, tau_ex_ );
  updateValue< double >( d, names::tau_syn_in, tau_in_ );

  updateValue< std::vector< double > >( d, names::tau_sfa, tau_sfa_ );
  updateValue< std::vector< double > >( d, names::q_sfa, q_sfa_ );
  updateValue< std::vector< double > >( d, names::tau_stc, tau_stc_ );
  updateValue< std::vector< double > >( d, names::q_stc, q_stc_ );


  if ( tau_sfa_.size() != q_sfa_.size() )
    throw BadProperty( String::compose(
      "'tau_sfa' and 'q_sfa' need to have the same dimensions.\nSize of "
      "tau_sfa: %1\nSize of q_sfa: %2",
      tau_sfa_.size(),
      q_sfa_.size() ) );

  if ( tau_stc_.size() != q_stc_.size() )
    throw BadProperty( String::compose(
      "'tau_stc' and 'q_stc' need to have the same dimensions.\nSize of "
      "tau_stc: %1\nSize of q_stc: %2",
      tau_stc_.size(),
      q_stc_.size() ) );

  if ( g_L_ <= 0 )
    throw BadProperty( "Membrane conductance must be strictly positive." );

  if ( delta_u_ <= 0 )
    throw BadProperty( "delta_u must be strictly positive." );

  if ( c_m_ <= 0 )
    throw BadProperty( "Capacitance must be strictly positive." );

  if ( t_ref_ < 0 )
    throw BadProperty( "Refractory time must not be negative." );

  for ( uint_t i = 0; i < tau_sfa_.size(); i++ )
    if ( tau_sfa_[ i ] <= 0 )
      throw BadProperty( "All time constants must be strictly positive." );

  for ( uint_t i = 0; i < tau_stc_.size(); i++ )
    if ( tau_stc_[ i ] <= 0 )
      throw BadProperty( "All time constants must be strictly positive." );


  if ( tau_ex_ <= 0 || tau_in_ <= 0 )
    throw BadProperty( "Synapse time constants must be strictly positive." );
}

void
nest::gif_psc_exp::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ );  // Membrane potential
  def< double >( d, names::E_sfa, q_ ); // Adaptive threshold potential
}

void
nest::gif_psc_exp::State_::set( const DictionaryDatum& d, const Parameters_& p )
{
  updateValue< double >( d, names::V_m, y3_ );
  updateValue< double >( d, names::E_sfa, q_ );
  initialized_ =
    false; // vectors of the state should be initialized with new parameter set.
}

nest::gif_psc_exp::Buffers_::Buffers_( gif_psc_exp& n )
  : logger_( n )
{
}

nest::gif_psc_exp::Buffers_::Buffers_( const Buffers_&, gif_psc_exp& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::gif_psc_exp::gif_psc_exp()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::gif_psc_exp::gif_psc_exp( const gif_psc_exp& n )
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
nest::gif_psc_exp::init_state_( const Node& proto )
{
  const gif_psc_exp& pr = downcast< gif_psc_exp >( proto );
  S_ = pr.S_;
  // S_.r_ref_ = Time(Time::ms(P_.t_ref_remaining_)).get_steps();
}

void
nest::gif_psc_exp::init_buffers_()
{
  B_.spikes_ex_.clear(); // includes resize
  B_.spikes_in_.clear(); // includes resize
  B_.currents_.clear();  //!< includes resize
  B_.logger_.reset();    //!< includes resize
  Archiving_Node::clear_history();
}

void
nest::gif_psc_exp::calibrate()
{

  B_.logger_.init();

  V_.h_ = Time::get_resolution().get_ms();
  V_.rng_ = kernel().rng_manager.get_rng( get_thread() );

  V_.P11ex_ = std::exp( -V_.h_ / P_.tau_ex_ );
  V_.P11in_ = std::exp( -V_.h_ / P_.tau_in_ );

  double_t tau_m = P_.c_m_ / P_.g_L_;

  // these are determined according to a numeric stability criterion
  V_.P21ex_ = propagator_32( P_.tau_ex_, tau_m, P_.c_m_, V_.h_ );
  V_.P21in_ = propagator_32( P_.tau_in_, tau_m, P_.c_m_, V_.h_ );


  V_.P33_ = std::exp( -V_.h_ / tau_m );
  V_.P30_ = 1 / P_.c_m_ * ( 1 - V_.P33_ ) * tau_m;
  V_.P31_ = ( 1 - V_.P33_ );

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  assert( V_.RefractoryCounts_
    >= 0 ); // since t_ref_ >= 0, this can only fail in error

  // initializing internal state
  if ( !S_.initialized_ )
  {
    for ( uint_t i = 0; i < P_.tau_sfa_.size(); i++ )
    {
      V_.Q33_.push_back( std::exp( -V_.h_ / P_.tau_sfa_[ i ] ) );
      S_.q_sfa_elems_.push_back( 0.0 );
    }

    for ( uint_t i = 0; i < P_.tau_stc_.size(); i++ )
    {
      V_.Q44_.push_back( std::exp( -V_.h_ / P_.tau_stc_[ i ] ) );
      S_.q_stc_elems_.push_back( 0.0 );
    }

    S_.initialized_ = true;
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::gif_psc_exp::update( Time const& origin,
  const long_t from,
  const long_t to )
{

  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  double_t q_temp_;

  for ( long_t lag = from; lag < to; ++lag )
  {

    q_temp_ = 0;
    for ( uint_t i = 0; i < S_.q_stc_elems_.size(); i++ )
    {

      q_temp_ += S_.q_stc_elems_[ i ];

      S_.q_stc_elems_[ i ] = V_.Q44_[ i ] * S_.q_stc_elems_[ i ];
    }

    S_.stc_ = q_temp_;

    q_temp_ = 0;
    for ( uint_t i = 0; i < S_.q_sfa_elems_.size(); i++ )
    {

      q_temp_ += S_.q_sfa_elems_[ i ];

      S_.q_sfa_elems_[ i ] = V_.Q33_[ i ] * S_.q_sfa_elems_[ i ];
    }

    S_.q_ = q_temp_ + P_.v_t_star_;

    ulong_t n_spikes = 0;

    if ( S_.r_ref_ == 0 ) // neuron not refractory, so evolve V
    {

      if ( S_.add_stc_sfa_ == true )
      {

        S_.add_stc_sfa_ = false;


        q_temp_ = 0;

        for ( uint_t i = 0; i < S_.q_stc_elems_.size(); i++ )
        {
          S_.q_stc_elems_[ i ] += P_.q_stc_[ i ];

          q_temp_ += P_.q_stc_[ i ];
        }

        S_.stc_ += q_temp_;


        q_temp_ = 0;

        for ( uint_t i = 0; i < S_.q_sfa_elems_.size(); i++ )
        {

          S_.q_sfa_elems_[ i ] += P_.q_sfa_[ i ];

          q_temp_ += P_.q_sfa_[ i ];
        }

        S_.q_ += q_temp_;
      }

      S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ - S_.stc_ ) + V_.P33_ * S_.y3_
        + V_.P31_ * P_.E_L_ + S_.i_syn_ex_ * V_.P21ex_
        + S_.i_syn_in_ * V_.P21in_;

      double_t lambda =
        P_.lambda0_ * std::exp( ( S_.y3_ - S_.q_ ) / P_.delta_u_ );

      if ( lambda > 0.0 )
      {

        // Draw random number and compare to prob to have a spike
        if ( V_.rng_->drand()
          <= -numerics::expm1( -lambda * ( V_.h_ / 1000.0 ) ) )
        {
          n_spikes = 1;
        }
      }
    }
    else
      --S_.r_ref_; // neuron is absolute refractory


    // exponential decaying PSCs
    S_.i_syn_ex_ *= V_.P11ex_;
    S_.i_syn_in_ *= V_.P11in_;

    S_.i_syn_ex_ += B_.spikes_ex_.get_value( lag );
    S_.i_syn_in_ += B_.spikes_in_.get_value( lag );

    if ( n_spikes > 0 ) // is there any spike?
    {

      S_.add_stc_sfa_ = true;

      S_.y3_ = P_.V_reset_; // reset the membrane potential
      S_.r_ref_ = V_.RefractoryCounts_;

      // And send the spike event
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      se.set_multiplicity( n_spikes );
      kernel().event_delivery_manager.send( *this, se, lag );
    }


    // Set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // Voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::gif_psc_exp::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  // EX: We must compute the arrival time of the incoming spike
  //     explicitly, since it depends on delay and offset within
  //     the update cycle.  The way it is done here works, but
  //     is clumsy and should be improved.
  if ( e.get_weight() >= 0.0 )
    B_.spikes_ex_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  else
    B_.spikes_in_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
}

void
nest::gif_psc_exp::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t c = e.get_current();
  const double_t w = e.get_weight();

  // Add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::gif_psc_exp::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
