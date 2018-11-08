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
 * Stores the global id of a presynaptic neuron
 * and the number of local targets, along with a flag, whether this
 * entry has been processed yet. Used in SourceTable.
 */
class Source
{
private:
  uint64_t gid_ : 62;  //!< gid of source
  bool processed_ : 1; //!< whether this target has already been moved
                       //!to the MPI buffer
  bool primary_ : 1;
  static constexpr uint64_t GID_DISABLED =
    ( static_cast< uint64_t >( 1 ) << 62 ) - 1; // 2 ** 62 - 1

public:
  Source();
  explicit Source( const uint64_t gid, const bool primary );

  /**
   * Sets gid_ to the specified value.
   */
  void set_gid( const uint64_t gid );

  /**
   * Returns this Source's GID.
   */
  uint64_t get_gid() const;

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
  : gid_( 0 )
  , processed_( false )
  , primary_( true )
{
}

inline Source::Source( const uint64_t gid, const bool is_primary )
  : gid_( gid )
  , processed_( false )
  , primary_( is_primary )
{
  assert( gid < GID_DISABLED );
}

inline void
Source::set_gid( const uint64_t gid )
{
  assert( gid < GID_DISABLED );
  gid_ = gid;
}

inline uint64_t
Source::get_gid() const
{
  return gid_;
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
  gid_ = GID_DISABLED;
}

inline bool
Source::is_disabled() const
{
  return gid_ == GID_DISABLED;
}

inline bool operator<( const Source& lhs, const Source& rhs )
{
  return ( lhs.gid_ < rhs.gid_ );
}

inline bool operator>( const Source& lhs, const Source& rhs )
{
  return operator<( rhs, lhs );
}

inline bool operator==( const Source& lhs, const Source& rhs )
{
  return ( lhs.gid_ == rhs.gid_ );
}

} // namespace nest

#endif // SOURCE_H
