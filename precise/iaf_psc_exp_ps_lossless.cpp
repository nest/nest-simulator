/*
 *  iaf_psc_exp_ps_lossless.cpp
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

#include "iaf_psc_exp_ps_lossless.h"

// C++ includes:
#include <limits>

// Includes from nestkernel:
#include "exceptions.h"
#include "universal_data_logger_impl.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "arraydatum.h"


/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_psc_exp_ps_lossless > nest::iaf_psc_exp_ps_lossless::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_psc_exp_ps_lossless >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_psc_exp_ps_lossless::get_V_m_ );
  insert_( names::I_syn, &iaf_psc_exp_ps_lossless::get_I_syn_ );
  insert_( names::I_syn_ex, &iaf_psc_exp_ps_lossless::get_I_syn_ex_ );
  insert_( names::I_syn_in, &iaf_psc_exp_ps_lossless::get_I_syn_in_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp_ps_lossless::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                       // ms
  , tau_ex_( 2.0 )                                       // ms
  , tau_in_( 2.0 )                                       // ms
  , c_m_( 250.0 )                                        // pF
  , t_ref_( 2.0 )                                        // ms
  , E_L_( -70.0 )                                        // mV
  , I_e_( 0.0 )                                          // pA
  , U_th_( -55.0 - E_L_ )                                // mV, rel to E_L_
  , U_min_( -std::numeric_limits< double >::infinity() ) // mV
  , U_reset_( -70.0 - E_L_ )                             // mV, rel to E_L_
{
}

nest::iaf_psc_exp_ps_lossless::State_::State_()
  : y0_( 0.0 )
  , I_syn_ex_( 0.0 )
  , I_syn_in_( 0.0 )
  , y2_( 0.0 )
  , is_refractory_( false )
  , last_spike_step_( -1 )
  , last_spike_offset_( 0.0 )
{
}

nest::iaf_psc_exp_ps_lossless::Buffers_::Buffers_( iaf_psc_exp_ps_lossless& n )
  : logger_( n )
{
}

nest::iaf_psc_exp_ps_lossless::Buffers_::Buffers_( const Buffers_&, iaf_psc_exp_ps_lossless& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */
void
nest::iaf_psc_exp_ps_lossless::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, U_th_ + E_L_ );
  def< double >( d, names::V_min, U_min_ + E_L_ );
  def< double >( d, names::V_reset, U_reset_ + E_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );
  def< double >( d, names::t_ref, t_ref_ );
}

double
nest::iaf_psc_exp_ps_lossless::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double E_L_old = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_E_L = E_L_ - E_L_old;

  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::tau_syn_ex, tau_ex_, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_in_, node );
  updateValueParam< double >( d, names::C_m, c_m_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );

  if ( updateValueParam< double >( d, names::V_th, U_th_, node ) )
  {
    U_th_ -= E_L_;
  }
  else
  {
    U_th_ -= delta_E_L;
  }

  if ( updateValueParam< double >( d, names::V_min, U_min_, node ) )
  {
    U_min_ -= E_L_;
  }
  else
  {
    U_min_ -= delta_E_L;
  }

  if ( updateValueParam< double >( d, names::V_reset, U_reset_, node ) )
  {
    U_reset_ -= E_L_;
  }
  else
  {
    U_reset_ -= delta_E_L;
  }

  if ( U_reset_ >= U_th_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }

  if ( U_reset_ < U_min_ )
  {
    throw BadProperty( "Reset potential must be greater than or equal to minimum potential." );
  }

  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  if ( tau_ex_ != tau_in_ )
  {
    throw BadProperty(
      "tau_syn_ex == tau_syn_in is required in the current implementation."
      " If you need unequal time constants, use iaf_psc_exp_ps for now."
      " See note in documentation, and github issue #921" );
  }

  if ( tau_m_ <= 0 or tau_ex_ <= 0 or tau_in_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  if ( tau_m_ == tau_ex_ or tau_m_ == tau_in_ )
  {
    throw BadProperty(
      "Membrane and synapse time constant(s) must differ."
      "See note in documentation." );
  }

  return delta_E_L;
}

