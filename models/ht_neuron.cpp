/*
 *  ht_neuron.cpp
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

#include "ht_neuron.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath>

// Includes from libnestutil:
#include "dict_util.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

namespace nest
{

RecordablesMap< ht_neuron > ht_neuron::recordablesMap_;

template <>
void
RecordablesMap< ht_neuron >::create()
{
  insert_( names::V_m, &ht_neuron::get_y_elem_< ht_neuron::State_::V_M > );
  insert_( names::theta, &ht_neuron::get_y_elem_< ht_neuron::State_::THETA > );
  insert_( names::g_AMPA, &ht_neuron::get_y_elem_< ht_neuron::State_::G_AMPA > );
  insert_( names::g_NMDA, &ht_neuron::get_g_NMDA_ );
  insert_( names::g_GABA_A, &ht_neuron::get_y_elem_< ht_neuron::State_::G_GABA_A > );
  insert_( names::g_GABA_B, &ht_neuron::get_y_elem_< ht_neuron::State_::G_GABA_B > );
  insert_( names::I_NaP, &ht_neuron::get_I_NaP_ );
  insert_( names::I_KNa, &ht_neuron::get_I_KNa_ );
  insert_( names::I_T, &ht_neuron::get_I_T_ );
  insert_( names::I_h, &ht_neuron::get_I_h_ );
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" inline int
ht_neuron_dynamics( double, const double y[], double f[], void* pnode )
{
  // shorthand
  typedef nest::ht_neuron::State_ S;

  // get access to node so we can almost work as in a member class
  assert( pnode );
  nest::ht_neuron& node = *( reinterpret_cast< nest::ht_neuron* >( pnode ) );

  // easier access to membrane potential, clamp if requested
  const double& V = node.P_.voltage_clamp ? node.V_.V_clamp_ : y[ S::V_M ];

  /*
   * NMDA conductance
   *
   * We need to take care to handle instantaneous blocking correctly.
   * If the unblock-variables m_{fast,slow}_NMDA are greater than the
   * equilibrium value m_eq_NMDA for the present membrane potential, we cannot
   * change m_NMDA_{fast,slow} values in State_[], since the ODE Solver may
   * call this function multiple times and in arbitrary temporal order. We thus
   * need to use local variables for the values at the current time, and check
   * the state variables once the ODE solver has completed the time step.
   */
  const double m_eq_NMDA = node.m_eq_NMDA_( V );
  const double m_fast_NMDA = std::min( m_eq_NMDA, y[ S::m_fast_NMDA ] );
  const double m_slow_NMDA = std::min( m_eq_NMDA, y[ S::m_slow_NMDA ] );
  const double m_NMDA = node.m_NMDA_( V, m_eq_NMDA, m_fast_NMDA, m_slow_NMDA );

  // Calculate sum of all synaptic channels.
  // Sign convention: For each current, write I = - g * ( V - E )
  //    then dV/dt ~ Sum(I)
  const double I_syn = -y[ S::G_AMPA ] * ( V - node.P_.E_rev_AMPA )
    - y[ S::G_NMDA_TIMECOURSE ] * m_NMDA * ( V - node.P_.E_rev_NMDA ) - y[ S::G_GABA_A ] * ( V - node.P_.E_rev_GABA_A )
    - y[ S::G_GABA_B ] * ( V - node.P_.E_rev_GABA_B );

  // The post-spike K-current, only while refractory
  const double I_spike = node.S_.ref_steps_ > 0 ? -( V - node.P_.E_K ) / node.P_.tau_spike : 0.0;

  // leak currents
  const double I_Na = -node.P_.g_NaL * ( V - node.P_.E_Na );
  const double I_K = -node.P_.g_KL * ( V - node.P_.E_K );

  // intrinsic currents
  // I_Na(p), m_inf^3 according to Compte et al, J Neurophysiol 2003 89:2707
  const double INaP_thresh = -55.7;
  const double INaP_slope = 7.7;
  const double m_inf_NaP = 1.0 / ( 1.0 + std::exp( -( V - INaP_thresh ) / INaP_slope ) );
  node.S_.I_NaP_ = -node.P_.g_peak_NaP * std::pow( m_inf_NaP, 3.0 ) * ( V - node.P_.E_rev_NaP );

  // I_DK
  const double d_half = 0.25;
  const double m_inf_KNa = 1.0 / ( 1.0 + std::pow( d_half / y[ S::D_IKNa ], 3.5 ) );
  node.S_.I_KNa_ = -node.P_.g_peak_KNa * m_inf_KNa * ( V - node.P_.E_rev_KNa );

  // I_T
  node.S_.I_T_ = -node.P_.g_peak_T * y[ S::m_IT ] * y[ S::m_IT ] * y[ S::h_IT ] * ( V - node.P_.E_rev_T );

  // I_h
  node.S_.I_h_ = -node.P_.g_peak_h * y[ S::m_Ih ] * ( V - node.P_.E_rev_h );

  // delta V
  f[ S::V_M ] = ( I_Na + I_K + I_syn + node.S_.I_NaP_ + node.S_.I_KNa_ + node.S_.I_T_ + node.S_.I_h_ + node.B_.I_stim_ )
      / node.P_.tau_m
    + I_spike;

  // delta theta
  f[ S::THETA ] = -( y[ S::THETA ] - node.P_.theta_eq ) / node.P_.tau_theta;

  // Synaptic channels

  // AMPA
  f[ S::DG_AMPA ] = -y[ S::DG_AMPA ] / node.P_.tau_rise_AMPA;
  f[ S::G_AMPA ] = y[ S::DG_AMPA ] - y[ S::G_AMPA ] / node.P_.tau_decay_AMPA;

  // NMDA
  f[ S::DG_NMDA_TIMECOURSE ] = -y[ S::DG_NMDA_TIMECOURSE ] / node.P_.tau_rise_NMDA;
  f[ S::G_NMDA_TIMECOURSE ] = y[ S::DG_NMDA_TIMECOURSE ] - y[ S::G_NMDA_TIMECOURSE ] / node.P_.tau_decay_NMDA;
  f[ S::m_fast_NMDA ] = ( m_eq_NMDA - m_fast_NMDA ) / node.P_.tau_Mg_fast_NMDA;
  f[ S::m_slow_NMDA ] = ( m_eq_NMDA - m_slow_NMDA ) / node.P_.tau_Mg_slow_NMDA;

  // GABA_A
  f[ S::DG_GABA_A ] = -y[ S::DG_GABA_A ] / node.P_.tau_rise_GABA_A;
  f[ S::G_GABA_A ] = y[ S::DG_GABA_A ] - y[ S::G_GABA_A ] / node.P_.tau_decay_GABA_A;

  // GABA_B
  f[ S::DG_GABA_B ] = -y[ S::DG_GABA_B ] / node.P_.tau_rise_GABA_B;
  f[ S::G_GABA_B ] = y[ S::DG_GABA_B ] - y[ S::G_GABA_B ] / node.P_.tau_decay_GABA_B;

  // I_KNa
  f[ S::D_IKNa ] = ( node.D_eq_KNa_( V ) - y[ S::D_IKNa ] ) / node.P_.tau_D_KNa;

  // I_T
  const double tau_m_T = 0.22 / ( std::exp( -( V + 132.0 ) / 16.7 ) + std::exp( ( V + 16.8 ) / 18.2 ) ) + 0.13;
  const double tau_h_T =
    8.2 + ( 56.6 + 0.27 * std::exp( ( V + 115.2 ) / 5.0 ) ) / ( 1.0 + std::exp( ( V + 86.0 ) / 3.2 ) );
  f[ S::m_IT ] = ( node.m_eq_T_( V ) - y[ S::m_IT ] ) / tau_m_T;
  f[ S::h_IT ] = ( node.h_eq_T_( V ) - y[ S::h_IT ] ) / tau_h_T;

  // I_h
  const double tau_m_h = 1.0 / ( std::exp( -14.59 - 0.086 * V ) + std::exp( -1.87 + 0.0701 * V ) );
  f[ S::m_Ih ] = ( node.m_eq_h_( V ) - y[ S::m_Ih ] ) / tau_m_h;

  return GSL_SUCCESS;
}

