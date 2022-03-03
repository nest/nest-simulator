/*
 *  dictionary.h
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

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <boost/any.hpp>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "sliexceptions.h"

// using dictionary = std::map< std::string, boost::any >;

std::string debug_type( const boost::any& operand );

bool is_int( const boost::any& operand );
bool is_long( const boost::any& operand );
bool is_size_t( const boost::any& operand );
bool is_double( const boost::any& operand );
bool is_bool( const boost::any& operand );
bool is_string( const boost::any& operand );
bool is_int_vector( const boost::any& operand );
bool is_double_vector( const boost::any& operand );
bool is_double_vector_vector( const boost::any& operand );
bool is_string_vector( const boost::any& operand );
bool is_any_vector( const boost::any& operand );
bool is_dict( const boost::any& operand );
bool is_parameter( const boost::any& operand );
bool is_nc( const boost::any& operand );

bool value_equal( const boost::any first, const boost::any second );

class dictionary : public std::map< std::string, boost::any >
{
  // TODO-PYNEST-NG: Meta-information about entries:
  //                   * Value type (enum?)
  //                   * Whether value is writable
  //                   * Docstring for each entry
private:
  using maptype_ = std::map< std::string, boost::any >;

public:
  template < typename T >
  T
  get( const std::string& key ) const
  {
    try
    {
      return boost::any_cast< T >( at( key ) );
    }
    catch ( const boost::bad_any_cast& )
    {
      std::string msg = std::string( "Failed to cast " ) + key + " from " + debug_type( at( key ) ) + " to type "
        + std::string( typeid( T ).name() );
      std::cerr << msg << "\n";
      throw TypeMismatch( msg );
    }
  }

  template < typename T >
  bool
  update_value( const std::string& key, T& value ) const
  {
    auto it = find( key );
    if ( it != end() )
    {
      value = boost::any_cast< T >( it->second );
      return true;
    }
    return false;
  }

  bool
  known( const std::string& key ) const
  {
    // Bypass find() function to not set access flag
    return maptype_::find( key ) != end();
  }

  bool operator==( const dictionary& other ) const;

  bool
  operator!=( const dictionary& other ) const
  {
    return not( *this == other );
  }

  void init_access_flags() const;

  /**
   * @brief Check that all elements in the dictionary have been accessed.
   *
   * @param where Which function the error occurs in
   * @param what Which parameter triggers the error
   *
   */
  void all_entries_accessed( const std::string where, const std::string what ) const;

  // Wrappers for access flags
  boost::any& operator[]( const std::string& key );
  boost::any& operator[]( std::string&& key );
  boost::any& at( const std::string& key );
  const boost::any& at( const std::string& key ) const;
  iterator find( const std::string& key );
  const_iterator find( const std::string& key ) const;
};

#endif /* DICTIONARY_H_ */
