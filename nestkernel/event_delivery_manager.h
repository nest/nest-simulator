/*
 *  event_delivery_manager.h
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

#ifndef EVENT_DELIVERY_MANAGER_H
#define EVENT_DELIVERY_MANAGER_H

// C++ includes:
#include <cassert>
#include <limits>
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch.h"

// Includes from nestkernel:
#include "event.h"
#include "mpi_manager.h" // OffGridSpike
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"
#include "per_thread_bool_indicator.h"
#include "target_table.h"
#include "spike_data.h"
#include "vp_manager.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
typedef MPIManager::OffGridSpike OffGridSpike;

class TargetData;
class SendBufferPosition;

class EventDeliveryManager : public ManagerInterface
{
public:
  EventDeliveryManager();
  virtual ~EventDeliveryManager();

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
   * Standard routine for sending events. This method decides if
   * the event has to be delivered locally or globally. It exists
   * to keep a clean and unitary interface for the event sending
   * mechanism.
   * @note Only specialization for SpikeEvent does remote sending.
   *       Specialized for DSSpikeEvent to avoid that these events
   *       are sent to remote processes.
   * \see send_local()
   */
  template < class EventT >
  void send( Node& source, EventT& e, const long lag = 0 );

  /**
   * Send a secondary event remote.
   */
  void send_secondary( Node& source, SecondaryEvent& e );

  /**
   * Send event e to all targets of node source on thread t
   */
  void send_local( thread t, Node& source, Event& e );

  /**
   * Add node ID of event sender to the spike_register.
   * An event sent through this method will remain in the queue until
   * the network time has advanced by min_delay_ steps. After this period
   * the buffers are collocated and sent to the partner machines.

   * Old documentation from network.h:
   * Place an event in the global event queue.
   * Add event to the queue to be delivered
   * when it is due.
   * At the delivery time, the target list of the sender is iterated
   * and the event is delivered to all targets.
   * The event is guaranteed to arrive at the receiver when all
   * elements are updated and the system is
   * in a synchronised (single threaded) state.
   * @see send_to_targets()
   */
  void send_remote( thread tid, SpikeEvent&, const long lag = 0 );

  /**
   * Add node ID of event sender to the spike_register.
   * Store event offset with node ID.
   * An event sent through this method will remain in the queue until
   * the network time has advanced by min_delay_ steps. After this period
   * the buffers are collocated and sent to the partner machines.

   * Old documentation from network.h:
   * Place an event in the global event queue.
   * Add event to the queue to be delivered
   * when it is due.
   * At the delivery time, the target list of the sender is iterated
   * and the event is delivered to all targets.
   * The event is guaranteed to arrive at the receiver when all
   * elements are updated and the system is
   * in a synchronised (single threaded) state.
   * @see send_to_targets()
   */
  void send_off_grid_remote( thread tid, SpikeEvent& e, const long lag = 0 );

  /**
   * Send event e directly to its target node. This should be
   * used only where necessary, e.g. if a node wants to reply
   * to a *RequestEvent immediately.
   */
  void send_to_node( Event& e );

  /**
   * return current communication style.
   * A result of true means off_grid, false means on_grid communication.
   */
  bool get_off_grid_communication() const;

  /**
   * set communication style to off_grid (true) or on_grid
   */
  void set_off_grid_communication( bool off_grid_spiking );

  /**
   * Return 0 for even, 1 for odd time slices.
   *
   * This is useful for buffers that need to be written alternatingly
   * by time slice. The value is given by get_slice_() % 2.
   * @see read_toggle
   */
  size_t write_toggle() const;

  /**
   * Return 1 - write_toggle().
   *
   * This is useful for buffers that need to be read alternatingly
   * by slice. The value is given by 1-write_toggle().
   * @see write_toggle
   */
  size_t read_toggle() const;

  /**
   * Return (T+d) mod max_delay.
   */
  delay get_modulo( delay d );


  /**
   * Index to slice-based buffer.
   * Return ((T+d)/min_delay) % ceil(max_delay/min_delay).
   */
  delay get_slice_modulo( delay d );

  /**
   * Resize spike_register and comm_buffer to correct dimensions.
   * Resizes also off_grid_*_buffer_.
   * This is done by simulate() when called for the first time.
   * The spike buffers cannot be reconfigured later, whence neither
   * the number of local threads or the min_delay can change after
   * simulate() has been called. ConnectorModel::check_delay() and
   * Network::set_status() ensure this.
   */
  void configure_spike_data_buffers();

  void configure_spike_register();

  void resize_send_recv_buffers_target_data();

  void configure_secondary_buffers();

  /**
   * Collocates spikes from register to MPI buffers, communicates via
   * MPI and delivers events to targets.
   */
  void gather_spike_data( const thread tid );

  /**
   * Collocates presynaptic connection information, communicates via
   * MPI and creates presynaptic connection infrastructure.
   */
  void gather_target_data( const thread tid );

  /**
   * Collocates presynaptic connection information for secondary events (MPI
   * buffer offsets), communicates via MPI and create presynaptic connection
   * infrastructure for secondary events.
   */
  void gather_secondary_target_data();

  void write_done_marker_secondary_events_( const bool done );

  void gather_secondary_events( const bool done );

  bool deliver_secondary_events( const thread tid, const bool called_from_wfr_update );

  /**
   * Update table of fixed modulos, including slice-based.
   */
  void update_moduli();

  /**
   * Initialize modulo table.
   *
   * TODO: can probably be private
   */
  void init_moduli();

  /**
   * Set local spike counter to zero.
   */
  virtual void reset_counters();

  /**
   * Set time measurements for internal profiling to zero (reg. prep.)
   */
  virtual void reset_timers_for_preparation();

  /**
   * Set time measurements for internal profiling to zero (reg. sim. dyn.)
   */
  virtual void reset_timers_for_dynamics();

