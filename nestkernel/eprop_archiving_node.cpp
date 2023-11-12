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
  // This function initializes the update history for synapses by ensuring there is an entry
  // at the start of the e-prop history timeline for each neuron. The 'shift' value determines
  // the starting point of this timeline, which varies based on whether the neuron is a readout
  // neuron or a recurrent neuron.
  // Important:
  // The total sum of 'access_counter_' values across all entries in 'update_history_' for a neuron
  // remains constant and is equal to the total number of incoming synapses to that neuron.
  // This constancy assumes that the network structure is static during the simulation,
  // meaning no synapses are added or removed after the initial setup.


  // Get 'shift' to determine the starting point of the e-prop history timeline.
  const long shift = get_shift();

  // Find the corresonding update history entry
  const auto it_hist = get_update_history( shift );

  // Check if there is an entry in the update history at the start of the timeline
  if ( it_hist == update_history_.end() or it_hist->t_ != shift )
  {
    // If no entry exists, create a new entry. Initialize the access counter
    // of this new entry to 1, indicating its first access.
    update_history_.insert( it_hist, HistEntryEpropUpdate( shift, 1 ) );
  }
  else
  {
    // If an entry already exists, increment its access counter. This increment
    // implies that a different synapse is registering this initial time point in the update history.
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
  // This function records the surrogate gradient into the e-prop history
  // This function is only called by recurrent neurons

  // Add a new entry to the e-prop history with the given time step and surrogate gradient,
  // learning signal is initialized to 0.0, as it will be updated separately
  eprop_history_.push_back( HistEntryEpropArchive( time_step, surrogate_gradient, 0.0 ) );
}

void
nest::EpropArchivingNode::write_error_signal_to_history( const long time_step, const double error_signal )
{
  // This function records the error signal into the e-prop history
  // This function is only called by readout neurons

  // Calculate the adjusted time step by subtracting the delay_out_norm_,
  // which accounts for the transmission time of the normalization signals
  const long shift = delay_out_norm_;

  // Add a new entry to the e-prop history with the adjusted time step and error signal
  // The surrogate gradient is initialized to 0.0 in this entry, as it is not needed
  eprop_history_.push_back( HistEntryEpropArchive( time_step - shift, 0.0, error_signal ) );
}

void
nest::EpropArchivingNode::write_learning_signal_to_history( const long time_step,
  const long delay_out_rec,
  const double learning_signal )
{
  // Calculate the total shift to adjust  'time_step'. This shift accounts for:
  // - delay_rec_out: The relative offset between the timelines of readout and recurrent neurons.
  // - delay_out_norm: The time corresponding to the transmission of normalization signals.
  // - delay_out_rec: The transmission time of the learning signal.
  // These 3 delays must be taken into account to place the learning signal in the correct location
  // in the e-prop history
  const long shift = delay_rec_out_ + delay_out_norm_ + delay_out_rec;

  // Obtain iterators to define a range in the e-prop history. This range corresponds to
  // the adjusted time step (time_step - shift) and spans over the transmission time of the learning signal
  auto it_hist = get_eprop_history( time_step - shift );
  const auto it_hist_end = get_eprop_history( time_step - shift + delay_out_rec );

  // Iterate over the e-prop history entries within this range
  for ( ; it_hist != it_hist_end; ++it_hist )
  {
    // Update the learning signal for each history entry within the range. In cases where multiple readout neurons
    // contribute to the learning signal, each neuron's contribution is added to the existing value. Hence,
    // the use of the '+=' operator to incrementally accumulate the learning signal
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
  // Returns the offset for the start of the e-prop history timeline. This is overridden
  // in derived classes based on neuron type (readout or recurrent)
  return 0;
}

std::vector< HistEntryEpropUpdate >::iterator
nest::EpropArchivingNode::get_update_history( const long time_step )
{
  // This function returns an iterator to the position in the update history that corresponds
  // to or is immediately after the specified time_step.
  return std::lower_bound( update_history_.begin(), update_history_.end(), time_step );
}

std::vector< HistEntryEpropArchive >::iterator
nest::EpropArchivingNode::get_eprop_history( const long time_step )
{
  // This function returns an iterator to the position in the e-prop history that corresponds
  // to or is immediately after the given time_step.
  return std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_step );
}