inline double
nest::ht_neuron::m_eq_h_( double V ) const
{
  const double I_h_Vthreshold = -75.0;
  return 1.0 / ( 1.0 + std::exp( ( V - I_h_Vthreshold ) / 5.5 ) );
}

inline double
nest::ht_neuron::h_eq_T_( double V ) const
{
  return 1.0 / ( 1.0 + std::exp( ( V + 83.0 ) / 4 ) );
}

inline double
nest::ht_neuron::m_eq_T_( double V ) const
{
  return 1.0 / ( 1.0 + std::exp( -( V + 59.0 ) / 6.2 ) );
}

inline double
nest::ht_neuron::D_eq_KNa_( double V ) const
{
  const double D_influx_peak = 0.025;
  const double D_thresh = -10.0;
  const double D_slope = 5.0;
  const double D_eq = 0.001;

  const double D_influx = D_influx_peak / ( 1.0 + std::exp( -( V - D_thresh ) / D_slope ) );
  return P_.tau_D_KNa * D_influx + D_eq;
}

inline double
nest::ht_neuron::m_eq_NMDA_( double V ) const
{
  return 1.0 / ( 1.0 + std::exp( -P_.S_act_NMDA * ( V - P_.V_act_NMDA ) ) );
}

inline double
nest::ht_neuron::m_NMDA_( double V, double m_eq, double m_fast, double m_slow ) const
{
  const double A1 = 0.51 - 0.0028 * V;
  const double A2 = 1 - A1;
  return P_.instant_unblock_NMDA ? m_eq : A1 * m_fast + A2 * m_slow;
}

