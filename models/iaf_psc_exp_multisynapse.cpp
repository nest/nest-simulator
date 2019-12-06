/*
 *  iaf_psc_exp_multisynapse.cpp
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

#include "iaf_psc_exp_multisynapse.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"
#include "propagator_stability.h"

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

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
DynamicRecordablesMap< iaf_psc_exp_multisynapse >::create( iaf_psc_exp_multisynapse& host )
{
  // use standard names wherever you can for consistency!
  insert( names::V_m, host.get_data_access_functor( iaf_psc_exp_multisynapse::State_::V_M ) );

  insert( names::I_syn, host.get_data_access_functor( iaf_psc_exp_multisynapse::State_::I ) );

  host.insert_current_recordables();
}

Name
iaf_psc_exp_multisynapse::get_i_syn_name( size_t elem )
{
  std::stringstream i_syn_name;
  i_syn_name << "I_syn_" << elem + 1;
  return Name( i_syn_name.str() );
}

void
iaf_psc_exp_multisynapse::insert_current_recordables( size_t first )
{
  for ( size_t receptor = first; receptor < P_.tau_syn_.size(); ++receptor )
  {
    size_t elem = iaf_psc_exp_multisynapse::State_::I_SYN
      + receptor * iaf_psc_exp_multisynapse::State_::NUM_STATE_ELEMENTS_PER_RECEPTOR;
    recordablesMap_.insert( get_i_syn_name( receptor ), this->get_data_access_functor( elem ) );
  }
}

DataAccessFunctor< iaf_psc_exp_multisynapse >
iaf_psc_exp_multisynapse::get_data_access_functor( size_t elem )
{
  return DataAccessFunctor< iaf_psc_exp_multisynapse >( *this, elem );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

iaf_psc_exp_multisynapse::Parameters_::Parameters_()
  : Tau_( 10.0 )             // in ms
  , C_( 250.0 )              // in pF
  , refractory_time_( 2.0 )  // in ms
  , E_L_( -70.0 )            // in mV
  , I_e_( 0.0 )              // in pA
  , V_reset_( -70.0 - E_L_ ) // in mV
  , Theta_( -55.0 - E_L_ )   // relative E_L_
  , tau_syn_( 1, 2.0 )       // in ms
  , has_connections_( false )
{
}

iaf_psc_exp_multisynapse::State_::State_()
  : I_const_( 0.0 )
  , V_m_( 0.0 )
  , current_( 0.0 )
  , refractory_steps_( 0 )
{
  i_syn_.clear();
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
iaf_psc_exp_multisynapse::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + E_L_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + E_L_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::t_ref, refractory_time_ );
  def< int >( d, names::n_synapses, n_receptors_() );
  def< bool >( d, names::has_connections, has_connections_ );

  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, names::tau_syn, tau_syn_ad );
}

double
iaf_psc_exp_multisynapse::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  if ( updateValueParam< double >( d, names::V_reset, V_reset_, node ) )
  {
    V_reset_ -= E_L_;
  }
  else
  {
    V_reset_ -= delta_EL;
  }
  if ( updateValueParam< double >( d, names::V_th, Theta_, node ) )
  {
    Theta_ -= E_L_;
  }
  else
  {
    Theta_ -= delta_EL;
  }

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, C_, node );
  updateValueParam< double >( d, names::tau_m, Tau_, node );
  updateValueParam< double >( d, names::t_ref, refractory_time_, node );

  if ( C_ <= 0 )
  {
    throw BadProperty( "Capacitance must be > 0." );
  }
  if ( Tau_ <= 0. )
  {
    throw BadProperty( "Membrane time constant must be strictly positive." );
  }
  const size_t old_n_receptors = this->n_receptors_();
  if ( updateValue< std::vector< double > >( d, "tau_syn", tau_syn_ ) )
  {
    if ( this->n_receptors_() != old_n_receptors && has_connections_ == true )
    {
      throw BadProperty( "The neuron has connections, therefore the number of ports cannot be reduced." );
    }
    for ( size_t i = 0; i < tau_syn_.size(); ++i )
    {
      if ( tau_syn_[ i ] <= 0 )
      {
        throw BadProperty( "All synaptic time constants must be strictly positive." );
      }
      if ( tau_syn_[ i ] == Tau_ )
      {
        throw BadProperty( "Membrane and synapse time constant(s) must differ. See note in documentation." );
      }
    }
  }

  if ( refractory_time_ < 0. )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }
  if ( V_reset_ >= Theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  return delta_EL;
}

void
iaf_psc_exp_multisynapse::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, V_m_ + p.E_L_ ); // Membrane potential
}

void
iaf_psc_exp_multisynapse::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  // If the dictionary contains a value for the membrane potential, V_m, adjust
  // it with the resting potential, E_L_. If not, adjust the membrane potential
  // with the provided change in resting potential.
  if ( updateValueParam< double >( d, names::V_m, V_m_, node ) )
  {
    V_m_ -= p.E_L_;
  }
  else
  {
    V_m_ -= delta_EL;
  }
}

iaf_psc_exp_multisynapse::Buffers_::Buffers_( iaf_psc_exp_multisynapse& n )
  : logger_( n )
{
}

iaf_psc_exp_multisynapse::Buffers_::Buffers_( const Buffers_&, iaf_psc_exp_multisynapse& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

iaf_psc_exp_multisynapse::iaf_psc_exp_multisynapse()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create( *this );
}

iaf_psc_exp_multisynapse::iaf_psc_exp_multisynapse( const iaf_psc_exp_multisynapse& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  recordablesMap_.create( *this );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
iaf_psc_exp_multisynapse::init_state_( const Node& proto )
{
  const iaf_psc_exp_multisynapse& pr = downcast< iaf_psc_exp_multisynapse >( proto );
  S_ = pr.S_;
}

void
iaf_psc_exp_multisynapse::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize

  B_.logger_.reset();

  Archiving_Node::clear_history();
}

void
nest::iaf_psc_exp_multisynapse::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  V_.P11_syn_.resize( P_.n_receptors_() );
  V_.P21_syn_.resize( P_.n_receptors_() );

  S_.i_syn_.resize( P_.n_receptors_() );

  B_.spikes_.resize( P_.n_receptors_() );

  V_.P22_ = std::exp( -h / P_.Tau_ );
  V_.P20_ = P_.Tau_ / P_.C_ * ( 1.0 - V_.P22_ );

  for ( size_t i = 0; i < P_.n_receptors_(); i++ )
  {
    V_.P11_syn_[ i ] = std::exp( -h / P_.tau_syn_[ i ] );
    // these are determined according to a numeric stability criterion
    V_.P21_syn_[ i ] = propagator_32( P_.tau_syn_[ i ], P_.Tau_, P_.C_, h );

    B_.spikes_[ i ].resize();
  }

  V_.RefractoryCounts_ = Time( Time::ms( P_.refractory_time_ ) ).get_steps();
}

void
iaf_psc_exp_multisynapse::update( const Time& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // evolve from timestep 'from' to timestep 'to' with steps of h each
  for ( long lag = from; lag < to; ++lag )
  {
    if ( S_.refractory_steps_ == 0 ) // neuron not refractory, so evolve V
    {
      S_.V_m_ = S_.V_m_ * V_.P22_ + ( P_.I_e_ + S_.I_const_ ) * V_.P20_; // not sure about this

      S_.current_ = 0.0;
      for ( size_t i = 0; i < P_.n_receptors_(); i++ )
      {
        S_.V_m_ += V_.P21_syn_[ i ] * S_.i_syn_[ i ];
        S_.current_ += S_.i_syn_[ i ]; // not sure about this
      }
    }
    else
    {
      --S_.refractory_steps_; // neuron is absolute refractory
    }
    for ( size_t i = 0; i < P_.n_receptors_(); i++ )
    {
      // exponential decaying PSCs
      S_.i_syn_[ i ] *= V_.P11_syn_[ i ];

      // collect spikes
      S_.i_syn_[ i ] += B_.spikes_[ i ].get_value( lag ); // not sure about this
    }

    if ( S_.V_m_ >= P_.Theta_ ) // threshold crossing
    {
      S_.refractory_steps_ = V_.RefractoryCounts_;
      S_.V_m_ = P_.V_reset_;

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input current
    S_.I_const_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

port
iaf_psc_exp_multisynapse::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type <= 0 || receptor_type > static_cast< port >( P_.n_receptors_() ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }

  P_.has_connections_ = true;
  return receptor_type;
}

void
iaf_psc_exp_multisynapse::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_[ e.get_rport() - 1 ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
iaf_psc_exp_multisynapse::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double I = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * I );
}

void
iaf_psc_exp_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
iaf_psc_exp_multisynapse::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  /*
   * Here is where we must update the recordablesMap_ if new receptors
   * are added!
   */
  if ( ptmp.tau_syn_.size() > P_.tau_syn_.size() ) // Number of receptors increased
  {
    for ( size_t i_syn = P_.tau_syn_.size(); i_syn < ptmp.tau_syn_.size(); ++i_syn )
    {
      size_t elem = iaf_psc_exp_multisynapse::State_::I_SYN
        + i_syn * iaf_psc_exp_multisynapse::State_::NUM_STATE_ELEMENTS_PER_RECEPTOR;
      recordablesMap_.insert( get_i_syn_name( i_syn ), get_data_access_functor( elem ) );
    }
  }
  else if ( ptmp.tau_syn_.size() < P_.tau_syn_.size() )
  { // Number of receptors decreased
    for ( size_t i_syn = ptmp.tau_syn_.size(); i_syn < P_.tau_syn_.size(); ++i_syn )
    {
      recordablesMap_.erase( get_i_syn_name( i_syn ) );
    }
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace
