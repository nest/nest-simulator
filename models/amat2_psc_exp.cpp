/*
 *  amat2_psc_exp.cpp
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

#include "amat2_psc_exp.h"

// C++ includes:
#include <limits>

// Includes from libnestutil:
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

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::amat2_psc_exp > nest::amat2_psc_exp::recordablesMap_;

namespace nest // template specialization must be placed in namespace
{
/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< amat2_psc_exp >::create()
{
  // use standard names wherever you can for consistency!
  insert_( names::V_m, &amat2_psc_exp::get_V_m_ );
  insert_( names::V_th, &amat2_psc_exp::get_V_th_ );
  insert_( names::V_th_v, &amat2_psc_exp::get_V_th_v_ );
  insert_( names::I_syn_ex, &amat2_psc_exp::get_I_syn_ex_ );
  insert_( names::I_syn_in, &amat2_psc_exp::get_I_syn_in_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::amat2_psc_exp::Parameters_::Parameters_()
  : Tau_( 10.0 )     // in ms
  , C_( 200.0 )      // in pF (R=50MOhm)
  , tau_ref_( 2.0 )  // in ms
  , E_L_( -70.0 )    // in mV
  , I_e_( 0.0 )      // in pA
  , tau_ex_( 1.0 )   // in ms
  , tau_in_( 3.0 )   // in ms
  , tau_1_( 10.0 )   // in ms
  , tau_2_( 200.0 )  // in ms
  , alpha_1_( 10.0 ) // in mV
  , alpha_2_( 0.0 )  // in mV
  , beta_( 0.0 )     // in mV
  , tau_v_( 5.0 )    // in ms
  , omega_( 5.0 )    // resting threshold relative to E_L_ in mV
                     // state V_th_ is initialized with the
                     // same value
{
}

nest::amat2_psc_exp::State_::State_()
  : i_0_( 0.0 )
  , I_syn_ex_( 0.0 )
  , I_syn_in_( 0.0 )
  , V_m_( 0.0 )
  , V_th_1_( 0.0 ) // relative to omega_
  , V_th_2_( 0.0 ) // relative to omega_
  , V_th_dv_( 0.0 )
  , V_th_v_( 0.0 )
  , r_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::amat2_psc_exp::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::E_L, E_L_ ); // Resting potential
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::C_m, C_ );
  def< double >( d, names::tau_m, Tau_ );
  def< double >( d, names::tau_syn_ex, tau_ex_ );
  def< double >( d, names::tau_syn_in, tau_in_ );
  def< double >( d, names::t_ref, tau_ref_ );
  def< double >( d, names::tau_1, tau_1_ );
  def< double >( d, names::tau_2, tau_2_ );
  def< double >( d, names::alpha_1, alpha_1_ );
  def< double >( d, names::alpha_2, alpha_2_ );
  def< double >( d, names::beta, beta_ );
  def< double >( d, names::tau_v, tau_v_ );
  def< double >( d, names::omega, omega_ + E_L_ );
}

double
nest::amat2_psc_exp::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  // if E_L_ is changed, we need to adjust all variables defined relative to
  // E_L_
  const double ELold = E_L_;
  updateValueParam< double >( d, names::E_L, E_L_, node );
  const double delta_EL = E_L_ - ELold;

  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::C_m, C_, node );
  updateValueParam< double >( d, names::tau_m, Tau_, node );
  updateValueParam< double >( d, names::tau_syn_ex, tau_ex_, node );
  updateValueParam< double >( d, names::tau_syn_in, tau_in_, node );
  updateValueParam< double >( d, names::t_ref, tau_ref_, node );
  updateValueParam< double >( d, names::tau_1, tau_1_, node );
  updateValueParam< double >( d, names::tau_2, tau_2_, node );
  updateValueParam< double >( d, names::alpha_1, alpha_1_, node );
  updateValueParam< double >( d, names::alpha_2, alpha_2_, node );
  updateValueParam< double >( d, names::beta, beta_, node );
  updateValueParam< double >( d, names::tau_v, tau_v_, node );

  if ( updateValueParam< double >( d, names::omega, omega_, node ) )
  {
    omega_ -= E_L_;
  }
  else
  {
    omega_ -= delta_EL;
  }
  if ( C_ <= 0 )
  {
    throw BadProperty( "Capacitance must be strictly positive." );
  }
  if ( Tau_ <= 0 || tau_ex_ <= 0 || tau_in_ <= 0 || tau_ref_ <= 0 || tau_1_ <= 0 || tau_2_ <= 0 || tau_v_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  if ( Tau_ == tau_ex_ || Tau_ == tau_in_ || Tau_ == tau_v_ )
  {
    throw BadProperty(
      "tau_m must differ from tau_syn_ex, tau_syn_in and tau_v. "
      "See note in documentation." );
  }

  if ( tau_v_ == tau_ex_ || tau_v_ == tau_in_ ) // tau_v_ == tau_m_  checked above
  {
    throw BadProperty(
      "tau_v must differ from tau_syn_ex, tau_syn_in and tau_m. "
      "See note in documentation." );
  }

  return delta_EL;
}

void
nest::amat2_psc_exp::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, V_m_ + p.E_L_ ); // Membrane potential
  // Adaptive threshold
  def< double >( d, names::V_th, p.E_L_ + p.omega_ + V_th_1_ + V_th_2_ + V_th_v_ );
  def< double >( d, names::V_th_alpha_1, V_th_1_ );
  def< double >( d, names::V_th_alpha_2, V_th_2_ );
  def< double >( d, names::V_th_v, V_th_v_ );
}

void
nest::amat2_psc_exp::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  if ( updateValueParam< double >( d, names::V_m, V_m_, node ) )
  {
    V_m_ -= p.E_L_;
  }
  else
  {
    V_m_ -= delta_EL;
  }

  updateValueParam< double >( d, names::V_th_alpha_1, V_th_1_, node );
  updateValueParam< double >( d, names::V_th_alpha_2, V_th_2_, node );
  updateValueParam< double >( d, names::V_th_v, V_th_v_, node );
}

nest::amat2_psc_exp::Buffers_::Buffers_( amat2_psc_exp& n )
  : logger_( n )
{
  // The other member variables are left uninitialised or are
  // automatically initialised by their default constructor.
}

nest::amat2_psc_exp::Buffers_::Buffers_( const Buffers_&, amat2_psc_exp& n )
  : logger_( n )
{
  // The other member variables are left uninitialised or are
  // automatically initialised by their default constructor.
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::amat2_psc_exp::amat2_psc_exp()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::amat2_psc_exp::amat2_psc_exp( const amat2_psc_exp& n )
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
nest::amat2_psc_exp::init_state_( const Node& proto )
{
  const amat2_psc_exp& pr = downcast< amat2_psc_exp >( proto );
  S_ = pr.S_;
}

void
nest::amat2_psc_exp::init_buffers_()
{
  ArchivingNode::clear_history();

  B_.spikes_ex_.clear(); // includes resize
  B_.spikes_in_.clear(); // includes resize
  B_.currents_.clear();  // includes resize

  B_.logger_.reset();
}

void
nest::amat2_psc_exp::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  // numbering of state variables:
  // membrane potential: i_0 = 0, i_syn_ = 1, V_m_ = 2
  // adaptive threshold: V_th_1_ = 1, V_th_2_ = 2

  // --------------------
  // membrane potential
  // --------------------

  const double c = P_.C_;
  const double beta = P_.beta_;

  const double taum = P_.Tau_;
  const double tauE = P_.tau_ex_;
  const double tauI = P_.tau_in_;
  const double tauV = P_.tau_v_;


  // these P are independent
  const double eE = std::exp( -h / P_.tau_ex_ );
  const double eI = std::exp( -h / P_.tau_in_ );
  const double em = std::exp( -h / P_.Tau_ );
  const double e1 = std::exp( -h / P_.tau_1_ );
  const double e2 = std::exp( -h / P_.tau_2_ );
  const double eV = std::exp( -h / P_.tau_v_ );

  // V_.P00 = 1;
  V_.P11_ = eE;
  V_.P22_ = eI;
  V_.P33_ = em;
  V_.P44_ = e1;
  V_.P55_ = e2;
  V_.P66_ = eV;
  V_.P77_ = eV;

  // these depend on the above. Please do not change the order.

  // TODO Shortcut for beta=0

  V_.P30_ = ( taum - em * taum ) / c;
  V_.P31_ = ( ( eE - em ) * tauE * taum ) / ( c * ( tauE - taum ) );
  V_.P32_ = ( ( eI - em ) * tauI * taum ) / ( c * ( tauI - taum ) );

  V_.P60_ = ( beta * ( em - eV ) * taum * tauV ) / ( c * ( taum - tauV ) );
  V_.P61_ = ( beta * tauE * taum * tauV * ( eV * ( -tauE + taum ) + em * ( tauE - tauV ) + eE * ( -taum + tauV ) ) )
    / ( c * ( tauE - taum ) * ( tauE - tauV ) * ( taum - tauV ) );
  V_.P62_ = ( beta * tauI * taum * tauV * ( eV * ( -tauI + taum ) + em * ( tauI - tauV ) + eI * ( -taum + tauV ) ) )
    / ( c * ( tauI - taum ) * ( tauI - tauV ) * ( taum - tauV ) );
  V_.P63_ = ( beta * ( -em + eV ) * tauV ) / ( taum - tauV );

  V_.P70_ = ( beta * taum * tauV * ( em * taum * tauV - eV * ( h * ( taum - tauV ) + taum * tauV ) ) )
    / ( c * std::pow( taum - tauV, 2 ) );
  V_.P71_ = ( beta * tauE * taum * tauV
              * ( ( em * taum * std::pow( tauE - tauV, 2 ) - eE * tauE * std::pow( taum - tauV, 2 ) ) * tauV
                  - eV * ( tauE - taum )
                    * ( h * ( tauE - tauV ) * ( taum - tauV ) + tauE * taum * tauV - std::pow( tauV, 3 ) ) ) )
    / ( c * ( tauE - taum ) * std::pow( tauE - tauV, 2 ) * std::pow( taum - tauV, 2 ) );
  V_.P72_ = ( beta * tauI * taum * tauV
              * ( ( em * taum * std::pow( tauI - tauV, 2 ) - eI * tauI * std::pow( taum - tauV, 2 ) ) * tauV
                  - eV * ( tauI - taum )
                    * ( h * ( tauI - tauV ) * ( taum - tauV ) + tauI * taum * tauV - std::pow( tauV, 3 ) ) ) )
    / ( c * ( tauI - taum ) * std::pow( tauI - tauV, 2 ) * std::pow( taum - tauV, 2 ) );
  V_.P73_ = ( beta * tauV * ( -( em * taum * tauV ) + eV * ( h * ( taum - tauV ) + taum * tauV ) ) )
    / std::pow( taum - tauV, 2 );
  V_.P76_ = eV * h;


  // tau_ref_ specifies the length of the total refractory period as
  // a double in ms. The grid based amat2_psc_exp can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion
  // requires 2 steps:
  //     1. A time object r is constructed defining representation of
  //        tau_ref_ in tics. This representation is then converted to
  //        computation time
  //        steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a
  //        member function of class nest::Time.
  //
  // Choosing a tau_ref_ that is not an integer multiple of the computation time
  // step h will led to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued
  // spike time may exhibit a different effective refractory time.

  V_.RefractoryCountsTot_ = Time( Time::ms( P_.tau_ref_ ) ).get_steps();

  if ( V_.RefractoryCountsTot_ < 1 )
  {
    throw BadProperty( "Total refractory time must be at least one time step." );
  }
}

/* ----------------------------------------------------------------
 * Integration and further update rules
 * ---------------------------------------------------------------- */

