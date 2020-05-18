/*
 *  iaf_neat.cpp
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

/* iaf_neat is a neuron where the potential jumps on each spike arrival. */

#include "iaf_neat.h"

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

namespace nest
{

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

RecordablesMap< iaf_neat > iaf_neat::recordablesMap_;

// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< iaf_neat >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::V_m, &iaf_neat::get_V_m_ );
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_neat::Parameters_::Parameters_()
  : tau_m_( 10.0 )                                  // ms
  , c_m_( 250.0 )                                   // pF
  , t_ref_( 2.0 )                                   // ms
  , E_L_( -70.0 )                                   // mV
  , I_e_( 0.0 )                                     // pA
  , V_th_( -55.0 - E_L_ )                           // mV, rel to E_L_
  , V_min_( -std::numeric_limits< double >::max() ) // relative E_L_-55.0-E_L_
  , V_reset_( -70.0 - E_L_ )                        // mV, rel to E_L_
  , with_refr_input_( false )
{
}

nest::iaf_neat::State_::State_()
  : y0_( 0.0 )
  , y3_( 0.0 )
  , r_( 0 )
  , refr_spikes_buffer_( 0.0 )
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_neat::Parameters_::get( DictionaryDatum& d ) const
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
}

double
nest::iaf_neat::Parameters_::set( const DictionaryDatum& d, Node* node )
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

  return delta_EL;
}

void
nest::iaf_neat::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_ + p.E_L_ ); // Membrane potential
}

void
nest::iaf_neat::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
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

nest::iaf_neat::Buffers_::Buffers_( iaf_neat& n )
  : logger_( n )
{
}

nest::iaf_neat::Buffers_::Buffers_( const Buffers_&, iaf_neat& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_neat::iaf_neat()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::iaf_neat::iaf_neat( const iaf_neat& n )
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
nest::iaf_neat::init_state_( const Node& proto )
{
  const iaf_neat& pr = downcast< iaf_neat >( proto );
  S_ = pr.S_;
}

void
nest::iaf_neat::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
  Archiving_Node::clear_history();
  m_cond_w->init();
  m_syn->init();
}

