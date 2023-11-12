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
  : Node()
  , n_spikes_( 0 )
{
}

nest::EpropArchivingNode::EpropArchivingNode( const EpropArchivingNode& n )
  : Node( n )
  , n_spikes_( n.n_spikes_ )
{
}

void
nest::EpropArchivingNode::init_update_history()
{
  // register first entry for every synapse, increase access counter if entry already in list

  const long shift = get_shift();

  const auto it_hist = get_update_history( shift );

  if ( it_hist == update_history_.end() or it_hist->t_ != shift )
  {
    update_history_.insert( it_hist, HistEntryEpropUpdate( shift, 1 ) );
  }
  else
  {
    ++it_hist->access_counter_;
  }
}

void
nest::EpropArchivingNode::write_update_to_history( const long t_previous_update, const long t_current_update )
{
  const long shift = get_shift();

  // Obtain an iterator to the history entry corresponding to the current update time
  const auto it_hist_curr = get_update_history( t_current_update + shift );

  if ( it_hist_curr != update_history_.end() and it_hist_curr->t_ == t_current_update + shift )
  {
    // If an entry already exists for the current update time, increment its access counter
    ++it_hist_curr->access_counter_; 
  }
  else
  {
    // If no entry exists for the current update time, create a new entry with a counter initialized to 1
    update_history_.insert( it_hist_curr, HistEntryEpropUpdate( t_current_update + shift, 1 ) );
  }

  // Obtain an iterator to the history entry corresponding to the previous update time
  const auto it_hist_prev = get_update_history( t_previous_update + shift );

  if ( it_hist_prev != update_history_.end() and it_hist_prev->t_ == t_previous_update + shift )
  {
    // If an entry exists for the previous update time, decrement its access counter
    --it_hist_prev->access_counter_; 
  }
}

void
nest::EpropArchivingNode::write_surrogate_gradient_to_history( const long time_step, const double surrogate_gradient )
{
  eprop_history_.push_back( HistEntryEpropArchive( time_step, surrogate_gradient, 0.0 ) );
}

void
nest::EpropArchivingNode::write_error_signal_to_history( const long time_step, const double error_signal )
{
  const long shift = delay_out_norm_;
  eprop_history_.push_back( HistEntryEpropArchive( time_step - shift, 0.0, error_signal ) );
}

void
nest::EpropArchivingNode::write_learning_signal_to_history( const long time_step,
  const long delay_out_rec,
  const double learning_signal )
{
  const long shift = delay_rec_out_ + delay_out_norm_ + delay_out_rec;

  auto it_hist = get_eprop_history( time_step - shift );
  const auto it_hist_end = get_eprop_history( time_step - shift + delay_out_rec );

  for ( ; it_hist != it_hist_end; ++it_hist )
  {
    it_hist->learning_signal_ += learning_signal;
  }
}

void
nest::EpropArchivingNode::write_firing_rate_reg_to_history( const long t_current_update,
  const double f_target,
  const double c_reg )
{
  const double update_interval = kernel().simulation_manager.get_eprop_update_interval().get_ms();
  const double dt = Time::get_resolution().get_ms();
  const long shift = Time::get_resolution().get_steps();

  const double f_av = n_spikes_ / update_interval * dt;
  const double f_target_ = f_target / 1000.0; // convert to kHz
  const double firing_rate_reg = c_reg * ( f_av - f_target_ ) / update_interval * dt;

  firing_rate_reg_history_.push_back( HistEntryEpropFiringRateReg( t_current_update + shift, firing_rate_reg ) );
}

long
nest::EpropArchivingNode::get_shift() const
{
  return 0;
}

std::vector< HistEntryEpropUpdate >::iterator
nest::EpropArchivingNode::get_update_history( const long time_step )
{
  return std::lower_bound( update_history_.begin(), update_history_.end(), time_step );
}

std::vector< HistEntryEpropArchive >::iterator
nest::EpropArchivingNode::get_eprop_history( const long time_step )
{
  return std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_step );
}

std::vector< HistEntryEpropFiringRateReg >::iterator
nest::EpropArchivingNode::get_firing_rate_reg_history( const long time_step )
{
  return std::lower_bound( firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_step );
}

void
nest::EpropArchivingNode::erase_unneeded_update_history()
{
  for ( auto it_hist = update_history_.begin(); it_hist != update_history_.end(); ++it_hist )
  {
    if ( it_hist->access_counter_ == 0 )
    {
      update_history_.erase( it_hist );
    }
  }
}

void
nest::EpropArchivingNode::erase_unneeded_eprop_history()
{
  if ( eprop_history_.empty()  // nothing to remove
    or update_history_.empty() // no time markers to check
  )
  {
    return;
  }

  const long update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();

  auto it_update_hist = update_history_.begin();

  for ( long t = update_history_.begin()->t_;
        t <= ( update_history_.end() - 1 )->t_ and it_update_hist != update_history_.end();
        t += update_interval )
  {
    if ( it_update_hist->t_ == t )
    {
      ++it_update_hist;
    }
    else
    {
      const auto it_eprop_hist_from = get_eprop_history( t );
      const auto it_eprop_hist_to = get_eprop_history( t + update_interval );
      eprop_history_.erase( it_eprop_hist_from, it_eprop_hist_to ); // erase found entries since no longer used
    }
  }
  const auto it_eprop_hist_from = get_eprop_history( 0 );
  const auto it_eprop_hist_to = get_eprop_history( update_history_.begin()->t_ );
  eprop_history_.erase( it_eprop_hist_from, it_eprop_hist_to ); // erase found entries since no longer used
}

void
nest::EpropArchivingNode::erase_unneeded_firing_rate_reg_history()
{
  auto it_update_hist = update_history_.begin();
  auto it_reg_hist = firing_rate_reg_history_.begin();
  for ( ; it_update_hist != update_history_.end() and it_reg_hist != firing_rate_reg_history_.end();
        ++it_update_hist, ++it_reg_hist )
  {
    if ( it_update_hist->access_counter_ == 0 )
    {
      firing_rate_reg_history_.erase( it_reg_hist );
    }
  }
}

void
nest::EpropArchivingNode::count_spike()
{
  ++n_spikes_;
}

void
nest::EpropArchivingNode::reset_spike_count()
{
  n_spikes_ = 0;
}

} // namespace nest
