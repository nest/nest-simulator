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
  insert_( names::IP3_astro, &astrocyte::get_y_elem_< astrocyte::State_::IP3_astro > );
  insert_( names::Ca_astro, &astrocyte::get_y_elem_< astrocyte::State_::Ca_astro > );
  insert_( names::f_IP3R_astro, &astrocyte::get_y_elem_< astrocyte::State_::f_IP3R_astro > );
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
  const double& ip3 = y[ S::IP3_astro ];
  const double& calc = y[ S::Ca_astro ];
  const double& f_ip3r = y[ S::f_IP3R_astro ];

  const double alpha_f_ip3r = node.P_.r_IP3R_astro_ * node.P_.K_inh_astro_ * ( ip3 + node.P_.K_IP3_1_astro_ ) / ( ip3 +
      node.P_.K_IP3_2_astro_ );
  const double beta_f_ip3r = node.P_.r_IP3R_astro_ * calc;
  const double I_pump = node.P_.v_SERCA_astro_ * std::pow(calc, 2) / (std::pow(node.P_.K_SERCA_astro_, 2) + std::pow(calc, 2));
  const double m_inf = ip3 / (ip3 + node.P_.K_IP3_1_astro_);
  const double n_inf = calc / (calc + node.P_.K_act_astro_);
  const double calc_ER = (node.P_.Ca_tot_astro_ - calc) / node.P_.r_ER_cyt_astro_;
  const double I_leak = node.P_.r_ER_cyt_astro_ * node.P_.r_L_astro_ * (calc_ER - calc);
  const double I_channel = node.P_.r_ER_cyt_astro_ * node.P_.v_IP3R_astro_ * std::pow(m_inf, 3) * std::pow(n_inf, 3) *
    std::pow(f_ip3r, 3) * (calc_ER - calc);


  // set I_gap depending on interpolation order
  double gap = 0.0;

  const double t = time / node.B_.step_;

  switch ( kernel().simulation_manager.get_wfr_interpolation_order() )
  {
  case 0:
    gap = -node.B_.sumj_g_ij_ * ip3 + node.B_.interpolation_coefficients[ node.B_.lag_ ];
    break;

  case 1:
    gap = -node.B_.sumj_g_ij_ * ip3 + node.B_.interpolation_coefficients[ node.B_.lag_ * 2 + 0 ]
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 2 + 1 ] * t;
    break;

  case 3:
    gap = -node.B_.sumj_g_ij_ * ip3 + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 0 ]
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 1 ] * t
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 2 ] * t * t
      + node.B_.interpolation_coefficients[ node.B_.lag_ * 4 + 3 ] * t * t * t;
    break;

  default:
    throw BadProperty( "Interpolation order must be 0, 1, or 3." );
  }

  const double I_gap = gap;

  f[ S::IP3_astro ] = ( node.P_.IP3_0_astro_ - ip3 ) / node.P_.tau_IP3_astro_;
  f[ S::Ca_astro ] = I_channel - I_pump + I_leak;
  f[ S::f_IP3R_astro ] = alpha_f_ip3r * ( 1.0 - f_ip3r ) - beta_f_ip3r * f_ip3r;

  return GSL_SUCCESS;
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::astrocyte::Parameters_::Parameters_()
  : tau_IP3_astro_(7142.0 )   // ms
  , r_IP3_astro_( 5.0 )      // uM / ms
  , K_IP3_1_astro_( 0.13 )    // uM
  , K_inh_astro_( 1.049 )    // uM
  , K_IP3_2_astro_( 0.9434 )    // uM
  , K_act_astro_( 0.08234 )    // uM
  , v_IP3R_astro_( 0.006 )    // 1 / ms
  , r_L_astro_( 0.00011 )    // 1 / ms
  , v_SERCA_astro_( 0.0009 )    // uM / ms
  , K_SERCA_astro_( 0.1 )    // uM
  , r_IP3R_astro_( 0.0002 )    // 1 / (uM*ms)
  , Ca_tot_astro_( 2.0 )   // uM
  , r_ER_cyt_astro_( 0.185 ) 
  , IP3_0_astro_( 0.16 )    // uM
{
}

