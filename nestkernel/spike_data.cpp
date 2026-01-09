/*
 *  spike_data.cpp
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

#include "spike_data.h"
#include "target.h"

#include <cassert>

namespace nest
{

// --- SpikeData ---

SpikeData::SpikeData()
  : lcid_( 0 )
  , marker_( SPIKE_DATA_ID_DEFAULT )
  , lag_( 0 )
  , tid_( 0 )
  , syn_id_( 0 )
{
}

SpikeData::SpikeData( const SpikeData& rhs )
  : lcid_( rhs.lcid_ )
  , marker_( rhs.marker_ )
  , lag_( rhs.lag_ )
  , tid_( rhs.tid_ )
  , syn_id_( rhs.syn_id_ )
{
}

SpikeData::SpikeData( const Target& target, const size_t lag )
  : lcid_( target.get_lcid() )
  , marker_( SPIKE_DATA_ID_DEFAULT )
  , lag_( lag )
  , tid_( target.get_tid() )
  , syn_id_( target.get_syn_id() )
{
}

SpikeData::SpikeData( const size_t tid, const synindex syn_id, const size_t lcid, const unsigned int lag )
  : lcid_( lcid )
  , marker_( SPIKE_DATA_ID_DEFAULT )
  , lag_( lag )
  , tid_( tid )
  , syn_id_( syn_id )
{
}

SpikeData&
SpikeData::operator=( const SpikeData& rhs )
{
  lcid_ = rhs.lcid_;
  marker_ = rhs.marker_;
  lag_ = rhs.lag_;
  tid_ = rhs.tid_;
  syn_id_ = rhs.syn_id_;
  return *this;
}

void
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

size_t
SpikeData::get_lcid() const
{
  return lcid_;
}

void
SpikeData::set_lcid( size_t value )
{
  assert( value < MAX_LCID );
  lcid_ = value;
}


double
SpikeData::get_offset() const
{
  return 0.0;
}

unsigned int
SpikeData::get_lag() const
{
  return lag_;
}

size_t
SpikeData::get_tid() const
{
  return tid_;
}

synindex
SpikeData::get_syn_id() const
{
  return syn_id_;
}

unsigned int
SpikeData::get_marker() const
{
  return marker_;
}

void
SpikeData::reset_marker()
{
  marker_ = SPIKE_DATA_ID_DEFAULT;
}

void
SpikeData::set_complete_marker()
{
  marker_ = SPIKE_DATA_ID_COMPLETE;
}

void
SpikeData::set_end_marker()
{
  marker_ = SPIKE_DATA_ID_END;
}

void
SpikeData::set_invalid_marker()
{
  marker_ = SPIKE_DATA_ID_INVALID;
}

bool
SpikeData::is_complete_marker() const
{
  return marker_ == SPIKE_DATA_ID_COMPLETE;
}

bool
SpikeData::is_end_marker() const
{
  return marker_ == SPIKE_DATA_ID_END;
}

bool
SpikeData::is_invalid_marker() const
{
  return marker_ == SPIKE_DATA_ID_INVALID;
}


// --- OffGridSpikeData ---

OffGridSpikeData::OffGridSpikeData()
  : SpikeData()
  , offset_( 0.0 )
{
}

OffGridSpikeData::OffGridSpikeData( const Target& target, const size_t lag, const double offset )
  : SpikeData( target, lag )
  , offset_( offset )
{
}

OffGridSpikeData::OffGridSpikeData( const size_t tid,
  const synindex syn_id,
  const size_t lcid,
  const unsigned int lag,
  const double offset )
  : SpikeData( tid, syn_id, lcid, lag )
  , offset_( offset )
{
}

OffGridSpikeData::OffGridSpikeData( const OffGridSpikeData& rhs )
  : SpikeData( rhs )
  , offset_( rhs.offset_ )
{
}

OffGridSpikeData&
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

OffGridSpikeData&
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

void
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
// --- SpikeDataWithRank ---

SpikeDataWithRank::SpikeDataWithRank( const Target& target, const size_t lag )
  : rank( target.get_rank() )
  , spike_data( target, lag )
{
}

// --- OffGridSpikeDataWithRank ---

OffGridSpikeDataWithRank::OffGridSpikeDataWithRank( const Target& target, const size_t lag, const double offset )
  : rank( target.get_rank() )
  , spike_data( target, lag, offset )
{
}

} // namespace nest
