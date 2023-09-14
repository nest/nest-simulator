/*
 *  iaf_wang_2002.cpp
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

#include "iaf_wang_2002.h"

#ifdef HAVE_GSL

// Includes from libnestutil:
#include "dictdatum.h"
#include "dict_util.h"
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
#include "lockptrdatum.h"

/* ---------------------------------------------------------------------------
 * Recordables map
 * --------------------------------------------------------------------------- */
nest::RecordablesMap< nest::iaf_wang_2002 > nest::iaf_wang_2002::recordablesMap_;

namespace nest
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_wang_2002 >::create()
{
  // add state variables to recordables map
  insert_( names::V_m, &iaf_wang_2002::get_ode_state_elem_< iaf_wang_2002::State_::V_m > );
  insert_( names::g_AMPA, &iaf_wang_2002::get_ode_state_elem_< iaf_wang_2002::State_::G_AMPA > );
  insert_( names::g_GABA, &iaf_wang_2002::get_ode_state_elem_< iaf_wang_2002::State_::G_GABA > );
  insert_( names::NMDA_sum, &iaf_wang_2002::get_NMDA_sum_ );
}
}
/* ---------------------------------------------------------------------------
 * Default constructors defining default parameters and state
 * --------------------------------------------------------------------------- */

nest::iaf_wang_2002::Parameters_::Parameters_()
  : E_L( -70.0 )          // mV
  , E_ex( 0.0 )           // mV
  , E_in( -70.0 )         // mV
  , V_th( -55.0 )         // mV
  , V_reset( -60.0 )      // mV
  , C_m( 500.0 )          // pF
  , g_L( 25.0 )           // nS
  , t_ref( 2.0 )          // ms
  , tau_AMPA( 2.0 )       // ms
  , tau_GABA( 5.0 )       // ms
  , tau_rise_NMDA( 2.0 )  // ms
  , tau_decay_NMDA( 100 ) // ms
  , alpha( 0.5 )          // 1 / ms
  , conc_Mg2( 1 )         // mM
  , gsl_error_tol( 1e-3 )
{
}

nest::iaf_wang_2002::State_::State_( const Parameters_& p )
  : r_( 0 )
  , sum_S_post_( 0 )
{
  ode_state_[ V_m ] = p.E_L; // initialize to reversal potential
  ode_state_[ G_AMPA ] = 0.0;
  ode_state_[ G_GABA ] = 0.0;
  ode_state_[ S_pre ] = 0.0;
  ode_state_[ X_pre ] = 0.0;

  dy_[ V_m ] = 0.0;
  dy_[ G_AMPA ] = 0.0;
  dy_[ G_GABA ] = 0.0;
  dy_[ S_pre ] = 0.0;
  dy_[ X_pre ] = 0.0;
}

nest::iaf_wang_2002::State_::State_( const State_& s )
  : r_( s.r_ )
  , sum_S_post_( s.sum_S_post_ )
{
  ode_state_[ V_m ] = s.ode_state_[ V_m ];
  ode_state_[ G_AMPA ] = s.ode_state_[ G_AMPA ];
  ode_state_[ G_GABA ] = s.ode_state_[ G_GABA ];
  ode_state_[ S_pre ] = s.ode_state_[ S_pre ];
  ode_state_[ X_pre ] = s.ode_state_[ X_pre ];

  dy_[ V_m ] = s.dy_[ V_m ];
  dy_[ G_AMPA ] = s.dy_[ G_AMPA ];
  dy_[ G_GABA ] = s.dy_[ G_GABA ];
  dy_[ S_pre ] = s.dy_[ S_pre ];
  dy_[ X_pre ] = s.dy_[ X_pre ];
}

nest::iaf_wang_2002::Buffers_::Buffers_( iaf_wang_2002& n )
  : logger_( n )
  , spikes_()
  , NMDA_cond_()
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
  , step_( Time::get_resolution().get_ms() )
  , integration_step_( step_ )
{
  // Initialization of the remaining members is deferred to init_buffers_().
}

nest::iaf_wang_2002::Buffers_::Buffers_( const Buffers_&, iaf_wang_2002& n )
  : logger_( n )
  , spikes_()
  , NMDA_cond_()
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
  , step_( Time::get_resolution().get_ms() )
  , integration_step_( step_ )
{
  // Initialization of the remaining members is deferred to init_buffers_().
}

/* ---------------------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * --------------------------------------------------------------------------- */

void
nest::iaf_wang_2002::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L );
  def< double >( d, names::E_ex, E_ex );
  def< double >( d, names::E_in, E_in );
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::C_m, C_m );
  def< double >( d, names::g_L, g_L );
  def< double >( d, names::t_ref, t_ref );
  def< double >( d, names::tau_AMPA, tau_AMPA );
  def< double >( d, names::tau_GABA, tau_GABA );
  def< double >( d, names::tau_rise_NMDA, tau_rise_NMDA );
  def< double >( d, names::tau_decay_NMDA, tau_decay_NMDA );
  def< double >( d, names::alpha, alpha );
  def< double >( d, names::conc_Mg2, conc_Mg2 );
  def< double >( d, names::gsl_error_tol, gsl_error_tol );
}

