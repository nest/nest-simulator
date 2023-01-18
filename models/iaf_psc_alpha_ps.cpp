/*
 *  iaf_psc_alpha_ps.cpp
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

#include "iaf_psc_alpha_ps.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "iaf_propagator.h"
#include "numerics.h"
#include "regula_falsi.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dictutils.h"

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_psc_alpha_ps > nest::iaf_psc_alpha_ps::recordablesMap_;

namespace nest
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_psc_alpha_ps >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &iaf_psc_alpha_ps::get_V_m_ );
  insert_( names::I_syn_ex, &iaf_psc_alpha_ps::get_I_ex_ );
  insert_( names::I_syn_in, &iaf_psc_alpha_ps::get_I_in_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_ps::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                       // ms
  , tau_syn_ex_( 2.0 )                                   // ms
  , tau_syn_in_( 2.0 )                                   // ms
  , c_m_( 250.0 )                                        // pF
  , t_ref_( 2.0 )                                        // ms
  , E_L_( -70.0 )                                        // mV
  , I_e_( 0.0 )                                          // pA
  , U_th_( -55.0 - E_L_ )                                // mV, rel to E_L_
  , U_min_( -std::numeric_limits< double >::infinity() ) // mV
  , U_reset_( -70.0 - E_L_ )                             // mV, rel to E_L_
{
}

nest::iaf_psc_alpha_ps::State_::State_()
  : y_input_( 0.0 )
  , I_ex_( 0.0 )
  , dI_ex_( 0.0 )
  , I_in_( 0.0 )
  , dI_in_( 0.0 )
  , V_m_( 0.0 )
  , is_refractory_( false )
  , last_spike_step_( -1 )
  , last_spike_offset_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_alpha_ps::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, U_th_ + E_L_ );
  def< double >( d, names::V_min, U_min_ + E_L_ );
  def< double >( d, names::V_reset, U_reset_ + E_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::tau_syn_ex, tau_syn_ex_ );
  def< double >( d, names::tau_syn_in, tau_syn_in_ );
  def< double >( d, names::t_ref, t_ref_ );
}

double
nest::iaf_psc_alpha_ps::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::tau_syn_ex, tau_syn_ex_, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_syn_in_, node );
  updateValueParam< double >( d, names::C_m, c_m_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );

  if ( updateValueParam< double >( d, names::V_th, U_th_, node ) )
  {
    U_th_ -= E_L_;
  }
  else
  {
    U_th_ -= delta_EL;
  }

  if ( updateValueParam< double >( d, names::V_min, U_min_, node ) )
  {
    U_min_ -= E_L_;
  }
  else
  {
    U_min_ -= delta_EL;
  }

  if ( updateValueParam< double >( d, names::V_reset, U_reset_, node ) )
  {
    U_reset_ -= E_L_;
  }
  else
  {
    U_reset_ -= delta_EL;
  }

  if ( U_reset_ >= U_th_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }

  if ( U_reset_ < U_min_ )
  {
    throw BadProperty( "Reset potential must be greater equal minimum potential." );
  }

  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( Time( Time::ms( t_ref_ ) ).get_steps() < 1 )
  {
    throw BadProperty( "Refractory time must be at least one time step." );
  }

  if ( tau_m_ <= 0 or tau_syn_ex_ <= 0 or tau_syn_in_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  return delta_EL;
}

void
nest::iaf_psc_alpha_ps::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, V_m_ + p.E_L_ ); // Membrane potential
  def< double >( d, names::I_syn_ex, I_ex_ );    // Excitatory synaptic current
  def< double >( d, names::I_syn_in, I_in_ );    // Inhibitory synaptic current
  def< double >( d, names::dI_syn_in, dI_in_ );  // Derivative inhib. current
  def< double >( d, names::dI_syn_ex, dI_ex_ );  // Derivative exc. current
  def< bool >( d, names::is_refractory, is_refractory_ );
}

void
nest::iaf_psc_alpha_ps::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
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

nest::iaf_psc_alpha_ps::Buffers_::Buffers_( iaf_psc_alpha_ps& n )
  : logger_( n )
{
}

nest::iaf_psc_alpha_ps::Buffers_::Buffers_( const Buffers_&, iaf_psc_alpha_ps& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_ps::iaf_psc_alpha_ps()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_alpha_ps::iaf_psc_alpha_ps( const iaf_psc_alpha_ps& n )
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
nest::iaf_psc_alpha_ps::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();

  ArchivingNode::clear_history();
}

void
nest::iaf_psc_alpha_ps::pre_run_hook()
{
  B_.logger_.init();

  V_.h_ms_ = Time::get_resolution().get_ms();

  V_.psc_norm_ex_ = 1.0 * numerics::e / P_.tau_syn_ex_;
  V_.psc_norm_in_ = 1.0 * numerics::e / P_.tau_syn_in_;

  // pre-compute matrix for full time step
  V_.expm1_tau_m_ = numerics::expm1( -V_.h_ms_ / P_.tau_m_ );
  V_.exp_tau_syn_ex_ = std::exp( -V_.h_ms_ / P_.tau_syn_ex_ );
  V_.exp_tau_syn_in_ = std::exp( -V_.h_ms_ / P_.tau_syn_in_ );

  V_.P30_ = -P_.tau_m_ / P_.c_m_ * V_.expm1_tau_m_;
  // these are determined according to a numeric stability criterion
  propagator_ex_ = IAFPropagatorAlpha( P_.tau_syn_ex_, P_.tau_m_, P_.c_m_ );
  std::tie( V_.P31_ex_, V_.P32_ex_ ) = propagator_ex_.evaluate( V_.h_ms_ );

  propagator_in_ = IAFPropagatorAlpha( P_.tau_syn_in_, P_.tau_m_, P_.c_m_ );
  std::tie( V_.P31_in_, V_.P32_in_ ) = propagator_in_.evaluate( V_.h_ms_ );

  // t_ref_ is the refractory period in ms
  // refractory_steps_ is the duration of the refractory period in whole
  // steps, rounded down
  V_.refractory_steps_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= sim step size, this can only fail in error
  assert( V_.refractory_steps_ >= 1 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

bool
nest::iaf_psc_alpha_ps::get_next_event_( const long T, double& ev_offset, double& ev_weight, bool& end_of_refract )
{
  return B_.events_.get_next_spike( T, false, ev_offset, ev_weight, end_of_refract );
}

void
nest::iaf_psc_alpha_ps::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 );
  assert( static_cast< delay >( from ) < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // at start of slice, tell input queue to prepare for delivery
  if ( from == 0 )
  {
    B_.events_.prepare_delivery();
  }

  /* Neurons may have been initialized to superthreshold potentials.
     We need to check for this here and issue spikes at the beginning of
     the interval.
  */
  if ( S_.V_m_ >= P_.U_th_ )
  {
    emit_instant_spike_( origin, from, V_.h_ms_ * ( 1 - std::numeric_limits< double >::epsilon() ) );
  }

  for ( long lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long T = origin.get_steps() + lag;
    // if neuron returns from refractoriness during this step, place
    // pseudo-event in queue to mark end of refractory period
    if ( S_.is_refractory_ and T + 1 - S_.last_spike_step_ == V_.refractory_steps_ )
    {
      B_.events_.add_refractory( T, S_.last_spike_offset_ );
    }

    // save state at beginning of interval for spike-time interpolation
    V_.y_input_before_ = S_.y_input_;
    V_.I_ex_before_ = S_.I_ex_;
    V_.I_in_before_ = S_.I_in_;
    V_.dI_ex_before_ = S_.dI_ex_;
    V_.dI_in_before_ = S_.dI_in_;
    V_.V_m_before_ = S_.V_m_;

    // get first event
    double ev_offset;
    double ev_weight;
    bool end_of_refract;

    if ( not get_next_event_( T, ev_offset, ev_weight, end_of_refract ) )
    {
      // No incoming spikes, handle with fixed propagator matrix.
      // Handling this case separately improves performance significantly
      // if there are many steps without input spikes.

      // update membrane potential
      if ( not S_.is_refractory_ )
      {
        // If we use S_.V_m_ * std::exp( -V_.h_ms_ / P_.tau_m_ ) instead of
        // V_.expm1_tau_m_ * S_.V_m_ + S_.V_m_ here, the accuracy decreases,
        // see test_iaf_ps_dc_t_accuracy.sli for details.
        S_.V_m_ = V_.P30_ * ( P_.I_e_ + S_.y_input_ ) + V_.P31_ex_ * S_.dI_ex_ + V_.P32_ex_ * S_.I_ex_
          + V_.P31_in_ * S_.dI_in_ + V_.P32_in_ * S_.I_in_ + V_.expm1_tau_m_ * S_.V_m_ + S_.V_m_;

        // lower bound of membrane potential
        S_.V_m_ = ( S_.V_m_ < P_.U_min_ ? P_.U_min_ : S_.V_m_ );
      }

      // update synaptic currents
      S_.I_ex_ = V_.exp_tau_syn_ex_ * V_.h_ms_ * S_.dI_ex_ + V_.exp_tau_syn_ex_ * S_.I_ex_;
      S_.dI_ex_ = V_.exp_tau_syn_ex_ * S_.dI_ex_;

      S_.I_in_ = V_.exp_tau_syn_in_ * V_.h_ms_ * S_.dI_in_ + V_.exp_tau_syn_in_ * S_.I_in_;
      S_.dI_in_ = V_.exp_tau_syn_in_ * S_.dI_in_;

      /* The following must not be moved before the y1_, dI_ex_ update,
         since the spike-time interpolation within emit_spike_ depends
         on all state variables having their values at the end of the
         interval.
      */
      if ( S_.V_m_ >= P_.U_th_ )
      {
        emit_spike_( origin, lag, 0, V_.h_ms_ );
      }
    }
    else
    {
      // We get here if there is at least one event,
      // which has been read above.  We can therefore use a do-while loop.

      // Time within step is measured by offsets, which are h at the beginning
      // and 0 at the end of the step.
      double last_offset = V_.h_ms_; // start of step

      do
      {
        // time is measured backward: inverse order in difference
        const double ministep = last_offset - ev_offset;

        propagate_( ministep );

        // check for threshold crossing during ministep
        // this must be done before adding the input, since
        // interpolation requires continuity
        if ( S_.V_m_ >= P_.U_th_ )
        {
          emit_spike_( origin, lag, V_.h_ms_ - last_offset, ministep );
        }


        // handle event
        if ( end_of_refract )
        {
          S_.is_refractory_ = false;
        } // return from refractoriness
        else
        {
          if ( ev_weight >= 0.0 )
          {
            S_.dI_ex_ += V_.psc_norm_ex_ * ev_weight; // exc. spike input
          }
          else
          {
            S_.dI_in_ += V_.psc_norm_in_ * ev_weight;
          } // inh. spike input
        }

        // store state
        V_.I_ex_before_ = S_.I_ex_;
        V_.I_in_before_ = S_.I_in_;
        V_.dI_ex_before_ = S_.dI_ex_;
        V_.dI_in_before_ = S_.dI_in_;
        V_.V_m_before_ = S_.V_m_;
        last_offset = ev_offset;

      } while ( get_next_event_( T, ev_offset, ev_weight, end_of_refract ) );

      // no events remaining, plain update step across remainder
      // of interval
      if ( last_offset > 0 ) // not at end of step, do remainder
      {
        propagate_( last_offset );
        if ( S_.V_m_ >= P_.U_th_ )
        {
          emit_spike_( origin, lag, V_.h_ms_ - last_offset, last_offset );
        }
      }
    } // else

    // Set new input current. The current change occurs at the
    // end of the interval and thus must come AFTER the threshold-
    // crossing interpolation
    S_.y_input_ = B_.currents_.get_value( lag );


    // logging
    B_.logger_.record_data( origin.get_steps() + lag );
  } // from lag = from ...
}


