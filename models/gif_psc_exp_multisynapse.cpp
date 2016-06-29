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
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &gif_psc_exp_multisynapse::get_V_m_ );
  insert_( names::E_sfa, &gif_psc_exp_multisynapse::get_E_sfa_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::gif_psc_exp_multisynapse::Parameters_::Parameters_()
  : g_L_( 4.0 )         // nS
  , E_L_( -70.0 )       // mV
  , c_m_( 80.0 )        // pF
  , V_reset_( -55.0 )   // mV
  , delta_u_( 1.5 )     // mV
  , v_t_star_( -35 )    // mV
  , lambda0_( 10000.0 ) // Hz
  , I_e_( 0.0 )         // pA
  , t_ref_( 4.0 )       // ms
  , num_of_receptors_( 0 )
  , has_connections_( false )

{
  tau_syn_.clear();

  tau_sfa_.clear();
  q_sfa_.clear();
  tau_stc_.clear();
  q_stc_.clear();
}

nest::gif_psc_exp_multisynapse::State_::State_()
  : y0_( 0.0 )
  , y3_( -70.0 )
  , q_( 0.0 )
  , r_ref_( 0 )
  , initialized_( false )
  , add_stc_sfa_( false )
{
  i_syn_.clear();

  q_sfa_elems_.clear();
  q_stc_elems_.clear();
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
  def< double >( d, names::delta_u, delta_u_ );
  def< double >( d, names::v_t_star, v_t_star_ );
  def< double >( d, "lambda0", lambda0_ );
  def< double >( d, names::t_ref, t_ref_ );

  def< int >( d, "n_synapses", num_of_receptors_ );
  def< bool >( d, names::has_connections, has_connections_ );

  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, names::taus_syn, tau_syn_ad );

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
  updateValue< double >( d, names::delta_u, delta_u_ );
  updateValue< double >( d, names::v_t_star, v_t_star_ );
  updateValue< double >( d, "lambda0", lambda0_ );
  updateValue< double >( d, names::t_ref, t_ref_ );

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


  std::vector< double > tau_tmp;
  if ( updateValue< std::vector< double > >( d, names::taus_syn, tau_tmp ) )
  {
    for ( size_t i = 0; i < tau_tmp.size(); ++i )
    {
      if ( tau_tmp.size() < tau_syn_.size() && has_connections_ == true )
        throw BadProperty(
          "The neuron has connections, therefore the number of ports cannot be "
          "reduced." );
      if ( tau_tmp[ i ] <= 0 )
        throw BadProperty( "All synaptic time constants must be > 0." );
      if ( tau_tmp[ i ] == ( c_m_ / g_L_ ) )
        throw BadProperty(
          "Membrane and synapse time constant(s) must differ. See note in "
          "documentation." );
    }

    tau_syn_ = tau_tmp;
    num_of_receptors_ = tau_syn_.size();
  }
}

void
nest::gif_psc_exp_multisynapse::State_::get( DictionaryDatum& d,
  const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ );  // Membrane potential
  def< double >( d, names::E_sfa, q_ ); // Adaptive threshold potential
}

void
nest::gif_psc_exp_multisynapse::State_::set( const DictionaryDatum& d,
  const Parameters_& p )
{
  updateValue< double >( d, names::V_m, y3_ );
  updateValue< double >( d, names::E_sfa, q_ );
  initialized_ =
    false; // vectors of the state should be initialized with new parameter set.
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
  // S_.r_ref_ = Time(Time::ms(P_.t_ref_remaining_)).get_steps();
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

  V_.h_ = Time::get_resolution().get_ms();
  V_.rng_ = kernel().rng_manager.get_rng( get_thread() );

  double_t tau_m = P_.c_m_ / P_.g_L_;

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


  P_.receptor_types_.resize( P_.num_of_receptors_ );
  for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
  {
    P_.receptor_types_[ i ] = i + 1;
  }

  V_.P11_syn_.resize( P_.num_of_receptors_ );
  V_.P21_syn_.resize( P_.num_of_receptors_ );

  S_.i_syn_.resize( P_.num_of_receptors_ );

  B_.spikes_.resize( P_.num_of_receptors_ );

  for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
  {
    V_.P11_syn_[ i ] = std::exp( -V_.h_ / P_.tau_syn_[ i ] );
    V_.P21_syn_[ i ] = propagator_32( P_.tau_syn_[ i ], tau_m, P_.c_m_, V_.h_ );

    B_.spikes_[ i ].resize();
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::gif_psc_exp_multisynapse::update( Time const& origin,
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

      S_.q_stc_elems_[ i ] =
        V_.Q44_[ i ] * S_.q_stc_elems_[ i ]; // exponential decaying stc kernel
    }

    S_.stc_ = q_temp_;

    q_temp_ = 0;
    for ( uint_t i = 0; i < S_.q_sfa_elems_.size(); i++ )
    {

      q_temp_ += S_.q_sfa_elems_[ i ];

      S_.q_sfa_elems_[ i ] =
        V_.Q33_[ i ] * S_.q_sfa_elems_[ i ]; // exponential decaying sfa kernel
    }

    S_.q_ = q_temp_ + P_.v_t_star_;


    double_t sum_syn_pot = 0.0;
    for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
    {

      sum_syn_pot += V_.P21_syn_[ i ] * S_.i_syn_[ i ]; // computing effect of
                                                        // synaptic currents on
                                                        // membrane potential

      S_.i_syn_[ i ] =
        V_.P11_syn_[ i ] * S_.i_syn_[ i ]; // exponential decaying PSCs

      S_.i_syn_[ i ] += B_.spikes_[ i ].get_value( lag ); // collecting spikes
    }

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
        + V_.P31_ * P_.E_L_ + sum_syn_pot; // effect of synaptic currents
                                           // (sum_syn_pot) is added here

      double_t lambda =
        P_.lambda0_ * std::exp( ( S_.y3_ - S_.q_ ) / P_.delta_u_ );

      if ( lambda > 0.0 )
      {

        // Draw random number and compare to prob to have a spike
        if ( V_.rng_->drand()
          <= -numerics::expm1( -lambda * ( V_.h_ / 1000.0 ) ) )
          n_spikes = 1;
      }
    }
    else
      --S_.r_ref_; // neuron is absolute refractory


    if ( n_spikes > 0 ) // is there any spike?
    {
      S_.add_stc_sfa_ = true;


      S_.y3_ = P_.V_reset_; // reset the membrane potential
      S_.r_ref_ = V_.RefractoryCounts_;


      // must compute spike time
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      // And send the spike event
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


port
gif_psc_exp_multisynapse::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type <= 0
    || receptor_type > static_cast< port >( P_.num_of_receptors_ ) )
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );

  P_.has_connections_ = true;
  return receptor_type;
}


void
gif_psc_exp_multisynapse::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
  {
    if ( P_.receptor_types_[ i ] == e.get_rport() )
    {
      B_.spikes_[ i ].add_value(
        e.get_rel_delivery_steps(
          kernel().simulation_manager.get_slice_origin() ),
        e.get_weight() * e.get_multiplicity() );
    }
  }
}

void
nest::gif_psc_exp_multisynapse::handle( CurrentEvent& e )
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
nest::gif_psc_exp_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
