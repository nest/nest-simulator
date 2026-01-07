/*
 *  astrocyte_lr_1994.cpp
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


#include "astrocyte_lr_1994.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath>
#include <cstdio>
#include <iostream>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dictutils.h"

nest::RecordablesMap< nest::astrocyte_lr_1994 > nest::astrocyte_lr_1994::recordablesMap_;

namespace nest
{
void
register_astrocyte_lr_1994( const std::string& name )
{
  register_node_model< astrocyte_lr_1994 >( name );
}

// Override the create() method with one call to RecordablesMap::insert_() for each quantity to be recorded.
template <>
void
RecordablesMap< astrocyte_lr_1994 >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::IP3, &astrocyte_lr_1994::get_y_elem_< astrocyte_lr_1994::State_::IP3 > );
  insert_( names::Ca_astro, &astrocyte_lr_1994::get_y_elem_< astrocyte_lr_1994::State_::Ca_astro > );
  insert_( names::h_IP3R, &astrocyte_lr_1994::get_y_elem_< astrocyte_lr_1994::State_::h_IP3R > );
}

extern "C" int
astrocyte_lr_1994_dynamics( double time, const double y[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::astrocyte_lr_1994::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::astrocyte_lr_1994& node = *( reinterpret_cast< nest::astrocyte_lr_1994* >( pnode ) );

  // y[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double& ip3 = y[ S::IP3 ];
  const double& h_ip3r = y[ S::h_IP3R ];

  // Ca_tot_ corresponds to the c_0 (total [Ca++] in terms of cytosolic vol)
  // in De Young & Keizer (1992) and Li & Rinzel (1994)
  const double calc = std::max( 0.0, std::min( y[ S::Ca_astro ], node.P_.Ca_tot_ ) ); // keep calcium within limits
  const double alpha_h_ip3r =
    node.P_.k_IP3R_ * node.P_.Kd_inh_ * ( ip3 + node.P_.Kd_IP3_1_ ) / ( ip3 + node.P_.Kd_IP3_2_ );
  const double beta_h_ip3r = node.P_.k_IP3R_ * calc;
  const double J_pump =
    node.P_.rate_SERCA_ * std::pow( calc, 2 ) / ( std::pow( node.P_.Km_SERCA_, 2 ) + std::pow( calc, 2 ) );
  const double m_inf = ip3 / ( ip3 + node.P_.Kd_IP3_1_ );
  const double n_inf = calc / ( calc + node.P_.Kd_act_ );
  const double calc_ER = ( node.P_.Ca_tot_ - calc ) / node.P_.ratio_ER_cyt_;
  const double J_leak = node.P_.ratio_ER_cyt_ * node.P_.rate_L_ * ( calc_ER - calc );
  const double J_channel = node.P_.ratio_ER_cyt_ * node.P_.rate_IP3R_ * std::pow( m_inf, 3 ) * std::pow( n_inf, 3 )
    * std::pow( h_ip3r, 3 ) * ( calc_ER - calc );

  f[ S::IP3 ] = ( node.P_.IP3_0_ - ip3 ) / node.P_.tau_IP3_;
  f[ S::Ca_astro ] = J_channel - J_pump + J_leak + node.B_.J_noise_;
  f[ S::h_IP3R ] = alpha_h_ip3r * ( 1.0 - h_ip3r ) - beta_h_ip3r * h_ip3r;

  return GSL_SUCCESS;
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::astrocyte_lr_1994::Parameters_::Parameters_()
  // parameters based on Nadkarni & Jung (2003)
  : Ca_tot_( 2.0 )      // µM
  , IP3_0_( 0.16 )      // µM
  , Kd_IP3_1_( 0.13 )   // µM
  , Kd_IP3_2_( 0.9434 ) // µM
  , Kd_act_( 0.08234 )  // µM
  , Kd_inh_( 1.049 )    // µM
  , Km_SERCA_( 0.1 )    // µM
  , SIC_scale_( 1.0 )
  , SIC_th_( 0.19669 )    // µM
  , delta_IP3_( 0.0002 )  // µM
  , k_IP3R_( 0.0002 )     // 1/(µM*ms)
  , rate_IP3R_( 0.006 )   // 1/ms
  , rate_L_( 0.00011 )    // 1/ms
  , rate_SERCA_( 0.0009 ) // µM/ms
  , ratio_ER_cyt_( 0.185 )
  , tau_IP3_( 7142.0 ) // ms
{
}

nest::astrocyte_lr_1994::State_::State_( const Parameters_& p )
{
  // initial values based on Li & Rinzel (1994) and Nadkarni & Jung (2003)
  y_[ IP3 ] = p.IP3_0_;
  y_[ Ca_astro ] = 0.073;
  y_[ h_IP3R ] = 0.793;
}

nest::astrocyte_lr_1994::State_::State_( const State_& s )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::astrocyte_lr_1994::State_&
nest::astrocyte_lr_1994::State_::operator=( const State_& s )
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
nest::astrocyte_lr_1994::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::Ca_tot, Ca_tot_ );
  def< double >( d, names::IP3_0, IP3_0_ );
  def< double >( d, names::Kd_act, Kd_act_ );
  def< double >( d, names::Kd_inh, Kd_inh_ );
  def< double >( d, names::Kd_IP3_1, Kd_IP3_1_ );
  def< double >( d, names::Kd_IP3_2, Kd_IP3_2_ );
  def< double >( d, names::Km_SERCA, Km_SERCA_ );
  def< double >( d, names::ratio_ER_cyt, ratio_ER_cyt_ );
  def< double >( d, names::delta_IP3, delta_IP3_ );
  def< double >( d, names::k_IP3R, k_IP3R_ );
  def< double >( d, names::SIC_scale, SIC_scale_ );
  def< double >( d, names::SIC_th, SIC_th_ );
  def< double >( d, names::rate_L, rate_L_ );
  def< double >( d, names::rate_IP3R, rate_IP3R_ );
  def< double >( d, names::rate_SERCA, rate_SERCA_ );
  def< double >( d, names::tau_IP3, tau_IP3_ );
}

void
nest::astrocyte_lr_1994::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::Ca_tot, Ca_tot_, node );
  updateValueParam< double >( d, names::IP3_0, IP3_0_, node );
  updateValueParam< double >( d, names::Kd_act, Kd_act_, node );
  updateValueParam< double >( d, names::Kd_inh, Kd_inh_, node );
  updateValueParam< double >( d, names::Kd_IP3_1, Kd_IP3_1_, node );
  updateValueParam< double >( d, names::Kd_IP3_2, Kd_IP3_2_, node );
  updateValueParam< double >( d, names::Km_SERCA, Km_SERCA_, node );
  updateValueParam< double >( d, names::ratio_ER_cyt, ratio_ER_cyt_, node );
  updateValueParam< double >( d, names::delta_IP3, delta_IP3_, node );
  updateValueParam< double >( d, names::k_IP3R, k_IP3R_, node );
  updateValueParam< double >( d, names::SIC_scale, SIC_scale_, node );
  updateValueParam< double >( d, names::SIC_th, SIC_th_, node );
  updateValueParam< double >( d, names::rate_L, rate_L_, node );
  updateValueParam< double >( d, names::rate_IP3R, rate_IP3R_, node );
  updateValueParam< double >( d, names::rate_SERCA, rate_SERCA_, node );
  updateValueParam< double >( d, names::tau_IP3, tau_IP3_, node );

  if ( Ca_tot_ <= 0 )
  {
    throw BadProperty( "Total free astrocytic calcium concentration in terms of cytosolic volume must be positive." );
  }
  if ( IP3_0_ < 0 )
  {
    throw BadProperty( "Baseline value of the astrocytic IP3 concentration must be non-negative." );
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
    throw BadProperty( "First astrocytic IP3R dissociation constant of IP3 must be positive." );
  }
  if ( Kd_IP3_2_ <= 0 )
  {
    throw BadProperty( "Second astrocytic IP3R dissociation constant of IP3 must be positive." );
  }
  if ( Km_SERCA_ <= 0 )
  {
    throw BadProperty( "Activation constant of astrocytic SERCA pump must be positive." );
  }
  if ( ratio_ER_cyt_ <= 0 )
  {
    throw BadProperty( "Ratio between astrocytic ER and cytosol volumes must be positive." );
  }
  if ( delta_IP3_ < 0 )
  {
    throw BadProperty(
      "Parameter determining the increase in astrocytic IP3 concentration induced by synaptic input must be "
      "non-negative." );
  }
  if ( k_IP3R_ < 0 )
  {
    throw BadProperty( "Astrocytic IP3R binding constant for calcium inhibition must be non-negative." );
  }
  if ( SIC_scale_ <= 0 )
  {
    throw BadProperty( "Parameter determining the scale of astrocytic SIC output must be positive." );
  }
  if ( SIC_th_ < 0 )
  {
    throw BadProperty(
      "Threshold that determines the minimal level of intracellular astrocytic calcium sufficient to induce SIC must "
      "be non-negative." );
  }
  if ( rate_L_ < 0 )
  {
    throw BadProperty( "Rate constant of calcium leak from astrocytic ER to cytosol must be non-negative." );
  }
  if ( rate_IP3R_ < 0 )
  {
    throw BadProperty( "Maximum rate of calcium release via astrocytic IP3R must be non-negative." );
  }
  if ( rate_SERCA_ < 0 )
  {
    throw BadProperty( "Maximum rate of calcium uptake by astrocytic SERCA pump must be non-negative." );
  }
  if ( tau_IP3_ <= 0 )
  {
    throw BadProperty( "Time constant of the exponential decay of astrocytic IP3 must be positive." );
  }
}

void
nest::astrocyte_lr_1994::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::IP3, y_[ IP3 ] );
  def< double >( d, names::Ca_astro, y_[ Ca_astro ] );
  def< double >( d, names::h_IP3R, y_[ h_IP3R ] );
}

void
nest::astrocyte_lr_1994::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::IP3, y_[ IP3 ], node );
  updateValueParam< double >( d, names::Ca_astro, y_[ Ca_astro ], node );
  updateValueParam< double >( d, names::h_IP3R, y_[ h_IP3R ], node );

  if ( y_[ IP3 ] < 0 )
  {
    throw BadProperty( "IP3 concentration must be non-negative." );
  }
  if ( y_[ Ca_astro ] < 0 )
  {
    throw BadProperty( "Calcium concentration must be non-negative." );
  }
  if ( y_[ h_IP3R ] < 0 || y_[ h_IP3R ] > 1 )
  {
    throw BadProperty( "The fraction of active IP3 receptors on the astrocytic ER must be between 0 and 1." );
  }
}

nest::astrocyte_lr_1994::Buffers_::Buffers_( astrocyte_lr_1994& n )
  : logger_( n )
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::astrocyte_lr_1994::Buffers_::Buffers_( const Buffers_&, astrocyte_lr_1994& n )
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

nest::astrocyte_lr_1994::astrocyte_lr_1994()
  : Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::astrocyte_lr_1994::astrocyte_lr_1994( const astrocyte_lr_1994& n )
  : Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::astrocyte_lr_1994::~astrocyte_lr_1994()
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
nest::astrocyte_lr_1994::init_buffers_()
{
  B_.spike_exc_.clear(); // includes resize
  B_.currents_.clear();
  B_.sic_values.resize(
    kernel::manager< ConnectionManager >.get_min_delay(), 0.0 ); // set size of SIC buffer according to min_delay

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
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );
  }

  if ( not B_.e_ )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = astrocyte_lr_1994_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.J_noise_ = 0.0;
}

void
nest::astrocyte_lr_1994::pre_run_hook()
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
nest::astrocyte_lr_1994::update( Time const& origin, const long from, const long to )
{
  for ( long lag = from; lag < to; ++lag )
  {
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

    // keep calcium within limits
    S_.y_[ State_::Ca_astro ] = std::max( 0.0, std::min( S_.y_[ State_::Ca_astro ], P_.Ca_tot_ ) );

    // this is to add the incoming spikes to IP3
    S_.y_[ State_::IP3 ] += P_.delta_IP3_ * B_.spike_exc_.get_value( lag );

    // SIC generation according to Nadkarni & Jung, 2003
    //   Suprathreshold log of calcium concentration determines SIC generation
    //   1000.0: change unit to nM as in the original paper
    const double calc_thr = ( S_.y_[ State_::Ca_astro ] - P_.SIC_th_ ) * 1000.0;
    const double sic_value = calc_thr > 1.0 ? std::log( calc_thr ) * P_.SIC_scale_ : 0.0;
    B_.sic_values[ lag ] = sic_value;

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

    // set new input current
    B_.J_noise_ = B_.currents_.get_value( lag );
  }

  // send SIC event
  SICEvent sic;
  sic.set_coeffarray( B_.sic_values );
  kernel::manager< EventDeliveryManager >.send_secondary( *this, sic );
}

void
nest::astrocyte_lr_1994::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() >= 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    throw KernelException( "astrocyte_lr_1994 cannot handle input spikes with negative weights." );
  }
}

void
nest::astrocyte_lr_1994::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  B_.currents_.add_value( e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ), w * c );
}

void
nest::astrocyte_lr_1994::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
