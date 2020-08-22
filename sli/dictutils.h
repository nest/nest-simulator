/*
 *  dictutils.h
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

#ifndef DICTUTILS_H
#define DICTUTILS_H

// C++ includes:
#include <algorithm>
#include <functional>
#include <string>

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "namedatum.h"
#include "tokenutils.h"

/**
 * @defgroup DictUtils How to access the value contained in a Token contained in
 * a dictionary.
 * @ingroup TokenHandling
 *
 * Class Dictionary defines the standard user interface for accessing tokens
 * from dictionaries (see there). However, this user interface returns
 * tokens, from which the actual value would still need to be
 * extracted. The utilitiy functions described in this group shortcut
 * this step and provide direct access to the underlying fundamental
 * values associated to a dictionary entry.
 */

/** Get the value of an existing dictionary entry.
 * @ingroup DictUtils
 * @throws UnknownName An entry of the given name is not known in the
 * dictionary.
 */
template < typename FT >
FT
getValue( const DictionaryDatum& d, Name const n )
{
  // We must take a reference, so that access information can be stored in the
  // token.
  const Token& t = d->lookup2( n );

  /* if (!t.empty()) */
  /*   throw UndefinedName(n.toString()); */

  return getValue< FT >( t );
}

/** Get the value of an existing dictionary entry and check that it is in a
 * specified range. The range is specified by two parameters min and max which
 * have the same type as the template argument.
 * The last parameter mode defines the type of the range:
 * Mode    Relation
 *-------------------
 *  0      min < x < max
 *  1      min <= x < max
 *  2      min <= x <= max
 *
 * @ingroup DictUtils
 * @throws UnknownName An entry of the given name is not known in the
 * dictionary.
 * @throws RangeCheck if a value is outside the range
 */
inline double
get_double_in_range( const DictionaryDatum& d, Name const n, double min, double max, int mode = 2 )
{
  // We must take a reference, so that access information can be stored in the
  // token.
  const Token& t = d->lookup2( n );
  DoubleDatum* dd = dynamic_cast< DoubleDatum* >( t.datum() );
  double x = 0.0;

  if ( dd != 0 )
  {
    x = dd->get();
  }
  else
  {
    IntegerDatum* id = dynamic_cast< IntegerDatum* >( t.datum() );
    if ( id == 0 )
    {
      throw TypeMismatch();
    }

    x = static_cast< double >( id->get() );
  }
  switch ( mode )
  {
  case 0:
    if ( min < x and x < max )
    {
      return x;
    }
    break;
  case 1:
    if ( min <= x and x < max )
    {
      return x;
    }
    break;
  case 2:
    if ( min <= x and x <= max )
    {
      return x;
    }
    break;
  default:
    return x;
  }
  throw RangeCheck();
}

/** Get the value of an existing dictionary entry and check that it is in a
 * specified range. The range is specified by two parameters min and max which
 * have the same type as the template argument.
 * The last parameter mode defines the type of the range:
 * Mode    Relation
 *-------------------
 *  0      min < x < max
 *  1      min <= x < max
 *  2      min <= x <= max
 *
 * @ingroup DictUtils
 * @throws UnknownName An entry of the given name is not known in the
 * dictionary.
 * @throws RangeCheck if a value is outside the range
 */
inline long
get_long_in_range( const DictionaryDatum& d, Name const n, long min, long max, int mode = 2 )
{
  // We must take a reference, so that access information can be stored in the
  // token.
  const Token& t = d->lookup2( n );
  DoubleDatum* dd = dynamic_cast< DoubleDatum* >( t.datum() );
  long x = 0;

  if ( dd != 0 )
  {
    x = dd->get();
  }
  else
  {
    IntegerDatum* id = dynamic_cast< IntegerDatum* >( t.datum() );
    if ( id == 0 )
    {
      throw TypeMismatch();
    }

    x = static_cast< double >( id->get() );
  }
  switch ( mode )
  {
  case 0:
    if ( min < x and x < max )
    {
      return x;
    }
    break;
  case 1:
    if ( min <= x and x < max )
    {
      return x;
    }
    break;
  case 2:
    if ( min <= x and x <= max )
    {
      return x;
    }
    break;
  default:
    return x;
  }
  throw RangeCheck();
}


/** Define a new dictionary entry from a fundamental type.
 * @ingroup DictUtils
 * @throws TypeMismatch Fundamental type and requested SLI type are
 * incompatible.
 */
template < typename FT, class D >
void
def2( DictionaryDatum& d, Name const n, FT const& value )
{
  Token t = newToken2< FT, D >( value );
  d->insert_move( n, t );
}

/** Define a new dictionary entry from a fundamental type.
 * @ingroup DictUtils
 * @throws TypeMismatch Creating a Token from the fundamental type failed,
 *         probably due to a missing template specialization.
 */
template < typename FT >
void
def( DictionaryDatum& d, Name const n, FT const& value )
{
  Token t( value ); // we hope that we have a constructor for this.
  d->insert_move( n, t );
}