nest::astrocyte::State_::State_( const Parameters_& p )
{
  y_[ IP3_astro ] = p.IP3_0_astro_;
  y_[ Ca_astro ] = 0.073;   // uM
  y_[ f_IP3R_astro ] = 0.793;
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
  assert( this != &s ); // would be bad logical error in program
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
  def< double >( d, names::Ca_tot_astro, Ca_tot_astro_ ); 
  def< double >( d, names::IP3_0_astro, IP3_0_astro_ );
  def< double >( d, names::K_act_astro, K_act_astro_ );
  def< double >( d, names::K_inh_astro, K_inh_astro_ );
  def< double >( d, names::K_IP3_1_astro, K_IP3_1_astro_ );
  def< double >( d, names::K_IP3_2_astro, K_IP3_2_astro_ );
  def< double >( d, names::K_SERCA_astro, K_SERCA_astro_ );
  def< double >( d, names::r_ER_cyt_astro, r_ER_cyt_astro_ );
  def< double >( d, names::r_IP3_astro, r_IP3_astro_ );
  def< double >( d, names::r_IP3R_astro, r_IP3R_astro_ );
  def< double >( d, names::r_L_astro, r_L_astro_ );
  def< double >( d, names::v_IP3R_astro, v_IP3R_astro_ );
  def< double >( d, names::v_SERCA_astro, v_SERCA_astro_ );
  def< double >( d, names::tau_IP3_astro, tau_IP3_astro_ );
}

void
nest::astrocyte::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::Ca_tot_astro, Ca_tot_astro_ ); 
  updateValue< double >( d, names::IP3_0_astro, IP3_0_astro_ );
  updateValue< double >( d, names::K_act_astro, K_act_astro_ );
  updateValue< double >( d, names::K_inh_astro, K_inh_astro_ );
  updateValue< double >( d, names::K_IP3_1_astro, K_IP3_1_astro_ );
  updateValue< double >( d, names::K_IP3_2_astro, K_IP3_2_astro_ );
  updateValue< double >( d, names::K_SERCA_astro, K_SERCA_astro_ );
  updateValue< double >( d, names::r_ER_cyt_astro, r_ER_cyt_astro_ );
  updateValue< double >( d, names::r_IP3_astro, r_IP3_astro_ );
  updateValue< double >( d, names::r_IP3R_astro, r_IP3R_astro_ );
  updateValue< double >( d, names::r_L_astro, r_L_astro_ );
  updateValue< double >( d, names::v_IP3R_astro, v_IP3R_astro_ );
  updateValue< double >( d, names::v_SERCA_astro, v_SERCA_astro_ );
  updateValue< double >( d, names::tau_IP3_astro, tau_IP3_astro_ );
  
  if ( Ca_tot_astro_ <= 0 )
  {
    throw BadProperty( "Total free astrocytic concentration must be positive." );
  }
  if ( IP3_0_astro_ < 0 )
  {
    throw BadProperty( "Baseline value of astrocytic IP3 must be non-negative." );
  }
  if ( K_act_astro_ <= 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of calcium (activation) must be positive." );
  }
  if ( K_inh_astro_ < 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of calcium (inhibition) must be non-negative." );
  }
  if ( K_IP3_1_astro_ <= 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of IP3 must be positive." );
  }
  if ( K_IP3_2_astro_ <= 0 )
  {
    throw BadProperty( "Astrocytic IP3R dissociation constant of IP3 must be positive." );
  }
  if ( K_SERCA_astro_ <= 0 )
  {
    throw BadProperty( "Activation constant of astrocytic SERCA pump must be positive." );
  }
  if ( r_ER_cyt_astro_ <= 0 )
  {
    throw BadProperty( "Ratio between astrocytic ER and cytosol volumes must be positive." );
  }
  if ( r_IP3_astro_ < 0 )
  {
    throw BadProperty( "Rate constant of strocytic IP3R production must be non-negative." );
  }
  if ( r_IP3R_astro_ < 0 )
  {
    throw BadProperty( "Astrocytic IP2R binding constant for calcium inhibition must be non-negative." );
  }
  if ( r_L_astro_ < 0 )
  {
    throw BadProperty( "Rate constant of calcium leak from astrocytic ER to cytosol must be non-negative." );
  }
  if ( v_IP3R_astro_ < 0 )
  {
    throw BadProperty( "Maximal rate of calcium release via astrocytic IP3R must be non-negative." );
  }
  if ( v_SERCA_astro_ < 0 )
  {
    throw BadProperty( "Maximal rate of calcium uptake by astrocytic SERCA pump must be non-negative." );
  }
  if ( tau_IP3_astro_ <= 0 )
  {
    throw BadProperty( "Time constant of astrocytic IP3 degradation must be positive." );
  }
}

