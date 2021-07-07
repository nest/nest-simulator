/*
 *  iaf_psc_alpha_canon.cpp
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

#include "iaf_psc_alpha_canon.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
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

nest::RecordablesMap< nest::iaf_psc_alpha_canon > nest::iaf_psc_alpha_canon::recordablesMap_;

namespace nest
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_psc_alpha_canon >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &iaf_psc_alpha_canon::get_V_m_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_canon::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                       // ms
  , tau_syn_( 2.0 )                                      // ms
  , c_m_( 250.0 )                                        // pF
  , t_ref_( 2.0 )                                        // ms
  , E_L_( -70.0 )                                        // mV
  , I_e_( 0.0 )                                          // pA
  , U_th_( -55.0 - E_L_ )                                // mV, rel to E_L_
  , U_min_( -std::numeric_limits< double >::infinity() ) // mV
  , U_reset_( -70.0 - E_L_ )                             // mV, rel to E_L_
  , Interpol_( iaf_psc_alpha_canon::LINEAR )
{
}

nest::iaf_psc_alpha_canon::State_::State_()
  : y0_( 0.0 )
  , y1_( 0.0 )
  , y2_( 0.0 )
  , y3_( 0.0 )
  , is_refractory_( false )
  , last_spike_step_( -1 )
  , last_spike_offset_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_alpha_canon::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, U_th_ + E_L_ );
  def< double >( d, names::V_min, U_min_ + E_L_ );
  def< double >( d, names::V_reset, U_reset_ + E_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::tau_syn, tau_syn_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< long >( d, names::Interpol_Order, Interpol_ );
}

double
nest::iaf_psc_alpha_canon::Parameters_::set( const DictionaryDatum& d )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValue< double >( d, names::E_L, E_L_ );
  const double delta_EL = E_L_ - ELold;

  updateValue< double >( d, names::tau_m, tau_m_ );
  updateValue< double >( d, names::tau_syn, tau_syn_ );
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

  long tmp;
  if ( updateValue< long >( d, names::Interpol_Order, tmp ) )
  {
    if ( NO_INTERPOL <= tmp && tmp < END_INTERP_ORDER )
    {
      Interpol_ = static_cast< interpOrder >( tmp );
    }
    else
    {
      throw BadProperty(
        "Invalid interpolation order. "
        "Valid orders are 0, 1, 2, 3." );
    }
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
  if ( tau_m_ <= 0 || tau_syn_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  return delta_EL;
}

void
nest::iaf_psc_alpha_canon::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ ); // Membrane potential
  def< double >( d, names::y1, y1_ );           // y1 state
  def< double >( d, names::y2, y2_ );           // y2 state
  def< bool >( d, names::is_refractory, is_refractory_ );
}

void
nest::iaf_psc_alpha_canon::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, y3_ ) )
  {
    y3_ -= p.E_L_;
  }
  else
  {
    y3_ -= delta_EL;
  }

  updateValue< double >( d, names::y1, y1_ );
  updateValue< double >( d, names::y2, y2_ );
}

nest::iaf_psc_alpha_canon::Buffers_::Buffers_( iaf_psc_alpha_canon& n )
  : logger_( n )
{
}

nest::iaf_psc_alpha_canon::Buffers_::Buffers_( const Buffers_&, iaf_psc_alpha_canon& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_canon::iaf_psc_alpha_canon()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_alpha_canon::iaf_psc_alpha_canon( const iaf_psc_alpha_canon& n )
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
nest::iaf_psc_alpha_canon::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear();
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();

  ArchivingNode::clear_history();
}

void
nest::iaf_psc_alpha_canon::calibrate()
{
  B_.logger_.init();

  V_.h_ms_ = Time::get_resolution().get_ms();

  V_.PSCInitialValue_ = 1.0 * numerics::e / P_.tau_syn_;

  V_.gamma_ = 1 / P_.c_m_ / ( 1 / P_.tau_syn_ - 1 / P_.tau_m_ );
  V_.gamma_sq_ = 1 / P_.c_m_ / ( ( 1 / P_.tau_syn_ - 1 / P_.tau_m_ ) * ( 1 / P_.tau_syn_ - 1 / P_.tau_m_ ) );

  // pre-compute matrix for full time step
  V_.expm1_tau_m_ = numerics::expm1( -V_.h_ms_ / P_.tau_m_ );
  V_.expm1_tau_syn_ = numerics::expm1( -V_.h_ms_ / P_.tau_syn_ );
  V_.P30_ = -P_.tau_m_ / P_.c_m_ * V_.expm1_tau_m_;
  // these are determined according to a numeric stability criterion
  V_.P31_ = propagator_31( P_.tau_syn_, P_.tau_m_, P_.c_m_, V_.h_ms_ );
  V_.P32_ = propagator_32( P_.tau_syn_, P_.tau_m_, P_.c_m_, V_.h_ms_ );

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

void
nest::iaf_psc_alpha_canon::update( Time const& origin, const long from, const long to )
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
  if ( S_.y3_ >= P_.U_th_ )
  {
    emit_instant_spike_( origin, from, V_.h_ms_ * ( 1 - std::numeric_limits< double >::epsilon() ) );
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

    // save state at beginning of interval for spike-time interpolation
    V_.y0_before_ = S_.y0_;
    V_.y2_before_ = S_.y2_;
    V_.y3_before_ = S_.y3_;

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
        S_.y3_ =
          V_.P30_ * ( P_.I_e_ + S_.y0_ ) + V_.P31_ * S_.y1_ + V_.P32_ * S_.y2_ + V_.expm1_tau_m_ * S_.y3_ + S_.y3_;

        // lower bound of membrane potential
        S_.y3_ = ( S_.y3_ < P_.U_min_ ? P_.U_min_ : S_.y3_ );
      }

      // update synaptic currents
      S_.y2_ = V_.expm1_tau_syn_ * V_.h_ms_ * S_.y1_ + V_.expm1_tau_syn_ * S_.y2_ + V_.h_ms_ * S_.y1_ + S_.y2_;
      S_.y1_ = V_.expm1_tau_syn_ * S_.y1_ + S_.y1_;

      /* The following must not be moved before the y1_, y2_ update,
         since the spike-time interpolation within emit_spike_ depends
         on all state variables having their values at the end of the
         interval.
      */
      if ( S_.y3_ >= P_.U_th_ )
      {
        emit_spike_( origin, lag, 0, V_.h_ms_ );
      }
    }
    else
    {
      // We only get here if there is at least on event,
      // which has been read above.  We can therefore use
      // a do-while loop.

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
        if ( S_.y3_ >= P_.U_th_ )
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
          S_.y1_ += V_.PSCInitialValue_ * ev_weight;
        } // spike input

        // store state
        V_.y2_before_ = S_.y2_;
        V_.y3_before_ = S_.y3_;
        last_offset = ev_offset;

      } while ( B_.events_.get_next_spike( T, true, ev_offset, ev_weight, end_of_refract ) );

      // no events remaining, plain update step across remainder
      // of interval
      if ( last_offset > 0 ) // not at end of step, do remainder
      {
        propagate_( last_offset );
        if ( S_.y3_ >= P_.U_th_ )
        {
          emit_spike_( origin, lag, V_.h_ms_ - last_offset, last_offset );
        }
      }
    } // else

    // Set new input current. The current change occurs at the
    // end of the interval and thus must come AFTER the threshold-
    // crossing interpolation
    S_.y0_ = B_.currents_.get_value( lag );


    // logging
    B_.logger_.record_data( origin.get_steps() + lag );
  } // from lag = from ...
}


