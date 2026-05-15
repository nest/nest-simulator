/*
 *  source.h
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

#ifndef SOURCE_H
#define SOURCE_H

// Includes from nestkernel:
#include "nest_types.h"

#include <cassert>

namespace nest
{

/**
 * Stores the node ID of a presynaptic neuron
 * and the number of local targets, along with a flag, whether this
 * entry has been processed yet. Used in SourceTable.
 */
class Source
{
private:
  uint64_t node_id_ : NUM_BITS_NODE_ID;  //!< node ID of source
  bool processed_ : 1;                   //!< whether this target has already been moved
                                         //!< to the MPI buffer
  bool primary_ : 1;                     //!< source of primary connection
  bool disabled_ : 1;                    //!< connection has been disabled

public:
  Source()
    : node_id_( 0 )
    , processed_( false )
    , primary_( true )
    , disabled_( false )
  {
  }

  explicit Source( const std::uint64_t node_id, const bool primary )
    : node_id_( node_id )
    , processed_( false )
    , primary_( primary )
    , disabled_( false )
  {
    assert( node_id <= MAX_NODE_ID );
  }

  std::uint64_t
  get_node_id() const
  {
    return node_id_;
  }

  void
  set_processed( const bool processed )
  {
    processed_ = processed;
  }

  bool
  is_processed() const
  {
    return processed_;
  }

  void
  set_primary( const bool primary )
  {
    primary_ = primary;
  }

  bool
  is_primary() const
  {
    return primary_;
  }

  void
  disable()
  {
    disabled_ = true;
  }

  [[gnu::visibility( "hidden" )]] bool
  is_disabled() const
  {
    return disabled_;
  }

  friend bool operator<( const Source& lhs, const Source& rhs );
  friend bool operator>( const Source& lhs, const Source& rhs );
  friend bool operator==( const Source& lhs, const Source& rhs );
};

inline bool
operator<( const Source& lhs, const Source& rhs )
{
  return lhs.node_id_ < rhs.node_id_;
}

inline bool
operator>( const Source& lhs, const Source& rhs )
{
  return rhs < lhs;
}

inline bool
operator==( const Source& lhs, const Source& rhs )
{
  return lhs.node_id_ == rhs.node_id_;
}

}  // namespace nest

#endif /* #ifndef SOURCE_H */
