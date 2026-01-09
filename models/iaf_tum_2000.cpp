/*
 *  iaf_tum_2000.cpp
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

#include "iaf_tum_2000.h"

// Includes from libnestutil:
#include "dict_util.h"
#include "iaf_propagator.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "ring_buffer_impl.h"

// Includes from sli:
#include "dictutils.h"

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_tum_2000 > nest::iaf_tum_2000::recordablesMap_;

namespace nest
{

void
register_iaf_tum_2000( const std::string& name )
{
  register_node_model< iaf_tum_2000 >( name );
}

// Override the create() method with one call to RecordablesMap::insert_() for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_tum_2000 >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_tum_2000::get_V_m_ );
  insert_( names::I_syn_ex, &iaf_tum_2000::get_I_syn_ex_ );
  insert_( names::I_syn_in, &iaf_tum_2000::get_I_syn_in_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_tum_2000::Parameters_::Parameters_()
  : Tau_( 10.0 )             // in ms
  , C_( 250.0 )              // in pF
  , t_ref_( 2.0 )            // in ms
  , E_L_( -70.0 )            // in mV
  , I_e_( 0.0 )              // in pA
  , Theta_( -55.0 - E_L_ )   // relative E_L_
  , V_reset_( -70.0 - E_L_ ) // in mV
  , tau_ex_( 2.0 )           // in ms
  , tau_in_( 2.0 )           // in ms
  , rho_( 0.01 )             // in 1/s
  , delta_( 0.0 )            // in mV
  , tau_fac_( 1000.0 )       // in ms
  , tau_psc_( 2.0 )          // in ms
  , tau_rec_( 400.0 )        // in ms
  , U_( 0.5 )                // dimensionless
{
}

nest::iaf_tum_2000::State_::State_()
  : i_0_( 0.0 )
  , i_1_( 0.0 )
  , i_syn_ex_( 0.0 )
  , i_syn_in_( 0.0 )
  , V_m_( 0.0 )
  , r_ref_( 0 )
  , x_( 0.0 )
  , y_( 0.0 )
  , u_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_tum_2000::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + E_L_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + E_L_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< double >( d, names::rho, rho_ );
  def< double >( d, names::delta, delta_ );
  def< double >( d, names::tau_fac, tau_fac_ );
  def< double >( d, names::tau_psc, tau_psc_ );
  def< double >( d, names::tau_rec, tau_rec_ );
  def< double >( d, names::U, U_ );
}

double
nest::iaf_tum_2000::Parameters_::set( const DictionaryDatum& d, Node* node )
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

  if ( updateValueParam< double >( d, names::V_th, Theta_, node ) )
  {
    Theta_ -= E_L_;
  }
  else
  {
    Theta_ -= delta_EL;
  }

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, C_, node );
  updateValueParam< double >( d, names::tau_m, Tau_, node );
  updateValueParam< double >( d, names::tau_syn_ex, tau_ex_, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_in_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  updateValueParam< double >( d, names::tau_fac, tau_fac_, node );
  updateValueParam< double >( d, names::tau_psc, tau_psc_, node );
  updateValueParam< double >( d, names::tau_rec, tau_rec_, node );
  updateValueParam< double >( d, names::U, U_, node );
  if ( V_reset_ >= Theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( C_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( Tau_ <= 0 or tau_ex_ <= 0 or tau_in_ <= 0 or tau_psc_ <= 0 or tau_rec_ <= 0 )
  {
    throw BadProperty( "Membrane and synapse time constants must be strictly positive." );
  }
  if ( tau_fac_ < 0.0 )
  {
    throw BadProperty( "'tau_fac' must be >= 0." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }
  if ( U_ > 1.0 or U_ < 0.0 )
  {
    throw BadProperty( "'U' must be in [0,1]." );
  }

  updateValue< double >( d, "rho", rho_ );
  if ( rho_ < 0 )
  {
    throw BadProperty( "Stochastic firing intensity must not be negative." );
  }

  updateValue< double >( d, "delta", delta_ );
  if ( delta_ < 0 )
  {
    throw BadProperty( "Width of threshold region must not be negative." );
  }

  return delta_EL;
}

void
nest::iaf_tum_2000::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, V_m_ + p.E_L_ ); // Membrane potential
  def< double >( d, names::x, x_ );
  def< double >( d, names::y, y_ );
  def< double >( d, names::u, u_ );
}

void
nest::iaf_tum_2000::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{

  double x = x_;
  double y = y_;
  updateValue< double >( d, names::x, x );
  updateValue< double >( d, names::y, y );

  if ( x + y > 1.0 )
  {
    throw BadProperty( "x + y must be <= 1.0." );
  }

  x_ = x;
  y_ = y;

  updateValueParam< double >( d, names::u, u_, node );
  if ( u_ > 1.0 or u_ < 0.0 )
  {
    throw BadProperty( "'u' must be in [0,1]." );
  }


  if ( updateValueParam< double >( d, names::V_m, V_m_, node ) )
  {
    V_m_ -= p.E_L_;
  }
  else
  {
    V_m_ -= delta_EL;
  }
}

nest::iaf_tum_2000::Buffers_::Buffers_( iaf_tum_2000& n )
  : logger_( n )
{
}

nest::iaf_tum_2000::Buffers_::Buffers_( const Buffers_&, iaf_tum_2000& n )
  : logger_( n )
{
}

inline double
nest::iaf_tum_2000::phi_() const
{
  assert( P_.delta_ > 0. );
  return P_.rho_ * std::exp( 1. / P_.delta_ * ( S_.V_m_ - P_.Theta_ ) );
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_tum_2000::iaf_tum_2000()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_tum_2000::iaf_tum_2000( const iaf_tum_2000& n )
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
nest::iaf_tum_2000::init_buffers_()
{
  B_.input_buffer_.clear(); // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

void
nest::iaf_tum_2000::pre_run_hook()
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

  // t_ref_ specifies the length of the absolute refractory period as
  // a double in ms. The grid based iaf_psc_exp can only handle refractory
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

  V_.rng_ = get_vp_specific_rng( get_thread() );
}

void
nest::iaf_tum_2000::update( const Time& origin, const long from, const long to )
{
  const double h = Time::get_resolution().get_ms();

  // evolve from timestep 'from' to timestep 'to' with steps of h each
  for ( long lag = from; lag < to; ++lag )
  {
    if ( S_.r_ref_ == 0 ) // neuron not refractory, so evolve V
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

    // add evolution of presynaptic input current
    S_.i_syn_ex_ += ( 1. - V_.P11ex_ ) * S_.i_1_;

    // get read access to the correct input-buffer slot
    const size_t input_buffer_slot = kernel::manager< EventDeliveryManager >.get_modulo( lag );
    auto& input = B_.input_buffer_.get_values_all_channels( input_buffer_slot );

    // the spikes arriving at T+1 have an immediate effect on the state of the
    // neuron

    V_.weighted_spikes_ex_ = input[ Buffers_::SYN_EX ];
    V_.weighted_spikes_in_ = input[ Buffers_::SYN_IN ];

    S_.i_syn_ex_ += V_.weighted_spikes_ex_;
    S_.i_syn_in_ += V_.weighted_spikes_in_;

    if ( ( P_.delta_ < 1e-10 and S_.V_m_ >= P_.Theta_ )                   // deterministic threshold crossing
      or ( P_.delta_ > 1e-10 and V_.rng_->drand() < phi_() * h * 1e-3 ) ) // stochastic threshold crossing
    {
      S_.r_ref_ = V_.RefractoryCounts_;
      S_.V_m_ = P_.V_reset_;

      // Retrieve the previos spike time
      double t_lastspike = get_spiketime_ms();

      // The initial value of ArchivingNode's last_spike_ is -1, but we need the initial value to be 0
      // TODO: A smarter solution than an if-test inside this loop should be sought. Note that we do not
      // want to create an actual spike at timestep 0.
      if ( t_lastspike < 0.0 )
      {
        t_lastspike = 0.0;
      }

      // Set current spike time
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      // Retrieve the current spike time
      const double t_spike = get_spiketime_ms();
      const double h_tsodyks = t_spike - t_lastspike;

      // propagator
      const double Puu = ( P_.tau_fac_ == 0.0 ) ? 0.0 : std::exp( -h_tsodyks / P_.tau_fac_ );
      const double Pyy = std::exp( -h_tsodyks / P_.tau_psc_ );
      const double Pzz = std::expm1( -h_tsodyks / P_.tau_rec_ );
      const double Pxy = ( Pzz * P_.tau_rec_ - ( Pyy - 1.0 ) * P_.tau_psc_ ) / ( P_.tau_psc_ - P_.tau_rec_ );

      const double z = 1.0 - S_.x_ - S_.y_;

      // propagation t_lastspike_ -> t_spike
      // don't change the order !
      S_.u_ *= Puu;
      S_.x_ += Pxy * S_.y_ - Pzz * z;
      S_.y_ *= Pyy;

      // delta function u
      S_.u_ += P_.U_ * ( 1.0 - S_.u_ );

      // postsynaptic current step caused by incoming spike
      const double delta_y_tsp = S_.u_ * S_.x_;

      // delta function x, y
      S_.x_ -= delta_y_tsp;
      S_.y_ += delta_y_tsp;

      // send spike with datafield
      SpikeEvent se;
      se.set_offset( delta_y_tsp );
      kernel::manager< EventDeliveryManager >.send( *this, se, lag );
    }

    // set new input current
    S_.i_0_ = input[ Buffers_::I0 ];
    S_.i_1_ = input[ Buffers_::I1 ];

    // reset all values in the currently processed input-buffer slot
    B_.input_buffer_.reset_values_all_channels( input_buffer_slot );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_tum_2000::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const size_t input_buffer_slot = kernel::manager< EventDeliveryManager >.get_modulo(
    e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ) );

  // Multiply with datafield from SpikeEvent to apply depression/facilitation computed by presynaptic neuron
  double s = e.get_weight() * e.get_multiplicity();

  if ( e.get_rport() == 1 )
  {
    s *= e.get_offset();
  }

  // separate buffer channels for excitatory and inhibitory inputs
  B_.input_buffer_.add_value( input_buffer_slot, s > 0 ? Buffers_::SYN_EX : Buffers_::SYN_IN, s );
}

void
nest::iaf_tum_2000::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  const size_t input_buffer_slot = kernel::manager< EventDeliveryManager >.get_modulo(
    e.get_rel_delivery_steps( kernel::manager< SimulationManager >.get_slice_origin() ) );

  if ( 0 == e.get_rport() )
  {
    B_.input_buffer_.add_value( input_buffer_slot, Buffers_::I0, w * c );
  }
  if ( 1 == e.get_rport() )
  {
    B_.input_buffer_.add_value( input_buffer_slot, Buffers_::I1, w * c );
  }
}

void
nest::iaf_tum_2000::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
