/*
 *  spike_data.h
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

#ifndef SPIKE_DATA_H
#define SPIKE_DATA_H

// C++ includes
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"
#include "target.h"


namespace nest
{

/**
 * Mark spike transmission status in SpikeData entries.
 *
 * @note Assumes that send buffer has at least two entries per rank, begin ≠ end.
 * @note To ensure that we only need two bits for this flag, flags will be interpreted differently
 *       depending on where they are used in a send buffer.
 *
 *  Below,
 *  - begpos and endpos refer to the first and last entries for a given rank-specific chunk of the send buffer
 *  - `local_max_spikes_per_rank` is the largest number of spikes a given rank needs to transmit to any other rank.
 *  - `global_max_spikes_per_rank` is the maximum of all `local_max_spikes_per_rank` values. It determines the
 *
 * `SpikeData` marker values are defined as follows: have
 *    - `DEFAULT`: Normal entry, cannot occur in endpos
 *    - `END`: Marks last entry containing data.
 *      - If it occurs in endpos,
 *        - it implies COMPLETE
 *        - it indicates that `local_max_spikes_per_rank` of the sending rank is equal to the current buffer size
 *    - COMPLETE: Can only occur in endpos and indicates that the sending rank could write all emitted spikes to the
 transmission buffer.
 *          - END is then in earlier position.
 *          - The LCID entry of endpos contains the `local_max_spikes_per_rank` of the corresponding sending rank.
 *     - INVALID:
 *          - In begpos indicates that no spikes are transmitted (@note: END at begpos means one spike transmitted)
 *          - In endpos, indicates that the pertaining rank could not send all spikes.
 *           - The LCID entry of endpos contains the `local_max_spikes_per_rank` of the corresponding sending rank.
 *
 *
 * @note Logic for reading from spike transmission buffer
 * 1. If marker at begpos for a rank is INVALID, there are no spikes to read
 * 2. Read until END marker is met. All entries including the one with END contain valid spikes.
 * 3. Check marker in endpos for completeness of transmission and required transmission buffer chunk size
 *     1. Completeness
 *         - If `COMPLETE` or `END`, transmission is complete
 *         - If `INVALID`, not all spikes could be sent, repeat with increased chunk size
 *         - If `DEFAULT`, something is seriously wrong
 *     2. Required chunk size
 *         - If marker is `END`, the required chunk size is equal to current chunk size (and LCID field contains LCID
 for spike in endpos)
 *         - If marker is `COMPLETE` or `INVALID`, the required chunk size is given by the valued stored in the LCID
 field of endpos
 *
 */
enum enum_status_spike_data_id
{
  SPIKE_DATA_ID_DEFAULT,
  SPIKE_DATA_ID_END,
  SPIKE_DATA_ID_COMPLETE,
  SPIKE_DATA_ID_INVALID
};

/**
 * Used to communicate spikes. These are the elements of the MPI
 * buffers.
 *
 * @see TargetData
 */
class SpikeData
{
protected:
  static constexpr int MAX_LAG = generate_max_value( NUM_BITS_LAG );

  size_t lcid_ : NUM_BITS_LCID;                      //!< local connection index
  unsigned int marker_ : NUM_BITS_MARKER_SPIKE_DATA; //!< status flag
  unsigned int lag_ : NUM_BITS_LAG;                  //!< lag in this min-delay interval
  unsigned int tid_ : NUM_BITS_TID;                  //!< thread index
  synindex syn_id_ : NUM_BITS_SYN_ID;                //!< synapse-type index

public:
  SpikeData();
  SpikeData( const SpikeData& rhs );
  SpikeData( const Target& target, const size_t lag );
  SpikeData( const size_t tid, const synindex syn_id, const size_t lcid, const unsigned int lag );

  SpikeData& operator=( const SpikeData& rhs );

  //! Required in connection with direct-send events.
  void set( const size_t tid, const synindex syn_id, const size_t lcid, const unsigned int lag, const double offset );

  template < class TargetT >
  void set( const TargetT& target, const unsigned int lag );

  /**
   * Returns local connection ID.
   */
  size_t get_lcid() const;

  /**
   * Sets lcid value.
   *
   * @note Allows each rank to send the locally required buffer size per rank.
   */
  void set_lcid( size_t );

  /**
   * Returns lag in min-delay interval.
   */
  unsigned int get_lag() const;

  /**
   * Returns thread index.
   */
  size_t get_tid() const;

  /**
   * Returns synapse-type index.
   */
  synindex get_syn_id() const;

  /**
   * Returns marker.
   */
  unsigned int get_marker() const;

  /**
   * Resets the status flag to default value.
   */
  void reset_marker();

