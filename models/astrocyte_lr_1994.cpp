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
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iostream>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dictutils.h"

nest::RecordablesMap< nest::astrocyte_lr_1994 > nest::astrocyte_lr_1994::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< astrocyte_lr_1994 >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::IP3, &astrocyte_lr_1994::get_y_elem_< astrocyte_lr_1994::State_::IP3 > );
  insert_( names::Ca, &astrocyte_lr_1994::get_y_elem_< astrocyte_lr_1994::State_::Ca > );
  insert_( names::h_IP3R, &astrocyte_lr_1994::get_y_elem_< astrocyte_lr_1994::State_::h_IP3R > );
  insert_( names::SIC, &astrocyte_lr_1994::get_sic_ ); // for testing, to be deleted
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

  // alpha shape for SIC
  const double& dsic = y[ S::DSIC ];
  const double& sic = y[ S::SIC ];
  f[ S::DSIC ] = -dsic / node.P_.tau_SIC_;
  f[ S::SIC ] = dsic - sic / node.P_.tau_SIC_;

  f[ S::IP3 ] = ( node.P_.IP3_0_ - ip3 ) / node.P_.tau_IP3_;
  f[ S::Ca ] = I_channel - I_pump + I_leak;
  f[ S::h_IP3R ] = alpha_f_ip3r * ( 1.0 - f_ip3r ) - beta_f_ip3r * f_ip3r;

  return GSL_SUCCESS;
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::astrocyte_lr_1994::Parameters_::Parameters_()
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
  , logarithmic_SIC_ ( true )
  , rate_L_( 0.00011 )     // 1/ms
  , SIC_scale_( 1.0 )
  , SIC_th_( 196.69 )      // nM
  , tau_IP3_( 7142.0 )     // ms
  , rate_IP3R_( 0.006 )    // 1/ms
  , rate_SERCA_( 0.0009 )  // uM/ms
  , alpha_SIC_( false )
  , tau_SIC_( 1000.0 ) // ms
  , delay_SIC_( 1000.0 ) // ms
  , SIC_reactivate_th_( 196.69 ) // nM
  , SIC_reactivate_time_( 100 ) // ms
{
}

nest::astrocyte_lr_1994::State_::State_( const Parameters_& p )
{
  y_[ IP3 ] = p.IP3_0_; // uM
  y_[ Ca ] = 0.073;           // uM
  y_[ h_IP3R ] = 0.793;
  y_[ SIC ] = 0.0; // pA
  y_[ DSIC ] = 0.0;
  // y_[ I_EXC ] = 0.0;
}

