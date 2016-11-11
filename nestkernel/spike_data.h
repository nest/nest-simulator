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

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

/**
 * Structure used to communicate spikes. These are the elements of the
 * MPI buffers.
 * SeeAlso: TargetData
 */
struct SpikeData
{
  index lcid : 27; //!< local connection index
  unsigned int marker : 2; //!< status flag
  unsigned int lag : 14; //!< lag in this min-delay interval
  thread tid : 10; //!< thread index
  synindex syn_index : 8; //!< synapse-type index
  const static unsigned int end_marker = 1;
  const static unsigned int complete_marker = 2;
  const static unsigned int invalid_marker = 3;
  SpikeData();
  SpikeData( const SpikeData& rhs );
  SpikeData( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag );
  void set( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag, const double offset );
  void reset_marker();
  void set_complete_marker();
  void set_end_marker();
  void set_invalid_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
  bool is_invalid_marker() const;
  double get_offset() const;
};

inline
SpikeData::SpikeData()
  : lcid( 0 )
  , marker( 0 )
  , lag( 0 )
  , tid( 0 )
  , syn_index( 0 )
{
}

inline
SpikeData::SpikeData( const SpikeData& rhs )
  : lcid( rhs.lcid )
  , marker( 0 ) // always initialize with default marker
  , lag( rhs.lag )
  , tid( rhs.tid )
  , syn_index( rhs.syn_index )
{
}

inline
SpikeData::SpikeData( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag )
  : lcid( lcid )
  , marker( 0 ) // always initialize with default marker
  , lag( lag )
  , tid( tid )
  , syn_index( syn_index )
{
}

inline void
SpikeData::set( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag, const double )
{
  (*this).lcid = lcid;
  marker = 0; // always initialize with default marker
  (*this).lag = lag;
  (*this).tid = tid;
  (*this).syn_index = syn_index;
}

inline void
SpikeData::reset_marker()
{
  marker = 0;
}

inline void
SpikeData::set_complete_marker()
{
  marker = complete_marker;
}

inline void
SpikeData::set_end_marker()
{
  marker = end_marker;
}

inline void
SpikeData::set_invalid_marker()
{
  marker = invalid_marker;
}

inline bool
SpikeData::is_complete_marker() const
{
  return marker == complete_marker;
}

inline bool
SpikeData::is_end_marker() const
{
  return marker == end_marker;
}

inline bool
SpikeData::is_invalid_marker() const
{
  return marker == invalid_marker;
}

inline double
SpikeData::get_offset() const
{
  return 0;
}

struct OffGridSpikeData : SpikeData
{
  double offset;
  void set( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag, const double offset );
  double get_offset() const;
};

inline void
OffGridSpikeData::set( const thread tid,
  const synindex syn_index,
  const index lcid,
  const unsigned int lag,
  const double offset)
{
  (*this).lcid = lcid;
  marker = 0; // always initialize with default marker
  (*this).lag = lag;
  (*this).tid = tid;
  (*this).syn_index = syn_index;
  (*this).offset = offset;
}

inline double
OffGridSpikeData::get_offset() const
{
  return offset;
}

} // namespace nest

#endif /* SPIKE_DATA_H */
