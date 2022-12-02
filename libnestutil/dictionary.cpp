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
#include <boost/core/demangle.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "dictionary.h"
#include "kernel_manager.h"
#include "parameter.h"

/**
 * General vector streamer.
 * This templated operator streams all elements of a std::vector<>.
 */
template < typename T >
std::ostream&
operator<<( std::ostream& os, const std::vector< T >& vec )
{
  os << "vector[";
  bool first = true;
  for ( const auto& element : vec )
  {
    if ( not first )
    {
      os << ", ";
    }
    os << element;
    first = false;
  }
  return os << "]";
}

// debug
std::string
debug_type( const boost::any& operand )
{
  return boost::core::demangle( operand.type().name() );
}

std::string
debug_dict_types( const dictionary& dict )
{
  std::string s = "[dictionary]\n";

  for ( auto& kv : dict )
  {
    s += kv.first + ": ";
    s += debug_type( kv.second ) + "\n";
  }
  return s;
}

std::ostream&
operator<<( std::ostream& os, const dictionary& dict )
{
  const auto max_key_length = std::max_element( dict.begin(),
    dict.end(),
    []( const dictionary::value_type s1, const dictionary::value_type s2 ) {
      return s1.first.length() < s2.first.length();
    } )->first.length();
  const std::string pre_padding = "    ";
  os << "dictionary{\n";
  for ( auto& kv : dict )
  {
    std::string type;
    std::stringstream value_stream;
    if ( is_type< int >( kv.second ) )
    {
      type = "int";
      value_stream << boost::any_cast< int >( kv.second ) << '\n';
    }
    else if ( is_type< unsigned int >( kv.second ) )
    {
      type = "unsigned int";
      value_stream << boost::any_cast< unsigned int >( kv.second ) << '\n';
    }
    else if ( is_type< long >( kv.second ) )
    {
      type = "long";
      value_stream << boost::any_cast< long >( kv.second ) << '\n';
    }
    else if ( is_type< size_t >( kv.second ) )
    {
      type = "size_t";
      value_stream << boost::any_cast< size_t >( kv.second ) << '\n';
    }
    else if ( is_type< double >( kv.second ) )
    {
      type = "double";
      value_stream << boost::any_cast< double >( kv.second ) << '\n';
    }
    else if ( is_type< bool >( kv.second ) )
    {
      type = "bool";
      const auto value = boost::any_cast< bool >( kv.second );
      value_stream << ( value ? "true" : "false" ) << '\n';
    }
    else if ( is_type< std::string >( kv.second ) )
    {
      type = "std::string";
      value_stream << "\"" << boost::any_cast< std::string >( kv.second ) << "\"\n";
    }
    else if ( is_type< std::vector< int > >( kv.second ) )
    {
      type = "std::vector<int>";
      value_stream << boost::any_cast< std::vector< int > >( kv.second ) << '\n';
    }
    else if ( is_type< std::vector< double > >( kv.second ) )
    {
      type = "std::vector<double>";
      value_stream << boost::any_cast< std::vector< double > >( kv.second ) << '\n';
    }
    else if ( is_type< std::vector< std::vector< double > > >( kv.second ) )
    {
      type = "vector<vector<double>>";
      value_stream << "vector<vector<double>>" << '\n';
    }
    else if ( is_type< std::vector< std::string > >( kv.second ) )
    {
      type = "std::vector<std::string>";
      value_stream << boost::any_cast< std::vector< std::string > >( kv.second ) << '\n';
    }
    else if ( is_type< std::vector< boost::any > >( kv.second ) )
    {
      type = "vector<boost::any>";
      value_stream << "vector<any>" << '\n';
    }
    else if ( is_type< dictionary >( kv.second ) )
    {
      type = "dictionary";
      value_stream << "dictionary" << '\n';
    }
    else if ( is_type< std::shared_ptr< nest::Parameter > >( kv.second ) )
    {
      type = "parameter";
      value_stream << "parameter" << '\n';
    }
    else if ( is_type< std::shared_ptr< nest::NodeCollection > >( kv.second ) )
    {
      type = "NodeCollection";
      const auto nc = boost::any_cast< std::shared_ptr< nest::NodeCollection > >( kv.second );
      nc->print_me( value_stream );
      value_stream << "\n";
    }
    else
    {
      throw TypeMismatch( "Type is not known" );
    }
    const auto s = value_stream.str();
    const auto post_padding = max_key_length - kv.first.length() + 5;
    os << pre_padding << kv.first << std::setw( post_padding ) << "(" << type << ")"
       << " " << std::setw( 25 - type.length() ) << s;
  }
  return os << "}";
}

bool
value_equal( const boost::any first, const boost::any second )
{
  if ( is_type< int >( first ) )
  {
    if ( not is_type< int >( second ) )
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
  else if ( is_type< long >( first ) )
  {
    if ( not is_type< long >( second ) )
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
  else if ( is_type< size_t >( first ) )
  {
    if ( not is_type< size_t >( second ) )
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
  else if ( is_type< double >( first ) )
  {
    if ( not is_type< double >( second ) )
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
  else if ( is_type< bool >( first ) )
  {
    if ( not is_type< bool >( second ) )
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
  else if ( is_type< std::string >( first ) )
  {
    if ( not is_type< std::string >( second ) )
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
  else if ( is_type< std::vector< int > >( first ) )
  {
    if ( not is_type< std::vector< int > >( second ) )
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
  else if ( is_type< std::vector< double > >( first ) )
  {
    if ( not is_type< std::vector< double > >( second ) )
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
  else if ( is_type< std::vector< std::vector< double > > >( first ) )
  {
    if ( not is_type< std::vector< std::vector< double > > >( second ) )
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
  else if ( is_type< std::vector< std::string > >( first ) )
  {
    if ( not is_type< std::vector< std::string > >( second ) )
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
  else if ( is_type< std::vector< size_t > >( first ) )
  {
    if ( not is_type< std::vector< size_t > >( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< std::vector< size_t > >( first );
    const auto other_value = boost::any_cast< std::vector< size_t > >( second );
    if ( this_value != other_value )
    {
      return false;
    }
  }
  else if ( is_type< dictionary >( first ) )
  {
    if ( not is_type< dictionary >( second ) )
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
  else if ( is_type< std::shared_ptr< nest::Parameter > >( first ) )
  {
    if ( not is_type< std::shared_ptr< nest::Parameter > >( second ) )
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
    std::string msg = std::string( "Unsupported type in dictionary::value_equal(): " ) + debug_type( first );
    std::cerr << msg << "\n";
    throw nest::TypeMismatch( msg );
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


boost::any&
dictionary::operator[]( const std::string& key )
{
  nest::kernel().get_dict_access_flag_manager().register_access( *this, key );
  return maptype_::operator[]( key );
}

boost::any&
dictionary::operator[]( std::string&& key )
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
