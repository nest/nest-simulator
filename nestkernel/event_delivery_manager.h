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
#include <vector>
#include <limits>

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch.h"

// Includes from nestkernel:
#include "mpi_manager.h" // OffGridSpike
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"
#include "target_table.h"
#include "spike_data.h"
#include "vp_manager.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
typedef MPIManager::OffGridSpike OffGridSpike;

struct TargetData;

/**
 * Struct to simplify keeping track of write position in MPI buffer
 * while collocating spikes.
 */
struct SendBufferPosition
{
  size_t num_spike_data_written;
  std::vector< unsigned int > idx;
  std::vector< unsigned int > begin;
  std::vector< unsigned int > end;
  SendBufferPosition( const AssignedRanks& assigned_ranks,
    const unsigned int send_recv_count_per_rank );
};

inline SendBufferPosition::SendBufferPosition(
  const AssignedRanks& assigned_ranks,
  const unsigned int send_recv_count_per_rank )
  : num_spike_data_written( 0 )
{
  idx.resize( assigned_ranks.size );
  begin.resize( assigned_ranks.size );
  end.resize( assigned_ranks.size );
  for ( thread rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    const thread lr_idx = rank % assigned_ranks.max_size;
    assert( lr_idx < assigned_ranks.size );
    idx[ lr_idx ] = rank * send_recv_count_per_rank;
    begin[ lr_idx ] = rank * send_recv_count_per_rank;
    end[ lr_idx ] = ( rank + 1 ) * send_recv_count_per_rank;
  }
}

class EventDeliveryManager : public ManagerInterface
{
public:
  EventDeliveryManager();
  virtual ~EventDeliveryManager();

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  // TODO@5g: check documentation of send functions
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
  void send( Node& source, EventT& e, const long_t lag = 0 );

  /**
   * Send a secondary event remote.
   */
  void send_secondary( const Node& source, SecondaryEvent& e );

  /**
   * Add global id of event sender to the spike_register.
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
  void send_remote( thread tid, SpikeEvent&, const long_t lag = 0 );

  void send_remote( thread t, SecondaryEvent& e );

  /**
   * Add global id of event sender to the spike_register.
   * Store event offset with global id.
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
  void send_off_grid_remote( thread tid, SpikeEvent& e, const long_t lag = 0 );

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
   * Clear all pending spikes, but do not otherwise manipulate scheduler.
   * @note This is used by Network::reset_network().
   */
  void clear_pending_spikes();

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
   * This is done by resume() when called for the first time.
   * The spike buffers cannot be reconfigured later, whence neither
   * the number of local threads or the min_delay can change after
   * simulate() has been called. ConnectorModel::check_delay() and
   * Network::set_status() ensure this.
   */
  void configure_spike_buffers();

  void configure_secondary_buffers();

  /**
   * Read all event buffers for thread t and send the corresponding
   * Events to the Nodes that are targeted.
   *
   * @note It is a crucial property of deliver_events_() that events
   * are delivered ordered by non-decreasing time stamps. BUT: this
   * ordering applies to time stamps only, it does NOT take into
   * account the offsets of precise spikes.
   */
  bool deliver_events( thread t );

  /**
   * Collocate buffers and exchange events with other MPI processes.
   */
  void gather_events( bool );

  /**
   * Collocates spikes from register to MPI buffers, communicates via
   * MPI and delivers events to targets.
   */
  void gather_spike_data( const thread tid );

  /**
   * Collocates presynaptic connection information, communicates via
   * MPI and creates presynaptic connection infrastructure.
   */
  void gather_target_data();

  /**
   * Collocates presynaptic connection information for secondary events (MPI
   * buffer offsets), communicates via MPI and create presynaptic connection
   * infrastructure for secondary events.
   */
  void gather_secondary_target_data();

  void gather_secondary_events( const bool done );

  bool deliver_secondary_events( const thread tid );

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

  Stopwatch sw_collocate;
  Stopwatch sw_communicate;
  Stopwatch sw_deliver;
  Stopwatch sw_send;

  Stopwatch sw_collocate_target_data;
  Stopwatch sw_communicate_target_data;
  Stopwatch sw_deliver_target_data;

  unsigned int comm_steps_target_data;
  unsigned int comm_rounds_target_data;
  unsigned int comm_steps_spike_data;
  unsigned int comm_rounds_spike_data;

private:
  /**
   * Rearrange the spike_register into a 2-dim structure. This is
   * done by collecting the spikes from all threads in each slice of
   * the min_delay_ interval.
   */
  void collocate_buffers_( bool );