/** Update a variable from a dictionary entry if it exists, skip call if it
 * doesn't.
 * @ingroup DictUtils
 * @throws see getValue(DictionaryDatum, Name)
 */
template < typename FT, typename VT >
bool
updateValue( DictionaryDatum const& d, Name const n, VT& value )
{
  // We will test for the name, and do nothing if it does not exist,
  // instead of simply trying to getValue() it and catching a possible
  // exception. The latter works, however, but non-existing names are
  // the rule with updateValue(), not the exception, hence using the
  // exception mechanism would be inappropriate. (Markus pointed this
  // out, 05.02.2001, Ruediger.)

  // We must take a reference, so that access information can be stored in the
  // token.
  const Token& t = d->lookup( n );

  if ( t.empty() )
  {
    return false;
  }

  value = getValue< FT >( t );
  return true;
}

/** Call a member function of an object, passing the value of an dictionary
 *  entry if it exists, skip call if it doesn't.
 * @ingroup DictUtils
 * @throws see getValue(DictionaryDatum, Name)
 */
template < typename FT, typename VT, class C >
void
updateValue2( DictionaryDatum const& d, Name const n, C& obj, void ( C::*setfunc )( VT ) )
{
  if ( d->known( n ) ) // Does name exist in the dictionary?
  {
    // yes, call the function for update.
    ( obj.*setfunc )( getValue< FT >( d, n ) );
  }
}


/** Create a property of type ArrayDatum in the dictionary, if it does not
 * already exist.
 * @ingroup DictUtils
 */
void initialize_property_array( DictionaryDatum& d, Name propname );


/** Create a property of type DoubleVectorDatum in the dictionary, if it does
 * not already exist.
 * @ingroup DictUtils
 */
void initialize_property_doublevector( DictionaryDatum& d, Name propname );


/** Create a property of type IntVectorDatum in the dictionary, if it does not
 * already exist.
 * @ingroup DictUtils
 */
void initialize_property_intvector( DictionaryDatum& d, Name propname );


/** Append a value to a property ArrayDatum in the dictionary.
 * This is the version for scalar values
 * @ingroup DictUtils
 */
template < typename PropT >
inline void
append_property( DictionaryDatum& d, Name propname, const PropT& prop )
{
  Token t = d->lookup( propname );
  assert( not t.empty() );

  ArrayDatum* arrd = dynamic_cast< ArrayDatum* >( t.datum() );
  assert( arrd != 0 );

  Token prop_token( prop );
  arrd->push_back_dont_clone( prop_token );
}

/** Append a value to a property DoubleVectorDatum in the dictionary.
 * This is a specialization for appending vector<double>s to vector<double>s
 * @ingroup DictUtils
 */
template <>
inline void
append_property< std::vector< double > >( DictionaryDatum& d, Name propname, const std::vector< double >& prop )
{
  Token t = d->lookup( propname );
  assert( not t.empty() );

  DoubleVectorDatum* arrd = dynamic_cast< DoubleVectorDatum* >( t.datum() );
  assert( arrd != 0 );

  ( *arrd )->insert( ( *arrd )->end(), prop.begin(), prop.end() );
}


/** Append a value to a property IntVectorDatum in the dictionary.
 * This is a specialization for appending vector<long>s to vector<long>s
 * @ingroup DictUtils
 */
template <>
inline void
append_property< std::vector< long > >( DictionaryDatum& d, Name propname, const std::vector< long >& prop )
{
  Token t = d->lookup( propname );
  assert( not t.empty() );

  IntVectorDatum* arrd = dynamic_cast< IntVectorDatum* >( t.datum() );
  assert( arrd != 0 );

  ( *arrd )->insert( ( *arrd )->end(), prop.begin(), prop.end() );
}


/** Provide a value to a property DoubleVectorDatum in the dictionary.
 * In contrast to append_property, this function adds the value only once
 * to the property. On all subsequent events, it ensures that the value
 * passed in is identical to the value present. This is needed by
 * recording_decive.
 * @ingroup DictUtils
 */
void provide_property( DictionaryDatum&, Name, const std::vector< double >& );

/** Provide a value to a property IntVectorDatum in the dictionary.
 * In contrast to append_property, this function adds the value only once
 * to the property. On all subsequent events, it ensures that the value
 * passed in is identical to the value present. This is needed by
 * recording_decive.
 * @ingroup DictUtils
 */
void provide_property( DictionaryDatum&, Name, const std::vector< long >& );


/** Add values of a vector<double> to a property DoubleVectorDatum in the
 * dictionary. This variant of append_property is for adding vector<double>s to
 * vector<double>s of the same size. It is required for collecting data across
 * threads when multimeter is running in accumulation mode.
 * @ingroup DictUtils
 */
void accumulate_property( DictionaryDatum&, Name, const std::vector< double >& );

#endif
