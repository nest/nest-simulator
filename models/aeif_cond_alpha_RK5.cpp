/*
 *  aeif_cond_alpha_RK5.cpp
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

#include "aeif_cond_alpha_RK5.h"

// C++ includes:
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
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

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::aeif_cond_alpha_RK5 >
  nest::aeif_cond_alpha_RK5::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< aeif_cond_alpha_RK5 >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m,
    &aeif_cond_alpha_RK5::get_y_elem_< aeif_cond_alpha_RK5::State_::V_M > );
  insert_( names::g_ex,
    &aeif_cond_alpha_RK5::get_y_elem_< aeif_cond_alpha_RK5::State_::G_EXC > );
  insert_( names::g_in,
    &aeif_cond_alpha_RK5::get_y_elem_< aeif_cond_alpha_RK5::State_::G_INH > );
  insert_( names::w,
    &aeif_cond_alpha_RK5::get_y_elem_< aeif_cond_alpha_RK5::State_::W > );
}
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::aeif_cond_alpha_RK5::Parameters_::Parameters_()
  : V_peak_( 0.0 )    // mV, should not be larger that V_th+10
  , V_reset_( -60.0 ) // mV
  , t_ref_( 0.0 )     // ms
  , g_L( 30.0 )       // nS
  , C_m( 281.0 )      // pF
  , E_ex( 0.0 )       // mV
  , E_in( -85.0 )     // mV
  , E_L( -70.6 )      // mV
  , Delta_T( 2.0 )    // mV
  , tau_w( 144.0 )    // ms
  , a( 4.0 )          // nS
  , b( 80.5 )         // pA
  , V_th( -50.4 )     // mV
  , tau_syn_ex( 0.2 ) // ms
  , tau_syn_in( 2.0 ) // ms
  , I_e( 0.0 )        // pA
  , MAXERR( 1.0e-10 ) // mV
  , HMIN( 1.0e-3 )    // ms
{
}

nest::aeif_cond_alpha_RK5::State_::State_( const Parameters_& p )
  : r_( 0 )
{
  y_[ 0 ] = p.E_L;
  for ( size_t i = 1; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0.0;
  }
}

nest::aeif_cond_alpha_RK5::State_::State_( const State_& s )
  : r_( s.r_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::aeif_cond_alpha_RK5::State_& nest::aeif_cond_alpha_RK5::State_::operator=(
  const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
  r_ = s.r_;
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::aeif_cond_alpha_RK5::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::tau_syn_ex, tau_syn_ex );
  def< double >( d, names::tau_syn_in, tau_syn_in );
  def< double >( d, names::a, a );
  def< double >( d, names::b, b );
  def< double >( d, names::Delta_T, Delta_T );
  def< double >( d, names::tau_w, tau_w );
  def< double >( d, names::I_e, I_e );
  def< double >( d, names::V_peak, V_peak_ );
  def< double >( d, names::MAXERR, MAXERR );
  def< double >( d, names::HMIN, HMIN );
}

void
nest::aeif_cond_alpha_RK5::Parameters_::set( const DictionaryDatum& d )
{
  double tmp = 0.0;

  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_peak, V_peak_ );
  updateValue< double >( d, names::t_ref, t_ref_ );
  updateValue< double >( d, names::E_L, E_L );
  updateValue< double >( d, names::V_reset, V_reset_ );
  updateValue< double >( d, names::E_ex, E_ex );
  updateValue< double >( d, names::E_in, E_in );

  updateValue< double >( d, names::C_m, C_m );
  updateValue< double >( d, names::g_L, g_L );

  updateValue< double >( d, names::tau_syn_ex, tau_syn_ex );
  updateValue< double >( d, names::tau_syn_in, tau_syn_in );

  updateValue< double >( d, names::a, a );
  updateValue< double >( d, names::b, b );
  updateValue< double >( d, names::Delta_T, Delta_T );
  updateValue< double >( d, names::tau_w, tau_w );

  updateValue< double >( d, names::I_e, I_e );

  if ( updateValue< double >( d, names::MAXERR, tmp ) )
  {
    if ( not( tmp > 0.0 ) )
    {
      throw BadProperty( "MAXERR must be positive." );
    }
    MAXERR = tmp;
  }

  if ( updateValue< double >( d, names::HMIN, tmp ) )
  {
    if ( not( tmp > 0.0 ) )
    {
      throw BadProperty( "HMIN must be positive." );
    }
    HMIN = tmp;
  }

  if ( V_peak_ <= V_th )
  {
    throw BadProperty( "V_peak must be larger than threshold." );
  }

  if ( Delta_T < 0. )
  {
    throw BadProperty( "Delta_T must be positive." );
  }
  else if ( Delta_T > 0. )
  {
    // check for possible numerical overflow with the exponential divergence at
    // spike time, keep a 1e20 margin for the subsequent calculations
    const double max_exp_arg =
      std::log( std::numeric_limits< double >::max() / 1e20 );
    if ( ( V_peak_ - V_th ) / Delta_T >= max_exp_arg )
    {
      throw BadProperty(
        "The current combination of V_peak, V_th and Delta_T"
        "will lead to numerical overflow at spike time; try"
        "for instance to increase Delta_T or to reduce V_peak"
        "to avoid this problem." );
    }
  }

  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }

  if ( tau_syn_ex <= 0 || tau_syn_in <= 0 || tau_w <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

void
nest::aeif_cond_alpha_RK5::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );
  def< double >( d, names::g_ex, y_[ G_EXC ] );
  def< double >( d, names::dg_ex, y_[ DG_EXC ] );
  def< double >( d, names::g_in, y_[ G_INH ] );
  def< double >( d, names::dg_in, y_[ DG_INH ] );
  def< double >( d, names::w, y_[ W ] );
}

void
nest::aeif_cond_alpha_RK5::State_::set( const DictionaryDatum& d,
  const Parameters_& )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );
  updateValue< double >( d, names::g_ex, y_[ G_EXC ] );
  updateValue< double >( d, names::dg_ex, y_[ DG_EXC ] );
  updateValue< double >( d, names::g_in, y_[ G_INH ] );
  updateValue< double >( d, names::dg_in, y_[ DG_INH ] );
  updateValue< double >( d, names::w, y_[ W ] );
  if ( y_[ G_EXC ] < 0 || y_[ G_INH ] < 0 )
  {
    throw BadProperty( "Conductances must not be negative." );
  }
}

nest::aeif_cond_alpha_RK5::Buffers_::Buffers_( aeif_cond_alpha_RK5& n )
  : logger_( n )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::aeif_cond_alpha_RK5::Buffers_::Buffers_( const Buffers_&,
  aeif_cond_alpha_RK5& n )
  : logger_( n )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::aeif_cond_alpha_RK5::aeif_cond_alpha_RK5()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::aeif_cond_alpha_RK5::aeif_cond_alpha_RK5( const aeif_cond_alpha_RK5& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::aeif_cond_alpha_RK5::~aeif_cond_alpha_RK5()
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::aeif_cond_alpha_RK5::init_state_( const Node& proto )
{
  const aeif_cond_alpha_RK5& pr = downcast< aeif_cond_alpha_RK5 >( proto );
  S_ = pr.S_;
}

void
nest::aeif_cond_alpha_RK5::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.spike_inh_.clear(); // includes resize
  B_.currents_.clear();  // includes resize
  Archiving_Node::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();

  // We must integrate this model with high-precision to obtain decent results
  B_.IntegrationStep_ = std::min( 0.01, B_.step_ );

  B_.I_stim_ = 0.0;
}

void
nest::aeif_cond_alpha_RK5::calibrate()
{
  B_.logger_
    .init(); // ensures initialization in case mm connected after Simulate

  V_.g0_ex_ = 1.0 * numerics::e / P_.tau_syn_ex;
  V_.g0_in_ = 1.0 * numerics::e / P_.tau_syn_in;

  // set model dynamics depending on the value of Delta_T
  if ( P_.Delta_T > 0. )
  {
    V_.V_peak = P_.V_peak_;
    V_.model_dynamics = &aeif_cond_alpha_RK5::aeif_cond_alpha_RK5_dynamics;
  }
  else
  {
    V_.V_peak = P_.V_th; // same as IAF dynamics for spikes if Delta_T == 0.
    V_.model_dynamics = &aeif_cond_alpha_RK5::aeif_cond_alpha_RK5_dynamics_DT0;
  }

  V_.refractory_counts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  assert( V_.refractory_counts_
    >= 0 ); // since t_ref_ >= 0, this can only fail in error
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

/**
 * Member function updating the neuron state by integrating the ODE.
 * @param origin
 * @param from
 * @param to
 */
