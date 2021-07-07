/*
 *  pp_pop_psc_delta.cpp
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

#include "pp_pop_psc_delta.h"

// C++ includes:
#include <algorithm>
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "compose.hpp"
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

namespace nest
{
/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< pp_pop_psc_delta > pp_pop_psc_delta::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< pp_pop_psc_delta >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &pp_pop_psc_delta::get_V_m_ );
  // n_events instead of E_sfa
  insert_( names::n_events, &pp_pop_psc_delta::get_n_events_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::pp_pop_psc_delta::Parameters_::Parameters_()
  : N_( 100 )
  , tau_m_( 10.0 )  // ms
  , c_m_( 250.0 )   // pF
  , rho_0_( 10.0 )  // 1/s
  , delta_u_( 1.0 ) // mV
  , len_kernel_( 5.0 )
  , I_e_( 0.0 ) // pA
{
  tau_eta_.push_back( 10.0 );
  val_eta_.push_back( 0.0 );
}

nest::pp_pop_psc_delta::State_::State_()
  : y0_( 0.0 )
  , h_( 0.0 )
  , p_age_occupations_( 0 )
  , p_n_spikes_past_( 0 )
  , initialized_( false )
{
  age_occupations_.clear();
  thetas_ages_.clear();
  n_spikes_past_.clear();
  n_spikes_ages_.clear();
  rhos_ages_.clear();
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::pp_pop_psc_delta::Parameters_::get( DictionaryDatum& d ) const
{
  def< long >( d, names::N, N_ );
  def< double >( d, names::rho_0, rho_0_ );
  def< double >( d, names::delta_u, delta_u_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::len_kernel, len_kernel_ );

  ArrayDatum tau_eta_list_ad( tau_eta_ );
  def< ArrayDatum >( d, names::tau_eta, tau_eta_list_ad );

  ArrayDatum val_eta_list_ad( val_eta_ );
  def< ArrayDatum >( d, names::val_eta, val_eta_list_ad );
}

void
nest::pp_pop_psc_delta::Parameters_::set( const DictionaryDatum& d, Node* node )
{

  updateValueParam< long >( d, names::N, N_, node );
  updateValueParam< double >( d, names::rho_0, rho_0_, node );
  updateValueParam< double >( d, names::delta_u, delta_u_, node );
  updateValueParam< double >( d, names::len_kernel, len_kernel_, node );

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, c_m_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValue< std::vector< double > >( d, names::tau_eta, tau_eta_ );
  updateValue< std::vector< double > >( d, names::val_eta, val_eta_ );


  if ( tau_eta_.size() != val_eta_.size() )
  {
    throw BadProperty( String::compose(
      "'tau_eta' and 'val_eta' need to have the same dimension.\nSize of "
      "tau_eta: %1\nSize of val_eta: %2",
      tau_eta_.size(),
      val_eta_.size() ) );
  }
  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "The time constants must be strictly positive." );
  }

  for ( unsigned int i = 0; i < tau_eta_.size(); i++ )
  {
    if ( tau_eta_[ i ] <= 0 )
    {
      throw BadProperty( "All time constants must be strictly positive." );
    }
  }
  if ( N_ <= 0 )
  {
    throw BadProperty( "Number of neurons must be positive." );
  }
  if ( rho_0_ < 0 )
  {
    throw BadProperty( "Rho_0 cannot be negative." );
  }
  if ( delta_u_ <= 0 )
  {
    throw BadProperty( "Delta_u must be positive." );
  }
}

void
nest::pp_pop_psc_delta::State_::get( DictionaryDatum& d, const Parameters_& ) const
{
  def< double >( d, names::V_m, h_ ); // Filterd version of input
  int n_spikes = n_spikes_past_.size() > 0 ? n_spikes_past_[ p_n_spikes_past_ ]
                                           : 0; // return 0 if n_spikes_past_ has not been initialized yet
  def< long >( d, names::n_events, n_spikes );  // Number of generated spikes
}

void
nest::pp_pop_psc_delta::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, h_, node );
  initialized_ = false; // vectors of the state should be initialized with new parameter set.
}

nest::pp_pop_psc_delta::Buffers_::Buffers_( pp_pop_psc_delta& n )
  : logger_( n )
{
}

nest::pp_pop_psc_delta::Buffers_::Buffers_( const Buffers_&, pp_pop_psc_delta& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::pp_pop_psc_delta::pp_pop_psc_delta()
  : Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::pp_pop_psc_delta::pp_pop_psc_delta( const pp_pop_psc_delta& n )
  : Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::pp_pop_psc_delta::init_buffers_()
{
  B_.spikes_.clear();   //!< includes resize
  B_.currents_.clear(); //!< includes resize
  B_.logger_.reset();   //!< includes resize
}


void
nest::pp_pop_psc_delta::calibrate()
{

  if ( P_.tau_eta_.size() == 0 )
  {
    throw BadProperty( "Time constant array should not be empty. " );
  }

  if ( P_.val_eta_.size() == 0 )
  {
    throw BadProperty( "Adaptation value array should not be empty. " );
  }

  B_.logger_.init();

  V_.h_ = Time::get_resolution().get_ms();
  V_.rng_ = get_vp_specific_rng( get_thread() );
  V_.min_double_ = std::numeric_limits< double >::min();

  double tau_eta_max = -1; // finding max of tau_eta_

  for ( unsigned int j = 0; j < P_.tau_eta_.size(); j++ )
  {
    if ( P_.tau_eta_.at( j ) > tau_eta_max )
    {
      tau_eta_max = P_.tau_eta_.at( j );
    }
  }

  V_.len_eta_ = tau_eta_max * ( P_.len_kernel_ / V_.h_ );

  V_.P33_ = std::exp( -V_.h_ / P_.tau_m_ );
  V_.P30_ = 1 / P_.c_m_ * ( 1 - V_.P33_ ) * P_.tau_m_;

  // initializing internal state
  if ( not S_.initialized_ )
  {

    V_.len_eta_ = tau_eta_max * ( P_.len_kernel_ / V_.h_ );

    for ( int j = 0; j < V_.len_eta_; j++ )
    {
      S_.n_spikes_past_.push_back( 0 );
    }

    std::vector< double > ts;
    ts.clear();
    for ( int j = 0; j < V_.len_eta_; j++ )
    {
      ts.push_back( j * V_.h_ );
    }

    double temp = 0;

    for ( int j = 0; j < V_.len_eta_; j++ )
    {
      for ( unsigned int i = 0; i < P_.tau_eta_.size(); i++ )
      {
        temp += std::exp( -ts[ j ] / P_.tau_eta_.at( i ) ) * ( -P_.val_eta_.at( i ) );
      }

      V_.theta_kernel_.push_back( temp );
      V_.eta_kernel_.push_back( std::exp( temp ) - 1 );
      temp = 0;
    }

    for ( int j = 0; j < V_.len_eta_; j++ )
    {
      S_.age_occupations_.push_back( 0 );
      S_.thetas_ages_.push_back( 0 );
      S_.n_spikes_ages_.push_back( 0 );
      S_.rhos_ages_.push_back( 0 );
    }
    S_.age_occupations_.push_back( P_.N_ );
    S_.thetas_ages_.push_back( 0 );
    S_.n_spikes_ages_.push_back( 0 );
    S_.rhos_ages_.push_back( 0 );

    S_.initialized_ = true;
  }
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::pp_pop_psc_delta::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {


    S_.h_ = S_.h_ * V_.P33_ + V_.P30_ * ( S_.y0_ + P_.I_e_ ) + B_.spikes_.get_value( lag );


    // get_thetas_ages
    std::vector< double > tmp_vector;
    double integral = 0;
    tmp_vector.clear();


    for ( unsigned int i = 0; i < V_.eta_kernel_.size(); i++ )
    {
      tmp_vector.push_back( V_.eta_kernel_[ i ]
        * S_.n_spikes_past_[ ( S_.p_n_spikes_past_ + i ) % S_.n_spikes_past_.size() ] * V_.h_ * 0.001 );
      integral += tmp_vector[ i ];
    }

    S_.thetas_ages_.clear();
    S_.thetas_ages_.push_back( integral );

    for ( unsigned int i = 1; i < V_.eta_kernel_.size(); i++ )
    {
      S_.thetas_ages_.push_back( S_.thetas_ages_[ i - 1 ] - tmp_vector[ i - 1 ] );
    }

    for ( unsigned int i = 0; i < V_.eta_kernel_.size(); i++ )
    {
      S_.thetas_ages_[ i ] += V_.theta_kernel_[ i ];
    }

    S_.thetas_ages_.push_back( 0 );

    // get_escape_rate
    for ( unsigned int i = 0; i < S_.rhos_ages_.size(); i++ )
    {
      S_.rhos_ages_[ i ] = P_.rho_0_ * std::exp( ( S_.h_ + S_.thetas_ages_[ i ] ) / P_.delta_u_ );
    }


    double p_argument;

    // generate_spikes
    for ( unsigned int i = 0; i < S_.age_occupations_.size(); i++ )
    {

      if ( S_.age_occupations_[ ( S_.p_age_occupations_ + i ) % S_.age_occupations_.size() ] > 0 )
      {

        p_argument = -numerics::expm1( -S_.rhos_ages_[ i ] * V_.h_ * 0.001 ); // V_.h_ is in ms, S_.rhos_ages_ is in Hz

        if ( p_argument > V_.min_double_ )
        {
          const auto n = S_.age_occupations_[ ( S_.p_age_occupations_ + i ) % S_.age_occupations_.size() ];
          binomial_distribution::param_type param( n, p_argument );
          S_.n_spikes_ages_[ i ] = V_.bino_dist_( V_.rng_, param );
        }
        else
        {
          S_.n_spikes_ages_[ i ] = 0;
        }
      }
      else
      {
        S_.n_spikes_ages_[ i ] = 0;
      }
    }


    S_.p_n_spikes_past_ =
      ( S_.p_n_spikes_past_ - 1 + S_.n_spikes_past_.size() ) % S_.n_spikes_past_.size(); // shift to the right

    int temp_sum = 0;
    for ( unsigned int i = 0; i < S_.n_spikes_ages_.size(); i++ ) // cumulative sum
    {
      temp_sum += S_.n_spikes_ages_[ i ];
    }

    S_.n_spikes_past_[ S_.p_n_spikes_past_ ] = temp_sum;


    // update_age_occupations
    for ( unsigned int i = 0; i < S_.age_occupations_.size(); i++ )
    {
      S_.age_occupations_[ ( S_.p_age_occupations_ + i ) % S_.age_occupations_.size() ] -= S_.n_spikes_ages_[ i ];
    }

    int last_element_value = S_.age_occupations_[ ( S_.p_age_occupations_ - 1 + S_.age_occupations_.size() )
      % S_.age_occupations_.size() ]; // save the last element

    S_.p_age_occupations_ =
      ( S_.p_age_occupations_ - 1 + S_.age_occupations_.size() ) % S_.age_occupations_.size(); // shift to the right
    S_.age_occupations_[ ( S_.p_age_occupations_ - 1 + S_.age_occupations_.size() ) % S_.age_occupations_.size() ] +=
      last_element_value;
    S_.age_occupations_[ S_.p_age_occupations_ ] = S_.n_spikes_past_[ S_.p_n_spikes_past_ ];

    // Set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // Voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );


    // test if S_.n_spikes_past_[S_.p_n_spikes_past_]!=0, generate spike and
    // send this number as the parameter

    if ( S_.n_spikes_past_[ S_.p_n_spikes_past_ ] > 0 ) // Is there any spike?
    {
      SpikeEvent se;
      se.set_multiplicity( S_.n_spikes_past_[ S_.p_n_spikes_past_ ] );
      kernel().event_delivery_manager.send( *this, se, lag );
    }
  }
}

void
nest::pp_pop_psc_delta::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // EX: We must compute the arrival time of the incoming spike
  //     explicitly, since it depends on delay and offset within
  //     the update cycle.  The way it is done here works, but
  //     is clumsy and should be improved.
  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::pp_pop_psc_delta::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // Add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::pp_pop_psc_delta::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
