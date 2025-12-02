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

template <>
double
Dictionary::cast_value_< double >( const boost::any& value, const std::string& key ) const
{
  try
  {
    if ( is_type< double >( value ) )
    {
      return boost::any_cast< double >( value );
    }
    if ( is_type< long >( value ) )
    {
      return static_cast< double >( boost::any_cast< long >( value ) );
    }
    if ( is_type< size_t >( value ) )
    {
      return static_cast< double >( boost::any_cast< size_t >( value ) );
    }
    if ( is_type< int >( value ) )
    {
      return static_cast< double >( boost::any_cast< int >( value ) );
    }
    throw boost::bad_any_cast(); // deflect to error handling below
  }
  catch ( const boost::bad_any_cast& )
  {
    const std::string msg =
      std::string( "Failed to cast '" ) + key + "' from " + debug_type( value ) + " to type double.";
    throw nest::TypeMismatch( msg );
  }
}

template <>
std::vector< double >
Dictionary::cast_value_< std::vector< double > >( const boost::any& value, const std::string& key ) const
{
  return cast_vector_value_< double >( value, key );
}


template <>
std::vector< double >
Dictionary::cast_vector_value_< double >( const boost::any& value, const std::string& key ) const
{
  // PyNEST passes vector with element type any if and only if it needs to pass
  // and empty vector, because the element type of empty lists cannot be inferred
  // at the Python level. The assertion just double-checks that we never get a
  // non-empty vector-of-any.
  if ( value.type() == typeid( std::vector< boost::any > ) )
  {
    assert( boost::any_cast< std::vector< boost::any > >( value ).empty() );
    return std::vector< double >();
  }

  try
  {
    if ( is_type< std::vector< double > >( value ) )
    {
      return boost::any_cast< std::vector< double > >( value );
    }
    if ( is_type< std::vector< long > >( value ) )
    {
      const std::vector< long > vlong = boost::any_cast< std::vector< long > >( value );
      std::vector< double > res;
      std::copy( vlong.begin(), vlong.end(), std::back_inserter( res ) );
      return res;
    }
    throw boost::bad_any_cast(); // deflect to error handling below
  }
  catch ( const boost::bad_any_cast& )
  {
    const std::string msg =
      std::string( "Failed to cast '" ) + key + "' from " + debug_type( value ) + " to type std::vector<double>.";
    throw nest::TypeMismatch( msg );
  }
}


std::string
debug_type( const boost::any& operand )
{
  return boost::core::demangle( operand.type().name() );
}

std::string
debug_dict_types( const Dictionary& dict )
{
  std::string s = "[Dictionary]\n";

  for ( auto& kv : dict )
  {
    s += kv.first + ": ";
    s += debug_type( kv.second.item ) + "\n";
  }
  return s;
}

std::ostream&
operator<<( std::ostream& os, const Dictionary& dict )
{
  const auto max_key_length = std::max_element( dict.begin(),
    dict.end(),
    []( const Dictionary::value_type s1, const Dictionary::value_type s2 ) {
      return s1.first.length() < s2.first.length();
    } )->first.length();
  const std::string pre_padding = "    ";
  os << "Dictionary{\n";
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
    else if ( is_type< Dictionary >( item ) )
    {
      type = "Dictionary";
      value_stream << "Dictionary" << '\n';
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
  else if ( is_type< Dictionary >( first ) )
  {
    if ( not is_type< Dictionary >( second ) )
    {
      return false;
    }
    const auto this_value = boost::any_cast< Dictionary >( first );
    const auto other_value = boost::any_cast< Dictionary >( second );
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
    std::string msg = std::string( "Unsupported type in Dictionary::value_equal(): " ) + debug_type( first );
    throw nest::TypeMismatch( msg );
  }
  return true;
}


bool
Dictionary::operator==( const Dictionary& other ) const
{
  if ( size() != other.size() )
  {
    return false;
  }
  // Iterate elements in the other Dictionary
  for ( const auto& [ other_key, other_entry ] : other )
  {
    // Check if it exists in this Dictionary
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
Dictionary::register_access_( const DictEntry_& entry ) const
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
Dictionary::operator[]( const std::string& key )
{
  auto& entry = maptype_::operator[]( key );
  // op[] inserts entry if key was not known before, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

boost::any&
Dictionary::operator[]( std::string&& key )
{
  auto& entry = maptype_::operator[]( key );
  // op[] inserts entry if key was not known before, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

boost::any&
Dictionary::at( const std::string& key )
{
  auto& entry = maptype_::at( key );
  // at() throws if key is not know, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

const boost::any&
Dictionary::at( const std::string& key ) const
{
  const auto& entry = maptype_::at( key );
  // at() throws if key is not know, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

Dictionary::iterator
Dictionary::find( const std::string& key )
{
  const auto it = maptype_::find( key );
  if ( it != end() )
  {
    register_access_( it->second );
  }
  return it;
}

Dictionary::const_iterator
Dictionary::find( const std::string& key ) const
{
  const auto it = maptype_::find( key );
  if ( it != end() )
  {
    register_access_( it->second );
  }
  return it;
}

void
Dictionary::init_access_flags( const bool thread_local_dict ) const
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
Dictionary::all_entries_accessed( const std::string& where,
  const std::string& what,
  const bool thread_local_dict ) const
{
  if ( not thread_local_dict )
  {
    nest::kernel().vp_manager.assert_single_threaded();
  }

  // Vector of elements in the Dictionary that are not accessed
  std::vector< Dictionary::key_type > not_accessed_keys;

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
      Dictionary::key_type(), // creates empty instance of key type (string)
      []( const Dictionary::key_type& a, const Dictionary::key_type& b ) { return a + " " + b; } );

    throw nest::UnaccessedDictionaryEntry( what, where, missed );
  }
}