inline double
nest::ht_neuron::get_g_NMDA_() const
{
  return S_.y_[ State_::G_NMDA_TIMECOURSE ] * m_NMDA_( S_.y_[ State_::V_M ],
                                                m_eq_NMDA_( S_.y_[ State_::V_M ] ),
                                                S_.y_[ State_::m_fast_NMDA ],
                                                S_.y_[ State_::m_slow_NMDA ] );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::ht_neuron::Parameters_::Parameters_()
  : E_Na( 30.0 ) // mV
  , E_K( -90.0 ) // mV
  , g_NaL( 0.2 )
  , g_KL( 1.0 )
  , tau_m( 16.0 )     // ms
  , theta_eq( -51.0 ) // mV
  , tau_theta( 2.0 )  // ms
  , tau_spike( 1.75 ) // ms
  , t_ref( 2.0 )      // ms
  , g_peak_AMPA( 0.1 )
  , tau_rise_AMPA( 0.5 )  // ms
  , tau_decay_AMPA( 2.4 ) // ms
  , E_rev_AMPA( 0.0 )     // mV
  , g_peak_NMDA( 0.075 )
  , tau_rise_NMDA( 4.0 )     // ms
  , tau_decay_NMDA( 40.0 )   // ms
  , E_rev_NMDA( 0.0 )        // mV
  , V_act_NMDA( -25.57 )     // mV
  , S_act_NMDA( 0.081 )      // mV
  , tau_Mg_slow_NMDA( 22.7 ) // ms
  , tau_Mg_fast_NMDA( 0.68 ) // ms
  , instant_unblock_NMDA( false )
  , g_peak_GABA_A( 0.33 )
  , tau_rise_GABA_A( 1.0 )  // ms
  , tau_decay_GABA_A( 7.0 ) // ms
  , E_rev_GABA_A( -70.0 )   // mV
  , g_peak_GABA_B( 0.0132 )
  , tau_rise_GABA_B( 60.0 )   // ms
  , tau_decay_GABA_B( 200.0 ) // ms
  , E_rev_GABA_B( -90.0 )     // mV
  , g_peak_NaP( 1.0 )
  , E_rev_NaP( 30.0 ) // mV
  , g_peak_KNa( 1.0 )
  , E_rev_KNa( -90.0 )  // mV
  , tau_D_KNa( 1250.0 ) // ms
  , g_peak_T( 1.0 )
  , E_rev_T( 0.0 ) // mV
  , g_peak_h( 1.0 )
  , E_rev_h( -40.0 ) // mV
  , voltage_clamp( false )
{
}

nest::ht_neuron::State_::State_( const ht_neuron& node, const Parameters_& p )
  : ref_steps_( 0 )
  , I_NaP_( 0.0 )
  , I_KNa_( 0.0 )
  , I_T_( 0.0 )
  , I_h_( 0.0 )
{
  // initialize with equilibrium values
  y_[ V_M ] = ( p.g_NaL * p.E_Na + p.g_KL * p.E_K ) / ( p.g_NaL + p.g_KL );
  y_[ THETA ] = p.theta_eq;

  for ( size_t i = 2; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = 0.0;
  }

  y_[ m_fast_NMDA ] = node.m_eq_NMDA_( y_[ V_M ] );
  y_[ m_slow_NMDA ] = node.m_eq_NMDA_( y_[ V_M ] );
  y_[ m_Ih ] = node.m_eq_h_( y_[ V_M ] );
  y_[ D_IKNa ] = node.D_eq_KNa_( y_[ V_M ] );
  y_[ m_IT ] = node.m_eq_T_( y_[ V_M ] );
  y_[ h_IT ] = node.h_eq_T_( y_[ V_M ] );
}

nest::ht_neuron::State_::State_( const State_& s )
  : ref_steps_( s.ref_steps_ )
  , I_NaP_( s.I_NaP_ )
  , I_KNa_( s.I_KNa_ )
  , I_T_( s.I_T_ )
  , I_h_( s.I_h_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }
}

