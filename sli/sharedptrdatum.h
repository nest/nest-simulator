/*
 *  sharedptrdatum.h
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

#ifndef SHAREDPTRDATUM_H
#define SHAREDPTRDATUM_H

#include <memory>

// Includes from sli:
#include "datum.h"

/**
 * @brief Smart pointer data object.
 */
template < class D, SLIType* slt >
class sharedPtrDatum : public std::shared_ptr< D >, public TypedDatum< slt >
{
  Datum*
  clone( void ) const
  {
    return new sharedPtrDatum< D, slt >( *this );
  }

public:
  sharedPtrDatum() = default;

  sharedPtrDatum( const std::shared_ptr< D > d )
    : std::shared_ptr< D >( d )
    , TypedDatum< slt >()
  {
  }

  /**
   * @brief Constructor
   * @param d a pointer to an object to manage
   *
   * The shared_ptr will control the destruction of the object. The object is automatically destroyed,
   * so no references to it should be kept after construction.
   */
  sharedPtrDatum( D* d )
    : std::shared_ptr< D >( d )
    , TypedDatum< slt >()
  {
  }

  ~sharedPtrDatum() = default;

  void
  print( std::ostream& out ) const
  {
    out << '<' << this->gettypename() << '>';
  }

  void
  pprint( std::ostream& out ) const
  {
    out << "<shared_ptr[" << this->use_count() << "]->" << this->gettypename() << '('
        << static_cast< void* >( this->get() ) << ")>";
  }

  void
  info( std::ostream& out ) const
  {
    pprint( out );
  }

  /**
   * @brief Tests for equality between this and another datum.
   * @param dat datum to check against
   */
  bool
  equals( const Datum* other ) const
  {
    const sharedPtrDatum< D, slt >* other_dc = dynamic_cast< const sharedPtrDatum< D, slt >* >( other );
    return other_dc && *this == *other_dc;
  }
};

#endif
