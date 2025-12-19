/*
 *  generic_factory_impl.h
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

#ifndef GENERIC_FACTORY_IMPL_H
#define GENERIC_FACTORY_IMPL_H

#include "generic_factory.h"

namespace nest
{

template < class BaseT >
inline BaseT*
GenericFactory< BaseT >::create( const Name& name, const DictionaryDatum& d ) const
{
  typename AssocMap::const_iterator i = associations_.find( name );
  if ( i != associations_.end() )
  {
    return ( i->second )( d );
  }
  throw UndefinedName( name.toString() );
}

template < class BaseT >
template < class T >
inline bool
GenericFactory< BaseT >::register_subtype( const Name& name )
{
  return register_subtype( name, new_from_dict_< T > );
}

template < class BaseT >
inline bool
GenericFactory< BaseT >::register_subtype( const Name& name, CreatorFunction creator )
{
  return associations_.insert( std::pair< Name, CreatorFunction >( name, creator ) ).second;
}

template < class BaseT >
template < class T >
BaseT*
GenericFactory< BaseT >::new_from_dict_( const DictionaryDatum& d )
{
  return new T( d );
}


} // namespace nest

#endif