// function handles exact spike times
void
nest::iaf_psc_alpha_ps::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  /* We need to compute the absolute time stamp of the delivery time
     of the spike, since spikes might spend longer than min_delay_
     in the queue.  The time is computed according to Time Memo, Rule 3.
  */
  const long Tdeliver = e.get_stamp().get_steps() + e.get_delay_steps() - 1;

  B_.events_.add_spike( e.get_rel_delivery_steps( nest::kernel().simulation_manager.get_slice_origin() ),
    Tdeliver,
    e.get_offset(),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_psc_alpha_ps::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( nest::kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::iaf_psc_alpha_ps::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

// auxiliary functions ---------------------------------------------

void
nest::iaf_psc_alpha_ps::propagate_( const double dt )
{
  // V_m_ remains unchanged at 0.0 while neuron is refractory
  if ( not S_.is_refractory_ )
  {
    const double expm1_tau_m = numerics::expm1( -dt / P_.tau_m_ );

    const double ps_P30 = -P_.tau_m_ / P_.c_m_ * expm1_tau_m;

    double ps_P31_ex;
    double ps_P32_ex;
    double ps_P31_in;
    double ps_P32_in;
    std::tie( ps_P31_ex, ps_P32_ex ) = propagator_ex_.evaluate( dt );
    std::tie( ps_P31_in, ps_P32_in ) = propagator_in_.evaluate( dt );

    S_.V_m_ = ps_P30 * ( P_.I_e_ + S_.y_input_ ) + ps_P31_ex * S_.dI_ex_ + ps_P32_ex * S_.I_ex_ + ps_P31_in * S_.dI_in_
      + ps_P32_in * S_.I_in_ + S_.V_m_ * expm1_tau_m + S_.V_m_;

    // lower bound of membrane potential
    S_.V_m_ = ( S_.V_m_ < P_.U_min_ ? P_.U_min_ : S_.V_m_ );
  }

  const double ps_e_TauSyn_ex = std::exp( -dt / P_.tau_syn_ex_ );
  const double ps_e_TauSyn_in = std::exp( -dt / P_.tau_syn_in_ );

  // now the synaptic components
  S_.I_ex_ = ps_e_TauSyn_ex * dt * S_.dI_ex_ + ps_e_TauSyn_ex * S_.I_ex_;
  S_.dI_ex_ = ps_e_TauSyn_ex * S_.dI_ex_;

  S_.I_in_ = ps_e_TauSyn_in * dt * S_.dI_in_ + ps_e_TauSyn_in * S_.I_in_;
  S_.dI_in_ = ps_e_TauSyn_in * S_.dI_in_;
}

void
nest::iaf_psc_alpha_ps::emit_spike_( Time const& origin, const long lag, const double t0, const double dt )
{
  // we know that the potential is subthreshold at t0, super at t0+dt

  // compute spike time relative to beginning of step
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = V_.h_ms_ - ( t0 + regula_falsi( *this, dt ) );

  assert( S_.last_spike_offset_ >= 0.0 );

  // reset neuron and make it refractory
  S_.V_m_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;
  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );

  return;
}

void
nest::iaf_psc_alpha_ps::emit_instant_spike_( Time const& origin, const long lag, const double spike_offs )
{
  assert( S_.V_m_ >= P_.U_th_ ); // ensure we are superthreshold

  // set stamp and offset for spike
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = spike_offs;

  // reset neuron and make it refractory
  S_.V_m_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;
  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );

  return;
}

double
nest::iaf_psc_alpha_ps::threshold_distance( double t_step ) const
{
  const double expm1_tau_m = numerics::expm1( -t_step / P_.tau_m_ );

  const double ps_P30 = -P_.tau_m_ / P_.c_m_ * expm1_tau_m;

  double ps_P31_ex;
  double ps_P32_ex;
  double ps_P31_in;
  double ps_P32_in;
  std::tie( ps_P31_ex, ps_P32_ex ) = propagator_ex_.evaluate( t_step );
  std::tie( ps_P31_in, ps_P32_in ) = propagator_in_.evaluate( t_step );

  const double V_m_root = ps_P30 * ( P_.I_e_ + V_.y_input_before_ ) + ps_P31_ex * V_.dI_ex_before_
    + ps_P32_ex * V_.I_ex_before_ + ps_P31_in * V_.dI_in_before_ + ps_P32_in * V_.I_in_before_
    + V_.V_m_before_ * expm1_tau_m + V_.V_m_before_;

  return V_m_root - P_.U_th_;
}
