/*
 *  iaf_psc_delta_ps.cpp
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

// iaf_psc_delta_ps is a neuron where the potential jumps on each spike
// arrival.

#include "iaf_psc_delta_ps.h"

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
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{
/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< iaf_psc_delta_ps > iaf_psc_delta_ps::recordablesMap_;

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_psc_delta_ps >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &iaf_psc_delta_ps::get_V_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_delta_ps::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                  // ms
  , c_m_( 250.0 )                                   // pF
  , t_ref_( 2.0 )                                   // ms
  , E_L_( -70.0 )                                   // mV
  , I_e_( 0.0 )                                     // pA
  , U_th_( -55.0 - E_L_ )                           // mV, rel to E_L_
  , U_min_( -std::numeric_limits< double >::max() ) // mV
  , U_reset_( -70.0 - E_L_ )                        // mV, rel to E_L_
{
}

nest::iaf_psc_delta_ps::State_::State_()
  : U_( 0.0 ) //  or U_ = U_reset_;
  , I_( 0. )
  , last_spike_step_( -1 )
  , last_spike_offset_( 0.0 )
  , is_refractory_( false )
  , with_refr_input_( false )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_delta_ps::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, U_th_ + E_L_ );
  def< double >( d, names::V_min, U_min_ + E_L_ );
  def< double >( d, names::V_reset, U_reset_ + E_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::t_ref, t_ref_ );
}

double
nest::iaf_psc_delta_ps::Parameters_::set( const DictionaryDatum& d )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValue< double >( d, names::E_L, E_L_ );
  const double delta_EL = E_L_ - ELold;

  updateValue< double >( d, names::tau_m, tau_m_ );
  updateValue< double >( d, names::C_m, c_m_ );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::I_e, I_e_ );

  if ( updateValue< double >( d, names::V_th, U_th_ ) )
  {
    U_th_ -= E_L_;
  }
  else
  {
    U_th_ -= delta_EL;
  }

  if ( updateValue< double >( d, names::V_min, U_min_ ) )
  {
    U_min_ -= E_L_;
  }
  else
  {
    U_min_ -= delta_EL;
  }

  if ( updateValue< double >( d, names::V_reset, U_reset_ ) )
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
  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  return delta_EL;
}

void
nest::iaf_psc_delta_ps::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, U_ + p.E_L_ ); // Membrane potential
  def< bool >( d, names::is_refractory, is_refractory_ );
  def< bool >( d, names::refractory_input, with_refr_input_ );
}

void
nest::iaf_psc_delta_ps::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, U_ ) )
  {
    U_ -= p.E_L_;
  }
  else
  {
    U_ -= delta_EL;
  }
}

nest::iaf_psc_delta_ps::Buffers_::Buffers_( iaf_psc_delta_ps& n )
  : logger_( n )
{
}

nest::iaf_psc_delta_ps::Buffers_::Buffers_( const Buffers_&, iaf_psc_delta_ps& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_delta_ps::iaf_psc_delta_ps()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_delta_ps::iaf_psc_delta_ps( const iaf_psc_delta_ps& n )
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
nest::iaf_psc_delta_ps::init_state_( const Node& proto )
{
  const iaf_psc_delta_ps& pr = downcast< iaf_psc_delta_ps >( proto );
  S_ = pr.S_;
}

void
nest::iaf_psc_delta_ps::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
  B_.currents_.clear();
  B_.logger_.reset();

  Archiving_Node::clear_history();
}

void
iaf_psc_delta_ps::calibrate()
{
  B_.logger_.init();

  V_.h_ms_ = Time::get_resolution().get_ms();

  V_.exp_t_ = std::exp( -V_.h_ms_ / P_.tau_m_ );
  V_.expm1_t_ = numerics::expm1( -V_.h_ms_ / P_.tau_m_ );
  V_.R_ = P_.tau_m_ / P_.c_m_;

  // t_ref_ is the refractory period in ms
  // refractory_steps_ is the duration of the refractory period in whole
  // steps, rounded down
  V_.refractory_steps_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= sim step size, this can only fail in error
  assert( V_.refractory_steps_ >= 1 );
}

void
iaf_psc_delta_ps::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 );
  assert( static_cast< delay >( from ) < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // at start of slice, tell input queue to prepare for delivery
  if ( from == 0 )
  {
    B_.events_.prepare_delivery();
  }

  /*
    The psc_delta neuron can fire only
    1. precisely upon spike arrival
    2. in between spike arrivals when threshold is reached due
       to the DC current.
    3. if the membrane potential is superthreshold at the BEGINNING
       of a slice due to initialization.
    In case 1, the spike time is known immediately.
    In case 2, the spike time can be found by solving the membrane
    equation.
    In case 3, the spike time is defined to be at from+epsilon.
    Thus, we can take arbitrary time steps within (from, to].
    Since slice_ring_buffer's delivery mechanism is built on time slices,
    we still need to step through the individual time steps to check
    for events.
    In typical network simulations and for typical step sizes, the probability
    of steps without input spikes is small. So the outer loop is over steps.
   */

  // check for super-threshold at beginning
  if ( S_.U_ >= P_.U_th_ )
  {
    emit_instant_spike_( origin, from, V_.h_ms_ * ( 1 - std::numeric_limits< double >::epsilon() ) );
  }

  for ( long lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long T = origin.get_steps() + lag;

    // Time within step is measured by offsets, which are h at the beginning
    // and 0 at the end of the step.
    double t = V_.h_ms_;

    // place pseudo-event in queue to mark end of refractory period
    if ( S_.is_refractory_ && ( T + 1 - S_.last_spike_step_ == V_.refractory_steps_ ) )
    {
      B_.events_.add_refractory( T, S_.last_spike_offset_ );
    }

    // get first event
    double ev_offset;
    double ev_weight;
    bool end_of_refract;

    if ( not B_.events_.get_next_spike( T, true, ev_offset, ev_weight, end_of_refract ) )
    { // No incoming spikes, handle with fixed propagator matrix.
      // Handling this case separately improves performance significantly
      // if there are many steps without input spikes.

      // update membrane potential
      if ( not S_.is_refractory_ )
      {
        /* The following way of updating U_ is numerically more precise
           than the more natural approach

              U_ = exp_t_ * U_ + I_contrib_;

           particularly when U_ * exp_t_ is close to -I_contrib_.
        */


        // contribution of the stepwise constant current
        const double I_ext = -V_.expm1_t_ * V_.R_ * ( S_.I_ + P_.I_e_ );
        S_.U_ = I_ext + V_.expm1_t_ * S_.U_ + S_.U_;

        S_.U_ = S_.U_ < P_.U_min_ ? P_.U_min_ : S_.U_; // lower bound on potential
        if ( S_.U_ >= P_.U_th_ )
        {
          emit_spike_( origin, lag, 0 ); // offset is zero at end of step
        }

        // We exploit here that the refractory period must be at least
        // one time step long. So even if the spike had happened at the
        // very beginning of the step, the neuron would remain refractory
        // for the rest of the step. We can thus ignore the time reset
        // issued by emit_spike_().
      }
      // nothing to do if neuron is refractory
    }
    else
    {
      // We only get here if there is at least one event,
      // which has been read above.  We can therefore use
      // a do-while loop.

      do
      {

        if ( S_.is_refractory_ )
        {
          // move time to time of event
          t = ev_offset;

          // normal spikes need to be accumulated
          if ( not end_of_refract )
          {
            if ( S_.with_refr_input_ )
            {
              V_.refr_spikes_buffer_ +=
                ev_weight * std::exp( -( ( S_.last_spike_step_ - T - 1 ) * V_.h_ms_
                                        - ( S_.last_spike_offset_ - ev_offset ) + P_.t_ref_ ) / P_.tau_m_ );
            }
          }
          else
          {
            // return from refractoriness---apply buffered spikes
            S_.is_refractory_ = false;

            if ( S_.with_refr_input_ )
            {
              S_.U_ += V_.refr_spikes_buffer_;
              V_.refr_spikes_buffer_ = 0.0;
            }

            // check if buffered spikes cause new spike
            if ( S_.U_ >= P_.U_th_ )
            {
              emit_instant_spike_( origin, lag, t );
            }
          }

          // nothing more to do in this loop iteration
          continue;
        }

        // we get here only if the neuron is not refractory
        // update neuron to time of event
        // time is measured backward: inverse order in difference
        propagate_( t - ev_offset );
        t = ev_offset;

        // Check whether we have passed the threshold. If yes, emit a
        // spike at the precise location of the crossing.
        // Time within the step need not be reset to the precise time
        // of the spike, since the neuron will be refractory for the
        // remainder of the step.
        // The event cannot be a return-from-refractoriness event,
        // since that violates the assumption that the neuron was not
        // refractory. We can thus ignore the event, since the neuron
        // is refractory after the spike and ignores all input.
        if ( S_.U_ >= P_.U_th_ )
        {
          emit_spike_( origin, lag, t );
          continue;
        }

        // neuron is not refractory: add input spike, check for output
        // spike
        S_.U_ += ev_weight;
        if ( S_.U_ >= P_.U_th_ )
        {
          emit_instant_spike_( origin, lag, t );
        }

      } while ( B_.events_.get_next_spike( T, true, ev_offset, ev_weight, end_of_refract ) );

      // no events remaining, plain update step across remainder
      // of interval
      if ( not S_.is_refractory_ && t > 0 ) // not at end of step, do remainder
      {
        propagate_( t );
        if ( S_.U_ >= P_.U_th_ )
        {
          emit_spike_( origin, lag, 0 );
        }
      }

    } // else

    // voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );

    S_.I_ = B_.currents_.get_value( lag );
  }
}

