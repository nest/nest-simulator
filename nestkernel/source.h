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

// C++ include
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"

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
  uint64_t node_id_ : NUM_BITS_NODE_ID; //!< node ID of source
  bool processed_ : 1;                  //!< whether this target has already been moved
                                        //!< to the MPI buffer
  bool primary_ : 1;

public:
  Source();
  explicit Source( const uint64_t node_id, const bool primary );

  /**
   * Sets node_id_ to the specified value.
   */
  void set_node_id( const uint64_t node_id );

  /**
   * Returns this Source's node ID.
   */
  uint64_t get_node_id() const;

  void set_processed( const bool processed );
  bool is_processed() const;

  /**
   * Sets whether Source is primary.
   */
  void set_primary( const bool primary );

  /**
   * Returns whether Source is primary.
   */
  bool is_primary() const;

  /**
   * Disables Source.
   */
  void disable();

  /**
   * Returns whether Source is disabled.
   */
  bool is_disabled() const;

  friend bool operator<( const Source& lhs, const Source& rhs );
  friend bool operator>( const Source& lhs, const Source& rhs );
  friend bool operator==( const Source& lhs, const Source& rhs );
};

inline Source::Source()
  : node_id_( 0 )
  , processed_( false )
  , primary_( true )
{
}

inline Source::Source( const uint64_t node_id, const bool is_primary )
  : node_id_( node_id )
  , processed_( false )
  , primary_( is_primary )
{
  assert( node_id <= MAX_NODE_ID );
}

inline void
Source::set_node_id( const uint64_t node_id )
{
  assert( node_id <= MAX_NODE_ID );
  node_id_ = node_id;
}

inline uint64_t
Source::get_node_id() const
{
  return node_id_;
}

inline void
Source::set_processed( const bool processed )
{
  processed_ = processed;
}

inline bool
Source::is_processed() const
{
  return processed_;
}

inline void
Source::set_primary( const bool primary )
{
  primary_ = primary;
}

inline bool
Source::is_primary() const
{
  return primary_;
}

inline void
Source::disable()
{
  node_id_ = DISABLED_NODE_ID;
}

inline bool
Source::is_disabled() const
{
  return node_id_ == DISABLED_NODE_ID;
}

inline bool operator<( const Source& lhs, const Source& rhs )
{
  return ( lhs.node_id_ < rhs.node_id_ );
}

inline bool operator>( const Source& lhs, const Source& rhs )
{
  return operator<( rhs, lhs );
}

inline bool operator==( const Source& lhs, const Source& rhs )
{
  return ( lhs.node_id_ == rhs.node_id_ );
}

} // namespace nest

#endif // SOURCE_H
