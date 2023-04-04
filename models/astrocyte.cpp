/*
 *  astrocyte.cpp
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


#include "astrocyte.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iostream>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dictutils.h"

nest::RecordablesMap< nest::astrocyte > nest::astrocyte::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< astrocyte >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::IP3, &astrocyte::get_y_elem_< astrocyte::State_::IP3 > );
  insert_( names::Ca, &astrocyte::get_y_elem_< astrocyte::State_::Ca > );
  insert_( names::h_IP3R, &astrocyte::get_y_elem_< astrocyte::State_::h_IP3R > );
  insert_( names::SIC, &astrocyte::get_sic_ ); // for testing, to be deleted
}

extern "C" int
astrocyte_dynamics( double time, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::astrocyte::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::astrocyte& node = *( reinterpret_cast< nest::astrocyte* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double& ip3 = y[ S::IP3 ];
  const double& calc = y[ S::Ca ];
  const double& f_ip3r = y[ S::h_IP3R ];

  const double alpha_f_ip3r = node.P_.k_IP3R_ * node.P_.Kd_inh_ * ( ip3 + node.P_.Kd_IP3_1_ ) / ( ip3 +
      node.P_.Kd_IP3_2_ );
  const double beta_f_ip3r = node.P_.k_IP3R_ * calc;
  const double I_pump = node.P_.rate_SERCA_ * std::pow(calc, 2) / (std::pow(node.P_.Km_SERCA_, 2) + std::pow(calc, 2));
  const double m_inf = ip3 / (ip3 + node.P_.Kd_IP3_1_);
  const double n_inf = calc / (calc + node.P_.Kd_act_);
  const double calc_ER = (node.P_.Ca_tot_ - calc) / node.P_.ratio_ER_cyt_;
  const double I_leak = node.P_.ratio_ER_cyt_ * node.P_.rate_L_ * (calc_ER - calc);
  const double I_channel = node.P_.ratio_ER_cyt_ * node.P_.rate_IP3R_ * std::pow(m_inf, 3) * std::pow(n_inf, 3) *
    std::pow(f_ip3r, 3) * (calc_ER - calc);

  f[ S::IP3 ] = ( node.P_.IP3_0_ - ip3 ) / node.P_.tau_IP3_;
  f[ S::Ca ] = I_channel - I_pump + I_leak;
  f[ S::h_IP3R ] = alpha_f_ip3r * ( 1.0 - f_ip3r ) - beta_f_ip3r * f_ip3r;

  return GSL_SUCCESS;
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::astrocyte::Parameters_::Parameters_()
  : Ca_tot_( 2.0 )         // uM
  , IP3_0_( 0.16 )         // uM
  , Kd_IP3_1_( 0.13 )      // uM
  , Kd_IP3_2_( 0.9434 )    // uM
  , Km_SERCA_( 0.1 )       // uM
  , Kd_act_( 0.08234 )     // uM
  , Kd_inh_( 1.049 )       // uM
  , ratio_ER_cyt_( 0.185 )
  , incr_IP3_( 5.0 )       // uM
  , k_IP3R_( 0.0002 )      // 1/(uM*ms)
  , rate_L_( 0.00011 )     // 1/ms
  , SIC_th_( 196.69 )      // nM
  , tau_IP3_( 7142.0 )     // ms
  , rate_IP3R_( 0.006 )    // 1/ms
  , rate_SERCA_( 0.0009 )  // uM/ms
{
}

nest::astrocyte::State_::State_( const Parameters_& p )
{
  y_[ IP3 ] = p.IP3_0_; // uM
  y_[ Ca ] = 0.073;           // uM
  y_[ h_IP3R ] = 0.793;
}

nest::astrocyte::State_::State_( const State_& s )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::astrocyte::State_& nest::astrocyte::State_::operator=( const State_& s )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
  return *this;
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::astrocyte::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::Ca_tot, Ca_tot_ );
  def< double >( d, names::IP3_0, IP3_0_ );
  def< double >( d, names::Kd_act, Kd_act_ );
  def< double >( d, names::Kd_inh, Kd_inh_ );
  def< double >( d, names::Kd_IP3_1, Kd_IP3_1_ );
  def< double >( d, names::Kd_IP3_2, Kd_IP3_2_ );
  def< double >( d, names::Km_SERCA, Km_SERCA_ );
  def< double >( d, names::ratio_ER_cyt, ratio_ER_cyt_ );
  def< double >( d, names::incr_IP3, incr_IP3_ );
  def< double >( d, names::k_IP3R, k_IP3R_ );
  def< double >( d, names::SIC_th, SIC_th_ );
  def< double >( d, names::rate_L, rate_L_ );
  def< double >( d, names::rate_IP3R, rate_IP3R_ );
  def< double >( d, names::rate_SERCA, rate_SERCA_ );
  def< double >( d, names::tau_IP3, tau_IP3_ );
}

void
nest::astrocyte::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::Ca_tot, Ca_tot_ );
  updateValue< double >( d, names::IP3_0, IP3_0_ );
  updateValue< double >( d, names::Kd_act, Kd_act_ );
  updateValue< double >( d, names::Kd_inh, Kd_inh_ );
  updateValue< double >( d, names::Kd_IP3_1, Kd_IP3_1_ );
  updateValue< double >( d, names::Kd_IP3_2, Kd_IP3_2_ );
  updateValue< double >( d, names::Km_SERCA, Km_SERCA_ );
  updateValue< double >( d, names::ratio_ER_cyt, ratio_ER_cyt_ );
  updateValue< double >( d, names::incr_IP3, incr_IP3_ );
  updateValue< double >( d, names::k_IP3R, k_IP3R_ );
  updateValue< double >( d, names::SIC_th, SIC_th_ );
  updateValue< double >( d, names::rate_L, rate_L_ );
  updateValue< double >( d, names::rate_IP3R, rate_IP3R_ );
  updateValue< double >( d, names::rate_SERCA, rate_SERCA_ );
  updateValue< double >( d, names::tau_IP3, tau_IP3_ );

  if ( Ca_tot_ <= 0 )
  {
    throw BadProperty( "Total free astrocytic concentration must be positive." );
  }
  if ( IP3_0_ < 0 )
  {
    throw BadProperty( "Baseline value of astrocytic IP3 must be non-negative." );
  }
  if ( Kd_act_ <= 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of calcium (activation) must be positive." );
  }
  if ( Kd_inh_ < 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of calcium (inhibition) must be non-negative." );
  }
  if ( Kd_IP3_1_ <= 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of IP3 must be positive." );
  }
  if ( Kd_IP3_2_ <= 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of IP3 must be positive." );
  }
  if ( Km_SERCA_ <= 0 )
  {
    throw BadProperty( "Activation constant of astrocytic SERCA pump must be positive." );
  }
  if ( ratio_ER_cyt_ <= 0 )
  {
    throw BadProperty( "Ratio between astrocytic ER and cytosol volumes must be positive." );
  }
  if ( incr_IP3_ < 0 )
  {
    throw BadProperty( "Rate constant of strocytic IP3R production must be non-negative." );
  }
  if ( k_IP3R_ < 0 )
  {
    throw BadProperty( "Astrocytic IP2R binding constant for calcium inhibition must be non-negative." );
  }
  if ( SIC_th_ < 0 )
  {
    throw BadProperty( "Calcium threshold for producing SIC must be non-negative." );
  }
  if ( rate_L_ < 0 )
  {
    throw BadProperty( "Rate constant of calcium leak from astrocytic ER to cytosol must be non-negative." );
  }
  if ( rate_IP3R_ < 0 )
  {
    throw BadProperty( "Maximal rate of calcium release via astrocytic IP3R must be non-negative." );
  }
  if ( rate_SERCA_ < 0 )
  {
    throw BadProperty( "Maximal rate of calcium uptake by astrocytic SERCA pump must be non-negative." );
  }
  if ( tau_IP3_ <= 0 )
  {
    throw BadProperty( "Time constant of astrocytic IP3 degradation must be positive." );
  }
}

void
nest::astrocyte::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::IP3, y_[ IP3 ] );
  def< double >( d, names::Ca, y_[ Ca ] );
  def< double >( d, names::h_IP3R, y_[ h_IP3R ] );
}

void
nest::astrocyte::State_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::IP3, y_[ IP3 ] );
  updateValue< double >( d, names::Ca, y_[ Ca ] );
  updateValue< double >( d, names::h_IP3R, y_[ h_IP3R ] );

  if ( y_[ IP3 ] <  0 )
  {
    throw BadProperty( "IP3 concentration in the astrocyte cytosol must be non-negative." );
  }
  if ( y_[ Ca ] < 0 )
  {
    throw BadProperty( "Calcium concentration in the astrocyte cytosol must be non-negative." );
  }
  if ( y_[ h_IP3R ] < 0 || y_[ h_IP3R ] > 1 )
  {
     throw BadProperty( "The fraction of active IP3 receptors on the astrocytic ER must be between 0 and 1." );
  }
}

nest::astrocyte::Buffers_::Buffers_( astrocyte& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::astrocyte::Buffers_::Buffers_( const Buffers_&, astrocyte& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::astrocyte::astrocyte()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
  // Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

nest::astrocyte::astrocyte( const astrocyte& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  // Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

nest::astrocyte::~astrocyte()
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
nest::astrocyte::init_state_( const Node& proto )
{
  const astrocyte& pr = downcast< astrocyte >( proto );
  S_ = pr.S_;
}

void
nest::astrocyte::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize

  ArchivingNode::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( not B_.s_ )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( not B_.c_ )
  {
    B_.c_ = gsl_odeiv_control_y_new( 1e-6, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-6, 0.0, 1.0, 0.0 );
  }

  if ( not B_.e_ )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = astrocyte_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
}

void
nest::astrocyte::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 *
 * Spikes are not generated by astrocytes and the related functions
 * are not needed. However, it is kept here to avoid possible
 * inconsistencies with the rest of the code.
 * TO DO: Check how to safely remove spike handling functions.
 * ---------------------------------------------------------------- */