// function handles exact spike times
void
nest::iaf_psc_alpha_canon::handle( SpikeEvent& e )
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
nest::iaf_psc_alpha_canon::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( nest::kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::iaf_psc_alpha_canon::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

// auxiliary functions ---------------------------------------------

void
nest::iaf_psc_alpha_canon::propagate_( const double dt )
{
  // needed in any case
  const double ps_e_TauSyn = numerics::expm1( -dt / P_.tau_syn_ );

  // y3_ remains unchanged at 0.0 while neuron is refractory
  if ( not S_.is_refractory_ )
  {
    const double ps_e_Tau = numerics::expm1( -dt / P_.tau_m_ );
    const double ps_P30 = -P_.tau_m_ / P_.c_m_ * ps_e_Tau;
    const double ps_P31 =
      V_.gamma_sq_ * ps_e_Tau - V_.gamma_sq_ * ps_e_TauSyn - dt * V_.gamma_ * ps_e_TauSyn - dt * V_.gamma_;
    const double ps_P32 = V_.gamma_ * ps_e_Tau - V_.gamma_ * ps_e_TauSyn;
    S_.y3_ = ps_P30 * ( P_.I_e_ + S_.y0_ ) + ps_P31 * S_.y1_ + ps_P32 * S_.y2_ + ps_e_Tau * S_.y3_ + S_.y3_;

    // lower bound of membrane potential
    S_.y3_ = ( S_.y3_ < P_.U_min_ ? P_.U_min_ : S_.y3_ );
  }

  // now the synaptic components
  S_.y2_ = ps_e_TauSyn * dt * S_.y1_ + ps_e_TauSyn * S_.y2_ + dt * S_.y1_ + S_.y2_;
  S_.y1_ = ps_e_TauSyn * S_.y1_ + S_.y1_;

  return;
}

void
nest::iaf_psc_alpha_canon::emit_spike_( Time const& origin, const long lag, const double t0, const double dt )
{
  // we know that the potential is subthreshold at t0, super at t0+dt

  // compute spike time relative to beginning of step
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = V_.h_ms_ - ( t0 + thresh_find_( dt ) );

  // reset neuron and make it refractory
  S_.y3_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;
  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );

  return;
}

