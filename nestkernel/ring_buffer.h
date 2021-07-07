/*
 *  ring_buffer.h
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

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

// C++ includes:
#include <array>
#include <list>
#include <vector>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_time.h"
#include "nest_types.h"

namespace nest
{

/**
   Buffer Layout.

   MODIFICATION 2005-06-19:
   The explanation below no longer holds if we allow direct delivery of events
   from devices such as the Poisson generator.  The reasoning below applies only
   to events in the central queue, which are held in that queue until the
   beginning of the next slice, when system time has been advance from T to
   T + min_delay.  Direct delivery events, in contrast are delivered when system
   time is still T.  Their earliest delivery time is

   min T_d = T + min_del

   and the latest

   max T_d = T + (min_del-1) + max_del = T + min_del + max_del - 1

   Since we still need to keep the entries 0..min_del-1 for readout during the
   time slice beginning at T, we need a buffer with min_del+max_del elements.

   SUPERSEEDED:
   Let S be the time at the beginning of the present time slice (from).
   All spikes arriving during this time slice, must have been emitted during
   the previous time slice, which started at S - min_del.  Then, the earliest
   spike delivery time (compare Time Memo) is

   min T_d = S-min_del + min_del = S

   and the latest

   max T_d = S-1 + max_del = S + (max_del - 1)

   Thus,

   0 <= S - T_d <= max_del - 1

   so that the ring buffer needs max_del elements.

   Each field represents an entry in the vector.

*/


class RingBuffer
{
public:
  RingBuffer();

  /**
   * Add a value to the ring buffer.
   * @param  offs     Arrival time relative to beginning of slice.
   * @param  double Value to add.
   */
  void add_value( const long offs, const double );

  /**
   * Set a ring buffer entry to a given value.
   * @param  offs     Arrival time relative to beginning of slice.
   * @param  double Value to set.
   */
  void set_value( const long offs, const double );

  /**
   * Read one value from ring buffer.
   * @param  offs  Offset of element to read within slice.
   * @returns value
   */
  double get_value( const long offs );

  /**
   * Read one value from ring buffer without deleting it afterwards.
   * @param  offs  Offset of element to read within slice.
   * @returns value
   */
  double get_value_wfr_update( const long offs );

  /**
   * Initialize the buffer with noughts.
   * Also resizes the buffer if necessary.
   */
  void clear();

  /**
   * Resize the buffer according to max_thread and max_delay.
   * New elements are filled with noughts.
   * @note resize() has no effect if the buffer has the correct size.
   */
  void resize();

  /**
   * Returns buffer size, for memory measurement.
   */
  size_t
  size() const
  {
    return buffer_.size();
  }

private:
  //! Buffered data
  std::vector< double > buffer_;

  /**
   * Obtain buffer index.
   * @param delay delivery delay for event
   * @returns index to buffer element into which event should be
   * recorded.
   */
  size_t get_index_( const delay d ) const;
};

inline void
RingBuffer::add_value( const long offs, const double v )
{
  buffer_[ get_index_( offs ) ] += v;
}

inline void
RingBuffer::set_value( const long offs, const double v )
{
  buffer_[ get_index_( offs ) ] = v;
}