void
nest::amat2_psc_exp::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  // evolve from timestep 'from' to timestep 'to' with steps of h each
  for ( long lag = from; lag < to; ++lag )
  {

    // evolve voltage dependency (6,7)
    S_.V_th_v_ = ( P_.I_e_ + S_.i_0_ ) * V_.P70_ + S_.I_syn_ex_ * V_.P71_ + S_.I_syn_in_ * V_.P72_ + S_.V_m_ * V_.P73_
      + S_.V_th_dv_ * V_.P76_ + S_.V_th_v_ * V_.P77_;

    S_.V_th_dv_ = ( P_.I_e_ + S_.i_0_ ) * V_.P60_ + S_.I_syn_ex_ * V_.P61_ + S_.I_syn_in_ * V_.P62_ + S_.V_m_ * V_.P63_
      + S_.V_th_dv_ * V_.P66_;


    // evolve membrane potential (3)
    S_.V_m_ = ( P_.I_e_ + S_.i_0_ ) * V_.P30_ + S_.I_syn_ex_ * V_.P31_ + S_.I_syn_in_ * V_.P32_ + S_.V_m_ * V_.P33_;


    // evolve adaptive threshold (4,5)
    S_.V_th_1_ *= V_.P44_;
    S_.V_th_2_ *= V_.P55_;

    // exponential decaying PSCs (1,2)
    S_.I_syn_ex_ *= V_.P11_;
    S_.I_syn_in_ *= V_.P22_;
    S_.I_syn_ex_ += B_.spikes_ex_.get_value( lag ); // the spikes arriving at T+1 have an
    S_.I_syn_in_ += B_.spikes_in_.get_value( lag ); // the spikes arriving at T+1 have an


    if ( S_.r_ == 0 ) // neuron is allowed to fire
    {
      if ( S_.V_m_ >= P_.omega_ + S_.V_th_2_ + S_.V_th_1_ + S_.V_th_v_ ) // threshold crossing
      {
        S_.r_ = V_.RefractoryCountsTot_;

        // procedure for adaptive potential
        S_.V_th_1_ += P_.alpha_1_; // short time
        S_.V_th_2_ += P_.alpha_2_; // long time

        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

        SpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }
    else
    {
      // neuron is totally refractory (cannot generate spikes)
      --S_.r_;
    }

    // set new input current
    S_.i_0_ = B_.currents_.get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}


void
nest::amat2_psc_exp::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  if ( e.get_weight() >= 0.0 )
  {
    B_.spikes_ex_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
  else
  {
    B_.spikes_in_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
      e.get_weight() * e.get_multiplicity() );
  }
}

void
nest::amat2_psc_exp::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::amat2_psc_exp::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}
