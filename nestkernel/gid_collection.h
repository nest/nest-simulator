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
#include <vector>

// Includes from libnestuil:
#include "lockptr.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "tokenarray.h"

namespace nest
{
class GIDCollection;
class GIDCollectionPrimitive;
class GIDCollectionComposite;
class GIDCollectionMetadata;

typedef lockPTR< GIDCollection > GIDCollectionPTR;
typedef lockPTR< GIDCollectionMetadata > GIDCollectionMetadataPTR;

/**
 * Class for Metadata attached to GIDCollection.
 *
 * NEST modules that want to add metadata to GIDCollections they
 * create need to implement their own concrete subclass.
 */
class GIDCollectionMetadata
{
public:
  GIDCollectionMetadata()
  {
  }
  virtual ~GIDCollectionMetadata()
  {
  }
};

class GIDPair
{
public:
  index gid;
  index model_id;
};

class gc_const_iterator
{
private:
  index current_;
  const GIDCollectionPrimitive& collection_;

public:
  gc_const_iterator( const GIDCollectionPrimitive& collection, index current )
    : current_( current )
    , collection_( collection )
  {
  }
  GIDPair operator*() const;
  const gc_const_iterator operator++();
  bool operator!=( const gc_const_iterator& rhs ) const;
};


class GIDCollection
{
  friend class gc_const_iterator;

public:
  typedef gc_const_iterator const_iterator;

  virtual ~GIDCollection()
  {
  }

  virtual void print_me( std::ostream& ) const = 0;

  virtual index operator[]( size_t ) const = 0;
  virtual GIDCollectionPTR operator+( GIDCollectionPTR ) const = 0;
  virtual bool operator==( GIDCollectionPTR ) const = 0;
  virtual bool operator!=( GIDCollectionPTR ) const;

  virtual const_iterator begin() const = 0;
  virtual const_iterator end() const = 0;

  virtual size_t size() const = 0;

  virtual bool contains( index gid ) const = 0;
  virtual GIDCollectionPTR
  slice( size_t first, size_t last, size_t step ) const = 0;

  virtual void set_metadata( GIDCollectionMetadataPTR );

  virtual GIDCollectionMetadataPTR get_metadata() const = 0;
};

class GIDCollectionPrimitive : public GIDCollection
{
  friend class gc_const_iterator;

private:
  index first_;
  index last_;
  index model_id_;
  GIDCollectionMetadataPTR metadata_;

public:
  typedef gc_const_iterator const_iterator;

  GIDCollectionPrimitive( index first,
    index last,
    index model_id,
    GIDCollectionMetadataPTR );
  GIDCollectionPrimitive( index first, index last, index model_id );
  GIDCollectionPrimitive( index first, index last );
  GIDCollectionPrimitive( const GIDCollectionPrimitive& );

  void print_me( std::ostream& ) const;

  index operator[]( const size_t ) const;
  GIDCollectionPTR operator+( GIDCollectionPTR rhs ) const;
  bool operator==( const GIDCollectionPTR rhs ) const;

  const_iterator begin() const;
  const_iterator end() const;

  size_t size() const;

  bool contains( index gid ) const;
  GIDCollectionPTR slice( size_t first, size_t last, size_t step = 1 ) const;

  void set_metadata( GIDCollectionMetadataPTR );

  GIDCollectionMetadataPTR get_metadata() const;
};


class GIDCollectionComposite : public GIDCollection
{
  friend class gc_const_iterator;

public:
  GIDCollectionComposite( const ArrayDatum iterable, index model_id );
  GIDCollectionComposite( index first, index last );
  GIDCollectionComposite( TokenArray gids );
  GIDCollectionComposite( IntVectorDatum gids );
  GIDCollectionComposite( const GIDCollectionPrimitive& prim,
    size_t start,
    size_t stop,
    size_t step );
};

inline bool GIDCollection::operator!=( GIDCollectionPTR rhs ) const
{
  return not( *this == rhs );
}

inline void GIDCollection::set_metadata( GIDCollectionMetadataPTR )
{
  throw KernelException( "Cannot set Metadata on this type of GIDCollection." );
}

inline GIDPair gc_const_iterator::operator*() const
{
  GIDPair gp;
  gp.gid = collection_.first_ + current_;
  gp.model_id = collection_.model_id_;
  return gp;
}

inline const gc_const_iterator gc_const_iterator::operator++()
{
  ++current_;
  return *this;
}

inline bool gc_const_iterator::operator!=( const gc_const_iterator& rhs ) const
{
  return current_ != rhs.current_;
}

inline index GIDCollectionPrimitive::operator[]( const size_t idx ) const
{
  // throw exception if outside of GIDCollection
  if ( first_ + idx > last_ )
    throw std::out_of_range( "pos points outside of the GIDCollection" );
  return first_ + idx;
}

inline bool GIDCollectionPrimitive::operator==( GIDCollectionPTR rhs ) const
{
  GIDCollectionPrimitive const* const rhs_ptr =
    dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );
  rhs.unlock();

  return first_ == rhs_ptr->first_ and last_ == rhs_ptr->last_
    and model_id_ == rhs_ptr->model_id_ and metadata_ == rhs_ptr->metadata_;
}

inline GIDCollectionPrimitive::const_iterator
GIDCollectionPrimitive::begin() const
{
  return const_iterator( *this, 0 );
}

inline GIDCollectionPrimitive::const_iterator
GIDCollectionPrimitive::end() const
{
  return const_iterator( *this, size() );
}

inline size_t
GIDCollectionPrimitive::size() const
{
  return last_ - first_ + 1;
}

inline bool
GIDCollectionPrimitive::contains( index gid ) const
{
  return first_ <= gid and gid <= last_;
}

inline void
GIDCollectionPrimitive::set_metadata( GIDCollectionMetadataPTR meta )
{
  metadata_ = meta;
}

inline GIDCollectionMetadataPTR
GIDCollectionPrimitive::get_metadata() const
{
  return metadata_;
}

} // namespace nest

#endif /* #ifndef GID_COLLECTION_H */
