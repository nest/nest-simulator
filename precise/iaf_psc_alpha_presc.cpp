/*
 *  iaf_psc_alpha_presc.cpp
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

#include "iaf_psc_alpha_presc.h"

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

nest::RecordablesMap< nest::iaf_psc_alpha_presc >
  nest::iaf_psc_alpha_presc::recordablesMap_;

namespace nest
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_psc_alpha_presc >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_psc_alpha_presc::get_V_m_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_presc::Parameters_::Parameters_()
  : tau_m_( 10.0 )        // ms
  , tau_syn_( 2.0 )       // ms
  , c_m_( 250.0 )         // pF
  , t_ref_( 2.0 )         // ms
  , E_L_( -70.0 )         // mV
  , I_e_( 0.0 )           // pA
  , U_th_( -55.0 - E_L_ ) // mV, rel to E_L_
  , U_min_( -std::numeric_limits< double >::infinity() )
  , U_reset_( -70.0 - E_L_ )
  , Interpol_( iaf_psc_alpha_presc::LINEAR )
{
}

nest::iaf_psc_alpha_presc::State_::State_()
  : y0_( 0.0 )
  , y1_( 0.0 )
  , y2_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
  , last_spike_step_( -1 )
  , last_spike_offset_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_psc_alpha_presc::Parameters_::get( DictionaryDatum& d ) const
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
nest::iaf_psc_alpha_presc::Parameters_::set( const DictionaryDatum& d )
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
    throw BadProperty(
      "Reset potential must be greater equal minimum potential." );
  }
  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }
  if ( tau_m_ <= 0 || tau_syn_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  return delta_EL;
}

void
nest::iaf_psc_alpha_presc::State_::get( DictionaryDatum& d,
  const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ ); // Membrane potential
}

void
nest::iaf_psc_alpha_presc::State_::set( const DictionaryDatum& d,
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

nest::iaf_psc_alpha_presc::Buffers_::Buffers_( iaf_psc_alpha_presc& n )
  : logger_( n )
{
}

nest::iaf_psc_alpha_presc::Buffers_::Buffers_( const Buffers_&,
  iaf_psc_alpha_presc& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_alpha_presc::iaf_psc_alpha_presc()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_psc_alpha_presc::iaf_psc_alpha_presc( const iaf_psc_alpha_presc& n )
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
nest::iaf_psc_alpha_presc::init_state_( const Node& proto )
{
  const iaf_psc_alpha_presc& pr = downcast< iaf_psc_alpha_presc >( proto );
  S_ = pr.S_;
}

void
nest::iaf_psc_alpha_presc::init_buffers_()
{
  B_.spike_y1_.clear(); // includes resize
  B_.spike_y2_.clear(); // includes resize
  B_.spike_y3_.clear(); // includes resize
  B_.currents_.clear(); // includes resize

  B_.logger_.reset();

  Archiving_Node::clear_history();
}

void
nest::iaf_psc_alpha_presc::calibrate()
{
  B_.logger_.init();

  V_.h_ms_ = Time::get_resolution().get_ms();

  V_.PSCInitialValue_ = 1.0 * numerics::e / P_.tau_syn_;

  V_.gamma_ = 1 / P_.c_m_ / ( 1 / P_.tau_syn_ - 1 / P_.tau_m_ );
  V_.gamma_sq_ = 1 / P_.c_m_ / ( ( 1 / P_.tau_syn_ - 1 / P_.tau_m_ )
                                 * ( 1 / P_.tau_syn_ - 1 / P_.tau_m_ ) );

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
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.refractory_steps_ >= 0 );
}


void
nest::iaf_psc_alpha_presc::update( Time const& origin,
  const long from,
  const long to )
{
  assert( to >= 0 );
  assert( static_cast< delay >( from )
    < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  /* Neurons may have been initialized to superthreshold potentials.
     We need to check for this here and issue spikes at the beginning of
     the interval.
  */
  if ( S_.y3_ >= P_.U_th_ )
  {
    S_.last_spike_step_ = origin.get_steps() + from + 1;
    S_.last_spike_offset_ =
      V_.h_ms_ * ( 1 - std::numeric_limits< double >::epsilon() );

    // reset neuron and make it refractory
    S_.y3_ = P_.U_reset_;
    S_.r_ = V_.refractory_steps_;

    // send spike
    set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );

    SpikeEvent se;
    se.set_offset( S_.last_spike_offset_ );
    kernel().event_delivery_manager.send( *this, se, from );
  }

  for ( long lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long T = origin.get_steps() + lag;

    // save state at beginning of interval for spike-time interpolation
    V_.y0_before_ = S_.y0_;
    V_.y1_before_ = S_.y1_;
    V_.y2_before_ = S_.y2_;
    V_.y3_before_ = S_.y3_;

    /* obtain input to y3_
       We need to collect this value even while the neuron is refractory,
       since we need to clear any spikes that have come in from the
       ring buffer.
    */
    const double dy3 = B_.spike_y3_.get_value( lag );

    if ( S_.r_ == 0 )
    {
      // neuron is not refractory
      S_.y3_ = V_.P30_ * ( P_.I_e_ + S_.y0_ ) + V_.P31_ * S_.y1_
        + V_.P32_ * S_.y2_ + V_.expm1_tau_m_ * S_.y3_ + S_.y3_;

      S_.y3_ += dy3; // add input
      // enforce lower bound
      S_.y3_ = ( S_.y3_ < P_.U_min_ ? P_.U_min_ : S_.y3_ );
    }
    else if ( S_.r_ == 1 )
    {
      // neuron returns from refractoriness during interval
      S_.r_ = 0;

      // Iterate third component (membrane pot) from end of
      // refractory period to end of interval.  As first-order
      // approximation, add a proportion of the effect of synaptic
      // input during the interval to membrane pot.  The proportion
      // is given by the part of the interval after the end of the
      // refractory period.
      S_.y3_ = P_.U_reset_ + // try fix 070623, md
        update_y3_delta_() + dy3
        - dy3 * ( 1 - S_.last_spike_offset_ / V_.h_ms_ );

      // enforce lower bound
      S_.y3_ = ( S_.y3_ < P_.U_min_ ? P_.U_min_ : S_.y3_ );
    }
    else
    {
      // neuron is refractory
      // y3_ remains unchanged at 0.0
      --S_.r_;
    }

    // update synaptic currents
    S_.y2_ = V_.expm1_tau_syn_ * V_.h_ms_ * S_.y1_ + V_.expm1_tau_syn_ * S_.y2_
      + V_.h_ms_ * S_.y1_ + S_.y2_;
    S_.y1_ = V_.expm1_tau_syn_ * S_.y1_ + S_.y1_;

    // add synaptic inputs from the ring buffer
    // this must happen BEFORE threshold-crossing interpolation,
    // since synaptic inputs occured during the interval
    S_.y1_ += B_.spike_y1_.get_value( lag );
    S_.y2_ += B_.spike_y2_.get_value( lag );


    // neuron spikes
    if ( S_.y3_ >= P_.U_th_ )
    {
      // compute spike time
      S_.last_spike_step_ = T + 1;

      // The time for the threshpassing
      S_.last_spike_offset_ = V_.h_ms_ - thresh_find_( V_.h_ms_ );

      // reset AFTER spike-time interpolation
      S_.y3_ = P_.U_reset_;
      S_.r_ = V_.refractory_steps_;

      // sent event
      set_spiketime( Time::step( S_.last_spike_step_ ), S_.last_spike_offset_ );

      SpikeEvent se;
      se.set_offset( S_.last_spike_offset_ );
      kernel().event_delivery_manager.send( *this, se, lag );
    }

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
nest::iaf_psc_alpha_presc::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const long Tdeliver = e.get_rel_delivery_steps(
    nest::kernel().simulation_manager.get_slice_origin() );

  const double spike_weight =
    V_.PSCInitialValue_ * e.get_weight() * e.get_multiplicity();
  const double dt = e.get_offset();

  // Building the new matrix for the offset of the spike
  // NOTE: We do not use get matrix, but compute only those
  //       components we actually need for spike registration
  // needed in any case
  const double ps_e_TauSyn = numerics::expm1( -dt / P_.tau_syn_ );
  const double ps_e_Tau = numerics::expm1( -dt / P_.tau_m_ );
  const double ps_P31 = V_.gamma_sq_ * ps_e_Tau - V_.gamma_sq_ * ps_e_TauSyn
    - dt * V_.gamma_ * ps_e_TauSyn - dt * V_.gamma_;

  B_.spike_y1_.add_value( Tdeliver, spike_weight * ps_e_TauSyn + spike_weight );
  B_.spike_y2_.add_value(
    Tdeliver, spike_weight * dt * ps_e_TauSyn + spike_weight * dt );
  B_.spike_y3_.add_value( Tdeliver, spike_weight * ps_P31 );
}