void
nest::astrocyte::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::IP3_astro, y_[ IP3_astro ] );
  def< double >( d, names::Ca_astro, y_[ Ca_astro ] );
  def< double >( d, names::f_IP3R_astro, y_[ f_IP3R_astro ] );
}

void
nest::astrocyte::State_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::IP3_astro, y_[ IP3_astro ] );
  updateValue< double >( d, names::Ca_astro, y_[ Ca_astro ] );
  updateValue< double >( d, names::f_IP3R_astro, y_[ f_IP3R_astro ] );

  if ( y_[ IP3_astro ] <  0 )
  {
    throw BadProperty( "IP3 concentration in the astrocyte cytosol must be non-negative." );
  }
  if ( y_[ Ca_astro ] < 0 )
  {
    throw BadProperty( "Calcium concentration in the astrocyte cytosol must be non-negative." );	  
  }
  if ( y_[ f_IP3R_astro ] < 0 || y_[ f_IP3R_astro ] > 1 )
  {
     throw BadProperty( "The fraction of active IP3 receptors on the astrocytic ER must be between 0 and 1." );
  }
}

nest::astrocyte::Buffers_::Buffers_( astrocyte& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::astrocyte::Buffers_::Buffers_( const Buffers_&, astrocyte& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
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
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

nest::astrocyte::astrocyte( const astrocyte& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
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
  B_.currents_.clear();  // includes resize

  // allocate strucure for gap events here
  // function is called from Scheduler::prepare_nodes() before the
  // first call to update
  // so we already know which interpolation scheme to use according
  // to the properties of this neurons
  // determine size of structure depending on interpolation scheme
  // and unsigned int Scheduler::min_delay() (number of simulation time steps
  // per min_delay step)

  // resize interpolation_coefficients depending on interpolation order
  const size_t buffer_size =
    kernel().connection_manager.get_min_delay() * ( kernel().simulation_manager.get_wfr_interpolation_order() + 1 );

  B_.interpolation_coefficients.resize( buffer_size, 0.0 );

  B_.last_y_values.resize( kernel().connection_manager.get_min_delay(), 0.0 );

  B_.sumj_g_ij_ = 0.0;

  ArchivingNode::clear_history();

  B_.logger_.reset();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( B_.s_ == 0 )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( B_.c_ == 0 )
  {
    B_.c_ = gsl_odeiv_control_y_new( 1e-6, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-6, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = astrocyte_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

void
nest::astrocyte::calibrate()
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

bool
nest::astrocyte::update_( Time const& origin, const long from, const long to, const bool called_from_wfr_update )
{

  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const size_t interpolation_order = kernel().simulation_manager.get_wfr_interpolation_order();
  const double wfr_tol = kernel().simulation_manager.get_wfr_tol();
  bool wfr_tol_exceeded = false;

  // allocate memory to store the new interpolation coefficients
  // to be sent by gap event
  const size_t buffer_size = kernel().connection_manager.get_min_delay() * ( interpolation_order + 1 );
  std::vector< double > new_coefficients( buffer_size, 0.0 );
  std::vector< double > sic_values( kernel().connection_manager.get_min_delay(), 0.0 );


  // parameters needed for piecewise interpolation
  double y_i = 0.0, y_ip1 = 0.0, hf_i = 0.0, hf_ip1 = 0.0;
  double f_temp[ State_::STATE_VEC_SIZE ];

  for ( long lag = from; lag < to; ++lag )
  {

    // B_.lag is needed by astrocyte_dynamics to
    // determine the current section
    B_.lag_ = lag;

    if ( called_from_wfr_update )
    {
      y_i = S_.y_[ State_::IP3_astro ];
      if ( interpolation_order == 3 )
      {
        astrocyte_dynamics( 0, S_.y_, f_temp, reinterpret_cast< void* >( this ) );
        hf_i = B_.step_ * f_temp[ State_::IP3_astro ];
      }
    }

    double t = 0.0;
    // TODO: deleted to much?
    const double U_old = S_.y_[ State_::IP3_astro ];

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

    if ( not called_from_wfr_update )
    {
      // log state data
      B_.logger_.record_data( origin.get_steps() + lag );

      // set new input current
      B_.I_stim_ = B_.currents_.get_value( lag );
    }
    else // if(called_from_wfr_update)
    {
      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded or fabs( S_.y_[ State_::IP3_astro ] - B_.last_y_values[ lag ] ) > wfr_tol;
      B_.last_y_values[ lag ] = S_.y_[ State_::IP3_astro ];

      // update different interpolations

      // constant term is the same for each interpolation order
      new_coefficients[ lag * ( interpolation_order + 1 ) + 0 ] = y_i;

      switch ( interpolation_order )
      {
      case 0:
        break;

      case 1:
        y_ip1 = S_.y_[ State_::IP3_astro ];

        new_coefficients[ lag * ( interpolation_order + 1 ) + 1 ] = y_ip1 - y_i;
        break;

      case 3:
        y_ip1 = S_.y_[ State_::IP3_astro ];
        astrocyte_dynamics( B_.step_, S_.y_, f_temp, reinterpret_cast< void* >( this ) );
        hf_ip1 = B_.step_ * f_temp[ State_::IP3_astro ];

        new_coefficients[ lag * ( interpolation_order + 1 ) + 1 ] = hf_i;
        new_coefficients[ lag * ( interpolation_order + 1 ) + 2 ] = -3 * y_i + 3 * y_ip1 - 2 * hf_i - hf_ip1;
        new_coefficients[ lag * ( interpolation_order + 1 ) + 3 ] = 2 * y_i - 2 * y_ip1 + hf_i + hf_ip1;
        break;

      default:
        throw BadProperty( "Interpolation order must be 0, 1, or 3." );
      }
    }

    if ( not called_from_wfr_update )
    {
      // this is to add the incoming spikes to the state variable
      S_.y_[ State_::IP3_astro ] += P_.r_IP3_astro_ * B_.spike_exc_.get_value( lag );
    }
    else
    {
      // this is to add the incoming spikes to the state variable
      S_.y_[ State_::IP3_astro ] += P_.r_IP3_astro_ * B_.spike_exc_.get_value_wfr_update( lag );
    }
    double calc_thr = S_.y_[ State_::Ca_astro ] * 1000.0 - 196.69;
    if ( calc_thr > 1.0 )
    {
      /* The SIC is converted to pA from uA/cm2 in the original publication */
      sic_values[ lag ] = std::pow(25, 2) * 3.14 * std::pow(10, -2) * std::log( calc_thr );
    }

  } // end for-loop

  // if not called_from_wfr_update perform constant extrapolation
  // and reset last_y_values
  if ( not called_from_wfr_update )
  {
    for ( long temp = from; temp < to; ++temp )
    {
      // TODO: this is for the connection between two astrocytes
      new_coefficients[ temp * ( interpolation_order + 1 ) + 0 ] = S_.y_[ State_::IP3_astro ];
    }

    std::vector< double >( kernel().connection_manager.get_min_delay(), 0.0 ).swap( B_.last_y_values );
  }

  // Send gap-event
  GapJunctionEvent ge;
  ge.set_coeffarray( new_coefficients );
  kernel().event_delivery_manager.send_secondary( *this, ge );
  
  // Send sic-event
  SICEvent sic;
  sic.set_coeffarray( sic_values );
  kernel().event_delivery_manager.send_secondary( *this, sic );

  // Reset variables
  B_.sumj_g_ij_ = 0.0;
  std::vector< double >( buffer_size, 0.0 ).swap( B_.interpolation_coefficients );

  return wfr_tol_exceeded;
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
nest::astrocyte::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::astrocyte::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
nest::astrocyte::handle( GapJunctionEvent& e )
{
  const double weight = e.get_weight();

  B_.sumj_g_ij_ += weight;

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    B_.interpolation_coefficients[ i ] += weight * e.get_coeffvalue( it );
    ++i;
  }
}

#endif // HAVE_GSL
