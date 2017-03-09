/*
 *  iaf_neuron.cpp
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

#include "iaf_neuron.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "numerics.h"
#include "propagator_stability.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
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

nest::RecordablesMap< nest::iaf_neuron > nest::iaf_neuron::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_neuron >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_neuron::get_V_m_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_neuron::Parameters_::Parameters_()
  : C_( 250.0 )              // pF
  , Tau_( 10.0 )             // ms
  , tau_syn_( 2.0 )          // ms
  , TauR_( 2.0 )             // ms
  , E_L_( -70.0 )            // mV
  , V_reset_( -70.0 - E_L_ ) // mV, rel to E_L_
  , Theta_( -55.0 - E_L_ )   // mV, rel to E_L_
  , I_e_( 0.0 )              // pA
{
}

nest::iaf_neuron::State_::State_()
  : y0_( 0.0 )
  , y1_( 0.0 )
  , y2_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_neuron::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // Resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + E_L_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + E_L_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::tau_syn, tau_syn_ );
  def< double >( d, names::t_ref, TauR_ );
}

double
nest::iaf_neuron::Parameters_::set( const DictionaryDatum& d )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValue< double >( d, names::E_L, E_L_ );
  const double delta_EL = E_L_ - ELold;

  if ( updateValue< double >( d, names::V_reset, V_reset_ ) )
  {
    V_reset_ -= E_L_; // here we use the new E_L_, no need for adjustments
  }
  else
  {
    V_reset_ -= delta_EL;
  } // express relative to new E_L_

  if ( updateValue< double >( d, names::V_th, Theta_ ) )
  {
    Theta_ -= E_L_;
  }
  else
  {
    Theta_ -= delta_EL;
  } // express relative to new E_L_

  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::C_m, C_ );
  updateValue< double >( d, names::tau_m, Tau_ );
  updateValue< double >( d, names::tau_syn, tau_syn_ );
  updateValue< double >( d, names::t_ref, TauR_ );
  if ( V_reset_ >= Theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( Tau_ <= 0 || tau_syn_ <= 0 || TauR_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  return delta_EL;
}

void
nest::iaf_neuron::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ ); // Membrane potential
}

void
nest::iaf_neuron::State_::set( const DictionaryDatum& d,
  const Parameters_& p,
  double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, y3_ ) )
  {
    y3_ -= p.E_L_;
  }
  else
  {
    y3_ -= delta_EL;
  }
}

nest::iaf_neuron::Buffers_::Buffers_( iaf_neuron& n )
  : logger_( n )
{
}

nest::iaf_neuron::Buffers_::Buffers_( const Buffers_&, iaf_neuron& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_neuron::iaf_neuron()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_neuron::iaf_neuron( const iaf_neuron& n )
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
nest::iaf_neuron::init_state_( const Node& proto )
{
  const iaf_neuron& pr = downcast< iaf_neuron >( proto );
  S_ = pr.S_;
}

void
nest::iaf_neuron::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // include resize
  B_.logger_.reset();   // includes resize
  Archiving_Node::clear_history();
}

void
nest::iaf_neuron::calibrate()
{
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  // these P are independent
  V_.P11_ = V_.P22_ = std::exp( -h / P_.tau_syn_ );
  V_.P33_ = std::exp( -h / P_.Tau_ );
  V_.P21_ = h * V_.P11_;

  // this depends on the above. Please do not change the order.
  V_.P30_ = 1 / P_.C_ * ( 1 - V_.P33_ ) * P_.Tau_;

  // these are determined according to a numeric stability criterion
  V_.P31_ = propagator_31( P_.tau_syn_, P_.Tau_, P_.C_, h );
  V_.P32_ = propagator_32( P_.tau_syn_, P_.Tau_, P_.C_, h );

  V_.PSCInitialValue_ = 1.0 * numerics::e / P_.tau_syn_;


  // TauR specifies the length of the absolute refractory period as
  // a double in ms. The grid based iaf_neuron can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion
  // requires 2 steps:
  //     1. A time object is constructed defining representation of
  //        TauR in tics. This representation is then converted to computation
  //        time steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a
  //        member function of class nest::Time.
  //
  // The definition of the refractory period of the iaf_neuron is consistent
  // the one of iaf_psc_alpha_ps.
  //
  // Choosing a TauR that is not an integer multiple of the computation time
  // step h will lead to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued
  // spike time may exhibit a different effective refractory time.

  V_.RefractoryCounts_ = Time( Time::ms( P_.TauR_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_neuron::update( Time const& origin, const long from, const long to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    if ( S_.r_ == 0 )
    {
      // neuron not refractory
      S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P31_ * S_.y1_
        + V_.P32_ * S_.y2_ + V_.P33_ * S_.y3_;
    }
    else
    {
      // neuron is absolute refractory
      --S_.r_;
    }

    // alpha shape PSCs
    S_.y2_ = V_.P21_ * S_.y1_ + V_.P22_ * S_.y2_;
    S_.y1_ *= V_.P11_;

    // Apply spikes delivered in this step: The spikes arriving at T+1 have an
    // immediate effect on the state of the neuron
    S_.y1_ += V_.PSCInitialValue_ * B_.spikes_.get_value( lag );

    // threshold crossing
    if ( S_.y3_ >= P_.Theta_ )
    {
      S_.r_ = V_.RefractoryCounts_;
      S_.y3_ = P_.V_reset_;

      // A supra-threshold membrane potential should never be observable.
      // The reset at the time of threshold crossing enables accurate
      // integration independent of the computation step size, see [2,3] for
      // details.
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_neuron::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_neuron::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::iaf_neuron::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