nest::ht_neuron::State_& nest::ht_neuron::State_::operator=( const State_& s )
{
  if ( this == &s )
  {
    return *this;
  }

  ref_steps_ = s.ref_steps_;
  I_NaP_ = s.I_NaP_;
  I_KNa_ = s.I_KNa_;
  I_T_ = s.I_T_;
  I_h_ = s.I_h_;

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
  {
    y_[ i ] = s.y_[ i ];
  }

  return *this;
}

nest::ht_neuron::State_::~State_()
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::ht_neuron::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_Na, E_Na );
  def< double >( d, names::E_K, E_K );
  def< double >( d, names::g_NaL, g_NaL );
  def< double >( d, names::g_KL, g_KL );
  def< double >( d, names::tau_m, tau_m );
  def< double >( d, names::theta_eq, theta_eq );
  def< double >( d, names::tau_theta, tau_theta );
  def< double >( d, names::t_ref, t_ref );
  def< double >( d, names::tau_spike, tau_spike );
  def< double >( d, names::g_peak_AMPA, g_peak_AMPA );
  def< double >( d, names::tau_rise_AMPA, tau_rise_AMPA );
  def< double >( d, names::tau_decay_AMPA, tau_decay_AMPA );
  def< double >( d, names::E_rev_AMPA, E_rev_AMPA );
  def< double >( d, names::g_peak_NMDA, g_peak_NMDA );
  def< double >( d, names::tau_rise_NMDA, tau_rise_NMDA );
  def< double >( d, names::tau_decay_NMDA, tau_decay_NMDA );
  def< double >( d, names::E_rev_NMDA, E_rev_NMDA );
  def< double >( d, names::V_act_NMDA, V_act_NMDA );
  def< double >( d, names::S_act_NMDA, S_act_NMDA );
  def< double >( d, names::tau_Mg_slow_NMDA, tau_Mg_slow_NMDA );
  def< double >( d, names::tau_Mg_fast_NMDA, tau_Mg_fast_NMDA );
  def< bool >( d, names::instant_unblock_NMDA, instant_unblock_NMDA );
  def< double >( d, names::g_peak_GABA_A, g_peak_GABA_A );
  def< double >( d, names::tau_rise_GABA_A, tau_rise_GABA_A );
  def< double >( d, names::tau_decay_GABA_A, tau_decay_GABA_A );
  def< double >( d, names::E_rev_GABA_A, E_rev_GABA_A );
  def< double >( d, names::g_peak_GABA_B, g_peak_GABA_B );
  def< double >( d, names::tau_rise_GABA_B, tau_rise_GABA_B );
  def< double >( d, names::tau_decay_GABA_B, tau_decay_GABA_B );
  def< double >( d, names::E_rev_GABA_B, E_rev_GABA_B );
  def< double >( d, names::g_peak_NaP, g_peak_NaP );
  def< double >( d, names::E_rev_NaP, E_rev_NaP );
  def< double >( d, names::g_peak_KNa, g_peak_KNa );
  def< double >( d, names::E_rev_KNa, E_rev_KNa );
  def< double >( d, names::tau_D_KNa, tau_D_KNa );
  def< double >( d, names::g_peak_T, g_peak_T );
  def< double >( d, names::E_rev_T, E_rev_T );
  def< double >( d, names::g_peak_h, g_peak_h );
  def< double >( d, names::E_rev_h, E_rev_h );
  def< bool >( d, names::voltage_clamp, voltage_clamp );
}

