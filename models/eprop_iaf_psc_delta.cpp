/*
 *  eprop_iaf_psc_delta.cpp
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

/* eprop_iaf_psc_delta is a neuron where the potential jumps on each spike arrival. */

#include "eprop_iaf_psc_delta.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
void
register_eprop_iaf_psc_delta( const std::string& name )
{
  register_node_model< eprop_iaf_psc_delta >( name );
}


/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< eprop_iaf_psc_delta > eprop_iaf_psc_delta::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< eprop_iaf_psc_delta >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::learning_signal, &eprop_iaf_psc_delta::get_learning_signal_ );
  insert_( names::surrogate_gradient, &eprop_iaf_psc_delta::get_surrogate_gradient_ );
  insert_( names::V_m, &eprop_iaf_psc_delta::get_V_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                  // ms
  , c_m_( 250.0 )                                   // pF
  , t_ref_( 2.0 )                                   // ms
  , E_L_( -70.0 )                                   // mV
  , I_e_( 0.0 )                                     // pA
  , V_th_( -55.0 - E_L_ )                           // mV, rel to E_L_
  , V_min_( -std::numeric_limits< double >::max() ) // relative E_L_-55.0-E_L_
  , V_reset_( -70.0 - E_L_ )                        // mV, rel to E_L_
  , with_refr_input_( false )
  , c_reg_( 0.0 )
  , f_target_( 0.01 )
  , beta_( 1.0 )
  , gamma_( 0.3 )
  , surrogate_gradient_function_( "piecewise_linear" )
  , kappa_( 0.97 )
  , kappa_reg_( 0.97 )
  , eprop_isi_trace_cutoff_( std::numeric_limits< long >::max() )
{
}

nest::eprop_iaf_psc_delta::State_::State_()
  : y0_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
  , refr_spikes_buffer_( 0.0 )
  , learning_signal_( 0.0 )
  , surrogate_gradient_( 0.0 )
  , z_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // Resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, V_th_ + E_L_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + E_L_ );
  def< double >( d, names::V_min, V_min_ + E_L_ );
  def< double >( d, names::C_m, c_m_ );
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< bool >( d, names::refractory_input, with_refr_input_ );
  def< double >( d, names::c_reg, c_reg_ );
  def< double >( d, names::f_target, f_target_ );
  def< double >( d, names::beta, beta_ );
  def< double >( d, names::gamma, gamma_ );
  def< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_ );
  def< double >( d, names::kappa, kappa_ );
  def< double >( d, names::kappa_reg, kappa_reg_ );
  def< long >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_ );
}

double
nest::eprop_iaf_psc_delta::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  if ( updateValueParam< double >( d, names::V_reset, V_reset_, node ) )
  {
    V_reset_ -= E_L_;
  }
  else
  {
    V_reset_ -= delta_EL;
  }

  if ( updateValueParam< double >( d, names::V_th, V_th_, node ) )
  {
    V_th_ -= E_L_;
  }
  else
  {
    V_th_ -= delta_EL;
  }

  if ( updateValueParam< double >( d, names::V_min, V_min_, node ) )
  {
    V_min_ -= E_L_;
  }
  else
  {
    V_min_ -= delta_EL;
  }

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, c_m_, node );
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::c_reg, c_reg_, node );
  if ( updateValueParam< double >( d, names::f_target, f_target_, node ) )
  {
    f_target_ /= 1000.0; // convert from spikes/s to spikes/ms
  }

  updateValueParam< double >( d, names::beta, beta_, node );
  updateValueParam< double >( d, names::gamma, gamma_, node );
  updateValueParam< std::string >( d, names::surrogate_gradient_function, surrogate_gradient_function_, node );
  updateValueParam< double >( d, names::kappa, kappa_, node );
  updateValueParam< double >( d, names::kappa_reg, kappa_reg_, node );
  updateValueParam< long >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_, node );

  if ( V_reset_ >= V_th_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( c_m_ <= 0 )
  {
    throw BadProperty( "Capacitance must be >0." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }
  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "Membrane time constant must be > 0." );
  }

  updateValueParam< bool >( d, names::refractory_input, with_refr_input_, node );
  if ( c_reg_ < 0 )
  {
    throw BadProperty( "Firing rate regularization prefactor c_reg ≥ 0 required." );
  }

  if ( f_target_ < 0 )
  {
    throw BadProperty( "Firing rate regularization target rate f_target ≥ 0 required." );
  }

  if ( kappa_ < 0.0 or kappa_ > 1.0 )
  {
    throw BadProperty( "Eligibility trace low-pass filter kappa from range [0, 1] required." );
  }

  if ( kappa_reg_ < 0.0 or kappa_reg_ > 1.0 )
  {
    throw BadProperty( "Firing rate low-pass filter for regularization kappa_reg from range [0, 1] required." );
  }

  if ( eprop_isi_trace_cutoff_ < 0 )
  {
    throw BadProperty( "Cutoff of integration of eprop trace between spikes eprop_isi_trace_cutoff ≥ 0 required." );
  }

  return delta_EL;
}