std::vector< HistEntryEpropFiringRateReg >::iterator
nest::EpropArchivingNode::get_firing_rate_reg_history( const long time_step )
{
  // This function returns an iterator to the position in the firing rate regularization history
  // that corresponds to or is immediately after the given time_step.
  return std::lower_bound( firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_step );
}

void
nest::EpropArchivingNode::erase_unneeded_update_history()
{
  // Iterate over the update history
  for ( auto it_hist = update_history_.begin(); it_hist != update_history_.end(); ++it_hist )
  {
    // The access_counter_ for each entry in update_history_ represents the number
    // of synapses that require information from that particular update time

    if ( it_hist->access_counter_ == 0 )
    {
      // If access_counter_ is 0, it implies that no synapses need information from this update time,
      // therefore the corresponding update history entry is no longer needed and can be removed
      // from the update history. This ensures update history only contains only
      // entries that are needed by at least one synapse
      update_history_.erase( it_hist );
    }
  }
}

void
nest::EpropArchivingNode::erase_unneeded_eprop_history()
{
  // Early exit if there is nothing to remove
  if ( eprop_history_.empty()  // nothing to remove
    or update_history_.empty() // no time markers to check
  )
  {
    return;
  }

  // Retrieve the update interval
  const long update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();

  // Iterate over time periods defined by the update interval in the update history
  for ( long t = update_history_.begin()->t_; t <= ( update_history_.end() - 1 )->t_; t += update_interval )
  {
    // Use std::find_if to check if an update history entry exists for the current time period
    // The lambda expression comperes each entry's time against the current time t
    auto it = std::find_if( update_history_.begin(),
      update_history_.end(),
      [ t ]( const HistEntryEpropUpdate& entry ) { return entry.t_ == t; } );

    // If no corresponding entry is found in the update histry for the current time,
    // it implies that the e-prop history entries for this time period are no longer needed
    if ( it == update_history_.end() )
    {
      // Get iterators difining the range of e-prop history entries to be removed
      auto it_eprop_hist_from = get_eprop_history( t );
      auto it_eprop_hist_to = get_eprop_history( t + update_interval );

      // Erase the e-prop history entries within the determined range
      eprop_history_.erase( it_eprop_hist_from, it_eprop_hist_to );
    }
  }

  // Additionally, clear e-prop history entries preceding the earliest entry in the update history
  eprop_history_.erase( get_eprop_history( 0 ), get_eprop_history( update_history_.begin()->t_ ) );
}


void
nest::EpropArchivingNode::erase_unneeded_firing_rate_reg_history()
{
  // Iterators for navigating through the update history and firing rate regularization history
  auto it_update_hist = update_history_.begin();
  auto it_reg_hist = firing_rate_reg_history_.begin();

  // Iterate simultaneously through update history and firing rate regularization history
  for ( ; it_update_hist != update_history_.end() and it_reg_hist != firing_rate_reg_history_.end();
        ++it_update_hist, ++it_reg_hist )
  {
    if ( it_update_hist->access_counter_ == 0 )
    {
      // If no synapses require updates for this time period (indicated by access_counter_ being 0),
      // the corresponding entry in the firing rate regularization history is no longer needed
      // and can be safely remove
      firing_rate_reg_history_.erase( it_reg_hist );
    }
  }
}

void
nest::EpropArchivingNode::count_spike()
{
  // Increments the count of spikes. This spike count is used in the calculation of the firing rate,
  // which is a component for firing rate regularization
  ++n_spikes_;
}

void
nest::EpropArchivingNode::reset_spike_count()
{
  // Resets the spike count to zero. This is called at the start of a new update interval and/or
  // when the neuron's state is reset
  n_spikes_ = 0;
}

} // namespace nest
