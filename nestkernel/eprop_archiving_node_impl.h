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

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

template < typename HistEntryT >
EpropArchivingNode< HistEntryT >::EpropArchivingNode()
  : Node()
  , FlushEventMechanism()
  , eprop_indegree_( 0 )
  , eprop_isi_trace_cutoff_( 1000.0 )
{
}

template < typename HistEntryT >
EpropArchivingNode< HistEntryT >::EpropArchivingNode( const EpropArchivingNode& n )
  : Node( n )
  , FlushEventMechanism( n )
  , eprop_indegree_( n.eprop_indegree_ )
  , eprop_isi_trace_cutoff_( n.eprop_isi_trace_cutoff_ )
{
}

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::register_eprop_connection()
{
  ++eprop_indegree_;
}

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::initialize_update_history()
{
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
  const bool is_flush_event,
  const bool previous_was_flush_event )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const long shift = model_dependent_history_shift_();
  const long t_curr_update_shifted = t_current_update + shift;
  const long t_prev_update_shifted = t_previous_update + shift;

  if ( not is_flush_event )
  {
    auto it_hist_curr = get_update_history( t_curr_update_shifted );

    if ( it_hist_curr != update_history_.end() and it_hist_curr->t_ == t_curr_update_shifted )
    {
      ++it_hist_curr->access_counter_;
    }
    else
    {
      update_history_.insert( it_hist_curr, HistEntryEpropUpdate( t_curr_update_shifted, 1 ) );
    }
  }

  if ( not previous_was_flush_event )
  {
    auto it_hist_prev = get_update_history( t_prev_update_shifted );

    if ( it_hist_prev != update_history_.end() and it_hist_prev->t_ == t_prev_update_shifted )
    {
      // If an entry exists for the previous update time, decrement its access counter
      --it_hist_prev->access_counter_;
      if ( it_hist_prev->access_counter_ == 0 )
      {
        update_history_.erase( it_hist_prev );
      }
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
      // erase no longer needed entries for update intervals with no spikes sent to the target neuron
      eprop_history_.erase( get_eprop_history( t ), get_eprop_history( t + update_interval ) );
    }
  }
  // erase no longer needed entries before the earliest current update
  eprop_history_.erase( get_eprop_history( 0 ), get_eprop_history( update_history_.begin()->t_ ) );
}

template < typename HistEntryT >
void
EpropArchivingNode< HistEntryT >::erase_used_eprop_history( const long t_spike, const long t_spike_previous )
{
  auto it_hist = get_update_history( t_spike );

  if ( it_hist != update_history_.end() and it_hist->t_ == t_spike )
  {
    ++it_hist->access_counter_;
  }
  else
  {
    update_history_.emplace( it_hist, t_spike, 1 );
  }

  if ( t_spike_previous > 0 )
  {
    auto it_hist_prev = get_update_history( t_spike_previous );

    if ( it_hist_prev != update_history_.end() and it_hist_prev->t_ == t_spike_previous )
    {
      if ( it_hist_prev->access_counter_ > 0 )
      {
        --it_hist_prev->access_counter_;
      }
      if ( it_hist_prev->access_counter_ == 0 )
      {
        update_history_.erase( it_hist_prev );
      }
    }

    const long cutoff = get_eprop_isi_trace_cutoff();

    const long erase_candidate_begin = t_spike_previous - 1;
    const long erase_candidate_end = erase_candidate_begin + cutoff;

    long erase_begin = erase_candidate_begin;
    long erase_end = erase_candidate_end;

    const long search_begin = std::max( 0L, erase_candidate_begin - cutoff );
    const long search_end = erase_candidate_end;

    auto it_search_begin = get_update_history( search_begin );
    auto it_search_end = std::lower_bound( it_search_begin, update_history_.end(), search_end + 1 );

    for ( auto it = it_search_begin; it != it_search_end and erase_begin < erase_end; ++it )
    {
      const long required_begin = it->t_ - 1;
      const long required_end = required_begin + cutoff;

      if ( required_begin >= erase_candidate_begin and required_begin <= erase_candidate_end )
      {
        erase_end = std::min( erase_end, required_begin );
      }
      if ( required_end >= erase_candidate_begin and required_end <= erase_candidate_end )
      {
        erase_begin = std::max( erase_begin, required_end );
      }
    }

    const auto it_erase_begin = get_eprop_history( erase_begin );
    const auto it_erase_end = get_eprop_history( erase_end );
    if ( it_erase_begin < it_erase_end )
    {
      eprop_history_.erase( it_erase_begin, it_erase_end );
    }
  }

  if ( update_history_.empty() or eprop_history_.empty() )
  {
    return;
  }

  const long time_keep = update_history_.front().t_ - 1;
  auto it_keep = get_eprop_history( time_keep );

  if ( it_keep != eprop_history_.end() and it_keep->t_ == time_keep and it_keep != eprop_history_.begin() )
  {
    eprop_history_.erase( eprop_history_.begin(), it_keep );
  }
}

template < typename HistEntryT >
inline double
EpropArchivingNode< HistEntryT >::get_eprop_history_duration() const
{
  return Time::get_resolution().get_ms() * eprop_history_.size();
}

template < typename HistEntryT >
inline void
EpropArchivingNode< HistEntryT >::get_status( DictionaryDatum& d ) const
{
  if ( get_name().find( "bsshslm_2020" ) == std::string::npos )
  {
    def< double >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_ );
  }

  if ( get_name().find( "readout" ) == std::string::npos )
  {
    def< double >( d, names::flush_event_send_interval, flush_event_send_interval_ );
  }
}

template < typename HistEntryT >
inline void
EpropArchivingNode< HistEntryT >::set_status( const DictionaryDatum& d )
{
  const std::string node_name = get_name();
  const bool is_readout = node_name.find( "readout" ) != std::string::npos;
  const bool is_bsshslm_2020 = node_name.find( "bsshslm_2020" ) != std::string::npos;

  if ( not is_readout )
  {
    FlushEventMechanism::set_status( d );
  }

  if ( not is_bsshslm_2020 )
  {
    updateValue< double >( d, names::eprop_isi_trace_cutoff, eprop_isi_trace_cutoff_ );

    if ( eprop_isi_trace_cutoff_ < 0.0 )
    {
      throw BadProperty( "Computation cutoff of eprop trace eprop_isi_trace_cutoff ≥ 0 required." );
    }

    if ( not is_readout and flush_event_send_interval_ < eprop_isi_trace_cutoff_ )
    {
      throw BadProperty(
        "Interval since previous event after which a flush event is sent flush_event_send_interval ≥ "
        "eprop_isi_trace_cutoff required." );
    }
  }
  else
  {
    if ( not is_readout
      and flush_event_send_interval_ < kernel().simulation_manager.get_eprop_update_interval().get_ms() )
    {
      throw BadProperty(
        "Interval since previous event after which a flush event is sent flush_event_send_interval ≥ "
        "eprop_update_interval required." );
    }
  }
}

} // namespace nest

#endif // EPROP_ARCHIVING_NODE_IMPL_H