void
nest::ht_neuron::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::E_Na, E_Na, node );
  updateValueParam< double >( d, names::E_K, E_K, node );
  updateValueParam< double >( d, names::g_NaL, g_NaL, node );
  updateValueParam< double >( d, names::g_KL, g_KL, node );
  updateValueParam< double >( d, names::tau_m, tau_m, node );
  updateValueParam< double >( d, names::theta_eq, theta_eq, node );
  updateValueParam< double >( d, names::tau_theta, tau_theta, node );
  updateValueParam< double >( d, names::tau_spike, tau_spike, node );
  updateValueParam< double >( d, names::t_ref, t_ref, node );
  updateValueParam< double >( d, names::g_peak_AMPA, g_peak_AMPA, node );
  updateValueParam< double >( d, names::tau_rise_AMPA, tau_rise_AMPA, node );
  updateValueParam< double >( d, names::tau_decay_AMPA, tau_decay_AMPA, node );
  updateValueParam< double >( d, names::E_rev_AMPA, E_rev_AMPA, node );
  updateValueParam< double >( d, names::g_peak_NMDA, g_peak_NMDA, node );
  updateValueParam< double >( d, names::tau_rise_NMDA, tau_rise_NMDA, node );
  updateValueParam< double >( d, names::tau_decay_NMDA, tau_decay_NMDA, node );
  updateValueParam< double >( d, names::E_rev_NMDA, E_rev_NMDA, node );
  updateValueParam< double >( d, names::V_act_NMDA, V_act_NMDA, node );
  updateValueParam< double >( d, names::S_act_NMDA, S_act_NMDA, node );
  updateValueParam< double >( d, names::tau_Mg_slow_NMDA, tau_Mg_slow_NMDA, node );
  updateValueParam< double >( d, names::tau_Mg_fast_NMDA, tau_Mg_fast_NMDA, node );
  updateValueParam< bool >( d, names::instant_unblock_NMDA, instant_unblock_NMDA, node );
  updateValueParam< double >( d, names::g_peak_GABA_A, g_peak_GABA_A, node );
  updateValueParam< double >( d, names::tau_rise_GABA_A, tau_rise_GABA_A, node );
  updateValueParam< double >( d, names::tau_decay_GABA_A, tau_decay_GABA_A, node );
  updateValueParam< double >( d, names::E_rev_GABA_A, E_rev_GABA_A, node );
  updateValueParam< double >( d, names::g_peak_GABA_B, g_peak_GABA_B, node );
  updateValueParam< double >( d, names::tau_rise_GABA_B, tau_rise_GABA_B, node );
  updateValueParam< double >( d, names::tau_decay_GABA_B, tau_decay_GABA_B, node );
  updateValueParam< double >( d, names::E_rev_GABA_B, E_rev_GABA_B, node );
  updateValueParam< double >( d, names::g_peak_NaP, g_peak_NaP, node );
  updateValueParam< double >( d, names::E_rev_NaP, E_rev_NaP, node );
  updateValueParam< double >( d, names::g_peak_KNa, g_peak_KNa, node );
  updateValueParam< double >( d, names::E_rev_KNa, E_rev_KNa, node );
  updateValueParam< double >( d, names::tau_D_KNa, tau_D_KNa, node );
  updateValueParam< double >( d, names::g_peak_T, g_peak_T, node );
  updateValueParam< double >( d, names::E_rev_T, E_rev_T, node );
  updateValueParam< double >( d, names::g_peak_h, g_peak_h, node );
  updateValueParam< double >( d, names::E_rev_h, E_rev_h, node );
  updateValueParam< bool >( d, names::voltage_clamp, voltage_clamp, node );

  if ( g_peak_AMPA < 0 )
  {
    throw BadParameter( "g_peak_AMPA >= 0 required." );
  }
  if ( g_peak_GABA_A < 0 )
  {
    throw BadParameter( "g_peak_GABA_A >= 0 required." );
  }
  if ( g_peak_GABA_B < 0 )
  {
    throw BadParameter( "g_peak_GABA_B >= 0 required." );
  }
  if ( g_peak_KNa < 0 )
  {
    throw BadParameter( "g_peak_KNa >= 0 required." );
  }
  if ( S_act_NMDA < 0 )
  {
    throw BadParameter( "S_act_NMDA >= 0 required." );
  }
  if ( g_peak_NMDA < 0 )
  {
    throw BadParameter( "g_peak_NMDA >= 0 required." );
  }
  if ( g_peak_T < 0 )
  {
    throw BadParameter( "g_peak_T >= 0 required." );
  }
  if ( g_peak_h < 0 )
  {
    throw BadParameter( "g_peak_h >= 0 required." );
  }
  if ( g_peak_NaP < 0 )
  {
    throw BadParameter( "g_peak_NaP >= 0 required." );
  }
  if ( g_KL < 0 )
  {
    throw BadParameter( "g_KL >= 0 required." );
  }
  if ( g_NaL < 0 )
  {
    throw BadParameter( "g_NaL >= 0 required." );
  }

  if ( t_ref < 0 )
  {
    throw BadParameter( "t_ref >= 0 required." );
  }

  if ( tau_rise_AMPA <= 0 )
  {
    throw BadParameter( "tau_rise_AMPA > 0 required." );
  }
  if ( tau_decay_AMPA <= 0 )
  {
    throw BadParameter( "tau_decay_AMPA > 0 required." );
  }
  if ( tau_rise_GABA_A <= 0 )
  {
    throw BadParameter( "tau_rise_GABA_A > 0 required." );
  }
  if ( tau_decay_GABA_A <= 0 )
  {
    throw BadParameter( "tau_decay_GABA_A > 0 required." );
  }
  if ( tau_rise_GABA_B <= 0 )
  {
    throw BadParameter( "tau_rise_GABA_B > 0 required." );
  }
  if ( tau_decay_GABA_B <= 0 )
  {
    throw BadParameter( "tau_decay_GABA_B > 0 required." );
  }
  if ( tau_rise_NMDA <= 0 )
  {
    throw BadParameter( "tau_rise_NMDA > 0 required." );
  }
  if ( tau_decay_NMDA <= 0 )
  {
    throw BadParameter( "tau_decay_NMDA > 0 required." );
  }
  if ( tau_Mg_fast_NMDA <= 0 )
  {
    throw BadParameter( "tau_Mg_fast_NMDA > 0 required." );
  }
  if ( tau_Mg_slow_NMDA <= 0 )
  {
    throw BadParameter( "tau_Mg_slow_NMDA > 0 required." );
  }
  if ( tau_spike <= 0 )
  {
    throw BadParameter( "tau_spike > 0 required." );
  }
  if ( tau_theta <= 0 )
  {
    throw BadParameter( "tau_theta > 0 required." );
  }
  if ( tau_m <= 0 )
  {
    throw BadParameter( "tau_m > 0 required." );
  }
  if ( tau_D_KNa <= 0 )
  {
    throw BadParameter( "tau_D_KNa > 0 required." );
  }

  if ( tau_rise_AMPA >= tau_decay_AMPA )
  {
    throw BadParameter( "tau_rise_AMPA < tau_decay_AMPA required." );
  }
  if ( tau_rise_GABA_A >= tau_decay_GABA_A )
  {
    throw BadParameter( "tau_rise_GABA_A < tau_decay_GABA_A required." );
  }
  if ( tau_rise_GABA_B >= tau_decay_GABA_B )
  {
    throw BadParameter( "tau_rise_GABA_B < tau_decay_GABA_B required." );
  }
  if ( tau_rise_NMDA >= tau_decay_NMDA )
  {
    throw BadParameter( "tau_rise_NMDA < tau_decay_NMDA required." );
  }
  if ( tau_Mg_fast_NMDA >= tau_Mg_slow_NMDA )
  {
    throw BadParameter( "tau_Mg_fast_NMDA < tau_Mg_slow_NMDA required." );
  }
}

