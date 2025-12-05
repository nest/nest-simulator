/*
 *  target_data.cpp
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

#include "target_data.h"

#include <cassert>
#include <limits>

namespace nest
{

// --- TargetDataFields ---

void
TargetDataFields::set_lcid( const size_t lcid )
{
  lcid_ = lcid;
}

size_t
TargetDataFields::get_lcid() const
{
  return lcid_;
}

void
TargetDataFields::set_tid( const size_t tid )
{
  tid_ = tid;
}

size_t
TargetDataFields::get_tid() const
{
  return tid_;
}

void
TargetDataFields::set_syn_id( const synindex syn_id )
{
  syn_id_ = syn_id;
}

synindex
TargetDataFields::get_syn_id() const
{
  return syn_id_;
}

// --- SecondaryTargetDataFields ---

void
SecondaryTargetDataFields::set_recv_buffer_pos( const size_t pos )
{
  assert( pos < std::numeric_limits< unsigned int >::max() );
  recv_buffer_pos_ = pos;
}

size_t
SecondaryTargetDataFields::get_recv_buffer_pos() const
{
  return recv_buffer_pos_;
}

void
SecondaryTargetDataFields::set_syn_id( const synindex syn_id )
{
  assert( syn_id < std::numeric_limits< unsigned char >::max() );
  syn_id_ = syn_id;
}

synindex
SecondaryTargetDataFields::get_syn_id() const
{
  return syn_id_;
}

// --- TargetData ---

void
TargetData::reset_marker()
{
  marker_ = TARGET_DATA_ID_DEFAULT;
}

void
TargetData::set_complete_marker()
{
  marker_ = TARGET_DATA_ID_COMPLETE;
}

void
TargetData::set_end_marker()
{
  marker_ = TARGET_DATA_ID_END;
}

void
TargetData::set_invalid_marker()
{
  marker_ = TARGET_DATA_ID_INVALID;
}

bool
TargetData::is_complete_marker() const
{
  return marker_ == TARGET_DATA_ID_COMPLETE;
}

bool
TargetData::is_end_marker() const
{
  return marker_ == TARGET_DATA_ID_END;
}

bool
TargetData::is_invalid_marker() const
{
  return marker_ == TARGET_DATA_ID_INVALID;
}

void
TargetData::set_source_lid( const size_t source_lid )
{
  assert( source_lid < MAX_LID );
  source_lid_ = source_lid;
}

void
TargetData::set_source_tid( const size_t source_tid )
{
  assert( source_tid < MAX_TID );
  source_tid_ = source_tid;
}

size_t
TargetData::get_source_lid() const
{
  return source_lid_;
}

size_t
TargetData::get_source_tid() const
{
  return source_tid_;
}

void
TargetData::set_is_primary( const bool is_primary )
{
  is_primary_ = is_primary;
}

bool
TargetData::is_primary() const
{
  return is_primary_;
}

} // namespace nest