void
nest::iaf_wang_2002::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // allow setting the membrane potential
  updateValueParam< double >( d, names::V_th, V_th, node );
  updateValueParam< double >( d, names::V_reset, V_reset, node );
  updateValueParam< double >( d, names::t_ref, t_ref, node );
  updateValueParam< double >( d, names::E_L, E_L, node );

  updateValueParam< double >( d, names::E_ex, E_ex, node );
  updateValueParam< double >( d, names::E_in, E_in, node );

  updateValueParam< double >( d, names::C_m, C_m, node );
  updateValueParam< double >( d, names::g_L, g_L, node );

  updateValueParam< double >( d, names::tau_AMPA, tau_AMPA, node );
  updateValueParam< double >( d, names::tau_GABA, tau_GABA, node );
  updateValueParam< double >( d, names::tau_rise_NMDA, tau_rise_NMDA, node );
  updateValueParam< double >( d, names::tau_decay_NMDA, tau_decay_NMDA, node );

  updateValueParam< double >( d, names::alpha, alpha, node );
  updateValueParam< double >( d, names::conc_Mg2, conc_Mg2, node );

  updateValueParam< double >( d, names::gsl_error_tol, gsl_error_tol, node );

  if ( V_reset >= V_th )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_m <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( t_ref < 0 )
  {
    throw BadProperty( "Refractory time cannot be negative." );
  }
  if ( tau_AMPA <= 0 or tau_GABA <= 0 or tau_rise_NMDA <= 0 or tau_decay_NMDA <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  if ( alpha <= 0 )
  {
    throw BadProperty( "alpha > 0 required." );
  }
  if ( conc_Mg2 <= 0 )
  {
    throw BadProperty( "Mg2 concentration must be strictly positive." );
  }
  if ( gsl_error_tol <= 0.0 )
  {
    throw BadProperty( "The gsl_error_tol must be strictly positive." );
  }
}

void
nest::iaf_wang_2002::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, ode_state_[ V_m ] ); // Membrane potential
  def< double >( d, names::g_AMPA, ode_state_[ G_AMPA ] );
  def< double >( d, names::g_GABA, ode_state_[ G_GABA ] );

  // total NMDA sum
  double NMDA_sum = get_NMDA_sum();
  def< double >( d, names::NMDA_sum, NMDA_sum );
}

void
nest::iaf_wang_2002::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, ode_state_[ V_m ], node );
  updateValueParam< double >( d, names::g_AMPA, ode_state_[ G_AMPA ], node );
  updateValueParam< double >( d, names::g_GABA, ode_state_[ G_GABA ], node );
}

/* ---------------------------------------------------------------------------
 * Default constructor for node
 * --------------------------------------------------------------------------- */

nest::iaf_wang_2002::iaf_wang_2002()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();

  calibrate();
}

/* ---------------------------------------------------------------------------
 * Copy constructor for node
 * --------------------------------------------------------------------------- */

nest::iaf_wang_2002::iaf_wang_2002( const iaf_wang_2002& n_ )
  : ArchivingNode( n_ )
  , P_( n_.P_ )
  , S_( n_.S_ )
  , B_( n_.B_, *this )
{
}

/* ---------------------------------------------------------------------------
 * Destructor for node
 * --------------------------------------------------------------------------- */

nest::iaf_wang_2002::~iaf_wang_2002()
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

/* ---------------------------------------------------------------------------
 * Node initialization functions
 * --------------------------------------------------------------------------- */

void
nest::iaf_wang_2002::init_state_()
{
}

void
nest::iaf_wang_2002::init_buffers_()
{
  B_.spikes_.resize( 2 );

  for ( auto& sb : B_.spikes_ )
  {
    sb.clear(); // includes resize
  }

  B_.NMDA_cond_.clear();
  B_.currents_.clear(); // includes resize

  B_.logger_.reset(); // includes resize
  ArchivingNode::clear_history();

  if ( B_.s_ == nullptr )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_step_reset( B_.s_ );
  }

  if ( B_.c_ == nullptr )
  {
    B_.c_ = gsl_odeiv_control_y_new( P_.gsl_error_tol, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, P_.gsl_error_tol, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == nullptr )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = iaf_wang_2002_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );
  B_.step_ = Time::get_resolution().get_ms();
  B_.integration_step_ = Time::get_resolution().get_ms();

  B_.I_stim_ = 0.0;
}

void
nest::iaf_wang_2002::calibrate()
{
  B_.logger_.init();

  // internals V_
  V_.RefractoryCounts = Time( Time::ms( ( double ) ( P_.t_ref ) ) ).get_steps();
}

