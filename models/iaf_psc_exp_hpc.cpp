/*
 *  iaf_psc_exp_hpc.cpp
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

#include "iaf_psc_exp_hpc.h"


// Includes from libnestutil:
#include "dict_util.h"
#include "iaf_propagator.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "ring_buffer_impl.h"
#include "universal_data_logger_impl.h"


namespace nest
{
/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< iaf_psc_exp_hpc > iaf_psc_exp_hpc::recordablesMap_;

void
register_iaf_psc_exp_hpc( const std::string& name )
{
  register_node_model< iaf_psc_exp_hpc >( name );
}

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_psc_exp_hpc >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &iaf_psc_exp_hpc::get_V_m_ );
  insert_( names::I_syn_ex, &iaf_psc_exp_hpc::get_I_syn_ex_ );
  insert_( names::I_syn_in, &iaf_psc_exp_hpc::get_I_syn_in_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

iaf_psc_exp_hpc::Parameters_::Parameters_()
  : Tau_( 10.0 )              // in ms
  , C_( 250.0 )               // in pF
  , t_ref_( 2.0 )             // in ms
  , E_L_( -70.0 )             // in mV
  , I_e_( 0.0 )               // in pA
  , Theta_( -55.0 - E_L_ )    // relative E_L_
  , V_reset_( -70.0 - E_L_ )  // in mV
  , tau_ex_( 2.0 )            // in ms
  , tau_in_( 2.0 )            // in ms
{
}

iaf_psc_exp_hpc::State_::State_()
  : i_0_( 0.0 )
  , i_syn_ex_( 0.0 )
  , i_syn_in_( 0.0 )
  , V_m_( 0.0 )
  , r_ref_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
iaf_psc_exp_hpc::Parameters_::get( Dictionary& d ) const
{
  d[ names::E_L ] = E_L_;  // resting potential
  d[ names::I_e ] = I_e_;
  d[ names::V_th ] = Theta_ + E_L_;  // threshold value
  d[ names::V_reset ] = V_reset_ + E_L_;
  d[ names::C_m ] = C_;
  d[ names::tau_m ] = Tau_;
  d[ names::tau_syn_ex ] = tau_ex_;
  d[ names::tau_syn_in ] = tau_in_;
  d[ names::t_ref ] = t_ref_;
}

double
iaf_psc_exp_hpc::Parameters_::set( const Dictionary& d, Node* node )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  update_value_param( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  if ( update_value_param( d, names::V_reset, V_reset_, node ) )
  {
    V_reset_ -= E_L_;
  }
  else
  {
    V_reset_ -= delta_EL;
  }

  if ( update_value_param( d, names::V_th, Theta_, node ) )
  {
    Theta_ -= E_L_;
  }
  else
  {
    Theta_ -= delta_EL;
  }

  update_value_param( d, names::I_e, I_e_, node );
  update_value_param( d, names::C_m, C_, node );
  update_value_param( d, names::tau_m, Tau_, node );
  update_value_param( d, names::tau_syn_ex, tau_ex_, node );
  update_value_param( d, names::tau_syn_in, tau_in_, node );
  update_value_param( d, names::t_ref, t_ref_, node );
  if ( V_reset_ >= Theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( Tau_ <= 0 or tau_ex_ <= 0 or tau_in_ <= 0 )
  {
    throw BadProperty( "Membrane and synapse time constants must be strictly positive." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  return delta_EL;
}

void
iaf_psc_exp_hpc::State_::get( Dictionary& d, const Parameters_& p ) const
{
  d[ names::V_m ] = V_m_ + p.E_L_;  // Membrane potential
}

void
iaf_psc_exp_hpc::State_::set( const Dictionary& d, const Parameters_& p, double delta_EL, Node* node )
{
  if ( update_value_param( d, names::V_m, V_m_, node ) )
  {
    V_m_ -= p.E_L_;
  }
  else
  {
    V_m_ -= delta_EL;
  }
}

iaf_psc_exp_hpc::Buffers_::Buffers_( iaf_psc_exp_hpc& n )
  : logger_( n )
{
}

iaf_psc_exp_hpc::Buffers_::Buffers_( const Buffers_&, iaf_psc_exp_hpc& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

iaf_psc_exp_hpc::iaf_psc_exp_hpc()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

iaf_psc_exp_hpc::iaf_psc_exp_hpc( const iaf_psc_exp_hpc& n )
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
iaf_psc_exp_hpc::init_buffers_()
{
  B_.input_buffer_.clear();  // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

void
iaf_psc_exp_hpc::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  // these P are independent
  V_.P11ex_ = std::exp( -h / P_.tau_ex_ );
  V_.P11in_ = std::exp( -h / P_.tau_in_ );

  V_.P22_ = std::exp( -h / P_.Tau_ );

  // these are determined according to a numeric stability criterion
  V_.P21ex_ = IAFPropagatorExp( P_.tau_ex_, P_.Tau_, P_.C_ ).evaluate( h );
  V_.P21in_ = IAFPropagatorExp( P_.tau_in_, P_.Tau_, P_.C_ ).evaluate( h );

  V_.P20_ = P_.Tau_ / P_.C_ * ( 1.0 - V_.P22_ );

  // t_ref_ specifies the length of the absolute refractory period as a double in ms. The model can only handle
  // refractory periods that are integer multiples of the computation step size (h); the conversion is carried out via
  // nest::Time, as in iaf_psc_exp.
  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );
}

void
iaf_psc_exp_hpc::update( const Time& origin, const long from, const long to )
{
  // The order of the operations is that of iaf_psc_exp, so that both models give identical results.
  for ( long lag = from; lag < to; ++lag )
  {
    if ( S_.r_ref_ == 0 )  // neuron not refractory, so evolve V
    {
      S_.V_m_ =
        S_.V_m_ * V_.P22_ + S_.i_syn_ex_ * V_.P21ex_ + S_.i_syn_in_ * V_.P21in_ + ( P_.I_e_ + S_.i_0_ ) * V_.P20_;
    }
    else
    {
      // neuron is absolute refractory
      --S_.r_ref_;
    }

    // exponential decaying PSCs
    S_.i_syn_ex_ *= V_.P11ex_;
    S_.i_syn_in_ *= V_.P11in_;

    // get read access to the correct input-buffer slot
    const size_t input_buffer_slot = kernel().event_delivery_manager.get_modulo( lag );
    auto& input = B_.input_buffer_.get_values_all_channels( input_buffer_slot );

    // the spikes arriving at T+1 have an immediate effect on the state of the neuron
    S_.i_syn_ex_ += input[ Buffers_::SYN_EX ];
    S_.i_syn_in_ += input[ Buffers_::SYN_IN ];

    if ( S_.V_m_ >= P_.Theta_ )  // threshold crossing
    {
      S_.r_ref_ = V_.RefractoryCounts_;
      S_.V_m_ = P_.V_reset_;

      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input current
    S_.i_0_ = input[ Buffers_::I0 ];

    // reset all values in the currently processed input-buffer slot
    B_.input_buffer_.reset_values_all_channels( input_buffer_slot );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
iaf_psc_exp_hpc::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const size_t input_buffer_slot = kernel().event_delivery_manager.get_modulo(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) );

  const double s = e.get_weight() * e.get_multiplicity();

  // separate buffer channels for excitatory and inhibitory inputs
  B_.input_buffer_.add_value( input_buffer_slot, s > 0 ? Buffers_::SYN_EX : Buffers_::SYN_IN, s );
}

void
iaf_psc_exp_hpc::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  const size_t input_buffer_slot = kernel().event_delivery_manager.get_modulo(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ) );

  B_.input_buffer_.add_value( input_buffer_slot, Buffers_::I0, w * c );
}

void
iaf_psc_exp_hpc::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

}  // namespace nest
