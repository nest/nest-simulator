/*
 *  aeif_cond_beta_multisynapse.cpp
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

#include "aeif_cond_beta_multisynapse.h"

#ifdef HAVE_GSL

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

nest::RecordablesMap< nest::aeif_cond_beta_multisynapse >
  nest::aeif_cond_beta_multisynapse::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< aeif_cond_beta_multisynapse >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m,
    &aeif_cond_beta_multisynapse::
      get_y_elem_< aeif_cond_beta_multisynapse::State_::V_M > );

  insert_( names::w,
    &aeif_cond_beta_multisynapse::
      get_y_elem_< aeif_cond_beta_multisynapse::State_::W > );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

aeif_cond_beta_multisynapse::Parameters_::Parameters_()
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
  , gsl_error_tol( 1e-6 )
  , num_of_receptors_( 1 )
  , has_connections_( false )
{
  taus_rise.clear();
  taus_decay.clear();
  taus_decay.push_back( 20.0 );
  taus_rise.push_back( 2.0 );
}

aeif_cond_beta_multisynapse::State_::State_( const Parameters_& p )
  : y_( STATE_VECTOR_MIN_SIZE, 0.0 )
  , r_( 0 )
{
  y_[ 0 ] = p.E_L;
}

aeif_cond_beta_multisynapse::State_::State_( const State_& s )
  : r_( s.r_ )
{
  y_ = s.y_;
}

aeif_cond_beta_multisynapse::State_& aeif_cond_beta_multisynapse::State_::
operator=( const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  y_ = s.y_;
  r_ = s.r_;
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
aeif_cond_beta_multisynapse::Parameters_::get( DictionaryDatum& d ) const
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
  def< double >( d, names::gsl_error_tol, gsl_error_tol );
  def< int >( d, "n_synapses", num_of_receptors_ );
  def< bool >( d, names::has_connections, has_connections_ );
}

void
aeif_cond_beta_multisynapse::Parameters_::set( const DictionaryDatum& d )
{

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
    if ( taus_rise.size() < taus_decay.size() )
    {
      for ( size_t i = 0; i < taus_decay.size(); ++i )
      {
        taus_rise.push_back( taus_decay[ i ] / 10. );
        // if taus_rise is not defined explicitly or if it has less elements
        // than taus_decay, it will be set to taus_decay/10
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
      if ( tau_tmp[ i ] > taus_decay[ i ] )
      {
        throw BadProperty(
          "Synaptic rise time must be smaller than or equal to decay time." );
      }
    }
    taus_rise = tau_tmp;
  }

  updateValue< double >( d, names::a, a );
  updateValue< double >( d, names::b, b );
  updateValue< double >( d, names::Delta_T, Delta_T );
  updateValue< double >( d, names::tau_w, tau_w );

  updateValue< double >( d, names::I_e, I_e );

  updateValue< double >( d, names::gsl_error_tol, gsl_error_tol );

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

  if ( gsl_error_tol <= 0. )
  {
    throw BadProperty( "The gsl_error_tol must be strictly positive." );
  }
}

void
aeif_cond_beta_multisynapse::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );

  std::vector< double >* dg = new std::vector< double >();
  std::vector< double >* g = new std::vector< double >();

  for ( size_t i = 0;
        i < ( ( y_.size() - State_::NUMBER_OF_FIXED_STATES_ELEMENTS )
              / State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR );
        ++i )
  {
    dg->push_back( y_[ State_::DG
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
    g->push_back( y_[ State_::G
      + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] );
  }

  ( *d )[ names::dg ] = DoubleVectorDatum( dg );
  ( *d )[ names::g ] = DoubleVectorDatum( g );

  def< double >( d, names::w, y_[ W ] );
}

void
aeif_cond_beta_multisynapse::State_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::V_m, y_[ V_M ] );

  if ( ( d->known( names::dg ) ) && ( d->known( names::g ) ) )
  {
    const std::vector< double > dg =
      getValue< std::vector< double > >( d->lookup( names::dg ) );
    const std::vector< double > g =
      getValue< std::vector< double > >( d->lookup( names::g ) );

    if ( ( dg.size() != g.size() ) )
    {
      throw BadProperty( "Conductances must have the same sizes." );
    }

    for ( size_t i = 0; i < dg.size(); ++i )
    {
      if ( ( dg[ i ] < 0 ) || ( g[ i ] < 0 ) )
      {
        throw BadProperty( "Conductances must not be negative." );
      }

      y_[ State_::DG
        + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] = dg[ i ];
      y_[ State_::G + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] =
        g[ i ];
    }
  }

  updateValue< double >( d, names::w, y_[ W ] );
}

aeif_cond_beta_multisynapse::Buffers_::Buffers_(
  aeif_cond_beta_multisynapse& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

aeif_cond_beta_multisynapse::Buffers_::Buffers_( const Buffers_&,
  aeif_cond_beta_multisynapse& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

aeif_cond_beta_multisynapse::aeif_cond_beta_multisynapse()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

aeif_cond_beta_multisynapse::aeif_cond_beta_multisynapse(
  const aeif_cond_beta_multisynapse& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

aeif_cond_beta_multisynapse::~aeif_cond_beta_multisynapse()
{
  // GSL structs may not have been allocated, so we need to protect destruction
  if ( B_.s_ )
  {
    gsl_odeiv_step_free( B_.s_ );
  }
  if ( B_.c_ )
  {
    gsl_odeiv_control_free( B_.c_ );
  }
  if ( B_.e_ )
  {
    gsl_odeiv_evolve_free( B_.e_ );
  }
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
aeif_cond_beta_multisynapse::init_state_( const Node& proto )
{
  const aeif_cond_beta_multisynapse& pr =
    downcast< aeif_cond_beta_multisynapse >( proto );
  S_ = pr.S_;
}

void
aeif_cond_beta_multisynapse::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  Archiving_Node::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();

  // We must integrate this model with high-precision to obtain decent results
  B_.IntegrationStep_ = std::min( 0.01, B_.step_ );

  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_yp_new( P_.gsl_error_tol, P_.gsl_error_tol );
  }
  else
  {
    gsl_odeiv_control_init(
      B_.c_, P_.gsl_error_tol, P_.gsl_error_tol, 0.0, 1.0 );
  }

  // Stepping function and evolution function are allocated in calibrate()

  B_.sys_.function = aeif_cond_beta_multisynapse_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.params = reinterpret_cast< void* >( this );
  // B_.sys_.dimension is assigned in calibrate()
  B_.I_stim_ = 0.0;
}

void
aeif_cond_beta_multisynapse::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  P_.receptor_types_.resize( P_.num_of_receptors_ );
  for ( size_t i = 0; i < P_.num_of_receptors_; i++ )
  {
    P_.receptor_types_[ i ] = i + 1;
  }

  V_.g0_.resize( P_.num_of_receptors_ );

  for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
  {
    // the denominator (denom1) that appears in the expression of the peak time
    // is computed here to check that it is != 0
    // another denominator denom2 appears in the expression of the
    // normalization factor g0
    // Both denom1 and denom2 are null if taus_decay = taus_rise, but they
    // can also be null if taus_decay and taus_rise are not equal but very
    // close to each other, due to the numerical precision limits.
    // In such case the beta function reduces to the alpha function,
    // and the normalization factor for the alpha function should be used.
    double denom1 = P_.taus_decay[ i ] - P_.taus_rise[ i ];
    double denom2 = 0;
    if ( denom1 != 0 )
    {
      // peak time
      const double t_p = P_.taus_decay[ i ] * P_.taus_rise[ i ]
        * std::log( P_.taus_decay[ i ] / P_.taus_rise[ i ] ) / denom1;
      // another denominator is computed here to check that it is != 0
      denom2 = std::exp( -t_p / P_.taus_decay[ i ] )
        - std::exp( -t_p / P_.taus_rise[ i ] );
    }
    if ( denom2 == 0 ) // if rise time == decay time use alpha function
    {                  // use normalization for alpha function in this case
      V_.g0_[ i ] = 1.0 * numerics::e / P_.taus_decay[ i ];
    }
    else // if rise time != decay time use beta function
    {
      V_.g0_[ i ] // normalization factor for conductance
        = ( 1. / P_.taus_rise[ i ] - 1. / P_.taus_decay[ i ] ) / denom2;
    }
  }
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  assert( V_.RefractoryCounts_
    >= 0 ); // since t_ref_ >= 0, this can only fail in error

  B_.spikes_.resize( P_.num_of_receptors_ );
  S_.y_.resize( State_::NUMBER_OF_FIXED_STATES_ELEMENTS
    + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR
                  * P_.num_of_receptors_ ) );

  // reallocate instance of stepping function for ODE GSL solver
  if ( B_.s_ != 0 )
  {
    gsl_odeiv_step_free( B_.s_ );
  }
  B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, S_.y_.size() );

  // reallocate instance of evolution function for ODE GSL solver
  if ( B_.e_ != 0 )
  {
    gsl_odeiv_evolve_free( B_.e_ );
  }
  B_.e_ = gsl_odeiv_evolve_alloc( S_.y_.size() );

  B_.sys_.dimension = S_.y_.size();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */
void
aeif_cond_beta_multisynapse::update( Time const& origin,
  const long from,
  const long to )
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

    B_.sys_.dimension = S_.y_.size();
    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // gsl_odeiv_evolve_apply performs only a single numerical
    // integration step, starting from t and bounded by step;
    // the while-loop ensures integration over the whole simulation
    // step (0, step] if more than one integration step is needed due
    // to a small integration step size;
    // note that (t+IntegrationStep > step) leads to integration over
    // (t, step] and afterwards setting t to step, but it does not
    // enforce setting IntegrationStep to step-t; this is of advantage
    // for a consistent and efficient integration across subsequent
    // simulation intervals

    while ( t < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,             // system of ODE
        &t,                   // from t
        B_.step_,             // to t <= step
        &B_.IntegrationStep_, // integration step size
        &S_.y_[ 0 ] );        // neuronal state converted to double[]

      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }

      // check for unreasonable values; we allow V_M to explode
      if ( S_.y_[ State_::V_M ] < -1e3 || S_.y_[ State_::W ] < -1e6
        || S_.y_[ State_::W ] > 1e6 )
      {
        throw NumericalInstability( get_name() );
      }
      // spikes are handled inside the while-loop
      // due to spike-driven adaptation
      if ( S_.r_ > 0 )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
      }
      else if ( S_.y_[ State_::V_M ] >= P_.V_peak_ )
      {
        S_.y_[ State_::V_M ] = P_.V_reset_;
        S_.y_[ State_::W ] += P_.b; // spike-driven adaptation
        S_.r_ = V_.RefractoryCounts_;

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }

    for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
    {
      S_.y_[ State_::DG
        + ( State_::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR * i ) ] +=
        B_.spikes_[ i ].get_value( lag ) * V_.g0_[ i ]; // add incoming spike
    }
    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

  } // for-loop
}

