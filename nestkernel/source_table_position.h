/*
 *  source_table_position.h
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

#ifndef SOURCE_TABLE_POSITION_H
#define SOURCE_TABLE_POSITION_H

// C++ includes:
#include "source.h"
#include <cassert>
#include <iostream>
#include <vector>

#include "block_vector.h"

namespace nest
{

/**
 * Three-tuple to store position in 3d vector of sources.
 */
struct SourceTablePosition
{
  long tid;    //!< thread index
  long syn_id; //!< synapse-type index
  long lcid;   //!< local connection index

  SourceTablePosition();
  SourceTablePosition( const long tid, const long syn_id, const long lcid );
  SourceTablePosition( const SourceTablePosition& rhs ) = default;
  SourceTablePosition& operator=( const SourceTablePosition& rhs ) = default;

  /**
   * Decreases indices until a valid entry is found.
   */
  void seek_to_next_valid_index( const std::vector< std::vector< BlockVector< Source > > >& sources );

  /**
   * Decreases the inner most index (lcid).
   */
  void decrease();

  /**
   * Returns true if the indices point outside the SourceTable, e.g.,
   * to signal that the end was reached.
   */
  bool is_invalid() const;
};

bool operator==( const SourceTablePosition& lhs, const SourceTablePosition& rhs );
bool operator!=( const SourceTablePosition& lhs, const SourceTablePosition& rhs );
bool operator<( const SourceTablePosition& lhs, const SourceTablePosition& rhs );
bool operator>( const SourceTablePosition& lhs, const SourceTablePosition& rhs );
bool operator<=( const SourceTablePosition& lhs, const SourceTablePosition& rhs );
bool operator>=( const SourceTablePosition& lhs, const SourceTablePosition& rhs );


} // namespace nest

#endif /* #ifndef SOURCE_TABLE_POSITION_H */
