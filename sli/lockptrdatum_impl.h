/*
 *  lockptrdatum_impl.h
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

#ifndef LOCKPTRDATUMIMPL_H
#define LOCKPTRDATUMIMPL_H

#include "lockptrdatum.h"

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
  return ddc && lockPTR< D >::operator==( *ddc );
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
