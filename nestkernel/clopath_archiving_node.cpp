/*
 *  clopath_archiving_node.cpp
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

#include "clopath_archiving_node.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

// member functions for Clopath_Archiving_Node

nest::Clopath_Archiving_Node::Clopath_Archiving_Node()
  : Archiving_Node()
  , A_LTD_( 14.0e-5 )
  , A_LTP_( 8.0e-5 )
  , u_ref_squared_( 60.0 )
  , theta_plus_( -45.3 )
  , theta_minus_( -70.6 )
  , A_LTD_const_( true )
  , delay_u_bars_( 5.0 )
  , ltd_hist_len_( 0 )
  , ltd_hist_current_( 0 )
{
}

nest::Clopath_Archiving_Node::Clopath_Archiving_Node( const Clopath_Archiving_Node& n )
  : Archiving_Node( n )
  , A_LTD_( n.A_LTD_ )
  , A_LTP_( n.A_LTP_ )
  , u_ref_squared_( n.u_ref_squared_ )
  , theta_plus_( n.theta_plus_ )
  , theta_minus_( n.theta_minus_ )
  , A_LTD_const_( n.A_LTD_const_ )
  , delay_u_bars_( n.delay_u_bars_ )
  , ltd_hist_len_( n.ltd_hist_len_ )
  , ltd_hist_current_( n.ltd_hist_current_ )
{
}

void
nest::Clopath_Archiving_Node::init_clopath_buffers()
{
  // Implementation of the delay of the convolved membrane potentials. This
  // delay is not described in Clopath et al. 2010 but is present in the code
  // (https://senselab.med.yale.edu/ModelDB/showmodel.cshtml?model=144566) on
  // ModelDB which was presumably used to create the figures in the paper.
  // Since we write into the buffer before we read from it, we have to
  // add 1 to the size of the buffers.
  delayed_u_bars_idx_ = 0;
  delay_u_bars_steps_ = Time::delay_ms_to_steps( delay_u_bars_ ) + 1;
  delayed_u_bar_plus_.resize( delay_u_bars_steps_ );
  delayed_u_bar_minus_.resize( delay_u_bars_steps_ );

  // initialize the ltp-history
  ltd_hist_current_ = 0;
  ltd_hist_len_ = kernel().connection_manager.get_max_delay() + 1;
  ltd_history_.resize( ltd_hist_len_, histentry_cl( 0.0, 0.0, 0 ) );
}

void
nest::Clopath_Archiving_Node::get_status( DictionaryDatum& d ) const
{
  Archiving_Node::get_status( d );

  def< double >( d, names::A_LTD, A_LTD_ );
  def< double >( d, names::A_LTP, A_LTP_ );
  def< double >( d, names::u_ref_squared, u_ref_squared_ );
  def< double >( d, names::theta_plus, theta_plus_ );
  def< double >( d, names::theta_minus, theta_minus_ );
  def< bool >( d, names::A_LTD_const, A_LTD_const_ );
  def< double >( d, names::delay_u_bars, delay_u_bars_ );
}

void
nest::Clopath_Archiving_Node::set_status( const DictionaryDatum& d )
{
  Archiving_Node::set_status( d );

  // We need to preserve values in case invalid values are set
  double new_A_LTD = A_LTD_;
  double new_A_LTP = A_LTP_;
  double new_theta_plus = theta_plus_;
  double new_theta_minus = theta_minus_;
  double new_u_ref_squared = u_ref_squared_;
  double new_A_LTD_const = A_LTD_const_;
  double new_delay_u_bars = delay_u_bars_;
  updateValue< double >( d, names::A_LTD, new_A_LTD );
  updateValue< double >( d, names::A_LTP, new_A_LTP );
  updateValue< double >( d, names::u_ref_squared, new_u_ref_squared );
  updateValue< double >( d, names::theta_plus, new_theta_plus );
  updateValue< double >( d, names::theta_minus, new_theta_minus );
  updateValue< bool >( d, names::A_LTD_const, new_A_LTD_const );
  updateValue< double >( d, names::delay_u_bars, new_delay_u_bars );
  A_LTD_ = new_A_LTD;
  A_LTP_ = new_A_LTP;
  u_ref_squared_ = new_u_ref_squared;

  if ( u_ref_squared_ <= 0 )
  {
    throw BadProperty( "Ensure that u_ref_squared > 0" );
  }

  theta_plus_ = new_theta_plus;
  theta_minus_ = new_theta_minus;
  A_LTD_const_ = new_A_LTD_const;
  delay_u_bars_ = new_delay_u_bars;
}

double
nest::Clopath_Archiving_Node::get_LTD_value( double t )
{
  std::vector< histentry_cl >::iterator runner;
  if ( ltd_history_.empty() || t < 0.0 )
  {
    return 0.0;
  }
  else
  {
    runner = ltd_history_.begin();
    while ( runner != ltd_history_.end() )
    {
      if ( fabs( t - runner->t_ ) < kernel().connection_manager.get_stdp_eps() )
      {
        return runner->dw_;
      }
      ( runner->access_counter_ )++;
      ++runner;
    }
  }
  // Return zero if there is no entry at time t
  return 0.0;
}

void
nest::Clopath_Archiving_Node::get_LTP_history( double t1,
  double t2,
  std::deque< histentry_cl >::iterator* start,
  std::deque< histentry_cl >::iterator* finish )
{
  *finish = ltp_history_.end();
  if ( ltp_history_.empty() )
  {
    *start = *finish;
    return;
  }
  else
  {
    std::deque< histentry_cl >::iterator runner = ltp_history_.begin();
    // To have a well defined discretization of the integral, we make sure
    // that we exclude the entry at t1 but include the one at t2 by subtracting
    // a small number so that runner->t_ is never equal to t1 or t2.
    while ( ( runner != ltp_history_.end() ) && ( runner->t_ - 1.0e-6 < t1 ) )
    {
      ++runner;
    }
    *start = runner;
    while ( ( runner != ltp_history_.end() ) && ( runner->t_ - 1.0e-6 < t2 ) )
    {
      ( runner->access_counter_ )++;
      ++runner;
    }
    *finish = runner;
  }
}

void
nest::Clopath_Archiving_Node::write_clopath_history( Time const& t_sp,
  double u,
  double u_bar_plus,
  double u_bar_minus,
  double u_bar_bar )
{
  const double t_ms = t_sp.get_ms();

  // write u_bar_[plus/minus] in ring buffer
  delayed_u_bar_plus_[ delayed_u_bars_idx_ ] = u_bar_plus;

  delayed_u_bar_minus_[ delayed_u_bars_idx_ ] = u_bar_minus;

  // increment pointer
  delayed_u_bars_idx_ = ( delayed_u_bars_idx_ + 1 ) % delay_u_bars_steps_;

  // read oldest values from buffers
  double del_u_bar_plus = delayed_u_bar_plus_[ delayed_u_bars_idx_ ];
  double del_u_bar_minus = delayed_u_bar_minus_[ delayed_u_bars_idx_ ];

  // save data for Clopath STDP if necessary
  if ( ( u > theta_plus_ ) && ( del_u_bar_plus > theta_minus_ ) )
  {
    write_LTP_history( t_ms, u, del_u_bar_plus );
  }

  if ( del_u_bar_minus > theta_minus_ )
  {
    write_LTD_history( t_ms, del_u_bar_minus, u_bar_bar );
  }
}

void
nest::Clopath_Archiving_Node::write_LTD_history( const double t_ltd_ms, double u_bar_minus, double u_bar_bar )
{
  if ( n_incoming_ )
  {
    const double dw = A_LTD_const_ ? A_LTD_ * ( u_bar_minus - theta_minus_ ) : A_LTD_ * u_bar_bar * u_bar_bar
        * ( u_bar_minus - theta_minus_ ) / u_ref_squared_;
    ltd_history_[ ltd_hist_current_ ] = histentry_cl( t_ltd_ms, dw, 0 );
    ltd_hist_current_ = ( ltd_hist_current_ + 1 ) % ltd_hist_len_;
  }
}

void
nest::Clopath_Archiving_Node::write_LTP_history( const double t_ltp_ms, double u, double u_bar_plus )
{
  if ( n_incoming_ )
  {
    // prune all entries from history which are no longer needed
    // except the penultimate one. we might still need it.
    while ( ltp_history_.size() > 1 )
    {
      if ( ltp_history_.front().access_counter_ >= n_incoming_ )
      {
        ltp_history_.pop_front();
      }
      else
      {
        break;
      }
    }
    // dw is not the change of the synaptic weight since the factor
    // x_bar is not included (but later in the synapse)
    const double dw = A_LTP_ * ( u - theta_plus_ ) * ( u_bar_plus - theta_minus_ ) * Time::get_resolution().get_ms();
    ltp_history_.push_back( histentry_cl( t_ltp_ms, dw, 0 ) );
  }
}

} // of namespace nest