void
nest::iaf_psc_delta_ps::propagate_( const double dt )
{
  assert( not S_.is_refractory_ ); // should not be called if neuron is
                                   // refractory

  // see comment on regular update above
  const double expm1_dt = numerics::expm1( -dt / P_.tau_m_ );
  const double v_inf = V_.R_ * ( S_.I_ + P_.I_e_ );
  S_.U_ = -v_inf * expm1_dt + S_.U_ * expm1_dt + S_.U_;

  return;
}

void
nest::iaf_psc_delta_ps::emit_spike_( Time const& origin, const long lag, const double offset_U )
{
  assert( S_.U_ >= P_.U_th_ ); // ensure we are superthreshold

  // compute time since threshold crossing
  const double v_inf = V_.R_ * ( S_.I_ + P_.I_e_ );
  const double dt = -P_.tau_m_ * std::log( ( v_inf - S_.U_ ) / ( v_inf - P_.U_th_ ) );

  // set stamp and offset for spike
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = offset_U + dt;

  // reset neuron and make it refractory
  S_.U_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;
  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );

  return;
}

void
nest::iaf_psc_delta_ps::emit_instant_spike_( Time const& origin, const long lag, const double spike_offs )
{
  assert( S_.U_ >= P_.U_th_ ); // ensure we are superthreshold

  // set stamp and offset for spike
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = spike_offs;

  // reset neuron and make it refractory
  S_.U_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;
  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );

  return;
}

void
iaf_psc_delta_ps::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  /* We need to compute the absolute time stamp of the delivery time
     of the spike, since spikes might spend longer than min_delay_
     in the queue.  The time is computed according to Time Memo, Rule 3.
  */
  const long Tdeliver = e.get_stamp().get_steps() + e.get_delay_steps() - 1;
  B_.events_.add_spike( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    Tdeliver,
    e.get_offset(),
    e.get_weight() * e.get_multiplicity() );
}

void
iaf_psc_delta_ps::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add stepwise constant current; MH 2009-10-14
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}


void
nest::iaf_psc_delta_ps::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
