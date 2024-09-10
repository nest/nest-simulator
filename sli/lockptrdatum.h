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
  clone() const
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

  /* operator==
    lockPTRDatum should only use the equals method for equality testing.
    Thus, the inherited lockPTR<D>::operator== is made private.  No
    implementation is defined.
  */
private:
  bool operator==( lockPTR< D >& );
};

/* equals(const Datum* datum)
   returns: true if *this and Datum *dat are both lockptr references
   to the same underlying object.

   The definition of the equals method assumes that no further
   distinguishing data is added by derivation.  Aka, the template
   class is never inherited from, and therefore type equality is
   guaranteed by template parameter equality.
*/
template < class D, SLIType* slt >
bool
lockPTRDatum< D, slt >::equals( const Datum* dat ) const
{
  const lockPTRDatum< D, slt >* ddc = dynamic_cast< const lockPTRDatum< D, slt >* >( dat );
  return ddc and lockPTR< D >::operator==( *ddc );
}

template < class D, SLIType* slt >
void
lockPTRDatum< D, slt >::pprint( std::ostream& out ) const
{
  out << "<lockPTR[" << this->references() << "]->" << this->gettypename() << '(' << static_cast< void* >( this->get() )
      << ")>";
  this->unlock();
}

template < class D, SLIType* slt >
void
lockPTRDatum< D, slt >::print( std::ostream& out ) const
{
  out << '<' << this->gettypename() << '>';
}

template < class D, SLIType* slt >
void
lockPTRDatum< D, slt >::info( std::ostream& out ) const
{
  //  out << *dynamic_cast<C *>(const_cast<lockPTR<C,slt> *>(this));
  pprint( out );
}

#endif
