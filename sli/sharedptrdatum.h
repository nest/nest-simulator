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


template < class D, SLIType* slt >
class sharedPtrDatum : public std::shared_ptr< D >, public TypedDatum< slt >
{
  Datum*
  clone( void ) const
  {
    return new sharedPtrDatum< D, slt >( *this );
  }

public:
  sharedPtrDatum()
  {
  }

  //   template<SLIType *st>
  //   sharedPtrDatum(const sharedPtrDatum<D,st> &d):lockPTR<D>(d),
  //   TypedDatum<slt>(){}

  sharedPtrDatum( const std::shared_ptr< D > d )
    : std::shared_ptr< D >( d )
    , TypedDatum< slt >()
  {
  }

  /* Constructor from D* d
     By the definition of lockPTR, d must be unique. It will be
     destructed/deallocated by the implementation of lockPTR,
     therefore no references should be kept after construction,
     including constructing any other instances of this class with
     that data, except via copy constructor.
  */
  sharedPtrDatum( D* d )
    : std::shared_ptr< D >( d )
    , TypedDatum< slt >()
  {
  }

  /* Constructor from D d
     Like the above, this is actually a constructor to a D*, so d
     should be dynamically allocated, and any reference discarded
     after this construction.
   */
  // sharedPtrDatum( D& d )
  //   : std::shared_ptr< D >( d )
  //   , TypedDatum< slt >()
  // {
  // }

  ~sharedPtrDatum()
  {
  } // this class must not be a base class

  void
  print( std::ostream& out ) const
  {
    out << '<' << this->gettypename() << '>';
  }

  void
  pprint( std::ostream& out ) const
  {
    throw NotImplemented( "pprint is not implemented for lockPTRDatum" );
    // out << "<lockPTR[" << this->references() << "]->" << this->gettypename()
    //     << '(' << static_cast< void* >( this->get() ) << ")>";
  }

  void
  info( std::ostream& out ) const
  {
    //  out << *dynamic_cast<C *>(const_cast<lockPTR<C,slt> *>(this));
    pprint( out );
  }

  // tests for equality via lockPTR<D>::operator==
  // It is defined as identity of the underly D, i.e. &this->D == &other->D
  bool
  equals( const Datum* dat ) const
  {
    const sharedPtrDatum< D, slt >* ddc = dynamic_cast< const sharedPtrDatum< D, slt >* >( dat );
    return ddc && *this == *ddc;
  }

  /* operator=
    The assignment operator is defaulted.
    Therefore, lockPTR<D>::operator= is called, and
    TypedDatum<slt>::operator= is called.
    The TypedDatum = is simply return *this.
  */
  // sharedPtrDatumLegacy<D, sli>& operator=(const sharedPtrDatumLegacy<D, sli>&)

  /* operator==
    sharedPtrDatum should only use the equals method for equality testing.
    Thus, the inherited lockPTR<D>::operator== is made private.  No
    implementation is defined.
  */
private:
  bool operator==( std::shared_ptr< D >& );
};

#endif