nest::astrocyte_lr_1994::State_::State_( const State_& s )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::astrocyte_lr_1994::State_& nest::astrocyte_lr_1994::State_::operator=( const State_& s )
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
  def< double >( d, names::incr_IP3, incr_IP3_ );
  def< double >( d, names::k_IP3R, k_IP3R_ );
  def< double >( d, names::SIC_scale, SIC_scale_ );
  def< double >( d, names::SIC_th, SIC_th_ );
  def< double >( d, names::rate_L, rate_L_ );
  def< double >( d, names::rate_IP3R, rate_IP3R_ );
  def< double >( d, names::rate_SERCA, rate_SERCA_ );
  def< double >( d, names::tau_IP3, tau_IP3_ );
  def< bool >( d, names::logarithmic_SIC, logarithmic_SIC_ );
  def< bool >( d, names::alpha_SIC, alpha_SIC_ );
  def< double >( d, names::tau_SIC, tau_SIC_ );
  def< double >( d, names::delay_SIC, delay_SIC_ );
  def< double >( d, names::SIC_reactivate_th, SIC_reactivate_th_ );
  def< double >( d, names::SIC_reactivate_time, SIC_reactivate_time_ );
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
  updateValueParam< double >( d, names::incr_IP3, incr_IP3_, node );
  updateValueParam< double >( d, names::k_IP3R, k_IP3R_, node );
  updateValueParam< double >( d, names::SIC_scale, SIC_scale_, node );
  updateValueParam< double >( d, names::SIC_th, SIC_th_, node );
  updateValueParam< double >( d, names::rate_L, rate_L_, node );
  updateValueParam< double >( d, names::rate_IP3R, rate_IP3R_, node );
  updateValueParam< double >( d, names::rate_SERCA, rate_SERCA_, node );
  updateValueParam< double >( d, names::tau_IP3, tau_IP3_, node );
  updateValueParam< bool >( d, names::logarithmic_SIC, logarithmic_SIC_, node );
  updateValueParam< bool >( d, names::alpha_SIC, alpha_SIC_, node );
  updateValueParam< double >( d, names::tau_SIC, tau_SIC_, node );
  updateValueParam< double >( d, names::delay_SIC, delay_SIC_, node );
  updateValueParam< double >( d, names::SIC_reactivate_th, SIC_reactivate_th_, node );
  updateValueParam< double >( d, names::SIC_reactivate_time, SIC_reactivate_time_, node );

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
  if ( SIC_scale_ <= 0 )
  {
    throw BadProperty( "Scale of SIC must be positive." );
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
  if ( tau_SIC_ <= 0 )
  {
    throw BadProperty( "Time constant of SIC must be positive." );
  }
  if ( delay_SIC_ < 0 )
  {
    throw BadProperty( "Delay of SIC must be non-negative." );
  }
  if ( SIC_reactivate_th_ < 0 or SIC_reactivate_th_ > SIC_th_ )
  {
    throw BadProperty( "Calcium threshold for reactivating SIC must be non-negative and not larger than calcium threshold for producing SIC." );
  }
  if ( SIC_reactivate_time_ < 0 )
  {
    throw BadProperty( "Time required for reactivating SIC must be non-negative." );
  }
}

void
nest::astrocyte_lr_1994::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::IP3, y_[ IP3 ] );
  def< double >( d, names::Ca, y_[ Ca ] );
  def< double >( d, names::h_IP3R, y_[ h_IP3R ] );
  def< double >( d, names::SIC, y_[ SIC ] );
}

