/*
 *  slice_ring_buffer.cpp
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

#include "slice_ring_buffer.h"

// C++ includes:
#include <cmath>
#include <limits>

#include "connection_manager.h"
#include "event_delivery_manager.h"

nest::SliceRingBuffer::SliceRingBuffer()
  : refract_( std::numeric_limits< long >::max(), 0, 0 )
{
  //  resize();  // sets up queue_
}

void
nest::SliceRingBuffer::resize()
{
  long newsize =
    static_cast< long >( std::ceil( static_cast< double >( kernel::manager< ConnectionManager >.get_min_delay()
                                      + kernel::manager< ConnectionManager >.get_max_delay() )
      / kernel::manager< ConnectionManager >.get_min_delay() ) );
  if ( queue_.size() != static_cast< unsigned long >( newsize ) )
  {
    queue_.resize( newsize );
    clear();
  }

#ifndef HAVE_STL_VECTOR_CAPACITY_BASE_UNITY
  // create 1-element buffers
  for ( size_t j = 0; j < queue_.size(); ++j )
  {
    queue_[ j ].reserve( 1 );
  }
#endif
}

void
nest::SliceRingBuffer::clear()
{
  for ( size_t j = 0; j < queue_.size(); ++j )
  {
    queue_[ j ].clear();
  }
}

void
nest::SliceRingBuffer::prepare_delivery()
{
  // vector to deliver from in this slice
  deliver_ = &( queue_[ kernel::manager< EventDeliveryManager >.get_slice_modulo( 0 ) ] );

  // sort events, first event last
  std::sort( deliver_->begin(), deliver_->end(), std::greater< SpikeInfo >() );
}

void
nest::SliceRingBuffer::discard_events()
{
  // vector to deliver from in this slice
  deliver_ = &( queue_[ kernel::manager< EventDeliveryManager >.get_slice_modulo( 0 ) ] );

  deliver_->clear();
}
bool
nest::SliceRingBuffer::SpikeInfo::operator>( const SpikeInfo& b ) const
{

  return stamp_ == b.stamp_ ? ps_offset_ < b.ps_offset_ : stamp_ > b.stamp_;
}

bool
nest::SliceRingBuffer::SpikeInfo::operator<=( const SpikeInfo& b ) const
{

  return not( *this > b );
}

bool
nest::SliceRingBuffer::SpikeInfo::operator<( const SpikeInfo& b ) const
{

  return stamp_ == b.stamp_ ? ps_offset_ > b.ps_offset_ : stamp_ < b.stamp_;
}

nest::SliceRingBuffer::SpikeInfo::SpikeInfo( long stamp, double ps_offset, double weight )
  : stamp_( stamp )
  , ps_offset_( ps_offset )
  , weight_( weight )
{
}

bool
nest::SliceRingBuffer::get_next_spike( const long req_stamp,
  bool accumulate_simultaneous,
  double& ps_offset,
  double& weight,
  bool& end_of_refract )
{

  end_of_refract = false;
  if ( deliver_->empty() or refract_ <= deliver_->back() )
  {
    if ( refract_.stamp_ == req_stamp )
    { // if relies on stamp_==long::max() if not refractory
      // return from refractoriness
      ps_offset = refract_.ps_offset_;
      weight = 0;
      end_of_refract = true;

      // mark as non-refractory
      refract_.stamp_ = std::numeric_limits< long >::max();
      return true;
    }
    else
    {
      return false;
    }
  }
  else if ( deliver_->back().stamp_ == req_stamp )
  {
    // we have an event to deliver
    ps_offset = deliver_->back().ps_offset_;
    weight = deliver_->back().weight_;
    deliver_->pop_back();

    if ( accumulate_simultaneous )
    {
      // add weights of all spikes with same stamp and offset
      while (
        not deliver_->empty() and deliver_->back().ps_offset_ == ps_offset and deliver_->back().stamp_ == req_stamp )
      {
        weight += deliver_->back().weight_;
        deliver_->pop_back();
      }
    }

    return true;
  }
  else
  {
    // ensure that we are not blocked by spike from the past, cf #404
    assert( deliver_->back().stamp_ > req_stamp );
    return false;
  }
}

void
nest::SliceRingBuffer::add_refractory( const long stamp, const double ps_offset )
{

  // We require that only one refractory-return pseudo-event is stored per
  // time step.
  //
  // We guard against violation using assert(): refract_.stamp_ must
  // be equal to the marker value for non-refractoriness. All else would mean
  // that a refractory neuron fired.
  assert( refract_.stamp_ == std::numeric_limits< long >::max() );

  refract_.stamp_ = stamp;
  refract_.ps_offset_ = ps_offset;
}

void
nest::SliceRingBuffer::add_spike( const long rel_delivery,
  const long stamp,
  const double ps_offset,
  const double weight )
{

  const long idx = kernel::manager< EventDeliveryManager >.get_slice_modulo( rel_delivery );
  assert( static_cast< size_t >( idx ) < queue_.size() );
  assert( ps_offset >= 0 );

  queue_[ idx ].push_back( SpikeInfo( stamp, ps_offset, weight ) );
}