inline void
nest::astrocyte::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 and ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // allocate memory to store the new interpolation coefficients
  // to be sent by SIC event
  std::vector< double > sic_values( kernel().connection_manager.get_min_delay(), 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    // B_.lag is needed by astrocyte_dynamics to
    // determine the current section
    B_.lag_ = lag;

    double t = 0.0;

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
        S_.y_ );              // neuronal state
      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }
    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

    // this is to add the incoming spikes to the state variable
    S_.y_[ State_::IP3 ] += P_.incr_IP3_ * B_.spike_exc_.get_value( lag );

    // normalize Calcium concentration
    // 1000.0: change unit from uM to nM
    double calc_thr = S_.y_[ State_::Ca ] * 1000.0 - P_.SIC_th_;
    if ( calc_thr > 1.0 )
    {
      // originally this is multiplyed by std::pow(25, 2)*3.14*std::pow(10, -2)
      // to convert to pA from uA/cm2; now users can set the SIC weight
      sic_values[ lag ] = std::log( calc_thr );
    }

  } // end for loop

  // Send SIC event
  SICEvent sic;
  sic.set_coeffarray( sic_values );
  kernel().event_delivery_manager.send_secondary( *this, sic );
  sic_ = sic_values[0]; // for testing, to be deleted
}

void
nest::astrocyte::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::astrocyte::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