void
nest::astrocyte_lr_1994::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::IP3, y_[ IP3 ], node );
  updateValueParam< double >( d, names::Ca, y_[ Ca ], node );
  updateValueParam< double >( d, names::h_IP3R, y_[ h_IP3R ], node );
  updateValueParam< double >( d, names::SIC, y_[ SIC ], node );

  if ( y_[ IP3 ] <  0 )
  {
    throw BadProperty( "IP3 concentration must be non-negative." );
  }
  if ( y_[ Ca ] < 0 )
  {
    throw BadProperty( "Calcium concentration must be non-negative." );
  }
  if ( y_[ h_IP3R ] < 0 || y_[ h_IP3R ] > 1 )
  {
     throw BadProperty( "The fraction of active IP3 receptors on the astrocytic ER must be between 0 and 1." );
  }
  if ( y_[ SIC ] < 0 )
  {
    throw BadProperty( "SIC must be non-negative." );
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
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
  // Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

nest::astrocyte_lr_1994::astrocyte_lr_1994( const astrocyte_lr_1994& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  // Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
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

  B_.sic_values.resize( kernel().connection_manager.get_min_delay(), 0.0 );

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

  B_.sys_.function = astrocyte_lr_1994_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
  B_.sic_on_ = false;
  B_.sic_on_timer_ = 0.0;
  B_.i0_ex_ = 1.0 * numerics::e / P_.tau_SIC_;
  B_.sic_started_flag_ = false;
  B_.sic_off_timer_ = 0.0;

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
    // B_.lag is needed by astrocyte_lr_1994_dynamics to
    // determine the current section
    B_.lag_ = lag;

    double t = 0.0;
    // get the last normalized calcium concentration before solving ODE
    // this is for threshold-crossing judgement
    double calc_thr_last = S_.y_[ State_::Ca ] * 1000.0 - P_.SIC_th_;

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

    // limit calcium concentration within boundaries
    // first boundary: Ca_cyt*V_cyt no larger than Ca_tot*V_tot
    // Ca_cyt (Ca here), V_cyt: calcium concentration and volumn of cytosol
    // Ca_tot, V_tot: calcium concentraion and volumn in total (cell)
    // ratio_ER_cyt_ is V_ER/V_cyt, and V_ER + V_cyt = V_tot
    if ( S_.y_[ State_::Ca ] > P_.Ca_tot_ + P_.Ca_tot_*P_.ratio_ER_cyt_ )
    {
      S_.y_[ State_::Ca ] = P_.Ca_tot_ + P_.Ca_tot_*P_.ratio_ER_cyt_;
    }
    // second boundary: Ca_cyt no smaller than 0
    else if ( S_.y_[ State_::Ca ] < 0.0 )
    {
      S_.y_[ State_::Ca ] = 0.0;
    }

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

    // this is to add the incoming spikes to the state variable
    S_.y_[ State_::IP3 ] += P_.incr_IP3_ * B_.spike_exc_.get_value( lag );

    // get normalized calcium concentration
    // 1000.0: change unit from uM to nM
    double calc_thr = S_.y_[ State_::Ca ] * 1000.0 - P_.SIC_th_;
    double sic_value = 0.0;
    // alpha-shaped SIC
    if ( P_.alpha_SIC_ == true )
    {
      sic_value = S_.y_[ State_::SIC ];
      // SIC-on (activated/deactivated) state
      if ( B_.sic_on_ == true )
      {
        // timer for delayed SIC
        B_.sic_on_timer_ += B_.step_;
        if ( B_.sic_started_flag_ == false and B_.sic_on_timer_ >= P_.delay_SIC_ )
        {
          // generate a SIC
          if ( P_.logarithmic_SIC_ == true )
          {
            if ( calc_thr > 1.0 )
            {
              S_.y_[ State_::DSIC ] += P_.SIC_scale_*std::log( calc_thr )*B_.i0_ex_;
            }
          }
          else
          {
            S_.y_[ State_::DSIC ] += 0.001*P_.SIC_scale_*calc_thr*B_.i0_ex_;
          }
          B_.sic_started_flag_ = true;
        }
        // condition to return to SIC-off (reactivated) state; to be discussed
        if ( S_.y_[ State_::Ca ] * 1000.0 < P_.SIC_reactivate_th_ )
        {
          // timer for SIC-off
          B_.sic_off_timer_ += B_.step_;
          if ( B_.sic_off_timer_ >= P_.SIC_reactivate_time_ )
          {
            B_.sic_on_ = false;
            B_.sic_started_flag_ = false;
            B_.sic_on_timer_ = 0.0;
            B_.sic_off_timer_ = 0.0;
          }
        }
        else
        {
          B_.sic_off_timer_ = 0.0;
        }
      }
      // SIC-off (reactivated) state
      else
      {
        // catch threshold-crossing point to activate SIC
        if ( calc_thr > 0.0 and calc_thr_last <= 0.0 )
        {
          B_.sic_on_ = true;
        }
      }
    }
    // original version of SIC
    else
    {
      // logarithmic
      if ( P_.logarithmic_SIC_ == true )
      {
        if ( calc_thr > 1.0 )
        {
          // multiplied by std::pow(25, 2)*3.14*std::pow(10, -2) in previous
          // version to convert from uA/cm2 to pA; now users can set the scale of
          // SIC by SIC_scale or SIC connection weight
          sic_value = std::log( calc_thr )*P_.SIC_scale_;
        }
      }
      // simply copy calcium
      else
      {
        if ( calc_thr > 0.0 )
        {
          sic_value = calc_thr*P_.SIC_scale_/1000.0;
        }
      }
    }
    B_.sic_values[ lag ] = sic_value;

  } // end for loop

  // Send SIC event
  SICEvent sic;
  sic.set_coeffarray( B_.sic_values );
  kernel().event_delivery_manager.send_secondary( *this, sic );
  sic_ = B_.sic_values[0]; // for testing, to be deleted
}

void
nest::astrocyte_lr_1994::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() > 0.0 )
  {
    B_.spike_exc_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::astrocyte_lr_1994::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