void
nest::iaf_neat::calibrate()
{
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();
  const double dt = h;

  // Tests added here //////////////////////////////////////////////////////////
  /*
  Tree structure for testing
  */
  m_c_tree.init();

  // variables needed for manual computations below
  const double ca0 = 1., gc0 = .1, gl0 = .1, el0 = -70.;
  const double ca1 = .1, gc1 = .1, gl1 = .01, el1 = -70.;

  /*
  Test 1: matrix inversion
  */
  std::cout << std::endl;
  std::cout << "--- Testing matrix inversion ---" << std::endl;
  // input current
  std::vector< double > i_in{.1, .2};
  // matrix
  double a00 = ca0/dt + gl0/2. + gc1/2.;
  double a01 = -gc1/2.;
  double a10 = -gc1/2.;
  double a11 = ca1/dt + gl1/2. + gc1/2.;
  // vector
  double b0 = ca0/dt * el0 - gl0 * (el0/2. - el0) + gc1 * (el0 - el1)/2. + i_in[0];
  double b1 = ca1/dt * el1 - gl1 * (el1/2. - el1) + gc1 * (el1 - el0)/2. + i_in[1];
  // hand-crafted solution
  double det = a00 * a11 - a10 * a01;
  double v0 = (b0 * a11 - b1 * a01) / det;
  double v1 = (b1 * a00 - b0 * a10) / det;
  // compartment tree solution
  m_c_tree.construct_matrix(i_in);
  m_c_tree.solve_matrix();
  std::vector< double > v_sol = m_c_tree.get_voltage();
  // print test result
  std::cout << "v_sol_manual = (" << v0 << ", " << v1 << ")" << std::endl;
  std::cout << "v_sol_comptr = (" << v_sol[0] << ", " << v_sol[1] << ")" << std::endl;

  /*
  Test 2: attenuation
  */
  std::cout << std::endl;
  std::cout << "--- Testing attenuation simulation ---" << std::endl;
  // attenuation 1->0
  m_c_tree.init();
  v0 = el0; v1 = el1;
  i_in[0] = 0., i_in[1] = 0.2;
  for(int ii = 0; ii < 10000; ii++){
    m_c_tree.construct_matrix(i_in);
    m_c_tree.solve_matrix();

  }
  v_sol = m_c_tree.get_voltage();
  std::cout << "attenuation 1->0 manual = " << gc1 / (gl0 + gc1) << std::endl;
  std::cout << "attenuation 1->0 comptr = " << (v_sol[0] - el0) / (v_sol[1] - el1) << std::endl;
  // attenuation 0->1
  m_c_tree.init();
  i_in[0] = 0.15, i_in[1] = 0.;
  for(int ii = 0; ii < 10000; ii++){
    m_c_tree.construct_matrix(i_in);
    m_c_tree.solve_matrix();
  }
  v_sol = m_c_tree.get_voltage();
  std::cout << "attenuation 0->1 manual = " << gc1 / (gl1 + gc1) << std::endl;
  std::cout << "attenuation 0->1 comptr = " << (v_sol[1] - el1) / (v_sol[0] - el0) << std::endl;
  //////////////////////////////////////////////////////////////////////////////


  V_.P33_ = std::exp( -h / P_.tau_m_ );
  V_.P30_ = 1 / P_.c_m_ * ( 1 - V_.P33_ ) * P_.tau_m_;


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

/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
nest::iaf_neat::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  double g_syn = 0., f_v = 0.;
  // for new synapses
  std::pair< double, double > gf_syn(0., 0.);
  std::vector< double > v_vals{0.};

  const double h = Time::get_resolution().get_ms();
  for ( long lag = from; lag < to; ++lag )
  {

    /*
    First model
    */
    // // advance the synapse
    // m_cond_w->update( lag );
    // // compute synaptic input
    // g_syn = m_cond_w->get_cond();
    // f_v = m_v_dep->f(get_V_m_());
    // // integration step
    // S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ + g_syn * f_v) + V_.P33_ * S_.y3_;

    /*
    Second model
    */
    m_syn->update( lag );
    // compute synaptic input
    v_vals[0] = get_V_m_();
    gf_syn = m_syn->f_numstep(v_vals);

    S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ + -(gf_syn.first + gf_syn.second * v_vals[0]) ) + V_.P33_ * S_.y3_;



    // if ( S_.r_ == 0 )
    // {
      // neuron not refractory
      // S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ ) + V_.P33_ * S_.y3_ + B_.spikes_.get_value( lag );

    //   // if we have accumulated spikes from refractory period,
    //   // add and reset accumulator
    //   if ( P_.with_refr_input_ && S_.refr_spikes_buffer_ != 0.0 )
    //   {
    //     S_.y3_ += S_.refr_spikes_buffer_;
    //     S_.refr_spikes_buffer_ = 0.0;
    //   }

    //   // lower bound of membrane potential
    //   S_.y3_ = ( S_.y3_ < P_.V_min_ ? P_.V_min_ : S_.y3_ );
    // }
    // else // neuron is absolute refractory
    // {
    //   // read spikes from buffer and accumulate them, discounting
    //   // for decay until end of refractory period
    //   if ( P_.with_refr_input_ )
    //   {
    //     S_.refr_spikes_buffer_ += B_.spikes_.get_value( lag ) * std::exp( -S_.r_ * h / P_.tau_m_ );
    //   }
    //   else
    //   {
    //     // clear buffer entry, ignore spike
    //     B_.spikes_.get_value( lag );
    //   }

    //   --S_.r_;
    // }

    // threshold crossing
    if ( S_.y3_ >= P_.V_th_ )
    {
      S_.r_ = V_.RefractoryCounts_;
      S_.y3_ = P_.V_reset_;

      // EX: must compute spike time
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );

      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }

    // set new input current
    S_.y0_ = B_.currents_.get_value( lag );

    // voltage logging
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_neat::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // EX: We must compute the arrival time of the incoming spike
  //     explicity, since it depends on delay and offset within
  //     the update cycle.  The way it is done here works, but
  // //     is clumsy and should be improved.
  // B_.spikes_.add_value(
  //   e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_multiplicity() );

  /*
  First model
  */
  // m_cond_w->handle(e);
  /*
  Second model
  */
  m_syn->handle(e);
}

void
nest::iaf_neat::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

void
nest::iaf_neat::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

} // namespace