void
nest::iaf_psc_exp_ps_lossless::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y2_ + p.E_L_ ); // Membrane potential
  def< bool >( d, names::is_refractory, is_refractory_ );
  def< double >( d, names::t_spike, Time( Time::step( last_spike_step_ ) ).get_ms() );
  def< double >( d, names::offset, last_spike_offset_ );
  def< double >( d, names::I_syn_ex, I_syn_ex_ );
  def< double >( d, names::I_syn_in, I_syn_in_ );
  def< double >( d, names::I_syn, I_syn_ex_ + I_syn_in_ );
}

void
nest::iaf_psc_exp_ps_lossless::State_::set( const DictionaryDatum& d,
  const Parameters_& p,
  double delta_EL,
  Node* node )
{
  if ( updateValueParam< double >( d, names::V_m, y2_, node ) )
  {
    y2_ -= p.E_L_;
  }
  else
  {
    y2_ -= delta_EL;
  }

  updateValueParam< double >( d, names::I_syn_ex, I_syn_ex_, node );
  updateValueParam< double >( d, names::I_syn_in, I_syn_in_, node );
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp_ps_lossless::iaf_psc_exp_ps_lossless()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_exp_ps_lossless::iaf_psc_exp_ps_lossless( const iaf_psc_exp_ps_lossless& n )
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
nest::iaf_psc_exp_ps_lossless::init_state_( const Node& proto )
{
  const iaf_psc_exp_ps_lossless& pr = downcast< iaf_psc_exp_ps_lossless >( proto );

  S_ = pr.S_;
}

void
nest::iaf_psc_exp_ps_lossless::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();
}

