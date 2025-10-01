/*
 *  iaf_bw_2001_exact.cpp
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

#include "iaf_bw_2001_exact.h"

#ifdef HAVE_GSL

// Includes from libnestutil:
#include "dict_util.h"
#include "dictdatum.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_impl.h"
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
nest::RecordablesMap< nest::iaf_bw_2001_exact > nest::iaf_bw_2001_exact::recordablesMap_;

namespace nest
{
void
register_iaf_bw_2001_exact( const std::string& name )
{
  register_node_model< iaf_bw_2001_exact >( name );
}
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_bw_2001_exact >::create()
{
  // add state variables to recordables map
  insert_( names::V_m, &iaf_bw_2001_exact::get_ode_state_elem_< iaf_bw_2001_exact::State_::V_m > );
  insert_( names::s_AMPA, &iaf_bw_2001_exact::get_ode_state_elem_< iaf_bw_2001_exact::State_::s_AMPA > );
  insert_( names::s_GABA, &iaf_bw_2001_exact::get_ode_state_elem_< iaf_bw_2001_exact::State_::s_GABA > );
  insert_( names::s_NMDA, &iaf_bw_2001_exact::get_s_NMDA_ );
  insert_( names::I_NMDA, &iaf_bw_2001_exact::get_I_NMDA_ );
  insert_( names::I_AMPA, &iaf_bw_2001_exact::get_I_AMPA_ );
  insert_( names::I_GABA, &iaf_bw_2001_exact::get_I_GABA_ );
}
}
/* ---------------------------------------------------------------------------
 * Default constructors defining default parameters and state
 * --------------------------------------------------------------------------- */

nest::iaf_bw_2001_exact::Parameters_::Parameters_()
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

nest::iaf_bw_2001_exact::State_::State_( const Parameters_& p )
  : state_vec_size( 0 )
  , ode_state_( nullptr )
  , num_ports_( SynapseTypes::GABA ) // only AMPA/GABA for now, add NMDA later
  , r_( 0 )
{
  ode_state_ = new double[ s_NMDA_base ];
  assert( ode_state_ );

  ode_state_[ V_m ] = p.E_L; // initialize to reversal potential
  ode_state_[ s_AMPA ] = 0.0;
  ode_state_[ s_GABA ] = 0.0;

  state_vec_size = s_NMDA_base;
}

nest::iaf_bw_2001_exact::State_::State_( const State_& s )
  : state_vec_size( s.state_vec_size )
  , ode_state_( nullptr )
  , num_ports_( s.num_ports_ )
  , r_( s.r_ )
{
  assert( s.num_ports_ == SynapseTypes::GABA );
  assert( state_vec_size == s_NMDA_base );

  ode_state_ = new double[ s_NMDA_base ];
  assert( ode_state_ );

  ode_state_[ V_m ] = s.ode_state_[ V_m ];
  ode_state_[ s_AMPA ] = s.ode_state_[ s_AMPA ];
  ode_state_[ s_GABA ] = s.ode_state_[ s_GABA ];
}

nest::iaf_bw_2001_exact::Buffers_::Buffers_( iaf_bw_2001_exact& n )
  : logger_( n )
  , spikes_()
  , weights_()
  , s_( nullptr )
  , c_( nullptr )
  , e_( nullptr )
  , step_( Time::get_resolution().get_ms() )
  , integration_step_( step_ )
{
  // Initialization of the remaining members is deferred to init_buffers_().
}

nest::iaf_bw_2001_exact::Buffers_::Buffers_( const Buffers_&, iaf_bw_2001_exact& n )
  : logger_( n )
  , spikes_()
  , weights_()
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
nest::iaf_bw_2001_exact::Parameters_::get( DictionaryDatum& d ) const
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
nest::iaf_bw_2001_exact::Parameters_::set( const DictionaryDatum& d, Node* node )
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
nest::iaf_bw_2001_exact::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, ode_state_[ V_m ] ); // Membrane potential
  def< double >( d, names::s_AMPA, ode_state_[ s_AMPA ] );
  def< double >( d, names::s_GABA, ode_state_[ s_GABA ] );
}