private:
  template < typename SpikeDataT >
  void gather_spike_data_( const thread tid,
    std::vector< SpikeDataT >& send_buffer,
    std::vector< SpikeDataT >& recv_buffer );

  void resize_send_recv_buffers_spike_data_();

  /**
   * Moves spikes from on grid and off grid spike registers to correct
   * locations in MPI buffers.
   */
  template < typename TargetT, typename SpikeDataT >
  bool collocate_spike_data_buffers_( const thread tid,
    const AssignedRanks& assigned_ranks,
    SendBufferPosition& send_buffer_position,
    std::vector< std::vector< std::vector< std::vector< TargetT > > > >& spike_register,
    std::vector< SpikeDataT >& send_buffer );

  /**
   * Marks end of valid regions in MPI buffers.
   */
  template < typename SpikeDataT >
  void set_end_and_invalid_markers_( const AssignedRanks& assigned_ranks,
    const SendBufferPosition& send_buffer_position,
    std::vector< SpikeDataT >& send_buffer );

  /**
   * Resets marker in MPI buffer that signals end of communication
   * across MPI ranks.
   */
  template < typename SpikeDataT >
  void reset_complete_marker_spike_data_( const AssignedRanks& assigned_ranks,
    const SendBufferPosition& send_buffer_position,
    std::vector< SpikeDataT >& send_buffer ) const;

  /**
   * Sets marker in MPI buffer that signals end of communication
   * across MPI ranks.
   */
  template < typename SpikeDataT >
  void set_complete_marker_spike_data_( const AssignedRanks& assigned_ranks,
    const SendBufferPosition& send_buffer_position,
    std::vector< SpikeDataT >& send_buffer ) const;

  /**
   * Reads spikes from MPI buffers and delivers them to ringbuffer of
   * nodes.
   */
  template < typename SpikeDataT >
  bool deliver_events_( const thread tid, const std::vector< SpikeDataT >& recv_buffer );

  /**
   * Deletes all spikes from spike registers and resets spike
   * counters.
   */
  void reset_spike_register_( const thread tid );

  /**
   * Resizes spike registers according minimal delay so it can
   * accommodate all possible lags.
   */
  void resize_spike_register_( const thread tid );

  /**
   * Returns true if spike has been moved to MPI buffer, such that it
   * can be removed by clean_spike_register. Required static function
   * by std::remove_if.
   */
  static bool is_marked_for_removal_( const Target& target );

  /**
   * Removes spikes that were successfully moved to MPI buffers from
   * spike register, such that they are not considered in (potential)
   * next communication round.
   */
  void clean_spike_register_( const thread tid );

  /**
   * Fills MPI buffer for communication of connection information from
   * presynaptic to postsynaptic side. Builds TargetData objects from
   * SourceTable and connections information.
   */
  bool collocate_target_data_buffers_( const thread tid,
    const AssignedRanks& assigned_ranks,
    SendBufferPosition& send_buffer_position );

  /**
   * Sets marker in MPI buffer that signals end of communication
   * across MPI ranks.
   */
  void set_complete_marker_target_data_( const AssignedRanks& assigned_ranks,
    const SendBufferPosition& send_buffer_position );

  /**
   * Reads TargetData objects from MPI buffers and creates Target
   * objects on TargetTable (presynaptic part of connection
   * infrastructure).
   */
  bool distribute_target_data_buffers_( const thread tid );

  /**
   * Sends event e to all targets of node source. Delivers events from
   * devices directly to targets.
   */
  template < class EventT >
  void send_local_( Node& source, EventT& e, const long lag );
  void send_local_( Node& source, SecondaryEvent& e, const long lag );

  //--------------------------------------------------//

  bool off_grid_spiking_; //!< indicates whether spikes are not constrained to
                          //!< the grid

  /**
   * Table of pre-computed modulos.
   * This table is used to map time steps, given as offset from now,
   * to ring-buffer bins.  There are min_delay+max_delay bins in a ring buffer,
   * and the moduli_ array is rotated by min_delay elements after
   * each slice is completed.
   * @see RingBuffer
   */
  std::vector< delay > moduli_;

  /**
   * Table of pre-computed slice-based modulos.
   * This table is used to map time steps, give as offset from now,
   * to slice-based ring-buffer bins.  There are ceil(max_delay/min_delay)
   * bins in a slice-based ring buffer, one per slice within max_delay.
   * Since max_delay may not be a multiple of min_delay, we cannot simply
   * rotate the table content after each slice, but have to recompute
   * the table anew.
   * @see SliceRingBuffer
   */
  std::vector< delay > slice_moduli_;

  /**
   * Register for node IDs of neurons that spiked. This is a 4-dim
   * structure. While spikes are written to the buffer they are
   * immediately sorted by the thread that will later move the spikes to the
   * MPI buffers.
   * - First dim: write threads (from node to register)
   * - Second dim: read threads (from register to MPI buffer)
   * - Third dim: lag
   * - Fourth dim: Target (will be converted in SpikeData)
   */
  std::vector< std::vector< std::vector< std::vector< Target > > > > spike_register_;

  /**
   * Register for node IDs of precise neurons that spiked. This is a 4-dim
   * structure. While spikes are written to the buffer they are
   * immediately sorted by the thread that will later move the spikes to the
   * MPI buffers.
   * - First dim: write threads (from node to register)
   * - Second dim: read threads (from register to MPI buffer)
   * - Third dim: lag
   * - Fourth dim: OffGridTarget (will be converted in OffGridSpikeData)
   */
  std::vector< std::vector< std::vector< std::vector< OffGridTarget > > > > off_grid_spike_register_;

  /**
   * Buffer to collect the secondary events
   * after serialization.
   */
  std::vector< unsigned int > send_buffer_secondary_events_;
  std::vector< unsigned int > recv_buffer_secondary_events_;

  /**
   * Number of generated spike events (both off- and on-grid) during the last
   * call to simulate.
   */
  std::vector< unsigned long > local_spike_counter_;

  std::vector< SpikeData > send_buffer_spike_data_;
  std::vector< SpikeData > recv_buffer_spike_data_;
  std::vector< OffGridSpikeData > send_buffer_off_grid_spike_data_;
  std::vector< OffGridSpikeData > recv_buffer_off_grid_spike_data_;

  std::vector< TargetData > send_buffer_target_data_;
  std::vector< TargetData > recv_buffer_target_data_;
  //!< whether size of MPI buffer for communication of connections was changed
  bool buffer_size_target_data_has_changed_;
  //!< whether size of MPI buffer for communication of spikes was changed
  bool buffer_size_spike_data_has_changed_;
  //!< whether size of MPI buffer for communication of spikes can be decreased
  bool decrease_buffer_size_spike_data_;

  PerThreadBoolIndicator gather_completed_checker_;