void
nest::iaf_psc_exp_ps_lossless::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.h_ms_ = Time::get_resolution().get_ms();

  V_.expm1_tau_m_ = numerics::expm1( -V_.h_ms_ / P_.tau_m_ );
  V_.expm1_tau_ex_ = numerics::expm1( -V_.h_ms_ / P_.tau_ex_ );
  V_.expm1_tau_in_ = numerics::expm1( -V_.h_ms_ / P_.tau_in_ );
  V_.P20_ = -P_.tau_m_ / P_.c_m_ * V_.expm1_tau_m_;
  V_.P21_ex_ = -P_.tau_m_ * P_.tau_ex_ / ( P_.tau_m_ - P_.tau_ex_ ) / P_.c_m_ * ( V_.expm1_tau_ex_ - V_.expm1_tau_m_ );
  V_.P21_in_ = -P_.tau_m_ * P_.tau_in_ / ( P_.tau_m_ - P_.tau_in_ ) / P_.c_m_ * ( V_.expm1_tau_in_ - V_.expm1_tau_m_ );
  V_.refractory_steps_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  assert( V_.refractory_steps_ >= 0 ); // since t_ref_ >= 0, this can only fail in error

  V_.a1_ = P_.tau_m_ * P_.tau_ex_;
  V_.a2_ = P_.tau_m_ * ( P_.tau_m_ - P_.tau_ex_ );
  V_.a3_ = P_.c_m_ * P_.U_th_ * ( P_.tau_m_ - P_.tau_ex_ );
  V_.a4_ = P_.c_m_ * ( P_.tau_m_ - P_.tau_ex_ );

  V_.b1_ = -P_.tau_m_ * P_.tau_m_;
  V_.b2_ = P_.tau_m_ * P_.tau_ex_;
  V_.b3_ = P_.tau_m_ * P_.c_m_ * P_.U_th_;
  V_.b4_ = -P_.c_m_ * ( P_.tau_m_ - P_.tau_ex_ );

  V_.c1_ = P_.tau_m_ / P_.c_m_;
  V_.c2_ = ( -P_.tau_m_ * P_.tau_ex_ ) / ( P_.c_m_ * ( P_.tau_m_ - P_.tau_ex_ ) );
  V_.c3_ = ( P_.tau_m_ * P_.tau_m_ ) / ( P_.c_m_ * ( P_.tau_m_ - P_.tau_ex_ ) );
  V_.c4_ = P_.tau_ex_ / P_.tau_m_;
  V_.c5_ = ( P_.c_m_ * P_.U_th_ ) / P_.tau_m_;
  V_.c6_ = 1 - ( P_.tau_ex_ / P_.tau_m_ );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_exp_ps_lossless::update( const Time& origin, const long from, const long to )
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
  if ( S_.y2_ >= P_.U_th_ )
  {
    emit_instant_spike_( origin, from, V_.h_ms_ * ( 1.0 - std::numeric_limits< double >::epsilon() ) );
  }

  for ( long lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long T = origin.get_steps() + lag;

    // if neuron returns from refractoriness during this step, place
    // pseudo-event in queue to mark end of refractory period
    if ( S_.is_refractory_ && ( T + 1 - S_.last_spike_step_ == V_.refractory_steps_ ) )
    {
      B_.events_.add_refractory( T, S_.last_spike_offset_ );
    }

    // save state at beginning of interval for spike-time approximation
    V_.y0_before_ = S_.y0_;
    V_.I_syn_ex_before_ = S_.I_syn_ex_;
    V_.I_syn_in_before_ = S_.I_syn_in_;
    V_.y2_before_ = S_.y2_;

    // get first event
    double ev_offset;
    double ev_weight;
    bool end_of_refract;

    if ( not B_.events_.get_next_spike( T, false, ev_offset, ev_weight, end_of_refract ) )
    {
      // No incoming spikes, handle with fixed propagator matrix.
      // Handling this case separately improves performance significantly
      // if there are many steps without input spikes.

      // update membrane potential
      if ( not S_.is_refractory_ )
      {
        S_.y2_ = V_.P20_ * ( P_.I_e_ + S_.y0_ ) + V_.P21_ex_ * S_.I_syn_ex_ + V_.P21_in_ * S_.I_syn_in_
          + V_.expm1_tau_m_ * S_.y2_ + S_.y2_;

        // lower bound of membrane potential
        S_.y2_ = ( S_.y2_ < P_.U_min_ ? P_.U_min_ : S_.y2_ );
      }

      // update synaptic currents
      S_.I_syn_ex_ = S_.I_syn_ex_ * V_.expm1_tau_ex_ + S_.I_syn_ex_;
      S_.I_syn_in_ = S_.I_syn_in_ * V_.expm1_tau_in_ + S_.I_syn_in_;

      /* The following must not be moved before the y1_, y2_ update,
         since the spike-time interpolation within emit_spike_ depends
         on all state variables having their values at the end of the
         interval.
      */

      const double spike_time_max = is_spike_( V_.h_ms_ );
      if ( not numerics::is_nan( spike_time_max ) )
      {
        emit_spike_( origin, lag, 0, spike_time_max );
      }
    }
    else
    {
      // We only get here if there is at least one event,
      // which has been read above.  We can therefore use
      // a do-while loop.

      // Time within step is measured by offsets, which are h at the beginning
      // and 0 at the end of the step.
      double last_offset = V_.h_ms_; // start of step

      do
      {
        // time is measured backward: inverse order in difference
        const double ministep = last_offset - ev_offset;
        assert( ministep >= 0.0 );

        // dt == 0 may occur if two spikes arrive simultaneously;
        // no propagation in that case; see #368
        if ( ministep > 0 )
        {
          propagate_( ministep );

          // check for threshold crossing during ministep
          // this must be done before adding the input, since
          // interpolation requires continuity
          const double spike_time_max = is_spike_( ministep );

          if ( not numerics::is_nan( spike_time_max ) )
          {
            emit_spike_( origin, lag, V_.h_ms_ - last_offset, spike_time_max );
          }
        }

        // handle event
        if ( end_of_refract )
        {
          S_.is_refractory_ = false; // return from refractoriness
        }
        else
        {
          if ( ev_weight >= 0.0 )
          {
            S_.I_syn_ex_ += ev_weight; // exc. spike input
          }
          else
          {
            S_.I_syn_in_ += ev_weight; // inh. spike input
          }
        }

        // store state
        V_.I_syn_ex_before_ = S_.I_syn_ex_;
        V_.I_syn_in_before_ = S_.I_syn_in_;
        V_.y2_before_ = S_.y2_;
        last_offset = ev_offset;
      } while ( B_.events_.get_next_spike( T, false, ev_offset, ev_weight, end_of_refract ) );

      // no events remaining, plain update step across remainder
      // of interval
      if ( last_offset > 0 ) // not at end of step, do remainder
      {
        const double spike_time_max = is_spike_( last_offset );
        propagate_( last_offset );
        if ( not numerics::is_nan( spike_time_max ) )
        {
          emit_spike_( origin, lag, V_.h_ms_ - last_offset, spike_time_max );
        }
      }
    } // else

    // Set new input current. The current change occurs at the
    // end of the interval and thus must come AFTER the threshold-
    // crossing approximation
    S_.y0_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  } // for
}

