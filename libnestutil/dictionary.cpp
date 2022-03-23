/*
 *  dictionary.cpp
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

#include <algorithm>
#include <boost/any.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "dictionary.h"
#include "kernel_manager.h"
#include "nest_datums.h"
#include "parameter.h"


// debug
std::string
debug_type( const boost::any& operand )
{
  return operand.type().name();
}

// int
bool
is_int( const boost::any& operand )
{
  return operand.type() == typeid( int );
}

bool
is_uint( const boost::any& operand )
{
  return operand.type() == typeid( unsigned int );
}

// long
bool
is_long( const boost::any& operand )
{
  return operand.type() == typeid( long );
}

bool
is_size_t( const boost::any& operand )
{
  return operand.type() == typeid( size_t );
}


// double
bool
is_double( const boost::any& operand )
{
  return operand.type() == typeid( double );
}

// bool
bool
is_bool( const boost::any& operand )
{
  return operand.type() == typeid( bool );
}

// string
bool
is_string( const boost::any& operand )
{
  return operand.type() == typeid( std::string );
}

// vector of ints
bool
is_int_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< int > );
}

// vector of doubles
bool
is_double_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< double > );
}

// vector of vector of doubles
bool
is_double_vector_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< std::vector< double > > );
}

// vector of strings
bool
is_string_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< std::string > );
}

// vector of boost::any
bool
is_any_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< boost::any > );
}

// dict
bool
is_dict( const boost::any& operand )
{
  return operand.type() == typeid( dictionary );
}

// parameter
bool
is_parameter( const boost::any& operand )
{
  return operand.type() == typeid( std::shared_ptr< nest::Parameter > );
}

// NodeCollection
bool
is_nc( const boost::any& operand )
{
  return operand.type() == typeid( NodeCollectionDatum );
}

bool
value_equal( const boost::any first, const boost::any second )
{
  if ( is_int( first ) )
  {
    if ( not is_int( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< int >( first );
    const auto other_value = boost::any_cast< int >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_long( first ) )
  {
    if ( not is_long( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< long >( first );
    const auto other_value = boost::any_cast< long >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_size_t( first ) )
  {
    if ( not is_size_t( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< size_t >( first );
    const auto other_value = boost::any_cast< size_t >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_double( first ) )
  {
    if ( not is_double( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< double >( first );
    const auto other_value = boost::any_cast< double >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_bool( first ) )
  {
    if ( not is_bool( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< bool >( first );
    const auto other_value = boost::any_cast< bool >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_string( first ) )
  {
    if ( not is_string( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::string >( first );
    const auto other_value = boost::any_cast< std::string >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_int_vector( first ) )
  {
    if ( not is_int_vector( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::vector< int > >( first );
    const auto other_value = boost::any_cast< std::vector< int > >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_double_vector( first ) )
  {
    if ( not is_double_vector( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::vector< double > >( first );
    const auto other_value = boost::any_cast< std::vector< double > >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_double_vector_vector( first ) )
  {
    if ( not is_double_vector_vector( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::vector< std::vector< double > > >( first );
    const auto other_value = boost::any_cast< std::vector< std::vector< double > > >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_string_vector( first ) )
  {
    if ( not is_string_vector( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::vector< std::string > >( first );
    const auto other_value = boost::any_cast< std::vector< std::string > >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_dict( first ) )
  {
    if ( not is_dict( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< dictionary >( first );
    const auto other_value = boost::any_cast< dictionary >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_parameter( first ) )
  {
    if ( not is_parameter( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::shared_ptr< nest::Parameter > >( first );
    const auto other_value = boost::any_cast< std::shared_ptr< nest::Parameter > >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else
  {
    // TODO-PYNEST-NG: raise error
    assert( false );
  }
  return true;
}


bool
dictionary::operator==( const dictionary& other ) const
{
  if ( size() != other.size() )
  {
    return false;
  }
  // Iterate elements in the other dictionary
  for ( auto& kv_pair : other )
  {
    // Check if it exists in this dictionary
    if ( not known( kv_pair.first ) )
    {
      return false;
    }
    // Check for equality
    const auto value = maptype_::at( kv_pair.first );
    if ( not value_equal( value, kv_pair.second ) )
    {
      return false;
    }
  }
  // All elements are equal
  return true;
}


boost::any& dictionary::operator[]( const std::string& key )
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::operator[]( key );
}

boost::any& dictionary::operator[]( std::string&& key )
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::operator[]( key );
}

boost::any&
dictionary::at( const std::string& key )
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::at( key );
}

const boost::any&
dictionary::at( const std::string& key ) const
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::at( key );
}

dictionary::iterator
dictionary::find( const std::string& key )
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::find( key );
}

dictionary::const_iterator
dictionary::find( const std::string& key ) const
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::find( key );
}

void
dictionary::init_access_flags() const
{
  nest::kernel().get_dict_access_flag_manager().init_access_flags( *this );
}

void
dictionary::all_entries_accessed( const std::string where, const std::string what ) const
{
  nest::kernel().get_dict_access_flag_manager().all_accessed( *this, where, what );
}

// TODO-PYNEST-NG: Convenience function for accessed()?