void nest::aeif_cond_alpha_RK5::update( Time const& origin,
  const long from,
  const long to ) // proceed in time
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );
  assert( State_::V_M == 0 );

  for ( long lag = from; lag < to; ++lag ) // proceed by stepsize B_.step_
  {
    double t = 0.0; // internal time of the integration period

    if ( S_.r_ > 0 ) // decrease remaining refractory steps if non-zero
    {
      --S_.r_;
    }

    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // The numerical integration of the model equations is performed by
    // a Dormand-Prince method (5th order Runge-Kutta method with
    // adaptive stepsize control) as desribed in William H. Press et
    // al., "Adaptive Stepsize Control for Runge-Kutta", Chapter 17.2
    // in Numerical Recipes (3rd edition, 2007), 910-914.  The solver
    // itself performs only a single NUMERICAL integration step,
    // starting from t and of size B_.IntegrationStep_ (bounded by
    // step); the while-loop ensures integration over the whole
    // SIMULATION step (0, step] of size B_.step_ if more than one
    // integration step is needed due to a small integration stepsize;
    // note that (t+IntegrationStep > step) leads to integration over
    // (t, step] and afterwards setting t to step, but it does not
    // enforce setting IntegrationStep to step-t; this is of advantage
    // for a consistent and efficient integration across subsequent
    // simulation intervals.

    double& h = B_.IntegrationStep_; // numerical integration step
    double& tend = B_.step_;         // end of simulation step

    const double& MAXERR = P_.MAXERR; // maximum error
    const double& HMIN = P_.HMIN;     // minimal integration step

    double err;
    double t_return = 0.0;

    while ( t < B_.step_ ) // while not yet reached end of simulation step
    {
      bool done = false;

      do
      {

        if ( tend - t < h ) // stop integration at end of simulation step
        {
          h = tend - t;
        }

        t_return = t + h; // update t

        // k1 = f(told, y)
        ( this->*( V_.model_dynamics ) )( S_.y_, S_.k1 );

        // k2 = f(told + h/5, y + h*k1 / 5)
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.yin[ i ] = S_.y_[ i ] + h * S_.k1[ i ] / 5.0;
        }
        ( this->*( V_.model_dynamics ) )( S_.yin, S_.k2 );

        // k3 = f(told + 3/10*h, y + 3/40*h*k1 + 9/40*h*k2)
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.yin[ i ] = S_.y_[ i ]
            + h * ( 3.0 / 40.0 * S_.k1[ i ] + 9.0 / 40.0 * S_.k2[ i ] );
        }
        ( this->*( V_.model_dynamics ) )( S_.yin, S_.k3 );

        // k4
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.yin[ i ] = S_.y_[ i ]
            + h * ( 44.0 / 45.0 * S_.k1[ i ] - 56.0 / 15.0 * S_.k2[ i ]
                    + 32.0 / 9.0 * S_.k3[ i ] );
        }
        ( this->*( V_.model_dynamics ) )( S_.yin, S_.k4 );

        // k5
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.yin[ i ] = S_.y_[ i ]
            + h
              * ( 19372.0 / 6561.0 * S_.k1[ i ] - 25360.0 / 2187.0 * S_.k2[ i ]
                  + 64448.0 / 6561.0 * S_.k3[ i ]
                  - 212.0 / 729.0 * S_.k4[ i ] );
        }
        ( this->*( V_.model_dynamics ) )( S_.yin, S_.k5 );

        // k6
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.yin[ i ] = S_.y_[ i ]
            + h * ( 9017.0 / 3168.0 * S_.k1[ i ] - 355.0 / 33.0 * S_.k2[ i ]
                    + 46732.0 / 5247.0 * S_.k3[ i ]
                    + 49.0 / 176.0 * S_.k4[ i ]
                    - 5103.0 / 18656.0 * S_.k5[ i ] );
        }
        ( this->*( V_.model_dynamics ) )( S_.yin, S_.k6 );

        // 5th order
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.ynew[ i ] = S_.y_[ i ]
            + h * ( 35.0 / 384.0 * S_.k1[ i ] + 500.0 / 1113.0 * S_.k3[ i ]
                    + 125.0 / 192.0 * S_.k4[ i ]
                    - 2187.0 / 6784.0 * S_.k5[ i ]
                    + 11.0 / 84.0 * S_.k6[ i ] );
        }
        ( this->*( V_.model_dynamics ) )( S_.ynew, S_.k7 );

        // 4th order
        for ( int i = 0; i < S_.STATE_VEC_SIZE; ++i )
        {
          S_.yref[ i ] = S_.y_[ i ]
            + h
              * ( 5179.0 / 57600.0 * S_.k1[ i ] + 7571.0 / 16695.0 * S_.k3[ i ]
                  + 393.0 / 640.0 * S_.k4[ i ]
                  - 92097.0 / 339200.0 * S_.k5[ i ]
                  + 187.0 / 2100.0 * S_.k6[ i ]
                  + 1.0 / 40.0 * S_.k7[ i ] );
        }

        err = std::fabs( S_.ynew[ 0 ] - S_.yref[ 0 ] ) / MAXERR
          + 1.0e-200; // error estimate,
        // based on different orders for stepsize prediction. Small value added
        // to prevent err==0

        // The following flag 'done' is needed to ensure that we accept the
        // result for h<=HMIN, irrespective of the error. (See below)

        done = ( h <= HMIN ); // Always exit loop if h was <=HMIN already

        // prediction of next integration stepsize. This step may result in a
        // stepsize below HMIN.
        // If this happens, we must
        //   1. set the stepsize to HMIN
        //   2. compute the result and accept it irrespective of the error,
        //      because we cannot decrease the stepsize any further.
        //  the 'done' flag, computed above ensure that the loop is terminated
        //  after the  result was computed.

        h *= 0.98 * std::pow( 1.0 / err, 1.0 / 5.0 );
        h = std::max( h, HMIN );

      } while ( ( err > 1.0 ) and ( not done ) ); // reject step if err > 1

      for ( unsigned int i = 0; i < S_.STATE_VEC_SIZE; ++i )
      {
        S_.y_[ i ] = S_.ynew[ i ]; // pass updated values
      }

      t = t_return;

      // check for unreasonable values; we allow V_M to explode
      if ( S_.y_[ State_::V_M ] < -1e3 || S_.y_[ State_::W ] < -1e6
        || S_.y_[ State_::W ] > 1e6 )
      {
        throw NumericalInstability( get_name() );
      }

      // spikes are handled inside the while-loop
      // due to spike-driven adaptation
      if ( S_.r_ > 0 ) // if neuron is still in refractory period
      {
        S_.y_[ State_::V_M ] = P_.V_reset_; // clamp it to V_reset
      }
      else if ( S_.y_[ State_::V_M ] >= V_.V_peak ) // V_m >= V_peak: spike
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
        S_.y_[ State_::W ] += P_.b;    // spike-driven adaptation
        S_.r_ = V_.refractory_counts_; // initialize refractory steps with
                                       // refractory period

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    } // while


    S_.y_[ State_::DG_EXC ] +=
      B_.spike_exc_.get_value( lag ) * V_.g0_ex_; // add incoming spikes
    S_.y_[ State_::DG_INH ] += B_.spike_inh_.get_value( lag ) * V_.g0_in_;

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

  } // for-loop
} // function update()


void
nest::aeif_cond_alpha_RK5::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spike_inh_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() );
  } // keep conductances positive
}

void
nest::aeif_cond_alpha_RK5::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * c );
}

void
nest::aeif_cond_alpha_RK5::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