void
nest::iaf_psc_alpha_canon::emit_instant_spike_( Time const& origin, const long lag, const double spike_offs )
{
  assert( S_.y3_ >= P_.U_th_ ); // ensure we are superthreshold

  // set stamp and offset for spike
  S_.last_spike_step_ = origin.get_steps() + lag + 1;
  S_.last_spike_offset_ = spike_offs;

  // reset neuron and make it refractory
  S_.y3_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );
  SpikeEvent se;
  se.set_offset( S_.last_spike_offset_ );
  kernel().event_delivery_manager.send( *this, se, lag );

  return;
}

// finds threshpassing
inline double
nest::iaf_psc_alpha_canon::thresh_find_( double const dt ) const
{
  switch ( P_.Interpol_ )
  {
  case NO_INTERPOL:
    return dt;
  case LINEAR:
    return thresh_find1_( dt );
  case QUADRATIC:
    return thresh_find2_( dt );
  case CUBIC:
    return thresh_find3_( dt );
  default:
    throw BadProperty( "Invalid interpolation order in iaf_psc_alpha_canon." );
  }
  return 0;
}

// finds threshpassing via linear interpolation
double
nest::iaf_psc_alpha_canon::thresh_find1_( double const dt ) const
{
  double tau = ( P_.U_th_ - V_.y3_before_ ) * dt / ( S_.y3_ - V_.y3_before_ );
  return tau;
}