void
nest::ht_neuron::State_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_m, y_[ V_M ] );     // Membrane potential
  def< double >( d, names::theta, y_[ THETA ] ); // Threshold
}

void
nest::ht_neuron::State_::set( const DictionaryDatum& d, const ht_neuron& node, Node* nodeptr )
{
  updateValueParam< double >( d, names::V_m, y_[ V_M ], nodeptr );
  updateValueParam< double >( d, names::theta, y_[ THETA ], nodeptr );

  bool equilibrate = false;
  updateValueParam< bool >( d, names::equilibrate, equilibrate, nodeptr );
  if ( equilibrate )
  {
    y_[ m_fast_NMDA ] = node.m_eq_NMDA_( y_[ V_M ] );
    y_[ m_slow_NMDA ] = node.m_eq_NMDA_( y_[ V_M ] );
    y_[ m_Ih ] = node.m_eq_h_( y_[ V_M ] );
    y_[ State_::D_IKNa ] = node.D_eq_KNa_( y_[ V_M ] );
    y_[ m_IT ] = node.m_eq_T_( y_[ V_M ] );
    y_[ h_IT ] = node.h_eq_T_( y_[ V_M ] );
  }
}

nest::ht_neuron::Buffers_::Buffers_( ht_neuron& n )
  : logger_( n )
  , spike_inputs_( std::vector< RingBuffer >( SUP_SPIKE_RECEPTOR - 1 ) )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
  , step_( Time::get_resolution().get_ms() )
  , integration_step_( step_ )
  , I_stim_( 0.0 )
{
}

