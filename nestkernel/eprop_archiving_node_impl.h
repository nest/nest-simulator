/*
 *  eprop_archiving_node_impl.h
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

#ifndef EPROP_ARCHIVING_NODE_IMPL_H
#define EPROP_ARCHIVING_NODE_IMPL_H

#include "eprop_archiving_node.h"

namespace nest
{

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::register_eprop_connection()
{
  ++eprop_indegree_;

  const long t_first_entry = model_dependent_history_shift_();

  const auto it_hist = get_update_history( t_first_entry );

  if ( it_hist == update_history_.end() or it_hist->t_ != t_first_entry )
  {
    update_history_.insert( it_hist, HistEntryEpropUpdate( t_first_entry, 1 ) );
  }
  else
  {
    ++it_hist->access_counter_;
  }
}

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::write_update_to_history( const long t_previous_update,
  const long t_current_update,
  const long eprop_isi_trace_cutoff )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const long shift = model_dependent_history_shift_();

  const auto it_hist_curr = get_update_history( t_current_update + shift );

  if ( it_hist_curr != update_history_.end() and it_hist_curr->t_ == t_current_update + shift )
  {
    ++it_hist_curr->access_counter_;
  }
  else
  {
    update_history_.insert( it_hist_curr, HistEntryEpropUpdate( t_current_update + shift, 1 ) );

    if ( not history_shift_required_() )
    {
      erase_used_eprop_history( eprop_isi_trace_cutoff );
    }
  }

  const auto it_hist_prev = get_update_history( t_previous_update + shift );

  if ( it_hist_prev != update_history_.end() and it_hist_prev->t_ == t_previous_update + shift )
  {
    // If an entry exists for the previous update time, decrement its access counter
    --it_hist_prev->access_counter_;
    if ( it_hist_prev->access_counter_ == 0 )
    {
      update_history_.erase( it_hist_prev );
    }
  }
}

template < typename HistEntryT >
std::vector< HistEntryEpropUpdate >::iterator
EpropArchivingNode< HistEntryT >::get_update_history( const long time_step )
{
  return std::lower_bound( update_history_.begin(), update_history_.end(), time_step );
}

template < typename HistEntryT >
typename std::vector< HistEntryT >::iterator
EpropArchivingNode< HistEntryT >::get_eprop_history( const long time_step )
{
  return std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_step );
}

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::erase_used_eprop_history()
{
  if ( eprop_history_.empty()  // nothing to remove
    or update_history_.empty() // no time markers to check
  )
  {
    return;
  }

  const long update_interval = kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps();

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
      // erase no longer needed entries for update intervals with no spikes sent to the target neuron
      eprop_history_.erase( get_eprop_history( t ), get_eprop_history( t + update_interval ) );
    }
  }
  // erase no longer needed entries before the earliest current update
  eprop_history_.erase( get_eprop_history( 0 ), get_eprop_history( update_history_.begin()->t_ ) );
}

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::erase_used_eprop_history( const long eprop_isi_trace_cutoff )
{
  if ( eprop_history_.empty()     // nothing to remove
    or update_history_.size() < 2 // no time markers to check
  )
  {
    return;
  }

  const long t_prev = ( update_history_.end() - 2 )->t_;
  const long t_curr = ( update_history_.end() - 1 )->t_;

  if ( t_prev + eprop_isi_trace_cutoff < t_curr )
  {
    // erase no longer needed entries to be ignored by trace cutoff
    eprop_history_.erase( get_eprop_history( t_prev + eprop_isi_trace_cutoff ), get_eprop_history( t_curr ) );
  }

  // erase no longer needed entries before the earliest current update
  eprop_history_.erase(
    get_eprop_history( std::numeric_limits< long >::min() ), get_eprop_history( update_history_.begin()->t_ - 1 ) );
}

template < typename HistEntryT >
inline double
EpropArchivingNode< HistEntryT >::get_eprop_history_duration() const
{
  return Time::get_resolution().get_ms() * eprop_history_.size();
}

} // namespace nest

#endif // EPROP_ARCHIVING_NODE_IMPL_H
