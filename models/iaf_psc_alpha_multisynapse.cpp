/*
 *  iaf_psc_alpha_multisynapse.cpp
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

#include "exceptions.h"
#include "iaf_psc_alpha_multisynapse.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"
#include "propagator_stability.h"

#include <limits>

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_psc_alpha_multisynapse >
  nest::iaf_psc_alpha_multisynapse::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_psc_alpha_multisynapse >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_psc_alpha_multisynapse::get_V_m_ );
  insert_( names::currents, &iaf_psc_alpha_multisynapse::get_current_ );
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

iaf_psc_alpha_multisynapse::Parameters_::Parameters_()
  : Tau_( 10.0 )            // ms
  , C_( 250.0 )             // pF
  , TauR_( 2.0 )            // ms
  , U0_( -70.0 )            // mV
  , I_e_( 0.0 )             // pA
  , V_reset_( -70.0 - U0_ ) // mV, rel to U0_
  , Theta_( -55.0 - U0_ )   // mV, rel to U0_
  , LowerBound_( -std::numeric_limits< double_t >::infinity() )
  , has_connections_( false )
{
  tau_syn_.clear();
}

iaf_psc_alpha_multisynapse::State_::State_()
  : y0_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
{
  y1_syn_.clear();
  y2_syn_.clear();
}


/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
iaf_psc_alpha_multisynapse::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, U0_ ); // resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + U0_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + U0_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::t_ref, TauR_ );
  def< double >( d, names::V_min, LowerBound_ + U0_ );
  def< int >( d, "n_synapses", num_of_receptors_ );
  def< bool >( d, names::has_connections, has_connections_ );

  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, "tau_syn", tau_syn_ad );
}

double
iaf_psc_alpha_multisynapse::Parameters_::set( const DictionaryDatum& d )
{
  // if U0_ is changed, we need to adjust all variables defined relative to U0_
  const double ELold = U0_;
  updateValue< double >( d, names::E_L, U0_ );
  const double delta_EL = U0_ - ELold;

  if ( updateValue< double >( d, names::V_reset, V_reset_ ) )
    V_reset_ -= U0_;
  else
    V_reset_ -= delta_EL;

  if ( updateValue< double >( d, names::V_th, Theta_ ) )
    Theta_ -= U0_;
  else
    Theta_ -= delta_EL;

  if ( updateValue< double >( d, names::V_min, LowerBound_ ) )
    LowerBound_ -= U0_;
  else
    LowerBound_ -= delta_EL;

  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::C_m, C_ );
  updateValue< double >( d, names::tau_m, Tau_ );
  updateValue< double >( d, names::t_ref, TauR_ );

  if ( C_ <= 0 )
    throw BadProperty( "Capacitance must be > 0." );

  if ( Tau_ <= 0. )
    throw BadProperty( "Membrane time constant must be > 0." );

  std::vector< double > tau_tmp;
  if ( updateValue< std::vector< double > >( d, "tau_syn", tau_tmp ) )
  {
    for ( size_t i = 0; i < tau_tmp.size(); ++i )
    {
      if ( tau_tmp.size() < tau_syn_.size() && has_connections_ == true )
        throw BadProperty(
          "The neuron has connections, therefore the number of ports cannot be reduced." );
      if ( tau_tmp[ i ] <= 0 )
        throw BadProperty( "All synaptic time constants must be > 0." );
    }

    tau_syn_ = tau_tmp;
    num_of_receptors_ = tau_syn_.size();
  }

  if ( TauR_ < 0. )
    throw BadProperty( "The refractory time t_ref can't be negative." );

  if ( V_reset_ >= Theta_ )
    throw BadProperty( "Reset potential must be smaller than threshold." );

  return delta_EL;
}

void
iaf_psc_alpha_multisynapse::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.U0_ ); // Membrane potential
}

void
iaf_psc_alpha_multisynapse::State_::set( const DictionaryDatum& d,
  const Parameters_& p,
  const double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, y3_ ) )
    y3_ -= p.U0_;
  else
    y3_ -= delta_EL;
}

iaf_psc_alpha_multisynapse::Buffers_::Buffers_( iaf_psc_alpha_multisynapse& n )
  : logger_( n )
{
}

iaf_psc_alpha_multisynapse::Buffers_::Buffers_( const Buffers_&, iaf_psc_alpha_multisynapse& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

iaf_psc_alpha_multisynapse::iaf_psc_alpha_multisynapse()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

iaf_psc_alpha_multisynapse::iaf_psc_alpha_multisynapse( const iaf_psc_alpha_multisynapse& n )
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
iaf_psc_alpha_multisynapse::init_state_( const Node& proto )
{
  const iaf_psc_alpha_multisynapse& pr = downcast< iaf_psc_alpha_multisynapse >( proto );
  S_ = pr.S_;
}

void
iaf_psc_alpha_multisynapse::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize

  B_.logger_.reset();

  Archiving_Node::clear_history();
}

void
iaf_psc_alpha_multisynapse::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  const double h = Time::get_resolution().get_ms();

  P_.receptor_types_.resize( P_.num_of_receptors_ );
  for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
  {
    P_.receptor_types_[ i ] = i + 1;
  }

  V_.P11_syn_.resize( P_.num_of_receptors_ );
  V_.P21_syn_.resize( P_.num_of_receptors_ );
  V_.P22_syn_.resize( P_.num_of_receptors_ );
  V_.P31_syn_.resize( P_.num_of_receptors_ );
  V_.P32_syn_.resize( P_.num_of_receptors_ );

  S_.y1_syn_.resize( P_.num_of_receptors_ );
  S_.y2_syn_.resize( P_.num_of_receptors_ );

  V_.PSCInitialValues_.resize( P_.num_of_receptors_ );

  B_.spikes_.resize( P_.num_of_receptors_ );

  V_.P33_ = std::exp( -h / P_.Tau_ );
  V_.P30_ = 1 / P_.C_ * ( 1 - V_.P33_ ) * P_.Tau_;

  for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
  {
    V_.P11_syn_[ i ] = V_.P22_syn_[ i ] = std::exp( -h / P_.tau_syn_[ i ] );
    V_.P21_syn_[ i ] = h * V_.P11_syn_[ i ];

    // these are determined according to a numeric stability criterion
    V_.P31_syn_[ i ] = propagator_31( P_.tau_syn_[ i ], P_.Tau_, P_.C_, h );
    V_.P32_syn_[ i ] = propagator_32( P_.tau_syn_[ i ], P_.Tau_, P_.C_, h );

    V_.PSCInitialValues_[ i ] = 1.0 * numerics::e / P_.tau_syn_[ i ];
    B_.spikes_[ i ].resize();
  }

  Time r = Time::ms( P_.TauR_ );
  V_.RefractoryCounts_ = r.get_steps();

  if ( V_.RefractoryCounts_ < 1 )
    throw BadProperty( "Absolute refractory time must be at least one time step." );
}

void
iaf_psc_alpha_multisynapse::update( Time const& origin, const long_t from, const long_t to )
{
  assert( to >= 0 && ( delay ) from < Scheduler::get_min_delay() );
  assert( from < to );

  for ( long_t lag = from; lag < to; ++lag )
  {
    if ( S_.r_ == 0 )
    {
      // neuron not refractory
      S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_;

      S_.current_ = 0.0;
      for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
      {
        S_.y3_ += V_.P31_syn_[ i ] * S_.y1_syn_[ i ] + V_.P32_syn_[ i ] * S_.y2_syn_[ i ];
        S_.current_ += S_.y2_syn_[ i ];
      }

      // lower bound of membrane potential
      S_.y3_ = ( S_.y3_ < P_.LowerBound_ ? P_.LowerBound_ : S_.y3_ );
    }
    else // neuron is absolute refractory
      --S_.r_;

    for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
    {
      // alpha shape PSCs
      S_.y2_syn_[ i ] = V_.P21_syn_[ i ] * S_.y1_syn_[ i ] + V_.P22_syn_[ i ] * S_.y2_syn_[ i ];
      S_.y1_syn_[ i ] *= V_.P11_syn_[ i ];

      // collect spikes
      S_.y1_syn_[ i ] += V_.PSCInitialValues_[ i ] * B_.spikes_[ i ].get_value( lag );
    }

    if ( S_.y3_ >= P_.Theta_ ) // threshold crossing
    {
      S_.r_ = V_.RefractoryCounts_;
      S_.y3_ = P_.V_reset_;
      // A supra-threshold membrane potential should never be observable.
      // The reset at the time of threshold crossing enables accurate integration
      // independent of the computation step size, see [2,3] for details.

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      network()->send( *this, se, lag );
    }

    // set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

port
iaf_psc_alpha_multisynapse::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type <= 0 || receptor_type > static_cast< port >( P_.num_of_receptors_ ) )
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );

  P_.has_connections_ = true;
  return receptor_type;
}

void
iaf_psc_alpha_multisynapse::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
  {
    if ( P_.receptor_types_[ i ] == e.get_rport() )
    {
      B_.spikes_[ i ].add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ),
        e.get_weight() * e.get_multiplicity() );
    }
  }
}

void
iaf_psc_alpha_multisynapse::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t I = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ), w * I );
}

void
iaf_psc_alpha_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
