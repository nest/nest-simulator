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
  thread tid : 10; //!< thread index
  synindex syn_index : 8; //!< synapse-type index
  unsigned int lag : 8; //!< lag in this min-delay interval
  unsigned int marker : 2;
  const static unsigned int end_marker; // =1
  const static unsigned int complete_marker; // =2
  const static unsigned int invalid_marker; // =3
  SpikeData();
  SpikeData( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag );
  void set( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag, const double_t offset );
  void reset_marker();
  void set_complete_marker();
  void set_end_marker();
  void set_invalid_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
  bool is_invalid_marker() const;
  double_t get_offset() const;
};

inline
SpikeData::SpikeData()
  : lcid( 0 )
  , tid( 0 )
  , syn_index( 0 )
  , lag( 0 )
  , marker( 0 )
{
}

inline
SpikeData::SpikeData( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag )
  : lcid( lcid )
  , tid( tid )
  , syn_index( syn_index )
  , lag( lag )
  , marker( 0 ) // always initialize with default marker
{
}

inline void
SpikeData::set( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag, const double_t )
{
  (*this).lcid = lcid;
  (*this).tid = tid;
  (*this).syn_index = syn_index;
  (*this).lag = lag;
  marker = 0; // always initialize with default marker
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

inline double_t
SpikeData::get_offset() const
{
  return 0;
}

struct OffGridSpikeData : SpikeData
{
  double_t offset;
  void set( const thread tid, const synindex syn_index, const index lcid, const unsigned int lag, const double_t offset );
  double_t get_offset() const;
};

inline void
OffGridSpikeData::set( const thread tid,
  const synindex syn_index,
  const index lcid,
  const unsigned int lag,
  const double_t offset)
{
  (*this).lcid = lcid;
  (*this).tid = tid;
  (*this).syn_index = syn_index;
  (*this).lag = lag;
  marker = 0; // always initialize with default marker
  (*this).offset = offset;
}

inline double_t
OffGridSpikeData::get_offset() const
{
  return offset;
}

} // namespace nest

#endif /* SPIKE_DATA_H */
