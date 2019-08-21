/*
 *  genericdatum.h
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

#ifndef GENERICDATUM_H
#define GENERICDATUM_H
/*
    Datum template for generic C/C++ data types
*/

// Includes from sli:
#include "datum.h"

/***********************************************************/
/* Concrete  Generic Data Objects                          */
/***********************************************************/

template < class D, SLIType* slt >
class GenericDatum : public TypedDatum< slt >
{

  virtual Datum*
  clone( void ) const
  {
    return new GenericDatum< D, slt >( *this );
  }

protected:
  D d;

public:
  GenericDatum()
  {
    TypedDatum< slt >::unset_executable();
  }
  virtual ~GenericDatum()
  {
  }

  GenericDatum( const D& d_s )
    : d( d_s )
  {
    TypedDatum< slt >::unset_executable();
  }
  GenericDatum( const GenericDatum< D, slt >& gd )
    : TypedDatum< slt >( gd )
    , d( gd.d )
  {
  }

  const D& operator=( const D& d_s )
  {
    d = d_s;
    return d;
  }

  const D&
  get( void ) const
  {
    return d;
  }

  D&
  get( void )
  {
    return d;
  }

  D&
  get_lval()
  {
    return d;
  }

  void
  print( std::ostream& o ) const
  {
    o << d;
  }

  void
  pprint( std::ostream& o ) const
  {
    o << d;
  }

  void
  info( std::ostream& out ) const
  {
    out << "GenericDatum<D,slt>::info\n";
    out << "d = " << d << std::endl;
  }

  bool
  equals( const Datum* dat ) const
  {
    const GenericDatum< D, slt >* ddc = dynamic_cast< GenericDatum< D, slt >* >( const_cast< Datum* >( dat ) );

    //    std::cerr << "d = " << d << " ddc = " << ddc << " dat = " << dat <<
    //    std::endl;
    if ( ddc == NULL )
    {
      return false;
    }

    return d == ddc->d;
  }
};


/******************************************/

#endif
