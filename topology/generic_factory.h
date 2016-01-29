/*
 *  generic_factory.h
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

#ifndef GENERIC_FACTORY_H
#define GENERIC_FACTORY_H

// C++ includes:
#include <map>

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
class AbstractGeneric;

/**
 * Generic Factory class for objects deriving from a base class
 * BaseT. Keeps a register of subtypes which may be created
 * dynamically. New subtypes may be added by registering either a class
 * (which must have a constructor taking as a parameter a dictionary
 * containing parameters for the mask) or a specialized factory function.
 * @see Alexandrescu, A (2001). Modern C++ Design, Addison-Wesley, ch. 8.
 */
template < class BaseT >
class GenericFactory
{
public:
  typedef BaseT* ( *CreatorFunction )( const DictionaryDatum& d );
  typedef std::map< Name, CreatorFunction > AssocMap;

  /**
   * Factory function.
   * @param name Subtype.
   * @param d    Dictionary containing parameters for this subtype.
   * @returns dynamically allocated new object.
   */
  BaseT* create( const Name& name, const DictionaryDatum& d ) const;

  /**
   * Register a new subtype. The type name must not already exist. The
   * class for the subtype is supplied via the template argument. This
   * class should have a constructor taking a const DictionaryDatum& as
   * parameter.
   * @param name subtype name.
   * @returns true if subtype was successfully registered.
   */
  template < class T >
  bool register_subtype( const Name& name );

  /**
   * Register a new subtype. The type name must not already exist.
   * @param name    Subtype name.
   * @param creator A factory function creating objects of this subtype
   *                from a const DictionaryDatum& containing parameters
   * @returns true if mask was successfully registered.
   */
  bool register_subtype( const Name& name, CreatorFunction creator );

private:
  template < class T >
  static BaseT* new_from_dict_( const DictionaryDatum& d );

  AssocMap associations_;
};

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
