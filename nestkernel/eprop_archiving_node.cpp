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
#include "eprop_archiving_node_impl.h"
#include "kernel_manager.h"

// sli
#include "dictutils.h"

namespace nest
{

EpropArchivingNodeRecurrent::EpropArchivingNodeRecurrent()
  : EpropArchivingNode()
  , n_spikes_( 0 )
{
}

EpropArchivingNodeRecurrent::EpropArchivingNodeRecurrent( const EpropArchivingNodeRecurrent& n )
  : EpropArchivingNode( n )
  , n_spikes_( n.n_spikes_ )
{
}

void
EpropArchivingNodeRecurrent::write_surrogate_gradient_to_history( const long time_step,
  const double surrogate_gradient )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  eprop_history_.emplace_back( time_step, surrogate_gradient, 0.0 );
}

void
EpropArchivingNodeRecurrent::write_learning_signal_to_history( const long time_step, const double learning_signal )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const long shift = delay_rec_out_ + delay_out_norm_ + delay_out_rec_;

  auto it_hist = get_eprop_history( time_step - shift );
  const auto it_hist_end = get_eprop_history( time_step - shift + delay_out_rec_ );

  for ( ; it_hist != it_hist_end; ++it_hist )
  {
    it_hist->learning_signal_ += learning_signal;
  }
}

void
EpropArchivingNodeRecurrent::write_firing_rate_reg_to_history( const long t_current_update,
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

  firing_rate_reg_history_.emplace_back( t_current_update + shift, firing_rate_reg );
}

std::vector< HistEntryEpropFiringRateReg >::iterator
EpropArchivingNodeRecurrent::get_firing_rate_reg_history( const long time_step )
{
  const auto it_hist = std::lower_bound( firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_step );
  assert( it_hist != firing_rate_reg_history_.end() );

  return it_hist;
}

double
EpropArchivingNodeRecurrent::get_learning_signal_from_history( const long time_step )
{
  const long shift = delay_rec_out_ + delay_out_norm_ + delay_out_rec_;

  const auto it = get_eprop_history( time_step - shift );
  if ( it == eprop_history_.end() )
  {
    return 0;
  }

  return it->learning_signal_;
}

void
EpropArchivingNodeRecurrent::erase_used_firing_rate_reg_history()
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

EpropArchivingNodeReadout::EpropArchivingNodeReadout()
  : EpropArchivingNode()
{
}

EpropArchivingNodeReadout::EpropArchivingNodeReadout( const EpropArchivingNodeReadout& n )
  : EpropArchivingNode( n )
{
}

void
EpropArchivingNodeReadout::write_error_signal_to_history( const long time_step, const double error_signal )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const long shift = delay_out_norm_;

  eprop_history_.emplace_back( time_step - shift, error_signal );
}


} // namespace nest