/* ---------------------------------------------------------------------------
 * Update and spike handling functions
 * --------------------------------------------------------------------------- */

extern "C" inline int
nest::iaf_wang_2002_dynamics( double, const double ode_state[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::iaf_wang_2002::State_ S;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  const nest::iaf_wang_2002& node = *( reinterpret_cast< nest::iaf_wang_2002* >( pnode ) );

  // ode_state[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.ode_state[].

  const double I_AMPA = ( ode_state[ S::V_m ] - node.P_.E_ex ) * ode_state[ S::G_AMPA ];

  const double I_rec_GABA = ( ode_state[ S::V_m ] - node.P_.E_in ) * ode_state[ S::G_GABA ];

  const double I_rec_NMDA = ( ode_state[ S::V_m ] - node.P_.E_ex )
    / ( 1 + node.P_.conc_Mg2 * std::exp( -0.062 * ode_state[ S::V_m ] ) / 3.57 ) * node.S_.sum_S_post_;

  const double I_syn = I_AMPA + I_rec_GABA + I_rec_NMDA - node.B_.I_stim_;

  f[ S::V_m ] = ( -node.P_.g_L * ( ode_state[ S::V_m ] - node.P_.E_L ) - I_syn ) / node.P_.C_m;

  f[ S::G_AMPA ] = -ode_state[ S::G_AMPA ] / node.P_.tau_AMPA;
  f[ S::G_GABA ] = -ode_state[ S::G_GABA ] / node.P_.tau_GABA;

    f[ S::S_pre ] =
      -ode_state[ S::S_pre ] / node.P_.tau_decay_NMDA + node.P_.alpha * ode_state[ S::X_pre ] * ( 1 - ode_state[ S::S_pre ] );
    f[ S::X_pre ] = -ode_state[ S::X_pre ] / node.P_.tau_rise_NMDA;

  return GSL_SUCCESS;
}

void
nest::iaf_wang_2002::update( Time const& origin, const long from, const long to )
{
  std::vector< double > s_vals( kernel().connection_manager.get_min_delay(), 0.0 );

  for ( long lag = from; lag < to; ++lag )
  {
    /*double t = 0.0;

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
        &B_.sys_,              // system of ODE
        &t,                    // from t
        B_.step_,              // to t <= step
        &B_.integration_step_, // integration step size
        S_.ode_state_ );       // neuronal state

      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }*/

    iaf_wang_2002_dynamics( 0, S_.ode_state_, S_.dy_, reinterpret_cast< void* >( this ) );
    for ( auto i = 0; i < State_::STATE_VEC_SIZE; ++i )
    {
      S_.ode_state_[ i ] += B_.step_ * S_.dy_[ i ];
    }

    // add incoming spikes
    S_.ode_state_[ State_::G_AMPA ] += B_.spikes_[ AMPA - 1 ].get_value( lag );
    S_.ode_state_[ State_::G_GABA ] += B_.spikes_[ GABA - 1 ].get_value( lag );
    S_.sum_S_post_ = B_.NMDA_cond_.get_value( lag );
    B_.NMDA_cond_.set_value( lag, 0.0 );

    // absolute refractory period
    if ( S_.r_ )
    {
      // neuron is absolute refractory
      --S_.r_;
      S_.ode_state_[ State_::V_m ] = P_.V_reset; // clamp potential
    }
    else if ( S_.ode_state_[ State_::V_m ] >= P_.V_th )
    {
      // neuron is not absolute refractory
      S_.r_ = V_.RefractoryCounts;
      S_.ode_state_[ State_::V_m ] = P_.V_reset;

      S_.ode_state_[ State_::X_pre ] += 1;

      // log spike with ArchivingNode
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // send NMDA update
    s_vals[ lag ] = S_.ode_state_[ State_::S_pre ];

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }

  DelayedRateConnectionEvent drce;
  drce.set_coeffarray( s_vals );
  kernel().event_delivery_manager.send_secondary( *this, drce );
}

// Do not move this function as inline to h-file. It depends on
// universal_data_logger_impl.h being included here.
void
nest::iaf_wang_2002::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
nest::iaf_wang_2002::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( e.get_rport() < NMDA );

  const double steps = e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() );

  const auto rport = e.get_rport();
  B_.spikes_[ rport - 1 ].add_value( steps, e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_wang_2002::handle( DelayedRateConnectionEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( e.get_rport() == NMDA );

  const double weight = e.get_weight();
  long delay = e.get_delay_steps();

  for ( auto it = e.begin(); it != e.end(); ++delay )
  {
    B_.NMDA_cond_.add_value( delay, weight * e.get_coeffvalue( it ) );
  }

}

void
nest::iaf_wang_2002::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

#endif // HAVE_GSL