void
nest::iaf_bw_2001_exact::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
  updateValueParam< double >( d, names::V_m, ode_state_[ V_m ], node );
  updateValueParam< double >( d, names::s_AMPA, ode_state_[ s_AMPA ], node );
  updateValueParam< double >( d, names::s_GABA, ode_state_[ s_GABA ], node );
}

/* ---------------------------------------------------------------------------
 * Default constructor for node
 * --------------------------------------------------------------------------- */

nest::iaf_bw_2001_exact::iaf_bw_2001_exact()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

/* ---------------------------------------------------------------------------
 * Copy constructor for node
 * --------------------------------------------------------------------------- */

nest::iaf_bw_2001_exact::iaf_bw_2001_exact( const iaf_bw_2001_exact& n_ )
  : ArchivingNode( n_ )
  , P_( n_.P_ )
  , S_( n_.S_ )
  , B_( n_.B_, *this )
{
}

/* ---------------------------------------------------------------------------
 * Destructor for node
 * --------------------------------------------------------------------------- */

nest::iaf_bw_2001_exact::~iaf_bw_2001_exact()
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

  if ( S_.ode_state_ )
  {
    delete[] S_.ode_state_;
  }
}

/* ---------------------------------------------------------------------------
 * Node initialization functions
 * --------------------------------------------------------------------------- */

void
nest::iaf_bw_2001_exact::init_state_()
{
  assert( S_.state_vec_size == State_::s_NMDA_base );

  double* old_state = S_.ode_state_;
  S_.state_vec_size = State_::s_NMDA_base + 2 * ( S_.num_ports_ - SynapseTypes::GABA );
  S_.ode_state_ = new double[ S_.state_vec_size ];

  assert( S_.ode_state_ );

  S_.ode_state_[ State_::V_m ] = old_state[ State_::V_m ];
  S_.ode_state_[ State_::s_AMPA ] = old_state[ State_::s_AMPA ];
  S_.ode_state_[ State_::s_GABA ] = old_state[ State_::s_GABA ];

  for ( size_t i = State_::s_NMDA_base; i < S_.state_vec_size; ++i )
  {
    S_.ode_state_[ i ] = 0.0;
  }

  delete[] old_state;
}

void
nest::iaf_bw_2001_exact::init_buffers_()
{
  B_.spikes_.resize( S_.num_ports_ );

  for ( auto& sb : B_.spikes_ )
  {
    sb.clear(); // includes resize
  }

  B_.currents_.clear(); // includes resize

  B_.weights_.resize( S_.num_ports_ - SynapseTypes::GABA + 1, 0.0 );

  B_.logger_.reset(); // includes resize
  ArchivingNode::clear_history();

  if ( B_.s_ == nullptr )
  {
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, S_.state_vec_size );
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
    B_.e_ = gsl_odeiv_evolve_alloc( S_.state_vec_size );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = iaf_bw_2001_exact_dynamics;
  B_.sys_.jacobian = nullptr;
  B_.sys_.dimension = S_.state_vec_size;
  B_.sys_.params = reinterpret_cast< void* >( this );
  B_.step_ = Time::get_resolution().get_ms();
  B_.integration_step_ = Time::get_resolution().get_ms();

  B_.I_stim_ = 0.0;
}

void
nest::iaf_bw_2001_exact::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.RefractoryCounts = Time( Time::ms( P_.t_ref ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts >= 0 );
}

/* ---------------------------------------------------------------------------
 * Update and spike handling functions
 * --------------------------------------------------------------------------- */

