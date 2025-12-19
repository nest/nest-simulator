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
  bool primary_ : 1;                    //!< source of primary connection
  bool disabled_ : 1;                   //!< connection has been disabled

public:
  Source();
  explicit Source( const uint64_t node_id, const bool primary );

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

} // namespace nest

#endif /* #ifndef SOURCE_H */
