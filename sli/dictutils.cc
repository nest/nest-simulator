/*
 *  dictutils.cc
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

#include "dictutils.h"

void
initialize_property_array( DictionaryDatum& d, Name propname )
{
  Token t = d->lookup( propname );
  if ( t.empty() )
  {
    ArrayDatum arrd;
    def< ArrayDatum >( d, propname, arrd );
  }
}

void
initialize_property_doublevector( DictionaryDatum& d, Name propname )
{
  Token t = d->lookup( propname );
  if ( t.empty() )
  {
    DoubleVectorDatum arrd( new std::vector< double > );
    def< DoubleVectorDatum >( d, propname, arrd );
  }
}

void
initialize_property_intvector( DictionaryDatum& d, Name propname )
{
  Token t = d->lookup( propname );
  if ( t.empty() )
  {
    IntVectorDatum arrd( new std::vector< long > );
    def< IntVectorDatum >( d, propname, arrd );
  }
}

void
provide_property( DictionaryDatum& d, Name propname, const std::vector< double >& prop )
{
  Token t = d->lookup2( propname );

  DoubleVectorDatum* arrd = dynamic_cast< DoubleVectorDatum* >( t.datum() );
  assert( arrd != 0 );

  if ( ( *arrd )->empty() && not prop.empty() ) // not data from before, add
  {
    ( *arrd )->insert( ( *arrd )->end(), prop.begin(), prop.end() );
  }

  assert( prop.empty() || **arrd == prop ); // not testing for **arrd.empty()
                                            // since that implies prop.empty()
}


void
provide_property( DictionaryDatum& d, Name propname, const std::vector< long >& prop )
{
  Token t = d->lookup2( propname );

  IntVectorDatum* arrd = dynamic_cast< IntVectorDatum* >( t.datum() );
  assert( arrd != 0 );

  if ( ( *arrd )->empty() && not prop.empty() ) // not data from before, add
  {
    ( *arrd )->insert( ( *arrd )->end(), prop.begin(), prop.end() );
  }

  assert( prop.empty() || **arrd == prop ); // not testing for **arrd.empty()
                                            // since that implies prop.empty()
}

void
accumulate_property( DictionaryDatum& d, Name propname, const std::vector< double >& prop )
{
  Token t = d->lookup2( propname );

  DoubleVectorDatum* arrd = dynamic_cast< DoubleVectorDatum* >( t.datum() );
  assert( arrd != 0 );

  if ( ( *arrd )->empty() ) // first data, copy
  {
    ( *arrd )->insert( ( *arrd )->end(), prop.begin(), prop.end() );
  }
  else
  {
    assert( ( *arrd )->size() == prop.size() );

    // add contents of prop to **arrd elementwise
    std::transform( ( *arrd )->begin(), ( *arrd )->end(), prop.begin(), ( *arrd )->begin(), std::plus< double >() );
  }
}
