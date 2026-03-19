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
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "dictionary.h"

#include "kernel_manager.h"

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

any_type&
Dictionary::operator[]( const std::string& key ) const
{
  return ( **this )[ key ];
}
any_type&
Dictionary::operator[]( std::string&& key ) const
{
  return ( **this )[ std::move( key ) ];
}
any_type&
Dictionary::at( const std::string& key )
{
  return ( *this )->at( key );
}
const any_type&
Dictionary::at( const std::string& key ) const
{
  return ( *this )->at( key );
}

template <>
double
dictionary_::cast_value_< double >( const any_type& value, const std::string& key ) const
{
  return std::visit(
    [ &key, debug_name = debug_type( value ) ]( auto&& arg ) -> double
    {
      using T = std::decay_t< decltype( arg ) >;
      if constexpr ( std::is_arithmetic_v< T > )
      {
        return static_cast< double >( arg );
      }
      else
      {
        const std::string msg = String::compose( "Failed to cast '%1' from %2 to type double.", key, debug_name );
        throw nest::TypeMismatch( msg );
      }
    },
    value );
}

template <>
std::vector< double >
dictionary_::cast_value_< std::vector< double > >( const any_type& value, const std::string& key ) const
{
  if ( std::holds_alternative< EmptyList >( value ) )
  {
    return std::vector< double >();
  }

  try
  {
    if ( const std::vector< double >* v = std::get_if< std::vector< double > >( &value ) )
    {
      return *v;
    }
    if ( const std::vector< long >* v = std::get_if< std::vector< long > >( &value ) )
    {
      std::vector< double > res;
      std::copy( v->begin(), v->end(), std::back_inserter( res ) );
      return res;
    }
    throw std::bad_variant_access();  // deflect to error handling below
  }
  catch ( const std::bad_variant_access& )
  {
    const std::string msg =
      String::compose( "Failed to cast '%1' from %2 to type std::vector<double>", key, debug_type( value ) );
    throw nest::TypeMismatch( msg );
  }
}


template <>
std::vector< std::string >
dictionary_::cast_value_< std::vector< std::string > >( const any_type& value, const std::string& key ) const
{
  if ( std::holds_alternative< EmptyList >( value ) )
  {
    return std::vector< std::string >();
  }

  try
  {
    if ( const std::vector< std::string >* v = std::get_if< std::vector< std::string > >( &value ) )
    {
      return *v;
    }
    throw std::bad_variant_access();  // deflect to error handling below
  }
  catch ( const std::bad_variant_access& )
  {
    const std::string msg =
      String::compose( "Failed to cast '%1' from %2 to type std::vector<double>", key, debug_type( value ) );
    throw nest::TypeMismatch( msg );
  }
}