// finds threshpassing via quadratic interpolation
double
nest::iaf_psc_alpha_canon::thresh_find2_( double const dt ) const
{
  const double h_sq = dt * dt;
  const double derivative = -V_.y3_before_ / P_.tau_m_ + ( P_.I_e_ + V_.y0_before_ + V_.y2_before_ ) / P_.c_m_;

  const double a = ( -V_.y3_before_ / h_sq ) + ( S_.y3_ / h_sq ) - ( derivative / dt );
  const double b = derivative;
  const double c = V_.y3_before_;

  const double sqr_ = std::sqrt( b * b - 4 * a * c + 4 * a * P_.U_th_ );
  const double tau1 = ( -b + sqr_ ) / ( 2 * a );
  const double tau2 = ( -b - sqr_ ) / ( 2 * a );
  if ( tau1 >= 0 )
  {
    return tau1;
  }
  else if ( tau2 >= 0 )
  {
    return tau2;
  }
  else
  {
    return thresh_find1_( dt );
  }
}

double
nest::iaf_psc_alpha_canon::thresh_find3_( double const dt ) const
{
  const double h_ms = dt;
  const double h_sq = h_ms * h_ms;
  const double h_cb = h_sq * h_ms;

  const double deriv_t1 = -V_.y3_before_ / P_.tau_m_ + ( P_.I_e_ + V_.y0_before_ + V_.y2_before_ ) / P_.c_m_;
  const double deriv_t2 = -S_.y3_ / P_.tau_m_ + ( P_.I_e_ + S_.y0_ + S_.y2_ ) / P_.c_m_;

  const double w3_ = ( 2 * V_.y3_before_ / h_cb ) - ( 2 * S_.y3_ / h_cb ) + ( deriv_t1 / h_sq ) + ( deriv_t2 / h_sq );
  const double w2_ =
    -( 3 * V_.y3_before_ / h_sq ) + ( 3 * S_.y3_ / h_sq ) - ( 2 * deriv_t1 / h_ms ) - ( deriv_t2 / h_ms );
  const double w1_ = deriv_t1;
  const double w0_ = V_.y3_before_;

  // normal form :    x^3 + r*x^2 + s*x + t with coefficients : r, s, t
  const double r = w2_ / w3_;
  const double s = w1_ / w3_;
  const double t = ( w0_ - P_.U_th_ ) / w3_;
  const double r_sq = r * r;

  // substitution y = x + r/3 :  y^3 + p*y + q == 0
  const double p = -r_sq / 3 + s;
  const double q = 2 * ( r_sq * r ) / 27 - r * s / 3 + t;

  // discriminante
  const double D = std::pow( ( p / 3 ), 3 ) + std::pow( ( q / 2 ), 2 );

  double tau1;
  double tau2;
  double tau3;

  if ( D < 0 )
  {
    const double roh = std::sqrt( -( p * p * p ) / 27 );
    const double phi = std::acos( -q / ( 2 * roh ) );
    const double a = 2 * std::pow( roh, ( 1.0 / 3.0 ) );
    tau1 = ( a * std::cos( phi / 3 ) ) - r / 3;
    tau2 = ( a * std::cos( phi / 3 + 2 * numerics::pi / 3 ) ) - r / 3;
    tau3 = ( a * std::cos( phi / 3 + 4 * numerics::pi / 3 ) ) - r / 3;
  }
  else
  {
    const double sgnq = ( q >= 0 ? 1 : -1 );
    const double u = -sgnq * std::pow( std::fabs( q ) / 2.0 + std::sqrt( D ), 1.0 / 3.0 );
    const double v = -p / ( 3 * u );
    tau1 = ( u + v ) - r / 3;
    if ( tau1 >= 0 )
    {
      return tau1;
    }
    else
    {
      return thresh_find2_( dt );
    }
  }

  // set tau to the smallest root above 0

  double tau = ( tau1 >= 0 ) ? tau1 : 2 * h_ms;
  if ( ( tau2 >= 0 ) && ( tau2 < tau ) )
  {
    tau = tau2;
  }
  if ( ( tau3 >= 0 ) && ( tau3 < tau ) )
  {
    tau = tau3;
  }
  return ( tau <= V_.h_ms_ ) ? tau : thresh_find2_( dt );
}
