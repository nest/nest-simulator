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
#include <iomanip>
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
    s += debug_type( kv.second.item ) + "\n";
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

    const auto& item = kv.second.item;

    if ( is_type< int >( item ) )
    {
      type = "int";
      value_stream << boost::any_cast< int >( item ) << '\n';
    }
    else if ( is_type< unsigned int >( item ) )
    {
      type = "unsigned int";
      value_stream << boost::any_cast< unsigned int >( item ) << '\n';
    }
    else if ( is_type< long >( item ) )
    {
      type = "long";
      value_stream << boost::any_cast< long >( item ) << '\n';
    }
    else if ( is_type< size_t >( item ) )
    {
      type = "size_t";
      value_stream << boost::any_cast< size_t >( item ) << '\n';
    }
    else if ( is_type< double >( item ) )
    {
      type = "double";
      value_stream << boost::any_cast< double >( item ) << '\n';
    }
    else if ( is_type< bool >( item ) )
    {
      type = "bool";
      const auto value = boost::any_cast< bool >( item );
      value_stream << ( value ? "true" : "false" ) << '\n';
    }
    else if ( is_type< std::string >( item ) )
    {
      type = "std::string";
      value_stream << "\"" << boost::any_cast< std::string >( item ) << "\"\n";
    }
    else if ( is_type< std::vector< int > >( item ) )
    {
      type = "std::vector<int>";
      value_stream << boost::any_cast< std::vector< int > >( item ) << '\n';
    }
    else if ( is_type< std::vector< double > >( item ) )
    {
      type = "std::vector<double>";
      value_stream << boost::any_cast< std::vector< double > >( item ) << '\n';
    }
    else if ( is_type< std::vector< std::vector< double > > >( item ) )
    {
      type = "vector<vector<double>>";
      value_stream << "vector<vector<double>>" << '\n';
    }
    else if ( is_type< std::vector< std::string > >( item ) )
    {
      type = "std::vector<std::string>";
      value_stream << boost::any_cast< std::vector< std::string > >( item ) << '\n';
    }
    else if ( is_type< std::vector< boost::any > >( item ) )
    {
      type = "vector<boost::any>";
      value_stream << "vector<any>" << '\n';
    }
    else if ( is_type< dictionary >( item ) )
    {
      type = "dictionary";
      value_stream << "dictionary" << '\n';
    }
    else if ( is_type< std::shared_ptr< nest::Parameter > >( item ) )
    {
      type = "parameter";
      value_stream << "parameter" << '\n';
    }
    else if ( is_type< std::shared_ptr< nest::NodeCollection > >( item ) )
    {
      type = "NodeCollection";
      const auto nc = boost::any_cast< std::shared_ptr< nest::NodeCollection > >( item );
      nc->print_me( value_stream );
      value_stream << "\n";
    }
    else
    {
      throw nest::TypeMismatch( "Type is not known" );
    }
    const auto s = value_stream.str();
    const auto post_padding = max_key_length - kv.first.length() + 5;
    os << pre_padding << kv.first << std::setw( post_padding ) << "(" << type << ")"
       << " " << std::setw( 25 - type.length() ) << s;
  }
  return os << "}";
}

bool
value_equal( const boost::any& first, const boost::any& second )
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
    std::string msg = std::string( "Unsupported type in dictionary::value_equal(): " ) + debug_type( first );
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
  for ( const auto& [ other_key, other_entry ] : other )
  {
    // Check if it exists in this dictionary
    if ( not known( other_key ) )
    {
      return false;
    }

    // Check for equality
    const auto& this_entry = maptype_::at( other_key );
    if ( not value_equal( this_entry.item, other_entry.item ) )
    {
      return false;
    }
  }
  // All elements are equal
  return true;
}

void
dictionary::register_access_( const DictEntry_& entry ) const
{
  if ( not entry.accessed )
  {
    // if() above avoids any unnecessary updates, atomic prevents any potential
    // data races in case the compiler does behind-the-scences magic.
#pragma omp atomic write
    entry.accessed = true; // accessed is mutable
  }
}

boost::any&
dictionary::operator[]( const std::string& key )
{
  auto& entry = maptype_::operator[]( key );
  // op[] inserts entry if key was not known before, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

boost::any&
dictionary::operator[]( std::string&& key )
{
  auto& entry = maptype_::operator[]( key );
  // op[] inserts entry if key was not known before, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

boost::any&
dictionary::at( const std::string& key )
{
  auto& entry = maptype_::at( key );
  // at() throws if key is not know, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

const boost::any&
dictionary::at( const std::string& key ) const
{
  const auto& entry = maptype_::at( key );
  // at() throws if key is not know, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

dictionary::iterator
dictionary::find( const std::string& key )
{
  const auto it = maptype_::find( key );
  if ( it != end() )
  {
    register_access_( it->second );
  }
  return it;
}

dictionary::const_iterator
dictionary::find( const std::string& key ) const
{
  const auto it = maptype_::find( key );
  if ( it != end() )
  {
    register_access_( it->second );
  }
  return it;
}

void
dictionary::init_access_flags( const bool thread_local_dict ) const
{
  if ( not thread_local_dict )
  {
    nest::kernel().vp_manager.assert_single_threaded();
  }
  for ( const auto& [ key, entry ] : *this )
  {
    entry.accessed = false;
  }
}

void
dictionary::all_entries_accessed( const std::string& where,
  const std::string& what,
  const bool thread_local_dict ) const
{
  if ( not thread_local_dict )
  {
    nest::kernel().vp_manager.assert_single_threaded();
  }

  // Vector of elements in the dictionary that are not accessed
  std::vector< dictionary::key_type > not_accessed_keys;

  for ( const auto& [ key, entry ] : *this )
  {
    if ( not entry.accessed )
    {
      not_accessed_keys.emplace_back( key );
    }
  }

  if ( not_accessed_keys.size() > 0 )
  {
    const auto missed = std::accumulate( not_accessed_keys.begin(),
      not_accessed_keys.end(),
      dictionary::key_type(), // creates empty instance of key type (string)
      []( const dictionary::key_type& a, const dictionary::key_type& b ) { return a + " " + b; } );

    throw nest::UnaccessedDictionaryEntry( what, where, missed );
  }
}

// TODO-PYNEST-NG: Convenience function for accessed()?