inline double
RingBuffer::get_value( const long offs )
{
  assert( 0 <= offs and ( size_t ) offs < buffer_.size() );
  assert( ( delay ) offs < kernel().connection_manager.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  double val = buffer_[ idx ];
  buffer_[ idx ] = 0.0; // clear buffer after reading
  return val;
}

inline double
RingBuffer::get_value_wfr_update( const long offs )
{
  assert( 0 <= offs and ( size_t ) offs < buffer_.size() );
  assert( ( delay ) offs < kernel().connection_manager.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  double val = buffer_[ idx ];
  return val;
}

inline size_t
RingBuffer::get_index_( const delay d ) const
{
  const long idx = kernel().event_delivery_manager.get_modulo( d );
  assert( 0 <= idx );
  assert( ( size_t ) idx < buffer_.size() );
  return idx;
}


class MultRBuffer
{
public:
  MultRBuffer();

  /**
   * Add a value to the ring buffer.
   * @param  offs     Arrival time relative to beginning of slice.
   * @param  double Value to add.
   */
  void add_value( const long offs, const double );

  /**
   * Read one value from ring buffer.
   * @param  offs  Offset of element to read within slice.
   * @returns value
   */
  double get_value( const long offs );

  /**
   * Initialize the buffer with noughts.
   */
  void clear();

  /**
   * Resize the buffer according to max_thread and max_delay.
   */
  void resize();

  /**
   * Returns buffer size, for memory measurement.
   */
  size_t
  size() const
  {
    return buffer_.size();
  }

private:
  //! Buffered data
  std::vector< double > buffer_;

  /**
   * Obtain buffer index.
   * @param delay delivery delay for event
   * @returns index to buffer element into which event should be
   * recorded.
   */
  size_t get_index_( const delay d ) const;
};

inline void
MultRBuffer::add_value( const long offs, const double v )
{
  assert( 0 <= offs and ( size_t ) offs < buffer_.size() );
  buffer_[ get_index_( offs ) ] *= v;
}

inline double
MultRBuffer::get_value( const long offs )
{
  assert( 0 <= offs and ( size_t ) offs < buffer_.size() );
  assert( ( delay ) offs < kernel().connection_manager.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  double val = buffer_[ idx ];
  buffer_[ idx ] = 0.0; // clear buffer after reading
  return val;
}

inline size_t
MultRBuffer::get_index_( const delay d ) const
{
  const long idx = kernel().event_delivery_manager.get_modulo( d );
  assert( 0 <= idx and ( size_t ) idx < buffer_.size() );
  return idx;
}


class ListRingBuffer
{
public:
  ListRingBuffer();

  /**
   * Append a value to the ring buffer list.
   * @param  offs     Arrival time relative to beginning of slice.
   * @param  double Value to append.
   */
  void append_value( const long offs, const double );

  std::list< double >& get_list( const long offs );

  /**
   * Initialize the buffer with empty lists.
   * Also resizes the buffer if necessary.
   */
  void clear();

  /**
   * Resize the buffer according to max_thread and max_delay.
   * New elements are filled with empty lists.
   * @note resize() has no effect if the buffer has the correct size.
   */
  void resize();

  /**
   * Returns buffer size, for memory measurement.
   */
  size_t
  size() const
  {
    return buffer_.size();
  }

private:
  //! Buffered data
  std::vector< std::list< double > > buffer_;

  /**
   * Obtain buffer index.
   * @param delay delivery delay for event
   * @returns index to buffer element into which event should be
   * recorded.
   */
  size_t get_index_( const delay d ) const;
};

inline void
ListRingBuffer::append_value( const long offs, const double v )
{
  buffer_[ get_index_( offs ) ].push_back( v );
}

inline std::list< double >&
ListRingBuffer::get_list( const long offs )
{
  assert( 0 <= offs and ( size_t ) offs < buffer_.size() );
  assert( ( delay ) offs < kernel().connection_manager.get_min_delay() );

  // offs == 0 is beginning of slice, but we have to
  // take modulo into account when indexing
  long idx = get_index_( offs );
  return buffer_[ idx ];
}

inline size_t
ListRingBuffer::get_index_( const delay d ) const
{
  const long idx = kernel().event_delivery_manager.get_modulo( d );
  assert( 0 <= idx );
  assert( ( size_t ) idx < buffer_.size() );
  return idx;
}


template < unsigned int num_channels >
class MultiChannelInputBuffer
{
public:
  MultiChannelInputBuffer();

  void add_value( const index slot, const index channel, const double value );

  const std::array< double, num_channels >& get_values_all_channels( const index slot ) const;
  void reset_values_all_channels( const index slot );

  void clear();

  void resize();

  size_t size() const;

private:
  /**
   * Buffered data stored in a vector of arrays of double values
   * 1st dimension: ring buffer slot (index into outer vector)
   * 2nd dimension: channel (index into inner array)
   */
  std::vector< std::array< double, num_channels > > buffer_;
};

template < unsigned int num_channels >
inline void
MultiChannelInputBuffer< num_channels >::reset_values_all_channels( const index slot )
{
  assert( slot < buffer_.size() );
  buffer_[ slot ].fill( 0.0 );
}

template < unsigned int num_channels >
inline void
MultiChannelInputBuffer< num_channels >::add_value( const index slot, const index channel, const double value )
{
  buffer_[ slot ][ channel ] += value;
}

template < unsigned int num_channels >
inline const std::array< double, num_channels >&
MultiChannelInputBuffer< num_channels >::get_values_all_channels( const index slot ) const
{
  assert( slot < buffer_.size() );
  return buffer_[ slot ];
}

template < unsigned int num_channels >
inline size_t
MultiChannelInputBuffer< num_channels >::size() const
{
  return buffer_.size();
}

} // namespace nest


#endif
