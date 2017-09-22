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

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch.h"

// Includes from nestkernel:
#include "event.h"
#include "mpi_manager.h" // OffGridSpike
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
typedef MPIManager::OffGridSpike OffGridSpike;

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
  void send_remote( thread p, SpikeEvent&, const long lag = 0 );

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
  void send_offgrid_remote( thread p, SpikeEvent&, const long lag = 0 );

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
   * Resizes also offgrid_*_buffer_.
   * This is done by resume() when called for the first time.
   * The spike buffers cannot be reconfigured later, whence neither
   * the number of local threads or the min_delay can change after
   * simulate() has been called. ConnectorModel::check_delay() and
   * Network::set_status() ensure this.
   */
  void configure_spike_buffers();

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
   * Set cumulative time measurements for collocating buffers
   * and for communication to zero; set local spike counter to zero.
   */
  virtual void reset_timers_counters();

private:
  /**
   * Rearrange the spike_register into a 2-dim structure. This is
   * done by collecting the spikes from all threads in each slice of
   * the min_delay_ interval.
   */
  void collocate_buffers_( bool );


private:
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
   * Register for gids of neurons that spiked. This is a 3-dim
   * structure.
   * - First dim: Each thread has its own vector to write to.
   * - Second dim: A vector for each slice of the min_delay interval
   * - Third dim: The gids.
   */
  std::vector< std::vector< std::vector< unsigned int > > > spike_register_;

  /**
   * Register for off-grid spikes.
   * This is a 3-dim structure.
   * - First dim: Each thread has its own vector to write to.
   * - Second dim: A vector for each slice of the min_delay interval
   * - Third dim: Struct containing GID and offset.
   */
  std::vector< std::vector< std::vector< OffGridSpike > > >
    offgrid_spike_register_;

  /**
   * Buffer to collect the secondary events
   * after serialization.
   */
  std::vector< std::vector< unsigned int > > secondary_events_buffer_;

  /**
   * Buffer containing the gids of local neurons that spiked in the
   * last min_delay_ interval. The single slices are separated by a
   * marker value.
   */
  std::vector< unsigned int > local_grid_spikes_;

  /**
   * Buffer containing the gids of all neurons that spiked in the
   * last min_delay_ interval. The single slices are separated by a
   * marker value
   */
  std::vector< unsigned int > global_grid_spikes_;

  /**
   * Buffer containing the gids and offsets for local neurons that
   * fired off-grid spikes in the last min_delay_ interval. The
   * single slices are separated by a marker value.
   */
  std::vector< OffGridSpike > local_offgrid_spikes_;

  /**
   * Buffer containing the gids and offsets for all neurons that
   * fired off-grid spikes in the last min_delay_ interval. The
   * single slices are separated by a marker value.
   */
  std::vector< OffGridSpike > global_offgrid_spikes_;

  /**
   * Buffer containing the starting positions for the spikes from
   * each process within the global_(off)grid_spikes_ buffer.
   */
  std::vector< int > displacements_;

  /**
   * Marker Value to be put between the data fields from different time
   * steps during communication.
   */
  const unsigned int comm_marker_;

  /**
   * Time that was spent on collocation of MPI buffers during the last call to
   * simulate.
   */
  double time_collocate_;

  /**
   * Time that was spent on communication of events during the last call to
   * simulate.
   */
  double time_communicate_;

  /**
   * Number of generated spike events (both off- and on-grid) during the last
   * call to simulate.
   */
  unsigned long local_spike_counter_;
};


inline void
EventDeliveryManager::send_to_node( Event& e )
{
  e();
}

inline void
EventDeliveryManager::send_remote( thread t, SpikeEvent& e, const long lag )
{
  // Put the spike in a buffer for the remote machines
  for ( int i = 0; i < e.get_multiplicity(); ++i )
  {
    spike_register_[ t ][ lag ].push_back( e.get_sender().get_gid() );
  }
}

inline void
EventDeliveryManager::send_offgrid_remote( thread t,
  SpikeEvent& e,
  const long lag )
{
  // Put the spike in a buffer for the remote machines
  OffGridSpike ogs( e.get_sender().get_gid(), e.get_offset() );
  for ( int i = 0; i < e.get_multiplicity(); ++i )
  {
    offgrid_spike_register_[ t ][ lag ].push_back( ogs );
  }
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
  size_t old_size = secondary_events_buffer_[ t ].size();

  secondary_events_buffer_[ t ].resize( old_size + e.size() );
  std::vector< unsigned int >::iterator it =
    secondary_events_buffer_[ t ].begin() + old_size;
  e >> it;
}

} // namespace nest

#endif /* EVENT_DELIVERY_MANAGER_H */
