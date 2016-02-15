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

#ifndef SLICE_RING_BUFFER_NEW_H
#define SLICE_RING_BUFFER_NEW_H

// C++ includes:
#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

// Generated includes:
#include "config.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_types.h"

namespace nest
{
/**
 * Queue for all spikes arriving into a neuron.
 * Spikes are stored unsorted on arrival, but are sorted when
 * prepare_delivery() is called.  They can then be retrieved
 * one by one in correct temporal order.  Coinciding spikes
 * are combined into one, see get_next_event().
 *
 * Data is organized as follows:
 * - The time of the next return from refractoriness is
 *   stored in a separate variable and checked explicitly;
 *   otherwise, we'd have to re-sort data during updating.
 * - We have a pseudo-ring of Nbuff=ceil((min_del+max_del)/min_del) elements.
 *   Each element is a vector storing incoming spikes that
 *   are due during a given time slice.
 *
 * @note Contrary to the previous implementation, this version of
 * slice_ring_buffer does not require that only one refractory event be stored
 * per timestep; we suppose here that coherent handling of refractory events
 * is performed in the neuron model.
 */

class SliceRingBufferNew
{
public:
  SliceRingBufferNew();

  /**
   * Add spike to queue.
   * @param  rel_delivery relative delivery time
   * @param  stamp      Delivery time
   * @param  ps_offset  Precise timing offset of spike time
   * @param  weight     Weight of spike.
   */
  void add_spike( const delay rel_delivery,
    const long_t stamp,
    const double ps_offset,
    const double weight );

  /**
   * Add refractory event to queue.
   * The refractory event is actually stored as a pseudo-event.
   * @param  stamp      Delivery time
   * @param  ps_offset  Precise timing offset of spike time
   */
  void set_refractory( const long_t stamp, const double_t ps_offset );

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
   * @param req_stamp  Request spike with this stamp.  Queue
   *                   should never contain spikes with smaller
   *                   stamps.  Spikes with larger stamps are
   *                   left in queue.
   * @param ps_offset  PS-sense offset of spike time
   * @param weight     Spike weight
   * @param end_of_refract True if spike is pseudo-spike marking
   *                   end of refractory period
   * @returns          true if spike available, false otherwise
   * @note             If return from refractoriness coincides with
   *                   a spike, return from refractoriness is returned.
   *                   If several spikes coincide, the sum of their
   *                   weights is returned a single spike.
   */
  void get_next_event(  const long_t req_stamp,
                        double_t& ps_offset,
                        double& weight_in,
                        double& weight_ex,
                        const double_t step_ );

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
    SpikeInfo( long_t stamp, double_t ps_offset, double_t weight );

    bool operator<( const SpikeInfo& b ) const;
    bool operator<=( const SpikeInfo& b ) const;
    bool operator>( const SpikeInfo& b ) const;

    // data elements must not be const, since heap implementation
    // in DEC STL uses operator=().
    long_t stamp_;       //<! spike's time stamp
    double_t ps_offset_; //<! spike offset is PS sense
    double_t weight_;    //<! spike weight
  };

  //! entire queue, one slot per min_delay block within max_delay
  std::vector< std::vector< SpikeInfo > > queue_;

  //! slot to deliver from
  std::vector< SpikeInfo >* deliver_;

  SpikeInfo refract_; //!< pseudo-event for return from refractoriness
};

inline void
SliceRingBufferNew::add_spike( const delay rel_delivery,
  const long_t stamp,
  const double ps_offset,
  const double weight )
{
  const delay idx = kernel().event_delivery_manager.get_slice_modulo( rel_delivery );
  assert( ( size_t ) idx < queue_.size() );
  assert( ps_offset >= 0 );

  queue_[ idx ].push_back( SpikeInfo( stamp, ps_offset, weight ) );
}

inline void
SliceRingBufferNew::set_refractory( const long_t stamp, const double_t ps_offset )
{
  refract_ = SpikeInfo( stamp, ps_offset, 0. );
}

inline void
SliceRingBufferNew::get_next_event( const long_t req_stamp,
  double_t& ps_offset,
  double& weight_in,
  double& weight_ex,
  const double_t step_ )
{
  // accumulate weights of all spikes with same stamp AND offset
  // weight_in accumulates absolute weights of inhibitory
  // weight_ex accumulates weights of excitatory
  // they are not set to zero because get_next_event might be applied several
  // times in case interpolation occurs before the spike reception occurs
  // (see for instance aeif_cond_alpha_ps::update)
  // refract_.stamp_==long_t::max() if neuron is not refractory


  if ( deliver_->empty() )
  {
    if ( refract_.stamp_ == req_stamp )
    {
      // set return from refractoriness
      ps_offset = step_ - refract_.ps_offset_;
      // mark as non-refractory
      refract_.stamp_ = std::numeric_limits< long_t >::max();
    }
    else
      ps_offset = step_;
  }
  else if ( deliver_->back().stamp_ == req_stamp )
  {
    // we have an event to deliver, register its offset
    double ps_offset_tmp = deliver_->back().ps_offset_;

    if ( refract_.stamp_ == req_stamp && refract_.ps_offset_ > ps_offset_tmp )
    {
      ps_offset = step_ - refract_.ps_offset_;
      refract_.stamp_ = std::numeric_limits< long_t >::max();
    }
    else
    {
      double weight_tmp;
      while ( !deliver_->empty() && deliver_->back().ps_offset_ == ps_offset_tmp
        && deliver_->back().stamp_ == req_stamp )
      {
        weight_tmp = deliver_->back().weight_;
        if ( weight_tmp <= 0 )
          weight_in -= weight_tmp;
        else
          weight_ex += weight_tmp;
        deliver_->pop_back();
      }
      ps_offset = step_ - ps_offset_tmp;
    }
  }
  else
  {
    // ensure that we are not blocked by spike from the past, cf #404
    assert( deliver_->back().stamp_ > req_stamp );
    if ( refract_.stamp_ == req_stamp )
    {
      ps_offset = step_ - refract_.ps_offset_;
      refract_.stamp_ = std::numeric_limits< long_t >::max();
    }
    else
      ps_offset = step_;
  }
}

inline SliceRingBufferNew::SpikeInfo::SpikeInfo( long_t stamp, double_t ps_offset, double_t weight )
  : stamp_( stamp )
  , ps_offset_( ps_offset )
  , weight_( weight )
{
}

inline bool SliceRingBufferNew::SpikeInfo::operator<( const SpikeInfo& b ) const
{
  return stamp_ == b.stamp_ ? ps_offset_ > b.ps_offset_ : stamp_ < b.stamp_;
}

inline bool SliceRingBufferNew::SpikeInfo::operator<=( const SpikeInfo& b ) const
{
  return !( *this > b );
}

inline bool SliceRingBufferNew::SpikeInfo::operator>( const SpikeInfo& b ) const
{
  return stamp_ == b.stamp_ ? ps_offset_ < b.ps_offset_ : stamp_ > b.stamp_;
}
}

#endif
