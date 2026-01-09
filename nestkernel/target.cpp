/*
 *  target.cpp
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

#include "target.h"

#include <cassert>

namespace nest
{

// ------------ Target ------------

Target::Target()
  : remote_target_id_( 0 )
{
}

Target::Target( const Target& target )
  : remote_target_id_( target.remote_target_id_ )
{
  set_status( TARGET_ID_UNPROCESSED ); // initialize
}

Target&
Target::operator=( const Target& other )
{
  remote_target_id_ = other.remote_target_id_;
  set_status( TARGET_ID_UNPROCESSED );
  return *this;
}

Target::Target( const size_t tid, const size_t rank, const synindex syn_id, const size_t lcid )
  : remote_target_id_( 0 )
{
  // We need to call set_*() methods to properly encode values in bitfield.
  // Validity of arguments is asserted in set_*() methods.
  set_lcid( lcid );
  set_rank( rank );
  set_tid( tid );
  set_syn_id( syn_id );
  set_status( TARGET_ID_UNPROCESSED ); // initialize
}

void
Target::set_lcid( const size_t lcid )
{
  assert( lcid < MAX_LCID );
  remote_target_id_ = ( remote_target_id_ & ( ~MASK_LCID ) ) | ( static_cast< uint64_t >( lcid ) << BITPOS_LCID );
}

size_t
Target::get_lcid() const
{
  return ( ( remote_target_id_ & MASK_LCID ) >> BITPOS_LCID );
}

void
Target::set_rank( const size_t rank )
{
  assert( rank <= MAX_RANK ); // MAX_RANK is allowed since it is not used as invalid value
  remote_target_id_ = ( remote_target_id_ & ( ~MASK_RANK ) ) | ( static_cast< uint64_t >( rank ) << BITPOS_RANK );
}

size_t
Target::get_rank() const
{
  return ( ( remote_target_id_ & MASK_RANK ) >> BITPOS_RANK );
}

void
Target::set_tid( const size_t tid )
{
  assert( tid <= MAX_TID ); // MAX_TID is allowed since it is not used as invalid value
  remote_target_id_ = ( remote_target_id_ & ( ~MASK_TID ) ) | ( static_cast< uint64_t >( tid ) << BITPOS_TID );
}

size_t
Target::get_tid() const
{
  return ( ( remote_target_id_ & MASK_TID ) >> BITPOS_TID );
}

void
Target::set_syn_id( const synindex syn_id )
{
  assert( syn_id < MAX_SYN_ID );
  remote_target_id_ = ( remote_target_id_ & ( ~MASK_SYN_ID ) ) | ( static_cast< uint64_t >( syn_id ) << BITPOS_SYN_ID );
}

synindex
Target::get_syn_id() const
{
  return ( ( remote_target_id_ & MASK_SYN_ID ) >> BITPOS_SYN_ID );
}

void
Target::set_status( enum_status_target_id set_status_to )
{
  switch ( set_status_to )
  {
  case TARGET_ID_PROCESSED:
    remote_target_id_ = remote_target_id_ | MASK_PROCESSED_FLAG; // set single bit
    break;
  case TARGET_ID_UNPROCESSED:
    remote_target_id_ = remote_target_id_ & ~MASK_PROCESSED_FLAG; // clear single bit
    break;
  default:
    throw InternalError( "Invalid remote target id status." );
  }
}

enum_status_target_id
Target::get_status() const
{
  if ( ( remote_target_id_ & MASK_PROCESSED_FLAG ) >> BITPOS_PROCESSED_FLAG ) // test single bit
  {
    return TARGET_ID_PROCESSED;
  }
  return TARGET_ID_UNPROCESSED;
}

bool
Target::is_processed() const
{
  return get_status() == TARGET_ID_PROCESSED;
}

double
Target::get_offset() const
{
  return 0.0;
}

void
Target::mark_for_removal()
{
  set_status( TARGET_ID_PROCESSED );
}


// --------- OffGridTarget ---------

OffGridTarget::OffGridTarget()
  : Target()
  , offset_( 0.0 )
{
}

OffGridTarget::OffGridTarget( const Target& target, const double offset )
  : Target( target )
  , offset_( offset )
{
}

double
OffGridTarget::get_offset() const
{
  return offset_;
}

} // namespace nest
