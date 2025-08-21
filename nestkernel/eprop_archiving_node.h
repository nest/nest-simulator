/*
 *  eprop_archiving_node.h
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

#ifndef EPROP_ARCHIVING_NODE_H
#define EPROP_ARCHIVING_NODE_H

// nestkernel
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

// sli
#include "dictdatum.h"

namespace nest
{
/**
 * @brief Base class implementing archiving for node models supporting e-prop plasticity.
 *
 * Base class implementing an intermediate archiving node model for node models supporting e-prop plasticity
 * according to Bellec et al. (2020) and supporting additional biological features described in Korcsak-Gorzo,
 * Stapmanns, and Espinoza Valverde et al. (in preparation).
 *
 * A node which archives the history of dynamic variables, the firing rate
 * regularization, and update times needed to calculate the weight updates for
 * e-prop plasticity. It further provides a set of get, write, and set functions
 * for these histories and the hardcoded shifts to synchronize the factors of
 * the plasticity rule.
 *
 * @tparam HistEntryT The type of history entry.
 */
template < typename HistEntryT >
class EpropArchivingNode : public Node
{

public:
  /**
   * Constructs a new EpropArchivingNode object.
   */
  EpropArchivingNode();

  /**
   * Constructs a new EpropArchivingNode object by copying another EpropArchivingNode object.
   *
   * @param other The other object to copy.
   */
  EpropArchivingNode( const EpropArchivingNode& other );

  void register_eprop_connection() override;

  void write_update_to_history( const long t_previous_update,
    const long t_current_update,
    const long eprop_isi_trace_cutoff = 0 ) override;

  /**
   * Retrieves the update history entry for a specific time step.
   *
   * @param time_step The time step.
   * @return An iterator pointing to the update history for the specified time step.
   */
  std::vector< HistEntryEpropUpdate >::iterator get_update_history( const long time_step );

  /**
   * Retrieves the eprop history entry for a specified time step.
   *
   * @param time_step The time step.
   * @return An iterator pointing to the eprop history entry for the specified time step.
   */
  typename std::vector< HistEntryT >::iterator get_eprop_history( const long time_step );

  /**
   * @brief Erases the used eprop history for `bsshslm_2020` models.
   *
   * Erases e-prop history entries for update intervals during which no spikes were sent to the target neuron,
   * and any entries older than the earliest time stamp required by the first update in the history.
   */
  void erase_used_eprop_history();

  /**
   * @brief Erases the used eprop history.
   *
   * Erases e-prop history entries between the last and penultimate updates if they exceed the inter-spike
   * interval trace cutoff and any entries older than the earliest time stamp required by the first update.
   *
   * @param eprop_isi_trace_cutoff The cutoff value for the inter-spike integration of the eprop trace.
   */
  void erase_used_eprop_history( const long eprop_isi_trace_cutoff );

  /**
   * @brief Retrieves eprop history size.
   *
   * Retrieves the size of the eprop history buffer.
   */
  double get_eprop_history_duration() const;

protected:
  //! Returns correct shift for history depending on whether it is a normal or a bsshslm_2020 model.
  virtual long model_dependent_history_shift_() const = 0;

  /**
   * Provides access to the template flag from the base class.
   *
   * @todo This should be removed once we revise the class structure.
   */
  virtual bool history_shift_required_() const = 0;

  //! Number of incoming eprop synapses
  size_t eprop_indegree_;

  //! History of updates still needed by at least one synapse.
  std::vector< HistEntryEpropUpdate > update_history_;

  //! History of dynamic variables needed for e-prop plasticity.
  std::vector< HistEntryT > eprop_history_;

  // The following shifts are, for now, hardcoded to 1 time step since the current
  // implementation only works if all the delays are equal to the simulation resolution.

  //! Offset since generator signals start from time step 1.
  const long offset_gen_ = 1;

  //! Transmission delay from input to recurrent neurons.
  const long delay_in_rec_ = 1;

  //! Transmission delay from recurrent to output neurons.
  const long delay_rec_out_ = 1;

  //! Transmission delay between output neurons for normalization.
  const long delay_out_norm_ = 1;

  //! Transmission delay from output neurons to recurrent neurons.
  const long delay_out_rec_ = 1;
};

template < typename HistEntryT >
EpropArchivingNode< HistEntryT >::EpropArchivingNode()
  : Node()
  , eprop_indegree_( 0 )
{
}

template < typename HistEntryT >
EpropArchivingNode< HistEntryT >::EpropArchivingNode( const EpropArchivingNode& n )
  : Node( n )
  , eprop_indegree_( n.eprop_indegree_ )
{
}

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

#endif // EPROP_ARCHIVING_NODE_H