extern "C" inline int
nest::iaf_bw_2001_exact_dynamics( double, const double ode_state[], double f[], void* pnode )
{
  // a shorthand
  typedef nest::iaf_bw_2001_exact::State_ State_;

  // get access to node so we can almost work as in a member function
  assert( pnode );
  nest::iaf_bw_2001_exact& node = *( reinterpret_cast< nest::iaf_bw_2001_exact* >( pnode ) );

  // ode_state[] here is---and must be---the state vector supplied by the integrator,
  // not the state vector in the node, node.S_.ode_state[].

  node.S_.I_AMPA_ = ( ode_state[ State_::V_m ] - node.P_.E_ex ) * ode_state[ State_::s_AMPA ];
  node.S_.I_GABA_ = ( ode_state[ State_::V_m ] - node.P_.E_in ) * ode_state[ State_::s_GABA ];

  // The sum of s_NMDA
  node.S_.s_NMDA_sum = 0;
  for ( size_t i = State_::s_NMDA_base + 1, w_idx = 0; i < node.S_.state_vec_size; i += 2, ++w_idx )
  {
    node.S_.s_NMDA_sum += ode_state[ i ] * node.B_.weights_.at( w_idx );
  }

  node.S_.I_NMDA_ = ( ode_state[ State_::V_m ] - node.P_.E_ex )
    / ( 1 + node.P_.conc_Mg2 * std::exp( -0.062 * ode_state[ State_::V_m ] ) / 3.57 ) * node.S_.s_NMDA_sum;

  const double I_syn = node.S_.I_AMPA_ + node.S_.I_GABA_ + node.S_.I_NMDA_;

  f[ State_::V_m ] =
    ( -node.P_.g_L * ( ode_state[ State_::V_m ] - node.P_.E_L ) - I_syn + node.B_.I_stim_ ) / node.P_.C_m;

  f[ State_::s_AMPA ] = -ode_state[ State_::s_AMPA ] / node.P_.tau_AMPA;
  f[ State_::s_GABA ] = -ode_state[ State_::s_GABA ] / node.P_.tau_GABA;

  for ( size_t i = State_::s_NMDA_base; i < node.S_.state_vec_size; i += 2 )
  {
    f[ i + 1 ] =
      -ode_state[ i + 1 ] / node.P_.tau_decay_NMDA + node.P_.alpha * ode_state[ i ] * ( 1 - ode_state[ i + 1 ] );
    f[ i ] = -ode_state[ i ] / node.P_.tau_rise_NMDA;
  }

  return GSL_SUCCESS;
}

void
nest::iaf_bw_2001_exact::update( Time const& origin, const long from, const long to )
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
        &B_.sys_,              // system of ODE
        &t,                    // from t
        B_.step_,              // to t <= step
        &B_.integration_step_, // integration step size
        S_.ode_state_ );       // neuronal state

      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }
    }

    // add incoming spikes
    S_.ode_state_[ State_::s_AMPA ] += B_.spikes_[ AMPA - 1 ].get_value( lag );
    S_.ode_state_[ State_::s_GABA ] += B_.spikes_[ GABA - 1 ].get_value( lag );

    for ( size_t i = NMDA - 1; i < B_.spikes_.size(); ++i )
    // i starts at 2, runs through all NMDA spikes
    {
      const size_t si = i - ( NMDA - 1 ); // index which starts at 0

      assert( si >= 0 );
      assert( State_::s_NMDA_base + si * 2 <= S_.state_vec_size );

      S_.ode_state_[ State_::s_NMDA_base + si * 2 ] += B_.spikes_.at( i ).get_value( lag );
    }

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

      // log spike with ArchivingNode
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    // voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

// Do not move this function as inline to h-file. It depends on
// universal_data_logger_impl.h being included here.
void
nest::iaf_bw_2001_exact::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
nest::iaf_bw_2001_exact::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( e.get_rport() <= B_.spikes_.size() );

  const double steps = e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() );
  const auto rport = e.get_rport();

  if ( rport < NMDA )
  {
    B_.spikes_[ rport - 1 ].add_value( steps, e.get_weight() * e.get_multiplicity() );
  }
  else
  // we need to scale each individual S_j variable by its weight,
  // so we store them
  {
    B_.spikes_[ rport - 1 ].add_value( steps, e.get_multiplicity() );
    // since we scale entire S_j variable by the weight it also affects previous spikes.
    // we therefore require them to be constant.
    const size_t w_idx = rport - NMDA;
    if ( B_.weights_[ w_idx ] == 0 )
    {
      B_.weights_[ w_idx ] = e.get_weight();
    }
    else if ( B_.weights_[ w_idx ] != e.get_weight() )
    {
      throw KernelException( "iaf_bw_2001_exact requires constant weights." );
    }
  }
}

void
nest::iaf_bw_2001_exact::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

#endif // HAVE_GSL