// function handles exact spike times
void
nest::iaf_psc_exp_ps_lossless::handle( SpikeEvent& e )
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
nest::iaf_psc_exp_ps_lossless::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( nest::kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::iaf_psc_exp_ps_lossless::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

// auxiliary functions ---------------------------------------------

void
nest::iaf_psc_exp_ps_lossless::propagate_( const double dt )
{
  // dt == 0 may occur if two spikes arrive simultaneously;
  // propagate_() shall not be called then; see #368.
  assert( dt > 0 );

  const double expm1_tau_ex = numerics::expm1( -dt / P_.tau_ex_ );
  const double expm1_tau_in = numerics::expm1( -dt / P_.tau_in_ );

  if ( not S_.is_refractory_ )
  {
    const double expm1_tau_m = numerics::expm1( -dt / P_.tau_m_ );

    const double P20 = -P_.tau_m_ / P_.c_m_ * expm1_tau_m;
    const double P21_ex =
      -P_.tau_m_ * P_.tau_ex_ / ( P_.tau_m_ - P_.tau_ex_ ) / P_.c_m_ * ( expm1_tau_ex - expm1_tau_m );
    const double P21_in =
      -P_.tau_m_ * P_.tau_in_ / ( P_.tau_m_ - P_.tau_in_ ) / P_.c_m_ * ( expm1_tau_in - expm1_tau_m );

    S_.y2_ = P20 * ( P_.I_e_ + S_.y0_ ) + P21_ex * S_.I_syn_ex_ + P21_in * S_.I_syn_in_ + expm1_tau_m * S_.y2_ + S_.y2_;
  }

  S_.I_syn_ex_ = S_.I_syn_ex_ * expm1_tau_ex + S_.I_syn_ex_;
  S_.I_syn_in_ = S_.I_syn_in_ * expm1_tau_in + S_.I_syn_in_;
}

void
nest::iaf_psc_exp_ps_lossless::emit_spike_( const Time& origin, const long lag, const double t0, const double dt )
{
  // dt == 0 may occur if two spikes arrive simultaneously;
  // emit_spike_() shall not be called then; see #368.
  assert( dt > 0 );

  // we know that the potential is subthreshold at t0, super at t0+dt

  // compute spike time relative to beginning of step
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = V_.h_ms_ - ( t0 + bisectioning_( dt ) );

  // reset neuron and make it refractory
  S_.y2_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;

  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );
}

void
nest::iaf_psc_exp_ps_lossless::emit_instant_spike_( const Time& origin, const long lag, const double spike_offs )
{
  assert( S_.y2_ >= P_.U_th_ ); // ensure we are superthreshold

  // set stamp and offset for spike
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = spike_offs;

  // reset neuron and make it refractory
  S_.y2_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;

  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );
}