  /**
   * Moves spikes from on grid and off grid spike registers to correct
   * locations in MPI buffers.
   */
  template < typename TargetT, typename SpikeDataT >
  bool collocate_spike_data_buffers_( const thread tid,
    const AssignedRanks& assigned_ranks,
    SendBufferPosition& send_buffer_position,
    std::vector< std::vector< std::vector< std::vector< TargetT > > >* >&
      spike_register,
    std::vector< SpikeDataT >& send_buffer );

  /**
   * Marks end of valid regions in MPI buffers.
   */
  template < typename SpikeDataT >
  void set_end_and_invalid_markers_( const AssignedRanks& assigned_ranks,
    const SendBufferPosition& send_buffer_position,
    std::vector< SpikeDataT >& send_buffer );

  /**
   * Sets marker in MPI buffer that signals end of communication
   * across MPI ranks.
   */
  template < typename SpikeDataT >
  void set_complete_marker_spike_data_( const AssignedRanks& assigned_ranks,
    std::vector< SpikeDataT >& send_buffer );

  /**
   * Reads spikes from MPI buffers and delivers them to ringbuffer of
   * nodes.
   */
  template < typename SpikeDataT >
  bool deliver_events_5g_( const thread tid,
    const std::vector< SpikeDataT >& recv_buffer );

  /**
   * Deletes all spikes from spike registers and resets spike
   * counters.
   */
  void reset_spike_register_5g_( const thread tid );

  /**
   * Resizes spike registers according minimal delay so it can
   * accomodate all possible lags.
   */
  void resize_spike_register_5g_( const thread tid );

  /**
   * Returns true if spike has been moved to MPI buffer, such that it
   * can be removed by clean_spike_register. Required static function
   * by std::remove_if
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
    const unsigned int num_target_data_per_rank,
    TargetData* send_buffer );

  /**
   * Sets marker in MPI buffer that signals end of communication
   * across MPI ranks.
   */
  void set_complete_marker_target_data_( const thread tid,
    const unsigned int num_target_data_per_rank,
    TargetData* send_buffer );

  /**
   * Reads TargetData objects from MPI buffers and creates Target
   * objects on TargetTable (presynaptic part of connection
   * infrastructure).
   */
  bool distribute_target_data_buffers_( const thread tid,
    const unsigned int num_target_data_per_rank,
    TargetData const* const recv_buffer );

  /**
   * Sends event e to all targets of node source. Delivers events from
   * devices directly to targets.
   */
  template < class EventT >
  void send_local_( Node& source, EventT& e, const long_t lag );

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
   * Register for gids of neurons that spiked. This is a 4-dim
   * structure. While spikes are written to the buffer they are
   * immediately sorted by the thread the will move the to the MPI
   * buffers.
   * - First dim: write threads (from node to register)
   * - Second dim: read threads (from register to MPI buffer)
   * - Third dim: lag
   * - Fourth dim: Target (will be converted in SpikeData)
   */
  std::vector< std::vector< std::vector< std::vector< Target > > >* >
    spike_register_5g_;

  /**
   * Register for gids of precise neurons that spiked. This is a 4-dim
   * structure. While spikes are written to the buffer they are
   * immediately sorted by the thread the will move the to the MPI
   * buffers.
   * - First dim: write threads (from node to register)
   * - Second dim: read threads (from register to MPI buffer)
   * - Third dim: lag
   * - Fourth dim: OffGridTarget (will be converted in OffGridSpikeData)
   */
  std::vector< std::vector< std::vector< std::vector< OffGridTarget > > >* >
    off_grid_spike_register_5g_;

  /**
   * Buffer to collect the secondary events after serialization.
   */
  std::vector< uint_t > send_buffer_secondary_events_;
  std::vector< uint_t > recv_buffer_secondary_events_;

  /**
   * Marker Value to be put between the data fields from different time
   * steps during communication.
   */
  const uint_t comm_marker_;

  std::vector< SpikeData > send_buffer_spike_data_;
  std::vector< SpikeData > recv_buffer_spike_data_;
  std::vector< OffGridSpikeData > send_buffer_off_grid_spike_data_;
  std::vector< OffGridSpikeData > recv_buffer_off_grid_spike_data_;

  unsigned int send_recv_count_spike_data_per_rank_;
  unsigned int send_recv_count_spike_data_in_int_per_rank_;
  unsigned int send_recv_count_off_grid_spike_data_in_int_per_rank_;

