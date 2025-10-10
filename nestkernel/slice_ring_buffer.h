/*
 *  slice_ring_buffer.h
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

#ifndef SLICE_RING_BUFFER_H
#define SLICE_RING_BUFFER_H

// C++ includes:
#include <vector>

namespace nest
{

/**
 * Queue for all spikes arriving into a neuron.
 *
 * Spikes are stored unsorted on arrival, but are sorted when
 * prepare_delivery() is called.  They can then be retrieved
 * one by one in correct temporal order.  Coinciding spikes
 * are combined into one, see get_next_spike().
 *
 * Data is organized as follows:
 * - The time of the next return from refractoriness is
 *   stored in a separate variable and checked explicitly;
 *   otherwise, we'd have to re-sort data during updating.
 * - We have a pseudo-ring of Nbuff=ceil((min_del+max_del)/min_del) elements.
 *   Each element is a vector storing incoming spikes that
 *   are due during a given time slice.
 *
 * @note The following assumptions underlie the handling of
 * pseudo-events for return from refractoriness:
 * - There is at most one such event per time step (value of time stamp).
 */
class SliceRingBuffer
{
public:
  SliceRingBuffer();

  /**
   * Add spike to queue.
   *
   * @param  rel_delivery relative delivery time
   * @param  stamp      Delivery time
   * @param  ps_offset  Precise timing offset of spike time
   * @param  weight     Weight of spike.
   */
  void add_spike( const long rel_delivery, const long stamp, const double ps_offset, const double weight );

  /**
   * Add refractory event to queue.
   *
   * The refractory event is actually stored as a pseudo-event.
   * @param  stamp      Delivery time
   * @param  ps_offset  Precise timing offset of spike time
   */
  void add_refractory( const long stamp, const double ps_offset );

  /**
   * Prepare for spike delivery in current slice by sorting.
   */
  void prepare_delivery();

  /**
   * Discard all events in current slice.
   */
  void discard_events();

  /**
   * Return next spike.
   *
   * @param req_stamp  Request spike with this stamp.  Queue
   *                   should never contain spikes with smaller
   *                   stamps.  Spikes with larger stamps are
   *                   left in queue.
   * @param accumulate_simultaneous If true, return summed weight
   *                   of simultaneous input spikes, otherwise return
   *                   one spike at a time.
   * @param ps_offset  PS-sense offset of spike time
   * @param weight     Spike weight
   * @param end_of_refract True if spike is pseudo-spike marking
   *                   end of refractory period
   * @returns          true if spike available, false otherwise
   * @note             If return from refractoriness coincides with
   *                   a spike, return from refractoriness is returned.
   */
  bool get_next_spike( const long req_stamp,
    bool accumulate_simultaneous,
    double& ps_offset,
    double& weight,
    bool& end_of_refract );

  /**
   * Clear buffer
   */
  void clear();

  /**
   * Resize the buffer according to min_delay and max_delay.
   */
  void resize();

private:
  /**
   * Information about spike.
   */
  struct SpikeInfo
  {
    SpikeInfo( long stamp, double ps_offset, double weight );

    bool operator<( const SpikeInfo& b ) const;
    bool operator<=( const SpikeInfo& b ) const;
    bool operator>( const SpikeInfo& b ) const;

    // data elements must not be const, since heap implementation
    // in DEC STL uses operator=().
    long stamp_;       //<! spike's time stamp
    double ps_offset_; //<! spike offset is PS sense
    double weight_;    //<! spike weight
  };

  //! entire queue, one slot per min_delay block within max_delay
  std::vector< std::vector< SpikeInfo > > queue_;

  //! slot to deliver from
  std::vector< SpikeInfo >* deliver_;

  SpikeInfo refract_; //!< pseudo-event for return from refractoriness
};


} // namespace nest

#endif /* #ifndef SLICE_RING_BUFFER_H */
