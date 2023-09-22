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
nest::EpropArchivingNode::write_v_m_pseudo_deriv_to_history( long time_step, double v_m_pseudo_deriv )
{
  if ( not n_incoming_ )
    return;

  eprop_history_.push_back(
    histentry_eprop_archive( Time( Time::step( time_step ) ).get_ms(), v_m_pseudo_deriv, 0.0 ) );
}

void
nest::EpropArchivingNode::write_error_signal_to_history( long time_step, double error_signal )
{
  eprop_history_.push_back( histentry_eprop_archive( Time( Time::step( time_step ) ).get_ms(), 0.0, error_signal ) );
}


void
nest::EpropArchivingNode::write_learning_signal_to_history( LearningSignalConnectionEvent& e )
{
  double shift = 2.0 * Time::get_resolution().get_ms();
  double t_ms = e.get_stamp().get_ms() - shift;

  std::deque< histentry_eprop_archive >::iterator start;
  std::deque< histentry_eprop_archive >::iterator finish;

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

void
nest::EpropArchivingNode::erase_unneeded_firing_rate_reg_history()
{
  auto it_update_hist = update_history_.begin();
  auto it_reg_hist = firing_rate_reg_history_.begin();
  for ( ; it_update_hist != update_history_.end() && it_reg_hist != firing_rate_reg_history_.end();
        ++it_update_hist, ++it_reg_hist )
  {
    if ( it_update_hist->access_counter_ == 0 )
      firing_rate_reg_history_.erase( it_reg_hist );
  }
}

void
nest::EpropArchivingNode::add_spike_to_counter()
{
  n_spikes_++;
}

void
nest::EpropArchivingNode::reset_spike_counter()
{
  n_spikes_ = 0;
}

void
nest::EpropArchivingNode::write_firing_rate_reg_to_history( double t_current_update, double f_target, double c_reg )
{

  double const update_interval = kernel().simulation_manager.get_eprop_update_interval();
  double const dt = Time::get_resolution().get_ms();

  double f_av = n_spikes_ / update_interval;
  double f_target_ = f_target / 1000.0; // convert to kHz
  double firing_rate_reg = c_reg * dt * ( f_av - f_target_ ) / update_interval;

  firing_rate_reg_history_.push_back( histentry_eprop_firing_rate_reg( t_current_update, firing_rate_reg ) );
}

double
nest::EpropArchivingNode::get_firing_rate_reg( double time_point )
{
  if ( firing_rate_reg_history_.empty() )
    return 0;

  double const update_interval = kernel().simulation_manager.get_eprop_update_interval();

  std::vector< histentry_eprop_firing_rate_reg >::iterator it;

  it = std::lower_bound(
    firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_point + update_interval - 1 + eps_ );

  return it->firing_rate_reg_;
}

} // namespace nest
