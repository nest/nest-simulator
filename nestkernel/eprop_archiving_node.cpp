/*
 *  eprop_archiving_node.cpp
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

// nestkernel
#include "eprop_archiving_node.h"
#include "kernel_manager.h"

// sli
#include "dictutils.h"

namespace nest
{

nest::EpropArchivingNode::EpropArchivingNode()
  : ArchivingNode()
{
}

nest::EpropArchivingNode::EpropArchivingNode( const EpropArchivingNode& n )
  : ArchivingNode( n )
{
}

void
nest::EpropArchivingNode::init_update_history( double delay )
{
  // register first entry for every synapse, increase access counter if entry already in list

  std::vector< histentry_eprop_update >::iterator it =
    std::lower_bound( update_history_.begin(), update_history_.end(), delay - eps_ );

  if ( it == update_history_.end() || fabs( delay - it->t_ ) > eps_ )
    update_history_.insert( it, histentry_eprop_update( delay, 1 ) );
  else
    ++it->access_counter_;
}

void
nest::EpropArchivingNode::write_update_to_history( double t_last_update, double t_current_update )
{
  std::vector< histentry_eprop_update >::iterator it;

  it = std::lower_bound( update_history_.begin(), update_history_.end(), t_current_update );

  if ( it != update_history_.end() or t_current_update == it->t_ )
    ++it->access_counter_;
  else
    update_history_.insert( it, histentry_eprop_update( t_current_update, 1 ) );

  it = std::lower_bound( update_history_.begin(), update_history_.end(), t_last_update );

  if ( it != update_history_.end() or t_last_update == it->t_ )
    --it->access_counter_;
}

void
nest::EpropArchivingNode::get_eprop_history( double time_point, std::deque< histentry_eprop_archive >::iterator* it )
{
  if ( eprop_history_.empty() )
  {
    *it = eprop_history_.end();
    return;
  }

  *it = std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_point - 1 + eps_ );
}

void
nest::EpropArchivingNode::get_spike_history( double t1,
  double t2,
  std::deque< double >::iterator* start,
  std::deque< double >::iterator* finish )
{
  // set pointer to spike history entries corresponding to times t1 and t2
  *finish = spike_history_.end();

  if ( spike_history_.empty() )
  {
    *start = *finish;
    return;
  }
  else
  {
    std::deque< double >::iterator runner1 =
      std::lower_bound( spike_history_.begin(), spike_history_.end(), t1 + eps_ );

    *start = runner1;

    std::deque< double >::iterator runner2 = std::lower_bound( runner1, spike_history_.end(), t2 + eps_ );

    *finish = runner2;
  }
}

void
nest::EpropArchivingNode::erase_unneeded_eprop_history()
{
  if ( eprop_history_.empty() )
    return;

  double update_interval = kernel().simulation_manager.get_eprop_update_interval();

  std::deque< histentry_eprop_archive >::iterator start;
  std::deque< histentry_eprop_archive >::iterator finish;

  auto it = update_history_.begin();

  for ( auto t = update_history_.begin()->t_; t <= ( update_history_.end() - 1 )->t_; t += update_interval )
  {
    if ( it->t_ == t )
    {
      it++;
    }
    else
    {
      get_eprop_history( t, &start );
      get_eprop_history( t + update_interval, &finish );
      eprop_history_.erase( start, finish ); // erase found entries since no longer used
    }
  }
  get_eprop_history( 0.0, &start );
  get_eprop_history( update_history_.begin()->t_, &finish );
  eprop_history_.erase( start, finish ); // erase found entries since no longer used
}

void
nest::EpropArchivingNode::erase_unneeded_spike_history()
{
  double earliest_time_to_keep = ( t_last_update_per_synapse_.begin() )->t_;
  while ( (  not spike_history_.empty() ) && ( spike_history_.front() + eps_ < earliest_time_to_keep ) )
  {
    spike_history_.pop_front();
  }
}

void
nest::EpropArchivingNode::write_v_m_pseudo_deriv_to_eprop_history( long time_step, double v_m_pseudo_deriv )
{
  if ( not n_incoming_ ) return;

    const Time time_step_ = Time::step( time_step );
    const double time_ms = time_step_.get_ms();
  eprop_history_.push_back( histentry_eprop_archive( time_ms, v_m_pseudo_deriv, 0.0 ) );
  }

void
nest::EpropArchivingNode::write_error_signal_to_eprop_history( long time_step, double error_signal )
{
  const Time time_step_ = Time::step( time_step );
  const double time_ms = time_step_.get_ms();
  eprop_history_.push_back( histentry_eprop_archive( time_ms, 0.0, error_signal ) );
}

void
nest::EpropArchivingNode::write_spike_history( long time_step )
{
  const Time time_step_ = Time::step( time_step );
  const double time_ms = time_step_.get_ms();
  spike_history_.push_back( time_ms );
}


void
nest::EpropArchivingNode::write_learning_signal_to_eprop_history( LearningSignalConnectionEvent& e )
{
  double shift = 2.0 * Time::get_resolution().get_ms();
  double t_ms = e.get_stamp().get_ms() - shift;

  std::deque< histentry_eprop_archive >::iterator start;
  std::deque< histentry_eprop_archive >::iterator finish;

  // get part of eprop history to which the learning signal is added
  get_eprop_history( t_ms, &start );
  get_eprop_history( t_ms + Time::delay_steps_to_ms( e.get_delay_steps() ), &finish );

  std::vector< unsigned int >::iterator it = e.begin();

  if ( start != finish && it != e.end() )
  {
    double error_signal = e.get_coeffvalue( it ); // implicitely decrease access counter
    start->learning_signal_ += e.get_weight() * error_signal;
    ++start;
  }
}

void
nest::EpropArchivingNode::erase_unneeded_update_history()
{
  for ( auto it = update_history_.begin(); it < update_history_.end(); it++ )
  {
    if ( it->access_counter_ == 0 )
      update_history_.erase( it );
  }
}
} // namespace nest