  /**
   * Sets the status flag to complete marker.
   */
  void set_complete_marker();

  /**
   * Sets the status flag to end marker.
   */
  void set_end_marker();

  /**
   * Sets the status flag to invalid marker.
   */
  void set_invalid_marker();

  /**
   * Returns whether the marker is the complete marker.
   */
  bool is_complete_marker() const;

  /**
   * Returns whether the marker is the end marker.
   */
  bool is_end_marker() const;

  /**
   * Returns whether the marker is the invalid marker.
   */
  bool is_invalid_marker() const;

  /**
   * Returns offset.
   */
  double get_offset() const;
};

//! check legal size
using success_spike_data_size = StaticAssert< sizeof( SpikeData ) == 8 >::success;

inline SpikeData::SpikeData()
  : lcid_( 0 )
  , marker_( SPIKE_DATA_ID_DEFAULT )
  , lag_( 0 )
  , tid_( 0 )
  , syn_id_( 0 )
{
}

inline SpikeData::SpikeData( const SpikeData& rhs )
  : lcid_( rhs.lcid_ )
  , marker_( rhs.marker_ )
  , lag_( rhs.lag_ )
  , tid_( rhs.tid_ )
  , syn_id_( rhs.syn_id_ )
{
}

inline SpikeData::SpikeData( const Target& target, const size_t lag )
  : lcid_( target.get_lcid() )
  , marker_( SPIKE_DATA_ID_DEFAULT )
  , lag_( lag )
  , tid_( target.get_tid() )
  , syn_id_( target.get_syn_id() )
{
}

inline SpikeData::SpikeData( const size_t tid, const synindex syn_id, const size_t lcid, const unsigned int lag )
  : lcid_( lcid )
  , marker_( SPIKE_DATA_ID_DEFAULT )
  , lag_( lag )
  , tid_( tid )
  , syn_id_( syn_id )
{
}

inline SpikeData&
SpikeData::operator=( const SpikeData& rhs )
{
  lcid_ = rhs.lcid_;
  marker_ = rhs.marker_;
  lag_ = rhs.lag_;
  tid_ = rhs.tid_;
  syn_id_ = rhs.syn_id_;
  return *this;
}

inline void
SpikeData::set( const size_t tid, const synindex syn_id, const size_t lcid, const unsigned int lag, const double )
{
  assert( tid <= MAX_TID ); // MAX_TID is allowed since it is not used as invalid value
  assert( syn_id < MAX_SYN_ID );
  assert( lcid < MAX_LCID );
  assert( lag < MAX_LAG );

  lcid_ = lcid;
  marker_ = SPIKE_DATA_ID_DEFAULT;
  lag_ = lag;
  tid_ = tid;
  syn_id_ = syn_id;
}


template < class TargetT >
inline void
SpikeData::set( const TargetT& target, const unsigned int lag )
{
  // the assertions in the above function are granted by the TargetT object!
  assert( lag < MAX_LAG );
  lcid_ = target.get_lcid();
  marker_ = SPIKE_DATA_ID_DEFAULT;
  lag_ = lag;
  tid_ = target.get_tid();
  syn_id_ = target.get_syn_id();
}

inline size_t
SpikeData::get_lcid() const
{
  return lcid_;
}

inline void
SpikeData::set_lcid( size_t value )
{
  assert( value < MAX_LCID );
  lcid_ = value;
}

inline unsigned int
SpikeData::get_lag() const
{
  return lag_;
}

inline size_t
SpikeData::get_tid() const
{
  return tid_;
}

inline synindex
SpikeData::get_syn_id() const
{
  return syn_id_;
}

inline unsigned int
SpikeData::get_marker() const
{
  return marker_;
}

inline void
SpikeData::reset_marker()
{
  marker_ = SPIKE_DATA_ID_DEFAULT;
}

inline void
SpikeData::set_complete_marker()
{
  marker_ = SPIKE_DATA_ID_COMPLETE;
}

inline void
SpikeData::set_end_marker()
{
  marker_ = SPIKE_DATA_ID_END;
}

inline void
SpikeData::set_invalid_marker()
{
  marker_ = SPIKE_DATA_ID_INVALID;
}

inline bool
SpikeData::is_complete_marker() const
{
  return marker_ == SPIKE_DATA_ID_COMPLETE;
}

inline bool
SpikeData::is_end_marker() const
{
  return marker_ == SPIKE_DATA_ID_END;
}

inline bool
SpikeData::is_invalid_marker() const
{
  return marker_ == SPIKE_DATA_ID_INVALID;
}

inline double
SpikeData::get_offset() const
{
  return 0;
}

