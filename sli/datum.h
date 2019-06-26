/*
 *  datum.h
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

#ifndef DATUM_H
#define DATUM_H

// Includes from sli:
#include "slitype.h"

/***********************************************************/
/* Datum                                                   */
/* -----                                                   */
/*  base class for all Data Objects                        */
/***********************************************************/
class Datum
{

  friend class Token;

  /**
   * Virtual copy constructor.
   * Use this function to lazily copy a datum.
   */
  virtual Datum* clone( void ) const = 0;


  /**
   * Returns a reference counted pointer to the datum, or a new pointer, if the
   * type does not support reference counting.
   * The prefix const indicates that the pointer should be trated as const
   * because changes affect all other references as well.
   */

  virtual Datum*
  get_ptr()
  {
    return clone();
  }

protected:
  // Putting the following variables here, avoids a number of virtual
  // functions.

  const SLIType* type;       //!< Pointer to type object.
  const SLIFunction* action; //!< Shortcut to the SLIType default action.
  mutable unsigned int reference_count_;
  bool executable_;


  Datum()
    : type( NULL )
    , action( NULL )
    , reference_count_( 1 )
    , executable_( true )
  {
  }


  Datum( const SLIType* t )
    : type( t )
    , action( t->getaction() )
    , reference_count_( 1 )
    , executable_( true )
  {
  }

  Datum( const Datum& d )
    : type( d.type )
    , action( d.action )
    , reference_count_( 1 )
    , executable_( d.executable_ )
  {
  }


public:
  virtual ~Datum(){};


  void
  addReference() const
  {
    ++reference_count_;
  }

  void
  removeReference()
  {
    --reference_count_;
    if ( reference_count_ == 0 )
    {
      delete this;
    }
  }

  size_t
  numReferences() const
  {
    return reference_count_;
  }

  bool
  is_executable() const
  {
    return executable_;
  }

  void
  set_executable()
  {
    executable_ = true;
  }

  void
  unset_executable()
  {
    executable_ = false;
  }

  virtual void print( std::ostream& ) const = 0;
  virtual void pprint( std::ostream& ) const = 0;

  virtual void
  list( std::ostream& out, std::string prefix, int length ) const
  {
    if ( length == 0 )
    {
      prefix = "-->" + prefix;
    }
    else
    {
      prefix = "   " + prefix;
    }
    out << prefix;
    print( out );
  }

  virtual void
  input_form( std::ostream& out ) const
  {
    pprint( out );
  }

  virtual bool
  equals( const Datum* d ) const
  {
    return this == d;
  }

  virtual void info( std::ostream& ) const;

  const Name&
  gettypename( void ) const
  {
    return type->gettypename();
  }

  bool
  isoftype( SLIType const& t ) const
  {
    // or: *type==t, there is only one t with same contents !
    return ( type == &t );
  }

  virtual void
  execute( SLIInterpreter* i )
  {
    action->execute( i );
  }
};

template < SLIType* slt >
class TypedDatum : public Datum
{
public:
  TypedDatum( void )
    : Datum( slt )
  {
  }

  // This is here solely for default assignment operators in
  // derived classes synthesized by the compiler. It does nothing but return
  // itself.
protected:
  TypedDatum( const TypedDatum< slt >& d )
    : Datum( d )
  {
  }
  const TypedDatum< slt >& operator=( const TypedDatum< slt >& );
};

template < SLIType* slt >
inline const TypedDatum< slt >& TypedDatum< slt >::operator=( const TypedDatum< slt >& )
{
  //  assert( type == d.type );
  return *this;
}


#endif