#ifdef TIMER_DETAILED
  // private stop watches for benchmarking purposes
  // (intended for internal core developers, not for use in the public API)
  Stopwatch sw_collocate_spike_data_;
  Stopwatch sw_communicate_spike_data_;
  Stopwatch sw_deliver_spike_data_;
  Stopwatch sw_communicate_target_data_;
#endif
};

inline void
EventDeliveryManager::reset_spike_register_( const thread tid )
{
  for ( std::vector< std::vector< std::vector< Target > > >::iterator it = spike_register_[ tid ].begin();
        it < spike_register_[ tid ].end();
        ++it )
  {
    for ( std::vector< std::vector< Target > >::iterator iit = it->begin(); iit < it->end(); ++iit )
    {
      ( *iit ).clear();
    }
  }

  for (
    std::vector< std::vector< std::vector< OffGridTarget > > >::iterator it = off_grid_spike_register_[ tid ].begin();
    it < off_grid_spike_register_[ tid ].end();
    ++it )
  {
    for ( std::vector< std::vector< OffGridTarget > >::iterator iit = it->begin(); iit < it->end(); ++iit )
    {
      iit->clear();
    }
  }
}

inline bool
EventDeliveryManager::is_marked_for_removal_( const Target& target )
{
  return target.is_processed();
}