void
nest::eprop_iaf_psc_delta::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ ); // Membrane potential
  def< double >( d, names::surrogate_gradient, surrogate_gradient_ );
  def< double >( d, names::learning_signal, learning_signal_ );
}

void
nest::eprop_iaf_psc_delta::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  if ( updateValueParam< double >( d, names::V_m, y3_, node ) )
  {
    y3_ -= p.E_L_;
  }
  else
  {
    y3_ -= delta_EL;
  }
}

nest::eprop_iaf_psc_delta::Buffers_::Buffers_( eprop_iaf_psc_delta& n )
  : logger_( n )
{
}

nest::eprop_iaf_psc_delta::Buffers_::Buffers_( const Buffers_&, eprop_iaf_psc_delta& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::eprop_iaf_psc_delta::eprop_iaf_psc_delta()
  : EpropArchivingNodeRecurrent()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::eprop_iaf_psc_delta::eprop_iaf_psc_delta( const eprop_iaf_psc_delta& n )
  : EpropArchivingNodeRecurrent( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::eprop_iaf_psc_delta::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
}

void
nest::eprop_iaf_psc_delta::pre_run_hook()
{
  B_.logger_.init();

  compute_surrogate_gradient = select_surrogate_gradient( P_.surrogate_gradient_function_ );

  const double h = Time::get_resolution().get_ms();


  V_.P33_ = std::exp( -h / P_.tau_m_ );
  V_.P30_ = 1 / P_.c_m_ * ( 1 - V_.P33_ ) * P_.tau_m_;

  V_.P_z_in_ = 1.0;

  // t_ref_ specifies the length of the absolute refractory period as
  // a double in ms. The grid based iaf_psp_delta can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion
  // requires 2 steps:
  //     1. A time object r is constructed, defining representation of
  //        t_ref_ in tics. This representation is then converted to computation
  //        time steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a
  //        member function of class nest::Time.
  //
  // Choosing a t_ref_ that is not an integer multiple of the computation time
  // step h will lead to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued
  // spike time may exhibit a different effective refractory time.

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );
}

long
eprop_iaf_psc_delta::get_shift() const
{
  return offset_gen_ + delay_in_rec_;
}

bool
eprop_iaf_psc_delta::is_eprop_recurrent_node() const
{
  return true;
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::eprop_iaf_psc_delta::update( Time const& origin, const long from, const long to )
{
  const double h = Time::get_resolution().get_ms();
  for ( long lag = from; lag < to; ++lag )
  {
    const long t = origin.get_steps() + lag;
    if ( S_.r_ == 0 )
    {
      // neuron not refractory
      S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + B_.spikes_.get_value( lag );

      // if we have accumulated spikes from refractory period,
      // add and reset accumulator
      if ( P_.with_refr_input_ and S_.refr_spikes_buffer_ != 0.0 )
      {
        S_.y3_ += S_.refr_spikes_buffer_;
        S_.refr_spikes_buffer_ = 0.0;
      }

      // lower bound of membrane potential
      S_.y3_ = ( S_.y3_ < P_.V_min_ ? P_.V_min_ : S_.y3_ );
    }
    else // neuron is absolute refractory
    {
      // read spikes from buffer and accumulate them, discounting
      // for decay until end of refractory period
      if ( P_.with_refr_input_ )
      {
        S_.refr_spikes_buffer_ += B_.spikes_.get_value( lag ) * std::exp( -S_.r_ * h / P_.tau_m_ );
      }
      else
      {
        // clear buffer entry, ignore spike
        B_.spikes_.get_value( lag );
      }

      --S_.r_;
    }

    // P_.V_th_ is passed twice to handle models without an adaptive threshold, serving as both v_th_adapt and V_th
    S_.surrogate_gradient_ =
      ( this->*compute_surrogate_gradient )( S_.r_, S_.y3_, P_.V_th_, P_.V_th_, P_.beta_, P_.gamma_ );

    // threshold crossing
    if ( S_.y3_ >= P_.V_th_ )
    {
      S_.r_ = V_.RefractoryCounts_;
      S_.y3_ = P_.V_reset_;

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );

      S_.z_ = 1.0;
    }

    append_new_eprop_history_entry( t );
    write_surrogate_gradient_to_history( t, S_.surrogate_gradient_ );
    write_firing_rate_reg_to_history( t, S_.z_, P_.f_target_, P_.kappa_reg_, P_.c_reg_ );

    S_.learning_signal_ = get_learning_signal_from_history( t, false );

    // set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::eprop_iaf_psc_delta::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // EX: We must compute the arrival time of the incoming spike
  //     explicity, since it depends on delay and offset within
  //     the update cycle.  The way it is done here works, but
  //     is clumsy and should be improved.
  B_.spikes_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );
}