nest::ht_neuron::Buffers_::Buffers_( const Buffers_&, ht_neuron& n )
  : logger_( n )
  , spike_inputs_( std::vector< RingBuffer >( SUP_SPIKE_RECEPTOR - 1 ) )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
  , step_( Time::get_resolution().get_ms() )
  , integration_step_( step_ )
  , I_stim_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::ht_neuron::ht_neuron()
  : Archiving_Node()
  , P_()
  , S_( *this, P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::ht_neuron::ht_neuron( const ht_neuron& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::ht_neuron::~ht_neuron()
{
  // GSL structs may not be initialized, so we need to protect destruction.
  if ( B_.e_ )
  {
    gsl_odeiv_evolve_free( B_.e_ );
  }
  if ( B_.c_ )
  {
    gsl_odeiv_control_free( B_.c_ );
  }
  if ( B_.s_ )
  {
    gsl_odeiv_step_free( B_.s_ );
  }
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::ht_neuron::init_state_( const Node& proto )
{
  const ht_neuron& pr = downcast< ht_neuron >( proto );
  S_ = pr.S_;
}

void
nest::ht_neuron::init_buffers_()
{
  // Reset spike buffers.
  for ( std::vector< RingBuffer >::iterator it = B_.spike_inputs_.begin(); it != B_.spike_inputs_.end(); ++it )
  {
    it->clear(); // include resize
  }

  B_.currents_.clear(); // include resize

  B_.logger_.reset();

  Archiving_Node::clear_history();

  B_.step_ = Time::get_resolution().get_ms();
  B_.integration_step_ = B_.step_;

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
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  }
  else
  {
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );
  }

  if ( B_.e_ == 0 )
  {
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  }
  else
  {
    gsl_odeiv_evolve_reset( B_.e_ );
  }

  B_.sys_.function = ht_neuron_dynamics;
  B_.sys_.jacobian = 0;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  B_.I_stim_ = 0.0;
}

double
nest::ht_neuron::get_synapse_constant( double tau_1, double tau_2, double g_peak )
{
  /* The solution to the beta function ODE obtained by the solver is
   *
   *   g(t) = c / ( a - b ) * ( e^(-b t) - e^(-a t) )
   *
   * with a = 1/tau_1, b = 1/tau_2, a != b. The maximum of this function is at
   *
   *   t* = 1/(a-b) ln a/b
   *
   * We want to scale the function so that
   *
   *   max g == g(t*) == g_peak
   *
   * We thus need to set
   *
   *   c = g_peak * ( a - b ) / ( e^(-b t*) - e^(-a t*) )
   *
   * See Rotter & Diesmann, Biol Cybern 81:381 (1999) and Roth and van Rossum,
   * Ch 6, in De Schutter, Computational Modeling Methods for Neuroscientists,
   * MIT Press, 2010.
   */

  const double t_peak = ( tau_2 * tau_1 ) * std::log( tau_2 / tau_1 ) / ( tau_2 - tau_1 );

  const double prefactor = ( 1 / tau_1 ) - ( 1 / tau_2 );

  const double peak_value = ( std::exp( -t_peak / tau_2 ) - std::exp( -t_peak / tau_1 ) );

  return g_peak * prefactor / peak_value;
}

void
nest::ht_neuron::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  // The code below initializes conductance step size for incoming pulses.
  V_.cond_steps_.resize( SUP_SPIKE_RECEPTOR - 1 );

  V_.cond_steps_[ AMPA - 1 ] = get_synapse_constant( P_.tau_rise_AMPA, P_.tau_decay_AMPA, P_.g_peak_AMPA );

  V_.cond_steps_[ NMDA - 1 ] = get_synapse_constant( P_.tau_rise_NMDA, P_.tau_decay_NMDA, P_.g_peak_NMDA );

  V_.cond_steps_[ GABA_A - 1 ] = get_synapse_constant( P_.tau_rise_GABA_A, P_.tau_decay_GABA_A, P_.g_peak_GABA_A );

  V_.cond_steps_[ GABA_B - 1 ] = get_synapse_constant( P_.tau_rise_GABA_B, P_.tau_decay_GABA_B, P_.g_peak_GABA_B );

  V_.PotassiumRefractoryCounts_ = Time( Time::ms( P_.t_ref ) ).get_steps();

  V_.V_clamp_ = S_.y_[ State_::V_M ];
}