inline void
EventDeliveryManager::clean_spike_register_( const thread tid )
{
  for ( std::vector< std::vector< std::vector< Target > > >::iterator it = spike_register_[ tid ].begin();
        it < spike_register_[ tid ].end();
        ++it )
  {
    for ( std::vector< std::vector< Target > >::iterator iit = it->begin(); iit < it->end(); ++iit )
    {
      std::vector< Target >::iterator new_end = std::remove_if( iit->begin(), iit->end(), is_marked_for_removal_ );
      iit->erase( new_end, iit->end() );
    }
  }
  for (
    std::vector< std::vector< std::vector< OffGridTarget > > >::iterator it = off_grid_spike_register_[ tid ].begin();
    it < off_grid_spike_register_[ tid ].end();
    ++it )
  {
    for ( std::vector< std::vector< OffGridTarget > >::iterator iit = it->begin(); iit < it->end(); ++iit )
    {
      std::vector< OffGridTarget >::iterator new_end =
        std::remove_if( iit->begin(), iit->end(), is_marked_for_removal_ );
      iit->erase( new_end, iit->end() );
    }
  }
}

inline void
EventDeliveryManager::send_to_node( Event& e )
{
  e();
}

inline bool
EventDeliveryManager::get_off_grid_communication() const
{
  return off_grid_spiking_;
}

inline void
EventDeliveryManager::set_off_grid_communication( bool off_grid_spiking )
{
  off_grid_spiking_ = off_grid_spiking;
}

inline size_t
EventDeliveryManager::read_toggle() const
{
  // define in terms of write_toggle() to ensure consistency
  return 1 - write_toggle();
}

inline delay
EventDeliveryManager::get_modulo( delay d )
{
  // Note, here d may be 0, since bin 0 represents the "current" time
  // when all events due are read out.
  assert( static_cast< std::vector< delay >::size_type >( d ) < moduli_.size() );

  return moduli_[ d ];
}

inline delay
EventDeliveryManager::get_slice_modulo( delay d )
{
  // Note, here d may be 0, since bin 0 represents the "current" time
  // when all events due are read out.
  assert( static_cast< std::vector< delay >::size_type >( d ) < slice_moduli_.size() );

  return slice_moduli_[ d ];
}

} // namespace nest

#endif /* EVENT_DELIVERY_MANAGER_H */