inline double
nest::iaf_psc_exp_ps_lossless::bisectioning_( const double dt ) const
{
  double root = 0.0;
  double y2_root = V_.y2_before_;
  double div = 2.0;
  while ( fabs( P_.U_th_ - y2_root ) > 1e-14 and ( dt / div > 0.0 ) )
  {
    if ( y2_root > P_.U_th_ )
    {
      root -= dt / div;
    }
    else
    {
      root += dt / div;
    }

    div *= 2.0;

    const double expm1_tau_ex = numerics::expm1( -root / P_.tau_ex_ );
    const double expm1_tau_in = numerics::expm1( -root / P_.tau_in_ );
    const double expm1_tau_m = numerics::expm1( -root / P_.tau_m_ );

    const double P20 = -P_.tau_m_ / P_.c_m_ * expm1_tau_m;
    const double P21_ex =
      -P_.tau_m_ * P_.tau_ex_ / ( P_.tau_m_ - P_.tau_ex_ ) / P_.c_m_ * ( expm1_tau_ex - expm1_tau_m );
    const double P21_in =
      -P_.tau_m_ * P_.tau_in_ / ( P_.tau_m_ - P_.tau_in_ ) / P_.c_m_ * ( expm1_tau_in - expm1_tau_m );

    y2_root = P20 * ( P_.I_e_ + V_.y0_before_ ) + P21_ex * V_.I_syn_ex_before_ + P21_in * V_.I_syn_in_before_
      + expm1_tau_m * V_.y2_before_ + V_.y2_before_;
  }
  return root;
}

double
nest::iaf_psc_exp_ps_lossless::is_spike_( const double dt )
{
  // dt == 0 may occur if two spikes arrive simultaneously;
  // is_spike_() shall not be called then; see #368.
  assert( dt > 0 );

  // synapse time constants are assumed to be equal in this implementation
  assert( P_.tau_ex_ == P_.tau_in_ );

  const double I_0 = V_.I_syn_ex_before_ + V_.I_syn_in_before_;
  const double V_0 = V_.y2_before_;
  const double exp_tau_s = numerics::expm1( dt / P_.tau_ex_ );
  const double exp_tau_m = numerics::expm1( dt / P_.tau_m_ );
  const double exp_tau_m_s = numerics::expm1( dt / P_.tau_m_ - dt / P_.tau_ex_ );
  const double I_e = V_.y0_before_ + P_.I_e_;

  /* Expressions for f and b below are rewritten but equivalent
     to those given in Krishnan et al. 2018.
     The expression for g given in the paper as eq.(49) is incorrect.
     It can instead be constructed as a line through the points (see Fig.6):
     (I_theta-I_e, V_th) and (i2, f(i2)) where i2=(I_theta-I_e)*exp(dt/tau_s).

     Note that there is a typo in Algorithm 1 and 2 of the paper:
     g and f are interchanged. (compare to Fig.6)
  */

  const double f = ( ( V_.a1_ * I_0 * exp_tau_m_s + exp_tau_m * ( V_.a3_ - I_e * V_.a2_ ) + V_.a3_ ) / V_.a4_ );


  // no-spike, NS_1, (V <= g_h,I_e(I) and V < f_h,I_e(I))
  if ( ( V_0 < ( ( ( I_0 + I_e ) * ( V_.b1_ * exp_tau_m + V_.b2_ * exp_tau_s ) + V_.b3_ * ( exp_tau_m - exp_tau_s ) )
                 / ( V_.b4_ * exp_tau_s ) ) ) and ( V_0 <= f ) )
  {
    return numerics::nan;
  }

  // spike, S_1, V >= f_h,I_e(I)
  else if ( V_0 >= f )
  {
    return dt;
  }
  // no-spike, NS_2, V < b(I)
  else if ( V_0
    < ( V_.c1_ * I_e + V_.c2_ * I_0 + V_.c3_ * std::pow( I_0, V_.c4_ ) * std::pow( ( V_.c5_ - I_e ), V_.c6_ ) ) )
  {
    return numerics::nan;
  }
  else
  // missed spike detected, S_2
  {
    return ( V_.a1_ / P_.tau_m_ * P_.tau_ex_ )
      * std::log( V_.b1_ * I_0 / ( V_.a2_ * I_e - V_.a1_ * I_0 - V_.a4_ * V_0 ) );
  }
}