std::string
debug_type( const any_type& operand )
{
  return std::visit(
    []( auto&& arg ) -> std::string
    {
      using T = std::decay_t< decltype( arg ) >;
      return boost::typeindex::type_id< T >().pretty_name();
    },
    operand );
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
operator<<( std::ostream& os, const std::vector< std::vector< long > >& )
{
  os << "vector<vector<long>>";
  return os;
}
std::ostream&
operator<<( std::ostream& os, const std::vector< std::vector< double > >& )
{
  os << "vector<vector<double>>";
  return os;
}
std::ostream&
operator<<( std::ostream& os, const std::vector< std::vector< std::vector< long > > >& )
{
  os << "vector<vector<vector<long>>>";
  return os;
}
std::ostream&
operator<<( std::ostream& os, const std::vector< std::vector< std::vector< double > > >& )
{
  os << "vector<vector<vector<double>>>";
  return os;
}
std::ostream&
operator<<( std::ostream& os, const std::shared_ptr< nest::Parameter >& )
{
  os << "parameter";
  return os;
}
std::ostream&
operator<<( std::ostream& os, const nest::VerbosityLevel& )
{
  os << "verbosity level";
  return os;
}

std::ostream&
operator<<( std::ostream& os, const Dictionary& dict )
{
  const auto max_key_length = std::max_element( dict.begin(),
    dict.end(),
    []( const dictionary_::value_type s1, const dictionary_::value_type s2 )
    {
      return s1.first.length() < s2.first.length();
    } )->first.length();
  const std::string pre_padding = "    ";
  os << "Dictionary{\n";
  for ( auto& kv : dict )
  {
    std::string type;
    std::stringstream value_stream;

    const auto& item = kv.second.item;

    const auto post_padding = max_key_length - kv.first.length() + 5;
    os << pre_padding << kv.first << std::setw( post_padding ) << "(" << debug_type( item ) << ")"
       << " " << std::setw( 25 - type.length() );
    std::visit( [ &os ]( const auto& arg ) { os << arg; }, item );
  }
  return os << "}";
}

std::ostream&
operator<<( std::ostream& os, const std::monostate& )
{
  os << "None";
  return os;
}

std::ostream&
operator<<( std::ostream& os, const AnyVector& av )
{
  os << "[";
  for ( auto v : av )
  {
    std::visit( []( const auto& arg ) { std::cout << arg << " "; }, v );
  }
  os << "\b]";  // \b removes empty space after last element
  return os;
}

std::ostream&
operator<<( std::ostream& os, const EmptyList& )
{
  return os << "[]";
}


bool
dictionary_::operator==( const dictionary_& other ) const
{
  if ( size() != other.size() )
  {
    return false;
  }

  // Iterate elements in the other Dictionary
  for ( const auto& [ other_key, other_entry ] : other )
  {
    // Check if it exists in this Dictionary
    // Because we know that both dicts have the same size, this suffices to
    // to confirm that both dictionaries have the same keys.
    if ( not known( other_key ) )
    {
      return false;
    }

    // Check for equality using operator==() for variants
    const auto& this_entry = maptype_::at( other_key );
    if ( this_entry != other_entry )
    {
      return false;
    }
  }

  // All elements are equal
  return true;
}

void
dictionary_::register_access_( const DictEntry_& entry ) const
{
  if ( not entry.accessed )
  {
    // if() above avoids any unnecessary updates, atomic prevents any potential
    // data races in case the compiler does behind-the-scences magic.
#pragma omp atomic write
    entry.accessed = true;  // accessed is mutable
  }
}

any_type&
dictionary_::operator[]( const std::string& key )
{
  auto& entry = maptype_::operator[]( key );
  // op[] inserts entry if key was not known before, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

any_type&
dictionary_::operator[]( std::string&& key )
{
  auto& entry = maptype_::operator[]( key );
  // op[] inserts entry if key was not known before, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

any_type&
dictionary_::at( const std::string& key )
{
  auto& entry = maptype_::at( key );
  // at() throws if key is not know, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

const any_type&
dictionary_::at( const std::string& key ) const
{
  const auto& entry = maptype_::at( key );
  // at() throws if key is not know, so we are sure entry exists
  register_access_( entry );
  return entry.item;
}

dictionary_::iterator
dictionary_::find( const std::string& key )
{
  const auto it = maptype_::find( key );
  if ( it != end() )
  {
    register_access_( it->second );
  }
  return it;
}

dictionary_::const_iterator
dictionary_::find( const std::string& key ) const
{
  const auto it = maptype_::find( key );
  if ( it != end() )
  {
    register_access_( it->second );
  }
  return it;
}

void
dictionary_::init_access_flags( const bool thread_local_dict ) const
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
dictionary_::all_entries_accessed( const std::string& where,
  const std::string& what,
  const bool thread_local_dict ) const
{
  if ( not nest::kernel().logging_manager.dict_miss_is_error() )
  {
    return;
  }

  if ( not thread_local_dict )
  {
    nest::kernel().vp_manager.assert_single_threaded();
  }

  // Vector of elements in the Dictionary that are not accessed
  std::vector< key_type > not_accessed_keys;

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
      key_type(),  // creates empty instance of key type (string)
      []( const key_type& a, const key_type& b ) { return a + " " + b; } );

    throw nest::UnaccessedDictionaryEntry( what, where, missed );
  }
}