class OffGridSpikeData : public SpikeData
{
private:
  double offset_;

public:
  OffGridSpikeData();
  OffGridSpikeData( const Target& target, const size_t lag, const double offset );
  OffGridSpikeData( const size_t tid,
    const synindex syn_id,
    const size_t lcid,
    const unsigned int lag,
    const double offset );
  OffGridSpikeData( const OffGridSpikeData& rhs );
  OffGridSpikeData& operator=( const OffGridSpikeData& rhs );
  OffGridSpikeData& operator=( const SpikeData& rhs );
  void set( const size_t tid, const synindex syn_id, const size_t lcid, const unsigned int lag, const double offset );

  template < class TargetT >
  void set( const TargetT& target, const unsigned int lag );
  double get_offset() const;
};

//! check legal size
using success_offgrid_spike_data_size = StaticAssert< sizeof( OffGridSpikeData ) == 16 >::success;

inline OffGridSpikeData::OffGridSpikeData()
  : SpikeData()
  , offset_( 0.0 )
{
}

inline OffGridSpikeData::OffGridSpikeData( const Target& target, const size_t lag, const double offset )
  : SpikeData( target, lag )
  , offset_( offset )
{
}

inline OffGridSpikeData::OffGridSpikeData( const size_t tid,
  const synindex syn_id,
  const size_t lcid,
  const unsigned int lag,
  const double offset )
  : SpikeData( tid, syn_id, lcid, lag )
  , offset_( offset )
{
}

inline OffGridSpikeData::OffGridSpikeData( const OffGridSpikeData& rhs )
  : SpikeData( rhs )
  , offset_( rhs.offset_ )
{
}

inline OffGridSpikeData&
OffGridSpikeData::operator=( const OffGridSpikeData& rhs )
{
  lcid_ = rhs.lcid_;
  marker_ = rhs.marker_;
  lag_ = rhs.lag_;
  tid_ = rhs.tid_;
  syn_id_ = rhs.syn_id_;
  offset_ = rhs.offset_;
  return *this;
}

inline OffGridSpikeData&
OffGridSpikeData::operator=( const SpikeData& rhs )
{
  // Need to use get_*() here, direct access to protected members of base-class instance is prohibited,
  // see example in https://en.cppreference.com/w/cpp/language/access.
  lcid_ = rhs.get_lcid();
  marker_ = rhs.get_marker();
  lag_ = rhs.get_lag();
  tid_ = rhs.get_tid();
  syn_id_ = rhs.get_syn_id();
  offset_ = 0;
  return *this;
}

inline void
OffGridSpikeData::set( const size_t tid,
  const synindex syn_id,
  const size_t lcid,
  const unsigned int lag,
  const double offset )
{
  assert( tid <= MAX_TID ); // MAX_TID is allowed since it is not used as invalid value
  assert( syn_id < MAX_SYN_ID );
  assert( lcid < MAX_LCID );
  assert( lag < MAX_LAG );

  lcid_ = lcid;
  marker_ = SPIKE_DATA_ID_DEFAULT;
  lag_ = lag;
  tid_ = tid;
  syn_id_ = syn_id;
  offset_ = offset;
}


template < class TargetT >
inline void
OffGridSpikeData::set( const TargetT& target, const unsigned int lag )
{
  SpikeData::set( target, lag );
  offset_ = target.get_offset();
}

inline double
OffGridSpikeData::get_offset() const
{
  return offset_;
}


/**
 * Combine target rank and spike data information for storage in emitted_spikes_register.
 *
 * @note Not using std::pair<> to be able to emplace_back().
 */
struct SpikeDataWithRank
{
  SpikeDataWithRank( const Target& target, const size_t lag );

  const size_t rank;          //!< rank of target neuron
  const SpikeData spike_data; //! data on spike transmitted
};

inline SpikeDataWithRank::SpikeDataWithRank( const Target& target, const size_t lag )
  : rank( target.get_rank() )
  , spike_data( target, lag )
{
}

/**
 * Combine target rank and spike data information for storage in emitted_off_grid_spikes_register.
 *
 * @note Not using std::pair<> to be able to emplace_back().
 */
struct OffGridSpikeDataWithRank
{
  OffGridSpikeDataWithRank( const Target& target, const size_t lag, const double offset );

  const size_t rank;                 //!< rank of target neuron
  const OffGridSpikeData spike_data; //! data on spike transmitted
};

inline OffGridSpikeDataWithRank::OffGridSpikeDataWithRank( const Target& target, const size_t lag, const double offset )
  : rank( target.get_rank() )
  , spike_data( target, lag, offset )
{
}


} // namespace nest

#endif /* SPIKE_DATA_H */
