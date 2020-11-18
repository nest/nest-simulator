/*
 *  glif_psc.cpp
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

#include "glif_psc.h"

// C++ includes:
#include <iostream>
#include <limits>

// Includes from libnestutil:
#include "numerics.h"
#include "propagator_stability.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "name.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "lockptrdatum.h"

using namespace nest;

nest::RecordablesMap< nest::glif_psc > nest::glif_psc::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< nest::glif_psc >::create()
{
  insert_( names::V_m, &nest::glif_psc::get_V_m_ );
  insert_( names::ASCurrents_sum, &nest::glif_psc::get_ASCurrents_sum_ );
  insert_( names::I, &nest::glif_psc::get_I_ );
  insert_( names::I_syn, &nest::glif_psc::get_I_syn_ );
  insert_( names::threshold, &nest::glif_psc::get_threshold_ );
  insert_( names::threshold_spike, &nest::glif_psc::get_threshold_spike_ );
  insert_( names::threshold_voltage, &nest::glif_psc::get_threshold_voltage_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::glif_psc::Parameters_::Parameters_()
  : G_( 9.43 )               // in nS
  , E_L_( -78.85 )           // in mV
  , th_inf_( -51.68 - E_L_ ) // in mv, rel to E_L_, - 51.68 - E_L_, i.e., 27.17
  , C_m_( 58.72 )            // in pF
  , t_ref_( 3.75 )           // in ms
  , V_reset_( 0.0 )          // in mV, rel to E_L_, -78.85 - E_L_
  , th_spike_add_( 0.37 )    // in mV
  , th_spike_decay_( 0.009 ) // in 1/ms
  , voltage_reset_fraction_( 0.20 )
  , voltage_reset_add_( 18.51 )                          // in mV
  , th_voltage_index_( 0.005 )                           // in 1/ms
  , th_voltage_decay_( 0.09 )                            // in 1/ms
  , asc_init_( std::vector< double >( 2, 0.0 ) )         // in pA
  , asc_decay_( std::vector< double >{ 0.003, 0.1 } )    // in 1/ms
  , asc_amps_( std::vector< double >{ -9.18, -198.94 } ) // in pA
  , asc_r_( std::vector< double >( 2, 1.0 ) )
  , tau_syn_( std::vector< double >( 1, 2.0 ) ) // in ms
  , has_connections_( false )
  , has_theta_spike_( false )
  , has_asc_( false )
  , has_theta_voltage_( false )
{
}

nest::glif_psc::State_::State_( const Parameters_& p )
  : U_( 0.0 )                  // in mV
  , threshold_( p.th_inf_ )    // in mV
  , threshold_spike_( 0.0 )    // in mV
  , threshold_voltage_( 0.0 )  // in mV
  , I_( 0.0 )                  // in pA
  , I_syn_( 0.0 )              // in pA
  , ASCurrents_( p.asc_init_ ) // in pA
  , ASCurrents_sum_( 0.0 )     // in pA
  , refractory_steps_( 0 )
{
  for ( std::size_t a = 0; a < p.asc_init_.size(); ++a )
  {
    ASCurrents_sum_ += ASCurrents_[ a ];
  }
  y1_.clear();
  y2_.clear();
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::glif_psc::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, th_inf_ + E_L_ );
  def< double >( d, names::g, G_ );
  def< double >( d, names::E_L, E_L_ );
  def< double >( d, names::C_m, C_m_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::V_reset, V_reset_ + E_L_ );

  def< double >( d, names::th_spike_add, th_spike_add_ );
  def< double >( d, names::th_spike_decay, th_spike_decay_ );
  def< double >( d, names::voltage_reset_fraction, voltage_reset_fraction_ );
  def< double >( d, names::voltage_reset_add, voltage_reset_add_ );

  def< double >( d, names::th_voltage_index, th_voltage_index_ );
  def< double >( d, names::th_voltage_decay, th_voltage_decay_ );

  def< std::vector< double > >( d, names::asc_init, asc_init_ );
  def< std::vector< double > >( d, names::asc_decay, asc_decay_ );
  def< std::vector< double > >( d, names::asc_amps, asc_amps_ );
  def< std::vector< double > >( d, names::asc_r, asc_r_ );
  ArrayDatum tau_syn_ad( tau_syn_ );
  def< ArrayDatum >( d, names::tau_syn, tau_syn_ad );
  def< bool >( d, names::has_connections, has_connections_ );
  def< bool >( d, names::spike_dependent_threshold, has_theta_spike_ );
  def< bool >( d, names::after_spike_currents, has_asc_ );
  def< bool >( d, names::adapting_threshold, has_theta_voltage_ );
}

double
nest::glif_psc::Parameters_::set( const DictionaryDatum& d )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValue< double >( d, names::E_L, E_L_ );
  const double delta_EL = E_L_ - ELold;

  if ( updateValue< double >( d, names::V_reset, V_reset_ ) )
  {
    V_reset_ -= E_L_;
  }
  else
  {
    V_reset_ -= delta_EL;
  }

  if ( updateValue< double >( d, names::V_th, th_inf_ ) )
  {
    th_inf_ -= E_L_;
  }
  else
  {
    th_inf_ -= delta_EL;
  }

  updateValue< double >( d, names::g, G_ );
  updateValue< double >( d, names::C_m, C_m_ );
  updateValue< double >( d, names::t_ref, t_ref_ );

  updateValue< double >( d, names::th_spike_add, th_spike_add_ );
  updateValue< double >( d, names::th_spike_decay, th_spike_decay_ );
  updateValue< double >( d, names::voltage_reset_fraction, voltage_reset_fraction_ );
  updateValue< double >( d, names::voltage_reset_add, voltage_reset_add_ );

  updateValue< double >( d, names::th_voltage_index, th_voltage_index_ );
  updateValue< double >( d, names::th_voltage_decay, th_voltage_decay_ );

  updateValue< std::vector< double > >( d, names::asc_init, asc_init_ );
  updateValue< std::vector< double > >( d, names::asc_decay, asc_decay_ );
  updateValue< std::vector< double > >( d, names::asc_amps, asc_amps_ );
  updateValue< std::vector< double > >( d, names::asc_r, asc_r_ );

  // set model mechanisms
  updateValue< bool >( d, names::spike_dependent_threshold, has_theta_spike_ );
  updateValue< bool >( d, names::after_spike_currents, has_asc_ );
  updateValue< bool >( d, names::adapting_threshold, has_theta_voltage_ );

  // check model mechanisms parameter
  if ( not( ( not has_theta_spike_ and not has_asc_ and not has_theta_voltage_ ) or // glif1
         ( has_theta_spike_ and not has_asc_ and not has_theta_voltage_ ) or        // glif2
         ( not has_theta_spike_ and has_asc_ and not has_theta_voltage_ ) or        // glif3
         ( has_theta_spike_ and has_asc_ and not has_theta_voltage_ ) or            // glif4
         ( has_theta_spike_ and has_asc_ and has_theta_voltage_ )                   // glif5
         ) )
  {
    throw BadProperty(
      "Incorrect model mechanism combination setting."
      "See documentation for setting of model mechanism parameters:"
      "spike_dependent_threshold, after_spike_currents, adapting_threshold." );
  }

  // check after ASC parameters' sizes and values
  if ( has_asc_ )
  {
    // check size
    const size_t asc_size = asc_decay_.size();
    if ( not(
           ( asc_init_.size() == asc_size ) and ( asc_amps_.size() == asc_size ) and ( asc_r_.size() == asc_size ) ) )
    {
      throw BadProperty(
        "All after spike current parameters (i.e., asc_init, k, asc_amps, r) "
        "must have the same size." );
    }

    // check values
    for ( std::size_t a = 0; a < asc_decay_.size(); ++a )
    {
      if ( asc_decay_[ a ] <= 0.0 )
      {
        throw BadProperty( "After-spike current time constant must be strictly positive." );
      }

      if ( asc_r_[ a ] < 0.0 or asc_r_[ a ] > 1.0 )
      {
        throw BadProperty( "After spike current fraction following spike coefficients r must be within [0.0, 1.0]." );
      }
    }
  }

  if ( V_reset_ >= th_inf_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }

  if ( C_m_ <= 0.0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }

  if ( G_ <= 0.0 )
  {
    throw BadProperty( "Membrane conductance must be strictly positive." );
  }

  if ( t_ref_ <= 0.0 )
  {
    throw BadProperty( "Refractory time constant must be strictly positive." );
  }

  if ( has_theta_voltage_ )
  {
    if ( th_voltage_decay_ <= 0.0 )
    {
      throw BadProperty( "Voltage-induced threshold time constant must be strictly positive." );
    }
  }

  // check spike component parameters
  if ( has_theta_spike_ )
  {
    if ( th_spike_decay_ <= 0.0 )
    {
      throw BadProperty( "Spike induced threshold time constant must be strictly positive." );
    }

    if ( voltage_reset_fraction_ < 0.0 or voltage_reset_fraction_ > 1.0 )
    {
      throw BadProperty( "Voltage fraction coefficient following spike must be within [0.0, 1.0]." );
    }
  }

  const size_t old_n_receptors = this->n_receptors_();
  if ( updateValue< std::vector< double > >( d, names::tau_syn, tau_syn_ ) )
  {
    if ( this->n_receptors_() != old_n_receptors && has_connections_ == true )
    {
      throw BadProperty(
        "The neuron has connections, therefore the number of ports cannot be "
        "reduced." );
    }
    for ( size_t i = 0; i < tau_syn_.size(); ++i )
    {
      if ( tau_syn_[ i ] <= 0 )
      {
        throw BadProperty( "All synaptic time constants must be strictly positive." );
      }
    }
  }

  return delta_EL;
}

void
nest::glif_psc::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, U_ + p.E_L_ );
  def< std::vector< double > >( d, names::ASCurrents, ASCurrents_ );
  def< double >( d, names::threshold_spike, threshold_spike_ );
  def< double >( d, names::threshold_voltage, threshold_voltage_ );
}

void
nest::glif_psc::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, U_ ) )
  {
    U_ -= p.E_L_;
  }
  else
  {
    U_ -= delta_EL;
  }

  bool asc_flag = updateValue< std::vector< double > >( d, names::ASCurrents, ASCurrents_ );
  if ( asc_flag and not p.has_asc_ )
  {
    throw BadProperty( "After spike currents are not supported or settable in the current model mechanisms." );
  }

  const size_t asc_size = p.asc_decay_.size();
  if ( asc_flag )
  {
    if ( ASCurrents_.size() != asc_size )
    {
      throw BadProperty( "After spike current values must have have the same size (" + std::to_string( asc_size )
        + ") of its parameters (i.e., asc_init, k, asc_amps, r)." );
    }
  }

  if ( updateValue< double >( d, names::threshold_spike, threshold_spike_ ) and not p.has_theta_spike_ )
  {
    throw BadProperty( "Threshold spike component is not supported or settable in the current model mechanisms." );
  }

  if ( updateValue< double >( d, names::threshold_voltage, threshold_voltage_ ) and not p.has_theta_voltage_ )
  {
    throw BadProperty( "Threshold voltage component is not supported or settable in the current model mechanisms." );
  }
}

nest::glif_psc::Buffers_::Buffers_( glif_psc& n )
  : logger_( n )
{
}

nest::glif_psc::Buffers_::Buffers_( const Buffers_&, glif_psc& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::glif_psc::glif_psc()
  : ArchivingNode()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

nest::glif_psc::glif_psc( const glif_psc& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::glif_psc::init_state_( const Node& proto )
{
  const glif_psc& pr = downcast< glif_psc >( proto );
  S_ = pr.S_;
}

void
nest::glif_psc::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // include resize
  B_.logger_.reset();   // includes resize
}

void
nest::glif_psc::calibrate()
{
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms(); // in ms

  // pre-computing of decay parameters
  if ( P_.has_theta_spike_ )
  {
    V_.theta_spike_decay_rate_ = std::exp( -P_.th_spike_decay_ * h );
    V_.theta_spike_refractory_decay_rate_ = std::exp( -P_.th_spike_decay_ * P_.t_ref_ );
  }

  if ( P_.has_asc_ )
  {
    V_.asc_decay_rates_.resize( P_.asc_decay_.size() );
    V_.asc_stable_coeff_.resize( P_.asc_decay_.size() );
    V_.asc_refractory_decay_rates_.resize( P_.asc_decay_.size() );
    for ( std::size_t a = 0; a < P_.asc_decay_.size(); ++a )
    {
      V_.asc_decay_rates_[ a ] = std::exp( -P_.asc_decay_[ a ] * h );
      V_.asc_stable_coeff_[ a ] = ( ( 1.0 / P_.asc_decay_[ a ] ) / h ) * ( 1.0 - V_.asc_decay_rates_[ a ] );
      V_.asc_refractory_decay_rates_[ a ] = P_.asc_r_[ a ] * std::exp( -P_.asc_decay_[ a ] * P_.t_ref_ );
    }
  }

  if ( P_.has_theta_voltage_ )
  {
    V_.potential_decay_rate_ = std::exp( -P_.G_ * h / P_.C_m_ );
    V_.theta_voltage_decay_rate_inverse_ = 1 / ( std::exp( P_.th_voltage_decay_ * h ) );
    V_.phi = P_.th_voltage_index_ / ( P_.th_voltage_decay_ - P_.G_ / P_.C_m_ );
    V_.abpara_ratio_voltage_ = P_.th_voltage_index_ / P_.th_voltage_decay_;
  }

  // postsynaptic currents
  V_.P11_.resize( P_.n_receptors_() );
  V_.P21_.resize( P_.n_receptors_() );
  V_.P22_.resize( P_.n_receptors_() );
  V_.P31_.resize( P_.n_receptors_() );
  V_.P32_.resize( P_.n_receptors_() );

  S_.y1_.resize( P_.n_receptors_() );
  S_.y2_.resize( P_.n_receptors_() );
  V_.PSCInitialValues_.resize( P_.n_receptors_() );

  B_.spikes_.resize( P_.n_receptors_() );

  double Tau_ = P_.C_m_ / P_.G_; // in ms
  V_.P33_ = std::exp( -h / Tau_ );
  V_.P30_ = 1 / P_.C_m_ * ( 1 - V_.P33_ ) * Tau_;

  for ( size_t i = 0; i < P_.n_receptors_(); i++ )
  {
    // these P are independent
    V_.P11_[ i ] = V_.P22_[ i ] = std::exp( -h / P_.tau_syn_[ i ] );

    V_.P21_[ i ] = h * V_.P11_[ i ];

    // these are determined according to a numeric stability criterion
    // input time parameter shall be in ms, capacity in pF
    V_.P31_[ i ] = propagator_31( P_.tau_syn_[ i ], Tau_, P_.C_m_, h );
    V_.P32_[ i ] = propagator_32( P_.tau_syn_[ i ], Tau_, P_.C_m_, h );

    V_.PSCInitialValues_[ i ] = 1.0 * numerics::e / P_.tau_syn_[ i ];
    B_.spikes_[ i ].resize();
  }

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::glif_psc::update( Time const& origin, const long from, const long to )
{

  double v_old = S_.U_;

  for ( long lag = from; lag < to; ++lag )
  {

    if ( S_.refractory_steps_ == 0 )
    {
      // neuron not refractory, integrate voltage and currents

      // update threshold via exact solution of dynamics of spike component of
      // threshold for glif2/4/5 models with "R"
      if ( P_.has_theta_spike_ )
      {
        S_.threshold_spike_ = S_.threshold_spike_ * V_.theta_spike_decay_rate_;
      }

      // Calculate new ASCurrents value using exponential methods
      S_.ASCurrents_sum_ = 0.0;
      // for glif3/4/5 models with "ASC"
      // take after spike current value at the beginning of the time to compute
      // the exact mean ASC for the time step and sum the exact ASCs of all ports;
      // and then update the current values to the value at the end of the time
      // step, ready for the next time step
      if ( P_.has_asc_ )
      {
        for ( std::size_t a = 0; a < S_.ASCurrents_.size(); ++a )
        {
          S_.ASCurrents_sum_ += ( V_.asc_stable_coeff_[ a ] * S_.ASCurrents_[ a ] );
          S_.ASCurrents_[ a ] = S_.ASCurrents_[ a ] * V_.asc_decay_rates_[ a ];
        }
      }

      // voltage dynamics of membranes, linear exact to find next V_m value
      S_.U_ = v_old * V_.P33_ + ( S_.I_ + S_.ASCurrents_sum_ ) * V_.P30_;

      // add synapse component for voltage dynamics
      S_.I_syn_ = 0.0;
      for ( size_t i = 0; i < P_.n_receptors_(); i++ )
      {
        S_.U_ += V_.P31_[ i ] * S_.y1_[ i ] + V_.P32_[ i ] * S_.y2_[ i ];
        S_.I_syn_ += S_.y2_[ i ];
      }

      // Calculate exact voltage component of the threshold for glif5 model with
      // "A"
      if ( P_.has_theta_voltage_ )
      {
        const double beta = ( S_.I_ + S_.ASCurrents_sum_ ) / P_.G_;
        S_.threshold_voltage_ = V_.phi * ( v_old - beta ) * V_.potential_decay_rate_
          + V_.theta_voltage_decay_rate_inverse_
            * ( S_.threshold_voltage_ - V_.phi * ( v_old - beta ) - V_.abpara_ratio_voltage_ * beta )
          + V_.abpara_ratio_voltage_ * beta;
      }

      S_.threshold_ = S_.threshold_spike_ + S_.threshold_voltage_ + P_.th_inf_;

      // Check if there is an action potential
      if ( S_.U_ > S_.threshold_ )
      {
        // Marks that the neuron is in a refractory period
        S_.refractory_steps_ = V_.RefractoryCounts_;

        // Reset ASC_currents for glif3/4/5 models with "ASC"
        if ( P_.has_asc_ )
        {
          for ( std::size_t a = 0; a < S_.ASCurrents_.size(); ++a )
          {
            S_.ASCurrents_[ a ] = P_.asc_amps_[ a ] + S_.ASCurrents_[ a ] * V_.asc_refractory_decay_rates_[ a ];
          }
        }

        // Reset voltage
        if ( not P_.has_theta_spike_ )
        {
          // Reset voltage for glif1/3 models without "R"
          S_.U_ = P_.V_reset_;
        }
        else
        {
          // Reset voltage for glif2/4/5 models with "R"
          S_.U_ = P_.voltage_reset_fraction_ * v_old + P_.voltage_reset_add_;

          // reset spike component of threshold
          // (decay for refractory period and then add additive constant)
          S_.threshold_spike_ = S_.threshold_spike_ * V_.theta_spike_refractory_decay_rate_ + P_.th_spike_add_;

          // rest the global threshold (voltage component of threshold: stay the
          // same)
          S_.threshold_ = S_.threshold_spike_ + S_.threshold_voltage_ + P_.th_inf_;
        }

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }
    else
    {
      // neuron is absolute refractory
      --S_.refractory_steps_;

      // While neuron is in refractory period count-down in time steps (since dt
      // may change while in refractory) while holding the voltage at last peak.
      S_.U_ = v_old;
      S_.threshold_ = S_.threshold_spike_ + S_.threshold_voltage_ + P_.th_inf_;
    }

    // alpha shape PSCs
    for ( size_t i = 0; i < P_.n_receptors_(); i++ )
    {
      S_.y2_[ i ] = V_.P21_[ i ] * S_.y1_[ i ] + V_.P22_[ i ] * S_.y2_[ i ];
      S_.y1_[ i ] *= V_.P11_[ i ];

      // Apply spikes delivered in this step: The spikes arriving at T+1 have an
      // immediate effect on the state of the neuron
      S_.y1_[ i ] += V_.PSCInitialValues_[ i ] * B_.spikes_[ i ].get_value( lag );
    }

    // Update any external currents
    S_.I_ = B_.currents_.get_value( lag );

    // Save voltage
    B_.logger_.record_data( origin.get_steps() + lag );
    v_old = S_.U_;
  }
}

nest::port
nest::glif_psc::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type <= 0 || receptor_type > static_cast< port >( P_.n_receptors_() ) )
  {
    throw IncompatibleReceptorType( receptor_type, get_name(), "SpikeEvent" );
  }

  P_.has_connections_ = true;
  return receptor_type;
}

void
nest::glif_psc::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.spikes_[ e.get_rport() - 1 ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::glif_psc::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

// Do not move this function as inline to h-file. It depends on
// universal_data_logger_impl.h being included here.
void
nest::glif_psc::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e ); // the logger does this for us
}
