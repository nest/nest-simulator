/*
 *  lockptrdatum.h
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

#ifndef LOCKPTRDATUM_H
#define LOCKPTRDATUM_H

// Includes from libnestutil:
#include "lockptr.h"

// Includes from sli:
#include "datum.h"

// prefixed all references to members of lockPTR, TypedDatum with this->,
// since HP's aCC otherwise complains about them not being declared
// according to ISO Standard Sec. 14.6.2(3) [temp.dep]
// HEP, 2001-08-09

/* lockPTRDatum<class D, SLIType *slt>:
   Constraints:
     This class must not be a base class.
     The equals operator depends on that fact.
*/
template < class D, SLIType* slt >
class lockPTRDatum : public lockPTR< D >, public TypedDatum< slt >
{
  Datum*
  clone( void ) const
  {
    return new lockPTRDatum< D, slt >( *this );
  }

public:
  lockPTRDatum()
  {
  }

  //   template<SLIType *st>
  //   lockPTRDatum(const lockPTRDatum<D,st> &d):lockPTR<D>(d),
  //   TypedDatum<slt>(){}

  lockPTRDatum( const lockPTR< D > d )
    : lockPTR< D >( d )
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
  lockPTRDatum( D* d )
    : lockPTR< D >( d )
    , TypedDatum< slt >()
  {
  }

  /* Constructor from D d
     Like the above, this is actually a constructor to a D*, so d
     should be dynamically allocated, and any reference discarded
     after this construction.
   */
  lockPTRDatum( D& d )
    : lockPTR< D >( d )
    , TypedDatum< slt >()
  {
  }

  ~lockPTRDatum()
  {
  } // this class must not be a base class

  void print( std::ostream& ) const;
  void pprint( std::ostream& ) const;
  void info( std::ostream& ) const;


  // tests for equality via lockPTR<D>::operator==
  // It is defined as identity of the underly D, i.e. &this->D == &other->D
  bool equals( const Datum* ) const;

  /* operator=
    The assignment operator is defaulted.
    Therefore, lockPTR<D>::operator= is called, and
    TypedDatum<slt>::operator= is called.
    The TypedDatum = is simply return *this.
  */
  // lockPTRDatum<D, sli>& operator=(const lockPTRDatum<D, sli>&)

  /* operator==
    lockPTRDatum should only use the equals method for equality testing.
    Thus, the inherited lockPTR<D>::operator== is made private.  No
    implementation is defined.
  */
private:
  bool operator==( lockPTR< D >& );
};


/******************************************/

#endif