  bool buffer_size_target_data_has_changed_; //!< whether size of MPI buffer for
                                             //communication of connections was
                                             //changed
  bool buffer_size_spike_data_has_changed_;  //!< whether size of MPI buffer for
                                             //communication of spikes was
                                             //changed
};

inline void
EventDeliveryManager::reset_spike_register_5g_( const thread tid )
{
  for ( std::vector< std::vector< std::vector< Target > > >::iterator it =
          ( *spike_register_5g_[ tid ] ).begin();
        it < ( *spike_register_5g_[ tid ] ).end();
        ++it )
  {
    for ( std::vector< std::vector< Target > >::iterator iit = ( *it ).begin();
          iit < ( *it ).end();
          ++iit )
    {
      ( *iit ).clear();
    }
  }

  for (
    std::vector< std::vector< std::vector< OffGridTarget > > >::iterator it =
      ( *off_grid_spike_register_5g_[ tid ] ).begin();
    it < ( *off_grid_spike_register_5g_[ tid ] ).end();
    ++it )
  {
    for ( std::vector< std::vector< OffGridTarget > >::iterator iit =
            ( *it ).begin();
          iit < ( *it ).end();
          ++iit )
    {
      ( *iit ).clear();
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
  for ( std::vector< std::vector< std::vector< Target > > >::iterator it =
          ( *spike_register_5g_[ tid ] ).begin();
        it < ( *spike_register_5g_[ tid ] ).end();
        ++it )
  {
    for ( std::vector< std::vector< Target > >::iterator iit = ( *it ).begin();
          iit < ( *it ).end();
          ++iit )
    {
      std::vector< Target >::iterator new_end = std::remove_if(
        ( *iit ).begin(), ( *iit ).end(), is_marked_for_removal_ );
      ( *iit ).erase( new_end, ( *iit ).end() );
    }
  }
  for (
    std::vector< std::vector< std::vector< OffGridTarget > > >::iterator it =
      ( *off_grid_spike_register_5g_[ tid ] ).begin();
    it < ( *off_grid_spike_register_5g_[ tid ] ).end();
    ++it )
  {
    for ( std::vector< std::vector< OffGridTarget > >::iterator iit =
            ( *it ).begin();
          iit < ( *it ).end();
          ++iit )
    {
      std::vector< OffGridTarget >::iterator new_end = std::remove_if(
        ( *iit ).begin(), ( *iit ).end(), is_marked_for_removal_ );
      ( *iit ).erase( new_end, ( *iit ).end() );
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
  // when all evens due are read out.
  assert(
    static_cast< std::vector< delay >::size_type >( d ) < moduli_.size() );

  return moduli_[ d ];
}

inline delay
EventDeliveryManager::get_slice_modulo( delay d )
{
  /// Note, here d may be 0, since bin 0 represents the "current" time
  // when all evens due are read out.
  assert( static_cast< std::vector< delay >::size_type >( d )
    < slice_moduli_.size() );

  return slice_moduli_[ d ];
}

inline void
EventDeliveryManager::send_remote( thread t, SecondaryEvent& e )
{

  // put the secondary events in a buffer for the remote machines
  // size_t old_size = secondary_events_buffer_[ t ].size();

  // secondary_events_buffer_[ t ].resize( old_size + e.size() );
  // std::vector< uint_t >::iterator it =
  //   secondary_events_buffer_[ t ].begin() + old_size;
  // e >> it;
  // |syn_id|gid|[SecondaryEventstuff]|

  // secondary_events_buffer_5g_: 3d structure
  // |write_threads->read_threads->syn_index(syn_id)->SecondaryEventDataWrapper|

  // SecondaryEventDataWrapper:
  // |syn_id|Target|[SecondaryEventStuff]*|

  // SecondaryEventData:
  // |tid|syn_index|lcid|marker|[SecondaryEventStuff]|

  // |rank|tid|syn_index|lcid|

  // prepare_simulation->event_delivery_manager->resize_secondary_events_buffers
  // { for node in local_nodes: pos = 0 ; pos[] =
  // node.register_secondary_targets( pos[] ); }
  // nodes.update{ send_secondary( ..., pos[] ) { offset[] = 0; for target in
  // targets: pos[target.rank] << e; offset[target.rank] += 1 }
}

} // namespace nest

#endif /* EVENT_DELIVERY_MANAGER_H */
