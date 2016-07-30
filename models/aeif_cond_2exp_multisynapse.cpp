/*
 *  aeif_cond_2exp_multisynapse.cpp
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

#include "aeif_cond_2exp_multisynapse.h"

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

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::aeif_cond_2exp_multisynapse >
  nest::aeif_cond_2exp_multisynapse::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< aeif_cond_2exp_multisynapse >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m,
    &aeif_cond_2exp_multisynapse::
      get_y_elem_< aeif_cond_2exp_multisynapse::State_::V_M > );

  insert_( names::w,
    &aeif_cond_2exp_multisynapse::
      get_y_elem_< aeif_cond_2exp_multisynapse::State_::W > );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

aeif_cond_2exp_multisynapse::Parameters_::Parameters_()
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
  , I_e( 0.0 )        // pA
  , MAXERR( 1.0e-10 ) // mV
  , HMIN( 1.0e-3 )    // ms
  , num_of_receptors_( 0 )
  , has_connections_( false )
{
  taus_rise.clear();
  taus_decay.clear();
}

aeif_cond_2exp_multisynapse::State_::State_( const Parameters_& p )
  : y_( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k1( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k2( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k3( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k4( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k5( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k6( STATE_VECTOR_MIN_SIZE, 0.0 )
  , k7( STATE_VECTOR_MIN_SIZE, 0.0 )
  , yin( STATE_VECTOR_MIN_SIZE, 0.0 )
  , ynew( STATE_VECTOR_MIN_SIZE, 0.0 )
  , yref( STATE_VECTOR_MIN_SIZE, 0.0 )
  , r_( 0 )
{
  y_[ 0 ] = p.E_L;
}

aeif_cond_2exp_multisynapse::State_::State_( const State_& s )
  : r_( s.r_ )
{
  y_ = s.y_;
  k1 = s.k1;
  k2 = s.k2;
  k3 = s.k3;
  k4 = s.k4;
  k5 = s.k5;
  k6 = s.k6;
  k7 = s.k7;
  yin = s.yin;
  ynew = s.ynew;
  yref = s.yref;
}

aeif_cond_2exp_multisynapse::State_& aeif_cond_2exp_multisynapse::State_::
operator=( const State_& s )
{
  assert( this != &s ); // would be bad logical error in program
  y_ = s.y_;
  k1 = s.k1;
  k2 = s.k2;
  k3 = s.k3;
  k4 = s.k4;
  k5 = s.k5;
  k6 = s.k6;
  k7 = s.k7;
  yin = s.yin;
  ynew = s.ynew;
  yref = s.yref;
  r_ = s.r_;
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
aeif_cond_2exp_multisynapse::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  ArrayDatum taus_rise_ad( taus_rise );
  ArrayDatum taus_decay_ad( taus_decay );
  def< ArrayDatum >( d, names::taus_rise, taus_rise_ad );
  def< ArrayDatum >( d, names::taus_decay, taus_decay_ad );
  def< double >( d, names::a, a );
  def< double >( d, names::b, b );
  def< double >( d, names::Delta_T, Delta_T );
  def< double >( d, names::tau_w, tau_w );
  def< double >( d, names::I_e, I_e );
  def< double >( d, names::V_peak, V_peak_ );
  def< double >( d, names::MAXERR, MAXERR );
  def< double >( d, names::HMIN, HMIN );
  def< int >( d, "n_synapses", num_of_receptors_ );
  def< bool >( d, names::has_connections, has_connections_ );
}

void
aeif_cond_2exp_multisynapse::Parameters_::set( const DictionaryDatum& d )
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

  std::vector< double > tau_tmp;
  if ( updateValue< std::vector< double > >( d, names::taus_decay, tau_tmp ) )
  {
    if ( tau_tmp.size() < taus_decay.size() && has_connections_ == true )
    {
      throw BadProperty(
        "The neuron has connections, therefore the number of ports cannot be "
        "reduced." );
    }
    for ( size_t i = 0; i < tau_tmp.size(); ++i )
    {
      if ( tau_tmp[ i ] <= 0 )
      {
        throw BadProperty(
          "All synaptic time constants must be strictly positive" );
      }
    }
    taus_decay = tau_tmp;
    num_of_receptors_ = taus_decay.size();
    tau_tmp.clear();
    if ( taus_rise.size() == 0 )
    {
      for ( size_t i = 0; i < taus_decay.size(); ++i )
      {
	taus_rise.push_back(taus_decay[i]/100.); // if taus_rise is not defined
	// explicitly, it will be set to taus_decay/100
      }
    }
  }

  if ( updateValue< std::vector< double > >( d, names::taus_rise, tau_tmp ) )
  {
    if ( taus_decay.size() == 0 )
    {
      throw BadProperty(
	"Synaptic decay times must be defined before rise times." );
    }
    if ( tau_tmp.size() != taus_decay.size() )
    {
      throw BadProperty(
	"The number of ports for synaptic rise times must be the same "
	"as that of decay times." );
    }

    for ( size_t i = 0; i < tau_tmp.size(); ++i )
    {
      if ( tau_tmp[ i ] <= 0 )
      {
        throw BadProperty(
          "All synaptic time constants must be strictly positive" );
      }
      if ( tau_tmp[ i ] >= taus_decay[ i ] )
      {
        throw BadProperty(
          "Synaptic rise time must be smaller than decay time." );
      }
    }
    taus_rise = tau_tmp;
  }

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

  if ( V_reset_ >= V_peak_ )
  {
    throw BadProperty( "Ensure that: V_reset < V_peak ." );
  }

  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }

  if ( tau_w <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

void
aeif_cond_2exp_multisynapse::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );

  std::vector< double_t >* g_exc_rise = new std::vector< double_t >();
  std::vector< double_t >* g_exc_decay = new std::vector< double_t >();
  std::vector< double_t >* g_inh_rise = new std::vector< double_t >();
  std::vector< double_t >* g_inh_decay = new std::vector< double_t >();

  for ( size_t i = 0;
        i < ( ( y_.size() - State_::NUMBER_OF_FIXED_STATES_ELEMENTS )
              / State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR );
        ++i )
  {
    g_exc_rise->push_back( y_[ State_::G_EXC_RISE
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
    g_exc_decay->push_back( y_[ State_::G_EXC_DECAY
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
    g_inh_rise->push_back( y_[ State_::G_INH_RISE
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
    g_inh_decay->push_back( y_[ State_::G_INH_DECAY
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
  }

  ( *d )[ names::g_ex_rise ] = DoubleVectorDatum( g_exc_rise );
  ( *d )[ names::g_ex_decay ] = DoubleVectorDatum( g_exc_decay );
  ( *d )[ names::g_in_rise ] = DoubleVectorDatum( g_inh_rise );
  ( *d )[ names::g_in_decay ] = DoubleVectorDatum( g_inh_decay );

  def< double >( d, names::w, y_[ W ] );
}

void
aeif_cond_2exp_multisynapse::State_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );

  if ( ( d->known( names::g_ex_rise ) ) && ( d->known( names::g_ex_decay ) )
    && ( d->known( names::g_in_rise ) ) && ( d->known( names::g_in_decay ) ) )
  {
    const std::vector< double_t > g_exc_rise =
      getValue< std::vector< double_t > >( d->lookup( names::g_ex_rise ) );
    const std::vector< double_t > g_exc_decay =
      getValue< std::vector< double_t > >( d->lookup( names::g_ex_decay ) );
    const std::vector< double_t > g_inh_rise =
      getValue< std::vector< double_t > >( d->lookup( names::g_in_rise ) );
    const std::vector< double_t > g_inh_decay =
      getValue< std::vector< double_t > >( d->lookup( names::g_in_decay ) );

    if ( ( g_exc_rise.size() != g_exc_decay.size() ) || ( g_exc_rise.size()
       != g_inh_rise.size() ) || ( g_exc_rise.size() != g_inh_decay.size() ) )
    {
      throw BadProperty( "Conductances must have the same sizes." );
    }

    for ( size_t i = 0; i < g_exc_rise.size(); ++i )
    {
      if ( ( g_exc_rise[ i ] < 0 ) || ( g_exc_decay[ i ] < 0 ) ||
	   ( g_inh_rise[ i ] < 0 ) || ( g_inh_decay[ i ] < 0 ) )
      {
        throw BadProperty( "Conductances must not be negative." );
      }

      y_[ State_::G_EXC_RISE
        + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	= g_exc_rise[ i ];
      y_[ State_::G_EXC_DECAY
	+ ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	= g_exc_decay[ i ];
      y_[ State_::G_INH_RISE
        + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	= g_inh_rise[ i ];
      y_[ State_::G_INH_DECAY
	+ ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	= g_inh_decay[ i ];
    }
  }

  updateValue< double >( d, names::w, y_[ W ] );
}

aeif_cond_2exp_multisynapse::Buffers_::Buffers_(
  aeif_cond_2exp_multisynapse& n )
  : logger_( n )
{
}

aeif_cond_2exp_multisynapse::Buffers_::Buffers_( const Buffers_&,
  aeif_cond_2exp_multisynapse& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

aeif_cond_2exp_multisynapse::aeif_cond_2exp_multisynapse()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

aeif_cond_2exp_multisynapse::aeif_cond_2exp_multisynapse(
  const aeif_cond_2exp_multisynapse& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

aeif_cond_2exp_multisynapse::~aeif_cond_2exp_multisynapse()
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
aeif_cond_2exp_multisynapse::init_state_( const Node& proto )
{
  const aeif_cond_2exp_multisynapse& pr =
    downcast< aeif_cond_2exp_multisynapse >( proto );
  S_ = pr.S_;
}

void
aeif_cond_2exp_multisynapse::init_buffers_()
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
aeif_cond_2exp_multisynapse::calibrate()
{
  B_.logger_
    .init(); // ensures initialization in case mm connected after Simulate

  P_.receptor_types_.resize( P_.num_of_receptors_ );
  for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
  {
    P_.receptor_types_[ i ] = i + 1;
  }

  V_.g0_ex_.resize( P_.num_of_receptors_ );
  V_.g0_in_.resize( P_.num_of_receptors_ );

  for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
  {
    double t_p = P_.taus_decay[i]*P_.taus_rise[i]
      / (P_.taus_decay[i] - P_.taus_rise[i])
      * std::log(P_.taus_decay[i]/P_.taus_rise[i]); // peak time
    V_.g0_ex_[ i ] = V_.g0_in_[ i ] // normalization factors for conductance
      = 1./(std::exp(-t_p/P_.taus_decay[i])-std::exp(-t_p/P_.taus_rise[i]));
  }
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  assert( V_.RefractoryCounts_
    >= 0 ); // since t_ref_ >= 0, this can only fail in error

  B_.spike_exc_.resize( P_.num_of_receptors_ );
  B_.spike_inh_.resize( P_.num_of_receptors_ );
  S_.y_.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k1.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k2.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k3.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k4.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k5.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k6.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.k7.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );
  S_.yin.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                   * P_.num_of_receptors_ ) );
  S_.ynew.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                    * P_.num_of_receptors_ ) );
  S_.yref.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                    * P_.num_of_receptors_ ) );
}

void
aeif_cond_2exp_multisynapse::update( Time const& origin,
  const long_t from,
  const long_t to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );
  assert( State_::V_M == 0 );

  for ( long_t lag = from; lag < to; ++lag ) // proceed by stepsize B_.step_
  {
    double t = 0.0; // internal time of the integration period

    if ( S_.r_ > 0 ) // decrease remaining refractory steps if non-zero
      --S_.r_;

    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // The numerical integration of the model equations is performed by
    // a Dormand-Prince method (5th order Runge-Kutta method with
    // adaptive stepsize control) as desribed in William H. Press et
    // al., “Adaptive Stepsize Control for Runge-Kutta”, Chapter 17.2
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

    double_t& h = B_.IntegrationStep_; // numerical integration step
    double_t& tend = B_.step_;         // end of simulation step

    const double_t& MAXERR = P_.MAXERR; // maximum error
    const double_t& HMIN = P_.HMIN;     // minimal integration step

    double_t err;
    double_t t_return = 0.0;

    while ( t < B_.step_ ) // while not yet reached end of simulation step
    {
      bool done = false;

      do
      {

        if ( tend - t < h ) // stop integration at end of simulation step
          h = tend - t;

        t_return = t + h; // update t

        // k1 = f(told, y)
        aeif_cond_2exp_multisynapse_dynamics( S_.y_, S_.k1 );

        // k2 = f(told + h/5, y + h*k1 / 5)
        for ( size_t i = 0; i < S_.y_.size(); ++i )
          S_.yin[ i ] = S_.y_[ i ] + h * S_.k1[ i ] / 5.0;
        aeif_cond_2exp_multisynapse_dynamics( S_.yin, S_.k2 );

        // k3 = f(told + 3/10*h, y + 3/40*h*k1 + 9/40*h*k2)
        for ( size_t i = 0; i < S_.y_.size(); ++i )
          S_.yin[ i ] = S_.y_[ i ]
            + h * ( 3.0 / 40.0 * S_.k1[ i ] + 9.0 / 40.0 * S_.k2[ i ] );
        aeif_cond_2exp_multisynapse_dynamics( S_.yin, S_.k3 );

        // k4
        for ( size_t i = 0; i < S_.y_.size(); ++i )
          S_.yin[ i ] = S_.y_[ i ]
            + h * ( 44.0 / 45.0 * S_.k1[ i ] - 56.0 / 15.0 * S_.k2[ i ]
                    + 32.0 / 9.0 * S_.k3[ i ] );
        aeif_cond_2exp_multisynapse_dynamics( S_.yin, S_.k4 );

        // k5
        for ( size_t i = 0; i < S_.y_.size(); ++i )
          S_.yin[ i ] = S_.y_[ i ]
            + h
              * ( 19372.0 / 6561.0 * S_.k1[ i ] - 25360.0 / 2187.0 * S_.k2[ i ]
                  + 64448.0 / 6561.0 * S_.k3[ i ]
                  - 212.0 / 729.0 * S_.k4[ i ] );
        aeif_cond_2exp_multisynapse_dynamics( S_.yin, S_.k5 );

        // k6
        for ( size_t i = 0; i < S_.y_.size(); ++i )
          S_.yin[ i ] = S_.y_[ i ]
            + h * ( 9017.0 / 3168.0 * S_.k1[ i ] - 355.0 / 33.0 * S_.k2[ i ]
                    + 46732.0 / 5247.0 * S_.k3[ i ]
                    + 49.0 / 176.0 * S_.k4[ i ]
                    - 5103.0 / 18656.0 * S_.k5[ i ] );
        aeif_cond_2exp_multisynapse_dynamics( S_.yin, S_.k6 );

        // 5th order
        for ( size_t i = 0; i < S_.y_.size(); ++i )
          S_.ynew[ i ] = S_.y_[ i ]
            + h * ( 35.0 / 384.0 * S_.k1[ i ] + 500.0 / 1113.0 * S_.k3[ i ]
                    + 125.0 / 192.0 * S_.k4[ i ]
                    - 2187.0 / 6784.0 * S_.k5[ i ]
                    + 11.0 / 84.0 * S_.k6[ i ] );
        aeif_cond_2exp_multisynapse_dynamics( S_.yin, S_.k7 );

        // 4th order
        for ( size_t i = 0; i < S_.y_.size(); ++i )
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
        //  after the result was computed.

        h *= 0.98 * std::pow( 1.0 / err, 1.0 / 5.0 );
        h = std::max( h, HMIN );

      } while ( ( err > 1.0 ) and ( not done ) ); // reject step if err > 1

      for ( size_t i = 0; i < S_.y_.size(); ++i )
        S_.y_[ i ] = S_.ynew[ i ]; // pass updated values

      t = t_return;

      // check for unreasonable values; we allow V_M to explode
      if ( S_.y_[ State_::V_M ] < -1e3 || S_.y_[ State_::W ] < -1e6
        || S_.y_[ State_::W ] > 1e6 )
        throw NumericalInstability( get_name() );

      // spikes are handled inside the while-loop
      // due to spike-driven adaptation
      if ( S_.r_ > 0 ) // if neuron is still in refractory period
        S_.y_[ State_::V_M ] = P_.V_reset_;          // clamp it to V_reset
      else if ( S_.y_[ State_::V_M ] >= P_.V_peak_ ) // V_m >= V_peak: spike
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
        S_.y_[ State_::W ] += P_.b;   // spike-driven adaptation
        S_.r_ = V_.RefractoryCounts_; // initialize refractory steps with
                                      // refractory period

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    } // while

    for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
    {
      // add incoming spikes
      double spike_exc =B_.spike_exc_[ i ].get_value( lag ) * V_.g0_ex_[ i ];
      S_.y_[ State_::G_EXC_RISE + 
	( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	+= spike_exc;
      S_.y_[ State_::G_EXC_DECAY + 
	( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	+= spike_exc;
      double spike_inh=B_.spike_inh_[ i ].get_value( lag ) * V_.g0_in_[ i ];
      S_.y_[ State_::G_INH_RISE +
	( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	+= spike_inh;
      S_.y_[ State_::G_INH_DECAY +
	( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ]
	+= spike_inh;
    }
    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

  } // for-loop
}

port
aeif_cond_2exp_multisynapse::handles_test_event( SpikeEvent&,
  rport receptor_type )
{
  if ( receptor_type <= 0
    || receptor_type > static_cast< port >( P_.num_of_receptors_ ) )
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );

  P_.has_connections_ = true;
  return receptor_type;
}

void
aeif_cond_2exp_multisynapse::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( ( e.get_rport() > 0 )
    && ( ( size_t ) e.get_rport() <= P_.num_of_receptors_ ) );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_[ e.get_rport() - 1 ].add_value(
      e.get_rel_delivery_steps(
        kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spike_inh_[ e.get_rport() - 1 ].add_value(
      e.get_rel_delivery_steps(
        kernel().simulation_manager.get_slice_origin() ),
      -e.get_weight() * e.get_multiplicity() ); // keep conductances positive
  }
}

void
aeif_cond_2exp_multisynapse::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t I = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * I );
}

void
aeif_cond_2exp_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
