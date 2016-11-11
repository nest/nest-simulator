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

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "tokenarray.h"

namespace nest
{

/*
class GIDCollection
{

public:
  class const_iterator
  {
    private:
      const_iterator( index current, const GIDCollection& collection_ )
        : current_( current )
        , collection_( collection )
      {
      }
      index current_;
      const GIDCollection& collection_;
      
      class GIDPair
      {
        public:
          index gid;
          index model_id;
      };

  public:
    GIDPair operator*() const;
    const iterator operator++();
    bool operator!=( const iterator& rhs ) const;
  };
  */
  /**
  * Class for Metadata attached to GIDCollection.
  *
  * NEST modules that want to add metadata to GIDCollections they
  * create need to implement their own concrete subclass.
  */
  /*
  class GIDCollectionMetadata
  {
  public:
    GIDCollectionMetadata() {}
    virtual ~GIDCollectionMetadata() = 0;
  };

  GIDCollection()
  {
  }

  void print_me( std::ostream& out ) const;

  virtual index operator[]( const size_t i ) = 0;
  virtual const GIDCollection& operator+( const GIDCollection& rhs ) = 0;
  virtual bool operator==( const GIDCollection& rhs ) = 0;

  virtual iterator begin() = 0;
  virtual iterator end() = 0;

  virtual size_t size() = 0;
  
  virtual bool contains( index gid ) = 0;
  virtual const GIDCollection& slice( long first, long last, long step ) = 0;
};

*/
class GIDPair
{
public:
  index gid;
  index model_id;
};  

class GIDCollectionPrimitive // public GIDCollection
{
private:
  index first_;
  index last_;
  long step_; // long/int?
  index model_id_;
  //void* metadata; // ???

public:
  class const_iterator
  {
    
  private:
    index current_;
    const GIDCollectionPrimitive& collection_;

  public: 
    const_iterator( const GIDCollectionPrimitive& collection, index current )
      : current_( current )
      , collection_( collection )
    {
    } 
    GIDPair operator*() const;
    const const_iterator operator++();
    bool operator!=( const const_iterator& rhs ) const;
  };
  

  GIDCollectionPrimitive()
  {
  }
  /*
  // bare i composite
  GIDCollectionPrimitive( const ArrayDatum iterable, index model_id );
  */
  GIDCollectionPrimitive( index first, index last, index model_id);
  GIDCollectionPrimitive( index first, index last);
  GIDCollectionPrimitive( TokenArray gids);
  GIDCollectionPrimitive( IntVectorDatum gids );
  

  void print_me( std::ostream& out ) const;

  index operator[]( const long i ) const;
  const GIDCollectionPrimitive& operator+( 
                                      const GIDCollectionPrimitive& rhs ) const;
  bool operator==( const GIDCollectionPrimitive& rhs ) const;

  const_iterator begin() const;
  const_iterator end() const;

  size_t size() const;
  
  bool contains( index gid ) const;
  const GIDCollectionPrimitive& slice( long first, long last, long step ) const;
};
typedef GIDCollectionPrimitive GIDCollection;

inline GIDPair GIDCollectionPrimitive::const_iterator::operator*() const
{
  GIDPair gp;
  gp.gid = collection_.first_ + current_;
  gp.model_id = collection_.model_id_;
  return gp; //( collection_ )[ current_ ];
}

inline const GIDCollectionPrimitive::const_iterator GIDCollectionPrimitive::const_iterator::
operator++()
{
  ++current_;
  return *this;
}

inline bool GIDCollectionPrimitive::const_iterator::operator!=(
  const GIDCollectionPrimitive::const_iterator& rhs ) const
{
  return current_ != rhs.current_;
}

inline index GIDCollectionPrimitive::operator[]( const long i ) const
{
  // throw exception if outside of GIDCollection
  if ( first_ + i > last_ )
    throw std::out_of_range( "pos points outside of the GIDCollection" );
  return first_ + i;
}

inline bool GIDCollectionPrimitive::operator==( const GIDCollectionPrimitive& rhs ) const
{
  return first_ == rhs.first_ && last_ == rhs.last_;
}

inline GIDCollectionPrimitive::const_iterator GIDCollectionPrimitive::begin() const
{
  return const_iterator( *this, 0 );
}

inline GIDCollectionPrimitive::const_iterator GIDCollectionPrimitive::end() const
{
  return const_iterator( *this, size() );
}

inline size_t
GIDCollectionPrimitive::size() const
{
  return last_ - first_ + 1;
}

} // namespace nest

#endif /* #ifndef GID_COLLECTION_H */