void
nest::iaf_psc_alpha_presc::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps(
      nest::kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::iaf_psc_alpha_presc::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

// auxiliary functions ---------------------------------------------

double
nest::iaf_psc_alpha_presc::update_y3_delta_() const
{
  /* We need to proceed in two steps:
     1. Update the synaptic currents as far as h_ms-last_spike_offset, when the
        refractory period ends.  y3_ is clamped to 0 during this time.
     2. Update y3_ from t_th to the end of the interval.  The synaptic
        currents need not be updated during this time, since they are
        anyways updated for the entire interval outside.

     Instead of calling get_matrix(), we compute only those components
     we actually need locally.
  */

  // update synaptic currents
  const double t_th = V_.h_ms_ - S_.last_spike_offset_;
  double ps_e_TauSyn = numerics::expm1( -t_th / P_.tau_syn_ );

  // ps_y2_ = ps_P21_*y1_before_ + ps_P22_* y2_before_;
  const double ps_y2 = t_th * ps_e_TauSyn * V_.y1_before_
    + ps_e_TauSyn * V_.y2_before_ + t_th * V_.y1_before_ + V_.y2_before_;

  // ps_y1_ = y1_before_*ps_P11_;
  const double ps_y1 = ps_e_TauSyn * V_.y1_before_ + V_.y1_before_;

  // update y3_ over remaineder of interval
  const double dt = S_.last_spike_offset_;
  ps_e_TauSyn = numerics::expm1( -dt / P_.tau_syn_ );
  const double ps_e_Tau = numerics::expm1( -dt / P_.tau_m_ );
  const double ps_P30 = -P_.tau_m_ / P_.c_m_ * ps_e_Tau;
  const double ps_P31 = V_.gamma_sq_ * ps_e_Tau - V_.gamma_sq_ * ps_e_TauSyn
    - dt * V_.gamma_ * ps_e_TauSyn - dt * V_.gamma_;
  const double ps_P32 = V_.gamma_ * ps_e_Tau - V_.gamma_ * ps_e_TauSyn;

  // y3_ == 0.0 at beginning of sub-step
  return ps_P30 * ( P_.I_e_ + V_.y0_before_ ) + ps_P31 * ps_y1 + ps_P32 * ps_y2;
}


// finds threshpassing
inline double
nest::iaf_psc_alpha_presc::thresh_find_( double const dt ) const
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
    LOG( M_ERROR,
      "iaf_psc_alpha_presc::thresh_find_()",
      "Invalid interpolation---Internal model error." );
    throw BadProperty();
  }
  return 0;
}

