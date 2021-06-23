/*
 *  iaf_psc_exp.cpp
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

#include "iaf_psc_exp.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"
#include "propagator_stability.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "ring_buffer_impl.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_psc_exp > nest::iaf_psc_exp::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_psc_exp >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_psc_exp::get_V_m_ );
  insert_( names::weighted_spikes_ex, &iaf_psc_exp::get_weighted_spikes_ex_ );
  insert_( names::weighted_spikes_in, &iaf_psc_exp::get_weighted_spikes_in_ );
  insert_( names::I_syn_ex, &iaf_psc_exp::get_I_syn_ex_ );
  insert_( names::I_syn_in, &iaf_psc_exp::get_I_syn_in_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp::Parameters_::Parameters_()
  : Tau_( 10.0 )             // in ms
  , C_( 250.0 )              // in pF
  , t_ref_( 2.0 )            // in ms
  , E_L_( -70.0 )            // in mV
  , I_e_( 0.0 )              // in pA
  , Theta_( -55.0 - E_L_ )   // relative E_L_
  , V_reset_( -70.0 - E_L_ ) // in mV
  , tau_ex_( 2.0 )           // in ms
  , tau_in_( 2.0 )           // in ms
  , rho_( 0.01 )             // in 1/s
  , delta_( 0.0 )            // in mV
{
}

nest::iaf_psc_exp::State_::State_()
  : i_0_( 0.0 )
  , i_1_( 0.0 )
  , i_syn_ex_( 0.0 )
  , i_syn_in_( 0.0 )
  , V_m_( 0.0 )
  , r_ref_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_exp::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + E_L_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + E_L_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::rho, rho_ );
  def< double >( d, names::delta, delta_ );
}

double
nest::iaf_psc_exp::Parameters_::set( const DictionaryDatum& d, Node* node )
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
  updateValueParam< double >( d, names::tau_syn_ex, tau_ex_, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_in_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  if ( V_reset_ >= Theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( Tau_ <= 0 || tau_ex_ <= 0 || tau_in_ <= 0 )
  {
    throw BadProperty( "Membrane and synapse time constants must be strictly positive." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  updateValue< double >( d, "rho", rho_ );
  if ( rho_ < 0 )
  {
    throw BadProperty( "Stochastic firing intensity must not be negative." );
  }

  updateValue< double >( d, "delta", delta_ );
  if ( delta_ < 0 )
  {
    throw BadProperty( "Width of threshold region must not be negative." );
  }

  return delta_EL;
}

void
nest::iaf_psc_exp::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, V_m_ + p.E_L_ ); // Membrane potential
}

void
nest::iaf_psc_exp::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  if ( updateValueParam< double >( d, names::V_m, V_m_, node ) )
  {
    V_m_ -= p.E_L_;
  }
  else
  {
    V_m_ -= delta_EL;
  }
}

nest::iaf_psc_exp::Buffers_::Buffers_( iaf_psc_exp& n )
  : logger_( n )
{
}

nest::iaf_psc_exp::Buffers_::Buffers_( const Buffers_&, iaf_psc_exp& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp::iaf_psc_exp()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_exp::iaf_psc_exp( const iaf_psc_exp& n )
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
nest::iaf_psc_exp::init_buffers_()
{
  B_.input_buffer_.clear(); // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

void
nest::iaf_psc_exp::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  // numbering of state vaiables: i_0 = 0, i_syn_ = 1, V_m_ = 2

  // commented out propagators: forward Euler
  // needed to exactly reproduce Tsodyks network

  // these P are independent
  V_.P11ex_ = std::exp( -h / P_.tau_ex_ );
  // P11ex_ = 1.0-h/tau_ex_;

  V_.P11in_ = std::exp( -h / P_.tau_in_ );
  // P11in_ = 1.0-h/tau_in_;

  V_.P22_ = std::exp( -h / P_.Tau_ );
  // P22_ = 1.0-h/Tau_;

  // these are determined according to a numeric stability criterion
  V_.P21ex_ = propagator_32( P_.tau_ex_, P_.Tau_, P_.C_, h );
  V_.P21in_ = propagator_32( P_.tau_in_, P_.Tau_, P_.C_, h );

  // P21ex_ = h/C_;
  // P21in_ = h/C_;

  V_.P20_ = P_.Tau_ / P_.C_ * ( 1.0 - V_.P22_ );
  // P20_ = h/C_;

  // t_ref_ specifies the length of the absolute refractory period as
  // a double in ms. The grid based iaf_psc_exp can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion
  // requires 2 steps:
  //     1. A time object r is constructed, defining representation of
  //        t_ref_ in tics. This representation is then converted to computation
  //        time steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a
  //        member function of class nest::Time.
  //
  // Choosing a t_ref_ that is not an integer multiple of the computation time
  // step h will lead to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued
  // spike time may exhibit a different effective refractory time.

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );

  V_.rng_ = get_vp_specific_rng( get_thread() );
}

void
nest::iaf_psc_exp::update( const Time& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const double h = Time::get_resolution().get_ms();

  // evolve from timestep 'from' to timestep 'to' with steps of h each
  for ( long lag = from; lag < to; ++lag )
  {
    if ( S_.r_ref_ == 0 ) // neuron not refractory, so evolve V
    {
      S_.V_m_ =
        S_.V_m_ * V_.P22_ + S_.i_syn_ex_ * V_.P21ex_ + S_.i_syn_in_ * V_.P21in_ + ( P_.I_e_ + S_.i_0_ ) * V_.P20_;
    }
    else
    {
      // neuron is absolute refractory
      --S_.r_ref_;
    }

    // exponential decaying PSCs
    S_.i_syn_ex_ *= V_.P11ex_;
    S_.i_syn_in_ *= V_.P11in_;

    // add evolution of presynaptic input current
    S_.i_syn_ex_ += ( 1. - V_.P11ex_ ) * S_.i_1_;

    // get read access to the correct input-buffer slot
    const index input_buffer_slot = kernel().event_delivery_manager.get_modulo( lag );
    auto& input = B_.input_buffer_.get_values_all_channels( input_buffer_slot );

    // the spikes arriving at T+1 have an immediate effect on the state of the
    // neuron

    V_.weighted_spikes_ex_ = input[ Buffers_::SYN_EX ];
    V_.weighted_spikes_in_ = input[ Buffers_::SYN_IN ];

    S_.i_syn_ex_ += V_.weighted_spikes_ex_;
    S_.i_syn_in_ += V_.weighted_spikes_in_;

    if ( ( P_.delta_ < 1e-10 and S_.V_m_ >= P_.Theta_ )                   // deterministic threshold crossing
      or ( P_.delta_ > 1e-10 and V_.rng_->drand() < phi_() * h * 1e-3 ) ) // stochastic threshold crossing
    {
      S_.r_ref_ = V_.RefractoryCounts_;
      S_.V_m_ = P_.V_reset_;

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input current
    S_.i_0_ = input[ Buffers_::I0 ];
    S_.i_1_ = input[ Buffers_::I1 ];

    // reset all values in the currently processed input-buffer slot
    B_.input_buffer_.reset_values_all_channels( input_buffer_slot );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_psc_exp::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const index input_buffer_slot = kernel().event_delivery_manager.get_modulo(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) );

  const double s = e.get_weight() * e.get_multiplicity();

  // separate buffer channels for excitatory and inhibitory inputs
  B_.input_buffer_.add_value( input_buffer_slot, s > 0 ? Buffers_::SYN_EX : Buffers_::SYN_IN, s );
}

void
nest::iaf_psc_exp::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  const index input_buffer_slot = kernel().event_delivery_manager.get_modulo(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) );

  // add weighted current; HEP 2002-10-04
  if ( 0 == e.get_rport() )
  {
    B_.input_buffer_.add_value( input_buffer_slot, Buffers_::I0, w * c );
  }
  if ( 1 == e.get_rport() )
  {
    B_.input_buffer_.add_value( input_buffer_slot, Buffers_::I1, w * c );
  }
}

void
nest::iaf_psc_exp::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