void
nest::ht_neuron::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );

  DictionaryDatum receptor_type = new Dictionary();

  ( *receptor_type )[ names::AMPA ] = AMPA;
  ( *receptor_type )[ names::NMDA ] = NMDA;
  ( *receptor_type )[ names::GABA_A ] = GABA_A;
  ( *receptor_type )[ names::GABA_B ] = GABA_B;

  ( *d )[ names::receptor_types ] = receptor_type;
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

void
nest::ht_neuron::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;      // temporary copy in case of errors
  ptmp.set( d, this );        // throws if BadProperty
  State_ stmp = S_;           // temporary copy in case of errors
  stmp.set( d, *this, this ); // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
ht_neuron::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    double tt = 0.0; // it's all relative!

    // adaptive step integration
    while ( tt < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,              // system of ODE
        &tt,                   // from t...
        B_.step_,              // ...to t=t+h
        &B_.integration_step_, // integration window (written on!)
        S_.y_ );               // neuron state

      if ( status != GSL_SUCCESS )
      {
        throw GSLSolverFailure( get_name(), status );
      }

      // Enforce voltage clamp
      if ( P_.voltage_clamp )
      {
        S_.y_[ State_::V_M ] = V_.V_clamp_;
      }

      // Enforce instantaneous blocking of NMDA channels
      const double m_eq_NMDA = m_eq_NMDA_( S_.y_[ State_::V_M ] );
      S_.y_[ State_::m_fast_NMDA ] = std::min( m_eq_NMDA, S_.y_[ State_::m_fast_NMDA ] );
      S_.y_[ State_::m_slow_NMDA ] = std::min( m_eq_NMDA, S_.y_[ State_::m_slow_NMDA ] );

      // A spike is generated if the neuron is not refractory and the membrane
      // potential exceeds the threshold.
      if ( S_.ref_steps_ == 0 and S_.y_[ State_::V_M ] >= S_.y_[ State_::THETA ] )
      {
        // Set V and theta to the sodium reversal potential.
        S_.y_[ State_::V_M ] = P_.E_Na;
        S_.y_[ State_::THETA ] = P_.E_Na;

        // Activate fast re-polarizing potassium current. Add 1 to compensate
        // to subtraction right after while loop.
        S_.ref_steps_ = V_.PotassiumRefractoryCounts_ + 1;

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }

    if ( S_.ref_steps_ > 0 )
    {
      --S_.ref_steps_;
    }

    /* Add arriving spikes.
     *
     * The input variable for the synapse type with buffer index i is
     * at position 2 + 2*i in the state variable vector.
     */
    for ( size_t i = 0; i < B_.spike_inputs_.size(); ++i )
    {
      S_.y_[ 2 + 2 * i ] += V_.cond_steps_[ i ] * B_.spike_inputs_[ i ].get_value( lag );
    }

    // set new input current
    B_.I_stim_ = B_.currents_.get_value( lag );

    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::ht_neuron::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );
  assert( e.get_rport() < static_cast< int >( B_.spike_inputs_.size() ) );

  B_.spike_inputs_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::ht_neuron::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double I = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * I );
}

void
nest::ht_neuron::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
}

#endif // HAVE_GSL
