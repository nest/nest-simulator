/*
 *  axonal_delay_archiving_node.cpp
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

#include "axonal_delay_archiving_node.h"

// Includes from nestkernel:
#include "kernel_manager.h"

namespace nest
{

AxonalDelayArchivingNode::AxonalDelayArchivingNode()
  : StructuralPlasticityNode()
  , has_predominant_stdp_ax_delay_( false )
{
  const size_t num_time_slots =
    kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
  correction_entries_stdp_ax_delay_.resize( num_time_slots );
}

AxonalDelayArchivingNode::AxonalDelayArchivingNode( const AxonalDelayArchivingNode& n )
  : StructuralPlasticityNode( n )
  , has_predominant_stdp_ax_delay_( false )
{
  const size_t num_time_slots =
    kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
  correction_entries_stdp_ax_delay_.resize( num_time_slots );
}

void
AxonalDelayArchivingNode::pre_run_hook_()
{
  if ( has_predominant_stdp_ax_delay_ )
  {
    const size_t num_time_slots =
      kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay();
    if ( correction_entries_stdp_ax_delay_.size() != num_time_slots )
    {
      correction_entries_stdp_ax_delay_.resize( num_time_slots );
    }
  }
}

void
AxonalDelayArchivingNode::register_axonal_delay_connection( const double dendritic_delay, const double axonal_delay )
{
  if ( axonal_delay >= dendritic_delay )
  {
    has_predominant_stdp_ax_delay_ = true;
  }
}

void
AxonalDelayArchivingNode::add_correction_entry_stdp_ax_delay( SpikeEvent& spike_event,
  const double t_last_pre_spike,
  const double weight_revert,
  const double new_weight,
  const double K_plus_revert,
  const double time_while_critical )
{
  assert( correction_entries_stdp_ax_delay_.size()
    == static_cast< size_t >(
      kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay() ) );

  const long lag = static_cast< long >( time_while_critical - 1 );
  const long idx = kernel().event_delivery_manager.get_modulo( lag );
  assert( static_cast< size_t >( idx ) < correction_entries_stdp_ax_delay_.size() );

  const SpikeData& spike_data = spike_event.get_sender_spike_data();
  const size_t lcid = spike_data.get_lcid();

  // The slot of a spike is stamp + axonal - dendritic - 1 relative to the slice origin, a pure function
  // of its own emission time, so the slot of the previous spike on this connection is this spike's slot
  // minus the interval between the two.  A negative result means that slot has already been cleared:
  // slots are only cleared by reset_correction_entries_stdp_ax_delay_() from update(), which runs after
  // delivery, so every slot from the current origin onwards is still live.
  const long offset_to_previous = Time::delay_ms_to_steps( spike_event.get_stamp().get_ms() - t_last_pre_spike );
  const long previous_lag = lag - offset_to_previous;
  if ( previous_lag >= 0 )
  {
    const size_t previous_idx = kernel().event_delivery_manager.get_modulo( previous_lag );
    for ( CorrectionEntrySTDPAxDelay& entry : correction_entries_stdp_ax_delay_[ previous_idx ] )
    {
      if ( entry.lcid_ == lcid )
      {
        entry.next_offset_ = offset_to_previous;
        break;
      }
    }
  }

  correction_entries_stdp_ax_delay_[ idx ].push_back( CorrectionEntrySTDPAxDelay(
    lcid, spike_data.get_syn_id(), t_last_pre_spike, weight_revert, new_weight, K_plus_revert ) );
}

void
AxonalDelayArchivingNode::reset_correction_entries_stdp_ax_delay_( const size_t lag )
{
  if ( has_predominant_stdp_ax_delay_ )
  {
    assert( correction_entries_stdp_ax_delay_.size()
      == static_cast< size_t >(
        kernel().connection_manager.get_min_delay() + kernel().connection_manager.get_max_delay() ) );

    const size_t idx = kernel().event_delivery_manager.get_modulo( lag );
    assert( static_cast< size_t >( idx ) < correction_entries_stdp_ax_delay_.size() );

    // iterate over all pre-synaptic spikes which are no longer critical
    for ( CorrectionEntrySTDPAxDelay& it_corr_entry : correction_entries_stdp_ax_delay_[ idx ] )
    {
      // This entry is final, so the next spike on the same connection has to revert to the weight it
      // ends up with.  next_offset_ is an interval, so it stays valid however often the buffer rotated
      // between the two spikes being delivered.
      if ( it_corr_entry.next_offset_ != -1 )
      {
        const size_t next_idx =
          kernel().event_delivery_manager.get_modulo( static_cast< long >( lag ) + it_corr_entry.next_offset_ );
        for ( CorrectionEntrySTDPAxDelay& next_entry : correction_entries_stdp_ax_delay_[ next_idx ] )
        {
          if ( next_entry.lcid_ == it_corr_entry.lcid_ )
          {
            next_entry.weight_revert_ = it_corr_entry.new_weight_;
            break;
          }
        }
      }
    }

    correction_entries_stdp_ax_delay_[ idx ].clear();
  }
}

void
AxonalDelayArchivingNode::update_weight_revert( const size_t lcid, const double weight_revert )
{
  for ( std::vector< CorrectionEntrySTDPAxDelay >& correction_entries_for_timestep : correction_entries_stdp_ax_delay_ )
  {
    for ( CorrectionEntrySTDPAxDelay& it_corr_entry : correction_entries_for_timestep )
    {
      if ( it_corr_entry.lcid_ == lcid )
      {
        it_corr_entry.weight_revert_ = weight_revert;
      }
    }
  }
}

size_t
AxonalDelayArchivingNode::correct_synapses_stdp_ax_delay_( const Time& t_spike )
{
  size_t num_corrections = 0;

  if ( has_predominant_stdp_ax_delay_ )
  {
    const Time& ori = kernel().simulation_manager.get_slice_origin();
    const Time& t_spike_rel = t_spike - ori;
    const long maxdelay_steps = kernel().connection_manager.get_max_delay();
    assert( correction_entries_stdp_ax_delay_.size()
      == static_cast< size_t >( kernel().connection_manager.get_min_delay() + maxdelay_steps ) );

    for ( long lag = t_spike_rel.get_steps() - 1; lag < maxdelay_steps + 1; ++lag )
    {
      const long idx = kernel().event_delivery_manager.get_modulo( lag );
      assert( static_cast< size_t >( idx ) < correction_entries_stdp_ax_delay_.size() );

      for ( CorrectionEntrySTDPAxDelay& it_corr_entry : correction_entries_stdp_ax_delay_[ idx ] )
      {
        kernel().connection_manager.correct_synapse_stdp_ax_delay( get_thread(),
          it_corr_entry.syn_id_,
          it_corr_entry.lcid_,
          it_corr_entry.t_last_pre_spike_,
          ( ori + Time::step( lag + 1 ) ).get_ms(),
          it_corr_entry.weight_revert_,
          it_corr_entry.new_weight_,
          it_corr_entry.K_plus_revert_,
          t_spike.get_ms() );
      }
      num_corrections += correction_entries_stdp_ax_delay_[ idx ].size();
    }
  }

  return num_corrections;
}

}  // of namespace nest