// finds threshpassing via linear interpolation
double
nest::iaf_psc_alpha_presc::thresh_find1_( double const dt ) const
{
  double tau = ( P_.U_th_ - V_.y3_before_ ) * dt / ( S_.y3_ - V_.y3_before_ );
  return tau;
}

// finds threshpassing via quadratic interpolation
double
nest::iaf_psc_alpha_presc::thresh_find2_( double const dt ) const
{
  const double h_sq = dt * dt;
  const double derivative = -V_.y3_before_ / P_.tau_m_
    + ( P_.I_e_ + V_.y0_before_ + V_.y2_before_ ) / P_.c_m_;

  const double a =
    ( -V_.y3_before_ / h_sq ) + ( S_.y3_ / h_sq ) - ( derivative / dt );
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
nest::iaf_psc_alpha_presc::thresh_find3_( double const dt ) const
{
  const double h_ms_ = dt;
  const double h_sq = h_ms_ * h_ms_;
  const double h_cb = h_sq * h_ms_;

  const double deriv_t1 = -V_.y3_before_ / P_.tau_m_
    + ( P_.I_e_ + V_.y0_before_ + V_.y2_before_ ) / P_.c_m_;
  const double deriv_t2 =
    -S_.y3_ / P_.tau_m_ + ( P_.I_e_ + S_.y0_ + S_.y2_ ) / P_.c_m_;

  const double w3_ = ( 2 * V_.y3_before_ / h_cb ) - ( 2 * S_.y3_ / h_cb )
    + ( deriv_t1 / h_sq ) + ( deriv_t2 / h_sq );
  const double w2_ = -( 3 * V_.y3_before_ / h_sq ) + ( 3 * S_.y3_ / h_sq )
    - ( 2 * deriv_t1 / h_ms_ ) - ( deriv_t2 / h_ms_ );
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
    const double u =
      -sgnq * std::pow( std::fabs( q ) / 2.0 + std::sqrt( D ), 1.0 / 3.0 );
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

  double tau = ( tau1 >= 0 ) ? tau1 : 2 * h_ms_;
  if ( ( tau2 >= 0 ) && ( tau2 < tau ) )
  {
    tau = tau2;
  }
  if ( ( tau3 >= 0 ) && ( tau3 < tau ) )
  {
    tau = tau3;
  }
  return ( tau <= h_ms_ ) ? tau : thresh_find2_( dt );
}
