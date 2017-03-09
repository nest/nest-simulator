/*
 *  gid_collection.h
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

#ifndef GID_COLLECTION_H
#define GID_COLLECTION_H

// C++ includes:
#include <ostream>
#include <stdexcept> // out_of_range
#include <utility>   // pair
#include <vector>

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "tokenarray.h"

namespace nest
{

class GIDCollection
{

  std::vector< index > gid_array_;
  std::pair< index, index > gid_range_;
  bool is_range_;

public:
  class const_iterator
  {
    friend class GIDCollection;
    const_iterator( const GIDCollection* gc, size_t offset )
      : gc_( gc )
      , offset_( offset )
    {
    }
    const GIDCollection* gc_;
    size_t offset_;

  public:
    index operator*() const;
    const const_iterator& operator++();
    bool operator!=( const const_iterator& rhs ) const;
  };

  GIDCollection()
  {
  }
  GIDCollection( index first, index last );
  GIDCollection( IntVectorDatum gids );
  GIDCollection( TokenArray gids );

  void print_me( std::ostream& out ) const;

  index operator[]( const size_t pos ) const;
  bool operator==( const GIDCollection& rhs ) const;
  int find( const index ) const;
  bool is_range() const;

  const_iterator begin() const;
  const_iterator end() const;

  size_t size() const;
};

inline int
GIDCollection::find( const index neuron_id ) const
{
  if ( is_range_ )
  {
    if ( neuron_id > gid_range_.second )
    {
      return -1;
    }
    else
    {
      return neuron_id - gid_range_.first;
    }
  }
  else
  {
    for ( size_t i = 0; i < gid_array_.size(); ++i )
    {
      if ( neuron_id == gid_array_[ i ] )
      {
        return i;
      }
    }
    return -1;
  }
}

inline bool
GIDCollection::is_range() const
{
  return is_range_;
}

inline index GIDCollection::const_iterator::operator*() const
{
  return ( *gc_ )[ offset_ ];
}

inline const GIDCollection::const_iterator& GIDCollection::const_iterator::
operator++()
{
  ++offset_;
  return *this;
}

inline bool GIDCollection::const_iterator::operator!=(
  const GIDCollection::const_iterator& rhs ) const
{
  return offset_ != rhs.offset_;
}

inline index GIDCollection::operator[]( const size_t pos ) const
{
  if ( ( is_range_ && pos + gid_range_.first > gid_range_.second )
    || ( not is_range_ && pos >= gid_array_.size() ) )
  {
    throw std::out_of_range( "pos points outside of the GIDCollection" );
  }
  if ( is_range_ )
  {
    return gid_range_.first + pos;
  }
  else
  {
    return gid_array_[ pos ];
  }
}

inline bool GIDCollection::operator==( const GIDCollection& rhs ) const
{
  if ( is_range_ )
  {
    return gid_range_ == rhs.gid_range_;
  }
  else
  {
    return gid_array_ == rhs.gid_array_;
  }
}

inline GIDCollection::const_iterator
GIDCollection::begin() const
{
  return const_iterator( this, 0 );
}

inline GIDCollection::const_iterator
GIDCollection::end() const
{
  return const_iterator( this, size() );
}

inline size_t
GIDCollection::size() const
{
  if ( is_range_ )
  {
    return gid_range_.second - gid_range_.first + 1;
  }
  else
  {
    return gid_array_.size();
  }
}

} // namespace nest

#endif /* #ifndef GID_COLLECTION_H */