port
aeif_cond_beta_multisynapse::handles_test_event( SpikeEvent&,
  rport receptor_type )
{
  if ( receptor_type <= 0
    || receptor_type > static_cast< port >( P_.num_of_receptors_ ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }
  P_.has_connections_ = true;
  return receptor_type;
}

void
aeif_cond_beta_multisynapse::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( ( e.get_rport() > 0 )
    && ( ( size_t ) e.get_rport() <= P_.num_of_receptors_ ) );

  B_.spikes_[ e.get_rport() - 1 ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
aeif_cond_beta_multisynapse::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double I = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * I );
}

void
aeif_cond_beta_multisynapse::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

extern "C" int
aeif_cond_beta_multisynapse_dynamics( double,
  const double y[],
  double f[],
  void* pnode )
{
  // a shorthand
  typedef nest::aeif_cond_beta_multisynapse::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::aeif_cond_beta_multisynapse& node =
    *( reinterpret_cast< nest::aeif_cond_beta_multisynapse* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double& V = y[ S::V_M ];
  const double& w = y[ S::W ];

  double I_syn = 0.0;

  for ( size_t i = 0; i < ( node.P_.num_of_receptors_
                            * S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR );
        i += S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR )
  {
    double g = y[ S::G + i ];
    if ( g > 0 )
    {
      I_syn += g * ( node.P_.E_ex - V ); // g>0, E_ex>V => I_syn increases
    }
    else
    {
      I_syn += g * ( V - node.P_.E_in ); // g<0, V>E_in => I_syn decreases
    }
  }

  // We pre-compute the argument of the exponential
  const double exp_arg = ( V - node.P_.V_th ) / node.P_.Delta_T;

  // Upper bound for exponential argument to avoid numerical instabilities
  const double MAX_EXP_ARG = 10.;

  // If the argument is too large, we clip it.
  const double I_spike =
    node.P_.Delta_T * std::exp( std::min( exp_arg, MAX_EXP_ARG ) );

  // dv/dt
  f[ S::V_M ] = ( -node.P_.g_L * ( ( V - node.P_.E_L ) - I_spike ) + I_syn - w
                  + node.P_.I_e + node.B_.I_stim_ ) / node.P_.C_m;

  // Adaptation current w.
  f[ S::W ] = ( node.P_.a * ( V - node.P_.E_L ) - w ) / node.P_.tau_w;

  for ( size_t i = 0; i < node.P_.num_of_receptors_; ++i )
  {
    size_t j = i * S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR;
    // Synaptic conductance derivative dG/dt
    f[ S::DG + j ] = -y[ S::DG + j ] / node.P_.taus_rise[ i ];
    f[ S::G + j ] = y[ S::DG + j ] - y[ S::G + j ] / node.P_.taus_decay[ i ];
  }

  return GSL_SUCCESS;
}

} // namespace nest

#endif // HAVE_GSL
