/*
 *  iaf_psc_alpha_gap.cpp
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

#include "iaf_psc_alpha_gap.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
#include "numerics.h"
#include "propagator_stability.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

nest::RecordablesMap< nest::iaf_psc_alpha_gap >
  nest::iaf_psc_alpha_gap::recordablesMap_;

namespace nest
{

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< iaf_psc_alpha_gap >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_psc_alpha_gap::get_V_m_ );
  insert_( names::weighted_spikes_ex, &iaf_psc_alpha_gap::get_weighted_spikes_ex_ );
  insert_( names::weighted_spikes_in, &iaf_psc_alpha_gap::get_weighted_spikes_in_ );
  insert_( names::I_syn_ex, &iaf_psc_alpha_gap::get_I_syn_ex_ );
  insert_( names::I_syn_in, &iaf_psc_alpha_gap::get_I_syn_in_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

iaf_psc_alpha_gap::Parameters_::Parameters_()
  : Tau_( 10.0 )             // ms
  , C_( 250.0 )              // pF
  , TauR_( 2.0 )             // ms
  , E_L_( -70.0 )            // mV
  , I_e_( 0.0 )              // pA
  , V_reset_( -70.0 - E_L_ ) // mV, rel to E_L_
  , Theta_( -55.0 - E_L_ )   // mV, rel to E_L_
  , LowerBound_( -std::numeric_limits< double >::infinity() )
  , tau_ex_( 2.0 ) // ms
  , tau_in_( 2.0 ) // ms
{
}

iaf_psc_alpha_gap::State_::State_()
  : y0_( 0.0 )
  , dI_ex_( 0.0 )
  , I_ex_( 0.0 )
  , dI_in_( 0.0 )
  , I_in_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
iaf_psc_alpha_gap::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // Resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, Theta_ + E_L_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ + E_L_ );
  def< double >( d, names::V_min, LowerBound_ + E_L_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::t_ref, TauR_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );
}

double
iaf_psc_alpha_gap::Parameters_::set( const DictionaryDatum& d )
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

  if ( updateValue< double >( d, names::V_th, Theta_ ) )
  {
    Theta_ -= E_L_;
  }
  else
  {
    Theta_ -= delta_EL;
  }

  if ( updateValue< double >( d, names::V_min, LowerBound_ ) )
  {
    LowerBound_ -= E_L_;
  }
  else
  {
    LowerBound_ -= delta_EL;
  }

  updateValue< double >( d, names::I_e, I_e_ );
  updateValue< double >( d, names::C_m, C_ );
  updateValue< double >( d, names::tau_m, Tau_ );
  updateValue< double >( d, names::tau_syn_ex, tau_ex_ );
  updateValue< double >( d, names::tau_syn_in, tau_in_ );
  updateValue< double >( d, names::t_ref, TauR_ );

  if ( C_ <= 0.0 )
  {
    throw BadProperty( "Capacitance must be > 0." );
  }

  if ( Tau_ <= 0.0 )
  {
    throw BadProperty( "Membrane time constant must be > 0." );
  }

  if ( tau_ex_ <= 0.0 || tau_in_ <= 0.0 )
  {
    throw BadProperty( "All synaptic time constants must be > 0." );
  }

  if ( TauR_ < 0.0 )
  {
    throw BadProperty( "The refractory time t_ref can't be negative." );
  }
  if ( V_reset_ >= Theta_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }

  return delta_EL;
}

void
iaf_psc_alpha_gap::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ ); // Membrane potential
}

void
iaf_psc_alpha_gap::State_::set( const DictionaryDatum& d,
  const Parameters_& p,
  double delta_EL )
{
  if ( updateValue< double >( d, names::V_m, y3_ ) )
  {
    y3_ -= p.E_L_;
  }
  else
  {
    y3_ -= delta_EL;
  }
}

iaf_psc_alpha_gap::Buffers_::Buffers_( iaf_psc_alpha_gap& n )
  : logger_( n )
{
}

iaf_psc_alpha_gap::Buffers_::Buffers_( const Buffers_&, iaf_psc_alpha_gap& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

iaf_psc_alpha_gap::iaf_psc_alpha_gap()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

iaf_psc_alpha_gap::iaf_psc_alpha_gap( const iaf_psc_alpha_gap& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
iaf_psc_alpha_gap::init_state_( const Node& proto )
{
  const iaf_psc_alpha_gap& pr = downcast< iaf_psc_alpha_gap >( proto );
  S_ = pr.S_;
}

void
iaf_psc_alpha_gap::init_buffers_()
{
  B_.ex_spikes_.clear(); // includes resize
  B_.in_spikes_.clear(); // includes resize
  B_.currents_.clear();  // includes resize

  B_.logger_.reset();

  Archiving_Node::clear_history();
}

void
iaf_psc_alpha_gap::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  V_.h_ = Time::get_resolution().get_ms();

  // these P are independent
  V_.P11_ex_ = V_.P22_ex_ = std::exp( -V_.h_ / P_.tau_ex_ );
  V_.P11_in_ = V_.P22_in_ = std::exp( -V_.h_ / P_.tau_in_ );

  V_.P33_ = std::exp( -V_.h_ / P_.Tau_ );

  V_.expm1_tau_m_ = numerics::expm1( -V_.h_ / P_.Tau_ );

  // these depend on the above. Please do not change the order.
  V_.P30_ = -P_.Tau_ / P_.C_ * numerics::expm1( -V_.h_ / P_.Tau_ );
  V_.P21_ex_ = V_.h_ * V_.P11_ex_;
  V_.P21_in_ = V_.h_ * V_.P11_in_;

  // these are determined according to a numeric stability criterion
  V_.P31_ex_ = propagator_31( P_.tau_ex_, P_.Tau_, P_.C_, V_.h_ );
  V_.P32_ex_ = propagator_32( P_.tau_ex_, P_.Tau_, P_.C_, V_.h_ );
  V_.P31_in_ = propagator_31( P_.tau_in_, P_.Tau_, P_.C_, V_.h_ );
  V_.P32_in_ = propagator_32( P_.tau_in_, P_.Tau_, P_.C_, V_.h_ );

  V_.EPSCInitialValue_ = 1.0 * numerics::e / P_.tau_ex_;
  V_.IPSCInitialValue_ = 1.0 * numerics::e / P_.tau_in_;

  // TauR specifies the length of the absolute refractory period as
  // a double in ms. The grid based iaf_psc_alpha can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion
  // requires 2 steps:
  //     1. A time object is constructed defining representation of
  //        TauR in tics. This representation is then converted to computation
  //        time steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a
  //        member function of class nest::Time.
  //
  // The definition of the refractory period of the iaf_psc_alpha is consistent
  // the one of iaf_psc_alpha_ps.
  //
  // Choosing a TauR that is not an integer multiple of the computation time
  // step h will lead to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued
  // spike time may exhibit a different effective refractory time.

  V_.RefractoryCounts_ = Time( Time::ms( P_.TauR_ ) ).get_steps();
  // since t_ref_ >= 0, this can only fail in error
  assert( V_.RefractoryCounts_ >= 0 );
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

bool
iaf_psc_alpha_gap::update_( Time const& origin, const long from, const long to, const bool called_from_wfr_update )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const size_t interpolation_order =
    kernel().simulation_manager.get_wfr_interpolation_order();
  const double wfr_tol = kernel().simulation_manager.get_wfr_tol();
  bool wfr_tol_exceeded = false;

  // allocate memory to store the new interpolation coefficients
  // to be sent by gap event
  const size_t buffer_size =
    kernel().connection_manager.get_min_delay() * ( interpolation_order + 1 );
  std::vector< double > new_coefficients( buffer_size, 0.0 );

  // parameters needed for piecewise interpolation
  // translation from paper (Hahne et al., 2015):
  // y_i: V_0
  // y_ip1: V_1
  // hf_i: h dot(V_0)
  // hf_ip1: h dot(V_1)
  // where 0 denotes before and 1 denotes after one h step
  double y_i = 0.0, y_ip1 = 0.0, hf_i = 0.0, hf_ip1 = 0.0;

  for ( long lag = from; lag < to; ++lag )
  {

    if ( called_from_wfr_update )
    {
      y_i = S_.y3_;
      if ( interpolation_order == 3 )
      {
        hf_i = V_.h_ * (
          -S_.y3_ / P_.Tau_
          + ( P_.I_e_ + S_.y0_ + S_.I_ex_ + S_.I_in_ ) / P_.C_ );
      }
    }
    
    if ( S_.r_ == 0 )
    {
      // neuron not refractory
      S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P31_ex_ * S_.dI_ex_
        + V_.P32_ex_ * S_.I_ex_ + V_.P31_in_ * S_.dI_in_ + V_.P32_in_ * S_.I_in_
        + V_.expm1_tau_m_ * S_.y3_ + S_.y3_;

      // lower bound of membrane potential
      S_.y3_ = ( S_.y3_ < P_.LowerBound_ ? P_.LowerBound_ : S_.y3_ );
    }
    else
    {
      // neuron is absolute refractory
      --S_.r_;
    }

    // alpha shape EPSCs
    S_.I_ex_ = V_.P21_ex_ * S_.dI_ex_ + V_.P22_ex_ * S_.I_ex_;
    S_.dI_ex_ *= V_.P11_ex_;

    // alpha shape IPSCs
    S_.I_in_ = V_.P21_in_ * S_.dI_in_ + V_.P22_in_ * S_.I_in_;
    S_.dI_in_ *= V_.P11_in_;

    if ( not called_from_wfr_update )
    {
      // Apply spikes delivered in this step; spikes arriving at T+1 have
      // an immediate effect on the state of the neuron
      V_.weighted_spikes_ex_ = B_.ex_spikes_.get_value( lag );
      S_.dI_ex_ += V_.EPSCInitialValue_ * V_.weighted_spikes_ex_;

      // Apply spikes delivered in this step; spikes arriving at T+1 have
      // an immediate effect on the state of the neuron
      V_.weighted_spikes_in_ = B_.in_spikes_.get_value( lag );
      S_.dI_in_ += V_.IPSCInitialValue_ * V_.weighted_spikes_in_;

      // threshold crossing
      if ( S_.y3_ >= P_.Theta_ )
      {
        S_.r_ = V_.RefractoryCounts_;
        S_.y3_ = P_.V_reset_;
        // A supra-threshold membrane potential should never be observable.
        // The reset at the time of threshold crossing enables accurate
        // integration independent of the computation step size, see [2,3] for
        // details.

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }

      // set new input current
      S_.y0_ = B_.currents_.get_value( lag );

      // log state data
      B_.logger_.record_data( origin.get_steps() + lag );
    }
    else
    {
      // Apply spikes delivered in this step; spikes arriving at T+1 have
      // an immediate effect on the state of the neuron
      V_.weighted_spikes_ex_ = B_.ex_spikes_.get_value_wfr_update( lag );
      S_.dI_ex_ += V_.EPSCInitialValue_ * V_.weighted_spikes_ex_;

      // Apply spikes delivered in this step; spikes arriving at T+1 have
      // an immediate effect on the state of the neuron
      V_.weighted_spikes_in_ = B_.in_spikes_.get_value_wfr_update( lag );
      S_.dI_in_ += V_.IPSCInitialValue_ * V_.weighted_spikes_in_;

      // check if deviation from last iteration exceeds wfr_tol
      wfr_tol_exceeded = wfr_tol_exceeded
        or fabs( S_.y3_ - B_.last_y_values[ lag ] ) > wfr_tol;
      B_.last_y_values[ lag ] = S_.y3_;

      // update different interpolations

      // constant term is the same for each interpolation order
      new_coefficients[ lag * ( interpolation_order + 1 ) + 0 ] = y_i;

      switch ( interpolation_order )
      {
      case 0:
        break;

      case 1:
        y_ip1 = S_.y3_;

        new_coefficients[ lag * ( interpolation_order + 1 ) + 1 ] = y_ip1 - y_i;
        break;

      case 3:
        y_ip1 = S_.y3_;
        hf_ip1 = V_.h_ * (
          -S_.y3_ / P_.Tau_
          + ( P_.I_e_ + S_.y0_ + S_.I_ex_ + S_.I_in_ ) / P_.C_ );

        new_coefficients[ lag * ( interpolation_order + 1 ) + 1 ] = hf_i;
        new_coefficients[ lag * ( interpolation_order + 1 ) + 2 ] =
          -3 * y_i + 3 * y_ip1 - 2 * hf_i - hf_ip1;
        new_coefficients[ lag * ( interpolation_order + 1 ) + 3 ] =
          2 * y_i - 2 * y_ip1 + hf_i + hf_ip1;
        break;

      default:
        throw BadProperty( "Interpolation order must be 0, 1, or 3." );
      }
    }
  } // of for

  // if not called_from_wfr_update perform constant extrapolation
  // and reset last_y_values
  if ( not called_from_wfr_update )
  {
    for ( long temp = from; temp < to; ++temp )
    {
      new_coefficients[ temp * ( interpolation_order + 1 ) + 0 ] =
        S_.y3_;
    }

    std::vector< double >( kernel().connection_manager.get_min_delay(), 0.0 )
      .swap( B_.last_y_values );
  }

  // Send gap-event
  GapJunctionEvent ge;
  ge.set_coeffarray( new_coefficients );
  kernel().event_delivery_manager.send_secondary( *this, ge );

  // Reset variables
  B_.sumj_g_ij_ = 0.0;
  std::vector< double >( buffer_size, 0.0 )
    .swap( B_.interpolation_coefficients );

  return wfr_tol_exceeded;
}

void
iaf_psc_alpha_gap::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  const double s = e.get_weight() * e.get_multiplicity();

  if ( e.get_weight() > 0.0 )
  {
    B_.ex_spikes_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      s );
  }
  else
  {
    B_.in_spikes_.add_value( e.get_rel_delivery_steps(
                               kernel().simulation_manager.get_slice_origin() ),
      s );
  }
}

void
iaf_psc_alpha_gap::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double I = e.get_current();
  const double w = e.get_weight();

  B_.currents_.add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    w * I );
}

void
iaf_psc_alpha_gap::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
nest::iaf_psc_alpha_gap::handle( GapJunctionEvent& e )
{
  B_.sumj_g_ij_ += e.get_weight();

  size_t i = 0;
  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    B_.interpolation_coefficients[ i ] +=
      e.get_weight() * e.get_coeffvalue( it );
    ++i;
  }
}

} // namespace
