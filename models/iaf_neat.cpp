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
  : t_ref_( 10.0 )                                   // ms
  , I_e_( 0.0 )                                     // pA
  , V_th_( -55.0 )                           // mV, rel to E_L_
  , V_reset_( -85.0 )                        // mV, rel to E_L_
  , F_pot_( 10.0 )
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
  def< double >( d, names::I_e, I_e_ );
  def< double >( d, names::V_th, V_th_ ); // threshold value
  def< double >( d, names::V_reset, V_reset_ );
  def< double >( d, names::t_ref, t_ref_ );
  def< bool >( d, names::refractory_input, with_refr_input_ );
}

double
nest::iaf_neat::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::V_th, V_th_, node );
  updateValueParam< double >( d, names::V_reset, V_reset_, node );
  updateValueParam< double >( d, names::I_e, I_e_, node );
  updateValueParam< double >( d, names::t_ref, t_ref_, node );
  if ( V_reset_ >= V_th_ )
  {
    throw BadProperty( "Reset potential must be smaller than threshold." );
  }
  if ( t_ref_ < 0 )
  {
    throw BadProperty( "Refractory time must not be negative." );
  }

  updateValueParam< bool >( d, names::refractory_input, with_refr_input_, node );

  return 0.;
}

void
nest::iaf_neat::State_::get( DictionaryDatum& d, const Parameters_& p ) const
{
  def< double >( d, names::V_m, y3_); // Membrane potential
}

void
nest::iaf_neat::State_::set( const DictionaryDatum& d, const Parameters_& p, double delta_EL, Node* node )
{
  // if ( updateValueParam< double >( d, names::V_m, y3_, node ) )
  // {
  //   y3_ -= p.E_L_;
  // }
  // else
  // {
  //   y3_ -= delta_EL;
  // }
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
}


void
nest::iaf_neat::calibrate()
{
  B_.logger_.init();

  const double h = Time::get_resolution().get_ms();

  CompNode* root = m_c_tree.find_node(0);
  std::shared_ptr< IonChannel > chan(new FakePotassium(P_.t_ref_, P_.V_reset_,
                                                       P_.F_pot_ * root->m_gl));
  root->m_chans.push_back(chan);


  m_c_tree.init( h );



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
    // m_syn->update( lag );
    // compute synaptic input
    // v_vals[0] = get_V_m_();
    // gf_syn = m_syn->f_numstep(v_vals);

     /*
    Third model
    */
    m_c_tree.construct_matrix(lag);
    m_c_tree.solve_matrix();

    // S_.y3_ = V_.P30_ * ( S_.y0_ + P_.I_e_ + -(gf_syn.first + gf_syn.second * v_vals[0]) ) + V_.P33_ * S_.y3_;

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
    if ( m_c_tree.get_node_voltage(0) >= P_.V_th_ )
    {
      m_c_tree.find_node(0)->m_chans[0]->add_spike();

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
  // copied and adapted from aeif_cond_alpha_multisynapse
  if ( e.get_weight() < 0 )
  {
    throw BadProperty(
      "Synaptic weights for conductance-based multisynapse models "
      "must be positive." );
  }
  assert( e.get_delay_steps() > 0 );
  assert( ( e.get_rport() >= 0 ) && ( ( size_t ) e.get_rport() < syn_receptors.size() ) );

  syn_receptors[ e.get_rport() ]->handle(e);
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
