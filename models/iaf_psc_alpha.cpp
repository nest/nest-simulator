/*
 *  iaf_psc_alpha.cpp
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
#include "iaf_psc_alpha.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"
#include "propagator_stability.h"

#include <limits>

nest::RecordablesMap< nest::iaf_psc_alpha > nest::iaf_psc_alpha::recordablesMap_;

namespace nest
{

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_psc_alpha >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_psc_alpha::get_V_m_ );
  insert_( names::weighted_spikes_ex, &iaf_psc_alpha::get_weighted_spikes_ex_ );
  insert_( names::weighted_spikes_in, &iaf_psc_alpha::get_weighted_spikes_in_ );
  insert_( names::input_currents_ex, &iaf_psc_alpha::get_input_currents_ex_ );
  insert_( names::input_currents_in, &iaf_psc_alpha::get_input_currents_in_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

iaf_psc_alpha::Parameters_::Parameters_()
  : Tau_( 10.0 )            // ms
  , C_( 250.0 )             // pF
  , TauR_( 2.0 )            // ms
  , U0_( -70.0 )            // mV
  , I_e_( 0.0 )             // pA
  , V_reset_( -70.0 - U0_ ) // mV, rel to U0_
  , Theta_( -55.0 - U0_ )   // mV, rel to U0_
  , LowerBound_( -std::numeric_limits< double_t >::infinity() )
  , tau_ex_( 2.0 ) // ms
  , tau_in_( 2.0 ) // ms
{
}

iaf_psc_alpha::State_::State_()
  : y0_( 0.0 )
  , y1_ex_( 0.0 )
  , y2_ex_( 0.0 )
  , y1_in_( 0.0 )
  , y2_in_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
iaf_psc_alpha::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, U0_ ); // Resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + U0_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + U0_ );
  def< double >( d, names::V_min, LowerBound_ + U0_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::t_ref, TauR_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );
}

double
iaf_psc_alpha::Parameters_::set( const DictionaryDatum& d )
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
  updateValue< double >( d, names::tau_syn_ex, tau_ex_ );
  updateValue< double >( d, names::tau_syn_in, tau_in_ );
  updateValue< double >( d, names::t_ref, TauR_ );

  if ( C_ <= 0.0 )
    throw BadProperty( "Capacitance must be > 0." );

  if ( Tau_ <= 0.0 )
    throw BadProperty( "Membrane time constant must be > 0." );

  if ( tau_ex_ <= 0.0 || tau_in_ <= 0.0 )
    throw BadProperty( "All synaptic time constants must be > 0." );

  if ( TauR_ < 0.0 )
    throw BadProperty( "The refractory time t_ref can't be negative." );

  if ( V_reset_ >= Theta_ )
    throw BadProperty( "Reset potential must be smaller than threshold." );

  return delta_EL;
}

void
iaf_psc_alpha::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.U0_ ); // Membrane potential
}

void
iaf_psc_alpha::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, y3_ ) )
    y3_ -= p.U0_;
  else
    y3_ -= delta_EL;
}

iaf_psc_alpha::Buffers_::Buffers_( iaf_psc_alpha& n )
  : logger_( n )
{
}

iaf_psc_alpha::Buffers_::Buffers_( const Buffers_&, iaf_psc_alpha& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

iaf_psc_alpha::iaf_psc_alpha()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

iaf_psc_alpha::iaf_psc_alpha( const iaf_psc_alpha& n )
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
iaf_psc_alpha::init_state_( const Node& proto )
{
  const iaf_psc_alpha& pr = downcast< iaf_psc_alpha >( proto );
  S_ = pr.S_;
}

void
iaf_psc_alpha::init_buffers_()
{
  B_.ex_spikes_.clear(); // includes resize
  B_.in_spikes_.clear(); // includes resize
  B_.currents_.clear();  // includes resize

  B_.logger_.reset();

  Archiving_Node::clear_history();
}

void
iaf_psc_alpha::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  const double h = Time::get_resolution().get_ms();

  // these P are independent
  V_.P11_ex_ = V_.P22_ex_ = std::exp( -h / P_.tau_ex_ );
  V_.P11_in_ = V_.P22_in_ = std::exp( -h / P_.tau_in_ );

  V_.P33_ = std::exp( -h / P_.Tau_ );

  V_.expm1_tau_m_ = numerics::expm1( -h / P_.Tau_ );

  // these depend on the above. Please do not change the order.
  V_.P30_ = -P_.Tau_ / P_.C_ * numerics::expm1( -h / P_.Tau_ );
  V_.P21_ex_ = h * V_.P11_ex_;
  V_.P21_in_ = h * V_.P11_in_;

  // these are determined according to a numeric stability criterion
  V_.P31_ex_ = propagator_31( P_.tau_ex_, P_.Tau_, P_.C_, h );
  V_.P32_ex_ = propagator_32( P_.tau_ex_, P_.Tau_, P_.C_, h );
  V_.P31_in_ = propagator_31( P_.tau_in_, P_.Tau_, P_.C_, h );
  V_.P32_in_ = propagator_32( P_.tau_in_, P_.Tau_, P_.C_, h );

  V_.EPSCInitialValue_ = 1.0 * numerics::e / P_.tau_ex_;
  V_.IPSCInitialValue_ = 1.0 * numerics::e / P_.tau_in_;

  // TauR specifies the length of the absolute refractory period as
  // a double_t in ms. The grid based iaf_psc_alpha can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion
  // requires 2 steps:
  //     1. A time object is constructed defining representation of
  //        TauR in tics. This representation is then converted to computation time
  //        steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a member
  //        function of class nest::Time.
  //
  // The definition of the refractory period of the iaf_psc_alpha is consistent
  // the one of iaf_psc_alpha_ps.
  //
  // Choosing a TauR that is not an integer multiple of the computation time
  // step h will lead to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued spike
  // time may exhibit a different effective refractory time.

  V_.RefractoryCounts_ = Time( Time::ms( P_.TauR_ ) ).get_steps();
  assert( V_.RefractoryCounts_ >= 0 ); // since t_ref_ >= 0, this can only fail in error
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
iaf_psc_alpha::update( Time const& origin, const long_t from, const long_t to )
{
  assert( to >= 0 && ( delay ) from < Scheduler::get_min_delay() );
  assert( from < to );

  for ( long_t lag = from; lag < to; ++lag )
  {
    if ( S_.r_ == 0 )
    {
      // neuron not refractory
      S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P31_ex_ * S_.y1_ex_ + V_.P32_ex_ * S_.y2_ex_
        + V_.P31_in_ * S_.y1_in_ + V_.P32_in_ * S_.y2_in_ + V_.expm1_tau_m_ * S_.y3_ + S_.y3_;

      // lower bound of membrane potential
      S_.y3_ = ( S_.y3_ < P_.LowerBound_ ? P_.LowerBound_ : S_.y3_ );
    }
    else // neuron is absolute refractory
      --S_.r_;

    // alpha shape EPSCs
    S_.y2_ex_ = V_.P21_ex_ * S_.y1_ex_ + V_.P22_ex_ * S_.y2_ex_;
    S_.y1_ex_ *= V_.P11_ex_;

    // Apply spikes delivered in this step; spikes arriving at T+1 have
    // an immediate effect on the state of the neuron
    V_.weighted_spikes_ex_ = B_.ex_spikes_.get_value( lag );
    S_.y1_ex_ += V_.EPSCInitialValue_ * V_.weighted_spikes_ex_;

    // alpha shape EPSCs
    S_.y2_in_ = V_.P21_in_ * S_.y1_in_ + V_.P22_in_ * S_.y2_in_;
    S_.y1_in_ *= V_.P11_in_;

    // Apply spikes delivered in this step; spikes arriving at T+1 have
    // an immediate effect on the state of the neuron
    V_.weighted_spikes_in_ = B_.in_spikes_.get_value( lag );
    S_.y1_in_ += V_.IPSCInitialValue_ * V_.weighted_spikes_in_;

    // threshold crossing
    if ( S_.y3_ >= P_.Theta_ )
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

void
iaf_psc_alpha::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t s = e.get_weight() * e.get_multiplicity();

  if ( e.get_weight() > 0.0 )
    B_.ex_spikes_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ), s );
  else
    B_.in_spikes_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ), s );
}

void
iaf_psc_alpha::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t I = e.get_current();
  const double_t w = e.get_weight();

  B_.currents_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ), w * I );
}

void
iaf_psc_alpha::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
