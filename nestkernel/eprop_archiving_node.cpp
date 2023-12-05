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

EpropArchivingNode::EpropArchivingNode()
  : Node()
  , n_spikes_( 0 )
  , eprop_indegree_( 0 )
{
}

EpropArchivingNode::EpropArchivingNode( const EpropArchivingNode& n )
  : Node( n )
  , n_spikes_( n.n_spikes_ )
  , eprop_indegree_( n.eprop_indegree_ )
{
}

void
EpropArchivingNode::register_eprop_connection()
{
  ++eprop_indegree_;

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
EpropArchivingNode::write_update_to_history( const long t_previous_update, const long t_current_update )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const long shift = get_shift();

  const auto it_hist_curr = get_update_history( t_current_update + shift );

  if ( it_hist_curr != update_history_.end() and it_hist_curr->t_ == t_current_update + shift )
  {
    ++it_hist_curr->access_counter_;
  }
  else
  {
    update_history_.insert( it_hist_curr, HistEntryEpropUpdate( t_current_update + shift, 1 ) );
  }

  const auto it_hist_prev = get_update_history( t_previous_update + shift );

  if ( it_hist_prev != update_history_.end() and it_hist_prev->t_ == t_previous_update + shift )
  {
    // If an entry exists for the previous update time, decrement its access counter
    --it_hist_prev->access_counter_;
  }
}

void
EpropArchivingNode::write_surrogate_gradient_to_history( const long time_step, const double surrogate_gradient )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  eprop_history_.push_back( HistEntryEpropArchive( time_step, surrogate_gradient, 0.0 ) );
}

void
EpropArchivingNode::write_error_signal_to_history( const long time_step, const double error_signal )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const long shift = delay_out_norm_;

  eprop_history_.push_back( HistEntryEpropArchive( time_step - shift, 0.0, error_signal ) );
}

void
EpropArchivingNode::write_learning_signal_to_history( const long time_step, const double learning_signal )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  // These 3 delays must be taken into account to place the learning signal in the correct location
  // in the e-prop history
  const long shift = delay_rec_out_ + delay_out_norm_ + delay_out_rec_;

  auto it_hist = get_eprop_history( time_step - shift );
  const auto it_hist_end = get_eprop_history( time_step - shift + delay_out_rec_ );

  for ( ; it_hist != it_hist_end; ++it_hist )
  {
    // Update the learning signal for each history entry within the range. In cases where multiple readout neurons
    // contribute to the learning signal, each neuron's contribution is added to the existing value. Hence,
    // the use of the '+=' operator to incrementally accumulate the learning signal
    it_hist->learning_signal_ += learning_signal;
  }
}

void
EpropArchivingNode::write_firing_rate_reg_to_history( const long t_current_update,
  const double f_target,
  const double c_reg )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const double update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();
  const double dt = Time::get_resolution().get_ms();
  const long shift = Time::get_resolution().get_steps();

  const double f_av = n_spikes_ / update_interval;
  const double f_target_ = f_target * dt; // convert from spikes/ms to spikes/step
  const double firing_rate_reg = c_reg * ( f_av - f_target_ ) / update_interval;

  firing_rate_reg_history_.push_back( HistEntryEpropFiringRateReg( t_current_update + shift, firing_rate_reg ) );
}

long
EpropArchivingNode::get_shift() const
{
  return 0;
}

std::vector< HistEntryEpropUpdate >::iterator
EpropArchivingNode::get_update_history( const long time_step )
{
  return std::lower_bound( update_history_.begin(), update_history_.end(), time_step );
}

std::vector< HistEntryEpropArchive >::iterator
EpropArchivingNode::get_eprop_history( const long time_step )
{
  return std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_step );
}

std::vector< HistEntryEpropFiringRateReg >::iterator
EpropArchivingNode::get_firing_rate_reg_history( const long time_step )
{
  const auto lb = std::lower_bound( firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_step );
  assert( lb != firing_rate_reg_history_.end() );

  return lb;
}

double
EpropArchivingNode::get_learning_signal( const long time_step )
{
  const long shift = delay_rec_out_ + delay_out_norm_ + delay_out_rec_;

  const auto it = get_eprop_history( time_step - shift );
  if ( it != eprop_history_.end() )
  {
    return it->learning_signal_;
  }
  else
  {
    return 0;
  }
}

void
EpropArchivingNode::erase_unneeded_update_history()
{
  auto it_hist = update_history_.begin();
  while ( it_hist != update_history_.end() )
  {
    if ( it_hist->access_counter_ == 0 )
    {
      // erase() invalidates the iterator, but returns a new, valid iterator
      it_hist = update_history_.erase( it_hist );
    }
    else
    {
      ++it_hist;
    }
  }
}

void
EpropArchivingNode::erase_unneeded_eprop_history()
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
EpropArchivingNode::erase_unneeded_firing_rate_reg_history()
{
  auto it_update_hist = update_history_.begin();
  auto it_reg_hist = firing_rate_reg_history_.begin();

  while ( it_update_hist != update_history_.end() and it_reg_hist != firing_rate_reg_history_.end() )
  {
    if ( it_update_hist->access_counter_ == 0 )
    {
      it_reg_hist = firing_rate_reg_history_.erase( it_reg_hist );
    }
    else
    {
      ++it_reg_hist;
    }
    ++it_update_hist;
  }
}

} // namespace nest