void
nest::eprop_iaf_psc_delta::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
eprop_iaf_psc_delta::handle( LearningSignalConnectionEvent& e )
{
  for ( auto it_event = e.begin(); it_event != e.end(); )
  {
    const long time_step = e.get_stamp().get_steps();
    const double weight = e.get_weight();
    const double error_signal = e.get_coeffvalue( it_event ); // get_coeffvalue advances iterator
    const double learning_signal = weight * error_signal;

    write_learning_signal_to_history( time_step, learning_signal, false );
  }
}

void
nest::eprop_iaf_psc_delta::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
eprop_iaf_psc_delta::compute_gradient( const long t_spike,
  const long t_spike_previous,
  double& z_previous_buffer,
  double& z_bar,
  double& e_bar,
  double& e_bar_reg,
  double& epsilon,
  double& weight,
  const CommonSynapseProperties& cp,
  WeightOptimizer* optimizer )
{
  double e = 0.0;                // eligibility trace
  double z = 0.0;                // spiking variable
  double z_current_buffer = 1.0; // buffer containing the spike that triggered the current integration
  double psi = 0.0;              // surrogate gradient
  double L = 0.0;                // learning signal
  double firing_rate_reg = 0.0;  // firing rate regularization term
  double grad = 0.0;             // gradient

  const EpropSynapseCommonProperties& ecp = static_cast< const EpropSynapseCommonProperties& >( cp );
  const auto optimize_each_step = ( *ecp.optimizer_cp_ ).optimize_each_step_;

  auto eprop_hist_it = get_eprop_history( t_spike_previous - 1 );

  const long t_compute_until = std::min( t_spike_previous + P_.eprop_isi_trace_cutoff_, t_spike );

  for ( long t = t_spike_previous; t < t_compute_until; ++t, ++eprop_hist_it )
  {
    z = z_previous_buffer;
    z_previous_buffer = z_current_buffer;
    z_current_buffer = 0.0;

    psi = eprop_hist_it->surrogate_gradient_;
    L = eprop_hist_it->learning_signal_;
    firing_rate_reg = eprop_hist_it->firing_rate_reg_;

    z_bar = V_.P33_ * z_bar + V_.P_z_in_ * z;
    e = psi * z_bar;
    e_bar = P_.kappa_ * e_bar + ( 1.0 - P_.kappa_ ) * e;
    e_bar_reg = P_.kappa_reg_ * e_bar_reg + ( 1.0 - P_.kappa_reg_ ) * e;

    if ( optimize_each_step )
    {
      grad = L * e_bar + firing_rate_reg * e_bar_reg;
      weight = optimizer->optimized_weight( *ecp.optimizer_cp_, t, grad, weight );
    }
    else
    {
      grad += L * e_bar + firing_rate_reg * e_bar_reg;
    }
  }

  if ( not optimize_each_step )
  {
    weight = optimizer->optimized_weight( *ecp.optimizer_cp_, t_compute_until, grad, weight );
  }

  const long power = t_spike - ( t_spike_previous + P_.eprop_isi_trace_cutoff_ );

  if ( power > 0 )
  {
    z_bar *= std::pow( V_.P33_, power );
    e_bar *= std::pow( P_.kappa_, power );
    e_bar_reg *= std::pow( P_.kappa_reg_, power );
  }
}

} // namespace
