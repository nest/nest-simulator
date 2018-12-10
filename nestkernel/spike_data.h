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
 * Used to communicate spikes. These are the elements of the MPI
 * buffers.
 *
 * @see TargetData
 */
class SpikeData
{
protected:
  const static unsigned int default_marker_ = 0;
  const static unsigned int end_marker_ = 1;
  const static unsigned int complete_marker_ = 2;
  const static unsigned int invalid_marker_ = 3;

  index lcid_ : 27;         //!< local connection index
  unsigned int marker_ : 2; //!< status flag
  unsigned int lag_ : 14;   //!< lag in this min-delay interval
  thread tid_ : 10;         //!< thread index
  synindex syn_id_ : 8;     //!< synapse-type index

public:
  SpikeData();
  SpikeData( const SpikeData& rhs );
  SpikeData( const thread tid,
    const synindex syn_id,
    const index lcid,
    const unsigned int lag );

  void set( const thread tid,
    const synindex syn_id,
    const index lcid,
    const unsigned int lag,
    const double offset );

  /**
   * Returns local connection ID.
   */
  index get_lcid() const;

  /**
   * Returns lag in min-delay interval.
   */
  unsigned int get_lag() const;

  /**
   * Returns thread index.
   */
  thread get_tid() const;

  /**
   * Returns synapse-type index.
   */
  synindex get_syn_id() const;

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

inline SpikeData::SpikeData()
  : lcid_( 0 )
  , marker_( default_marker_ )
  , lag_( 0 )
  , tid_( 0 )
  , syn_id_( 0 )
{
}

inline SpikeData::SpikeData( const SpikeData& rhs )
  : lcid_( rhs.lcid_ )
  , marker_( default_marker_ )
  , lag_( rhs.lag_ )
  , tid_( rhs.tid_ )
  , syn_id_( rhs.syn_id_ )
{
}

inline SpikeData::SpikeData( const thread tid,
  const synindex syn_id,
  const index lcid,
  const unsigned int lag )
  : lcid_( lcid )
  , marker_( default_marker_ )
  , lag_( lag )
  , tid_( tid )
  , syn_id_( syn_id )
{
}

inline void
SpikeData::set( const thread tid,
  const synindex syn_id,
  const index lcid,
  const unsigned int lag,
  const double )
{
  assert( tid <= Target::MAX_TID );
  assert( syn_id <= Target::MAX_SYN_ID );
  assert( lcid <= Target::MAX_LCID );
  assert( lag < 16384 );

  lcid_ = lcid;
  marker_ = default_marker_;
  lag_ = lag;
  tid_ = tid;
  syn_id_ = syn_id;
}

inline index
SpikeData::get_lcid() const
{
  return lcid_;
}

inline unsigned int
SpikeData::get_lag() const
{
  return lag_;
}

inline thread
SpikeData::get_tid() const
{
  return tid_;
}

inline synindex
SpikeData::get_syn_id() const
{
  return syn_id_;
}

inline void
SpikeData::reset_marker()
{
  marker_ = default_marker_;
}

inline void
SpikeData::set_complete_marker()
{
  marker_ = complete_marker_;
}

inline void
SpikeData::set_end_marker()
{
  marker_ = end_marker_;
}

inline void
SpikeData::set_invalid_marker()
{
  marker_ = invalid_marker_;
}

inline bool
SpikeData::is_complete_marker() const
{
  return marker_ == complete_marker_;
}

inline bool
SpikeData::is_end_marker() const
{
  return marker_ == end_marker_;
}

inline bool
SpikeData::is_invalid_marker() const
{
  return marker_ == invalid_marker_;
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
  OffGridSpikeData( const thread tid,
    const synindex syn_id,
    const index lcid,
    const unsigned int lag,
    const double offset );
  void set( const thread tid,
    const synindex syn_id,
    const index lcid,
    const unsigned int lag,
    const double offset );
  double get_offset() const;
};

inline OffGridSpikeData::OffGridSpikeData()
  : SpikeData()
  , offset_( 0. )
{
}

inline OffGridSpikeData::OffGridSpikeData( const thread tid,
  const synindex syn_id,
  const index lcid,
  const unsigned int lag,
  const double offset )
  : SpikeData( tid, syn_id, lcid, lag )
  , offset_( offset )
{
}

inline void
OffGridSpikeData::set( const thread tid,
  const synindex syn_id,
  const index lcid,
  const unsigned int lag,
  const double offset )
{
  assert( tid <= Target::MAX_TID );
  assert( syn_id <= Target::MAX_SYN_ID );
  assert( lcid <= Target::MAX_LCID );
  assert( lag < 16384 );

  lcid_ = lcid;
  marker_ = default_marker_;
  lag_ = lag;
  tid_ = tid;
  syn_id_ = syn_id;
  offset_ = offset;
}

inline double
OffGridSpikeData::get_offset() const
{
  return offset_;
}

} // namespace nest

#endif /* SPIKE_DATA_H */
