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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

#include "exceptions.h"

class dictionary;

/**
 * @brief Get the typename of the operand.
 *
 * @param operand to get the typename of.
 * @return std::string of the typename.
 */
std::string debug_type( const boost::any& operand );

std::string debug_dict_types( const dictionary& dict );

template < typename T >
bool
is_type( const boost::any& operand )
{
  return operand.type() == typeid( T );
}

/**
 * @brief Check whether two boost::any values are equal.
 *
 * @param first The first value.
 * @param second The other value.
 * @return Whether the values are equal, both in type and value.
 */
bool value_equal( const boost::any& first, const boost::any& second );

/**
 * @brief A Python-like dictionary, based on std::map.
 *
 * Values are stored as boost::any objects, with std::string keys.
 */
struct DictEntry_
{
  //! Constructor without arguments needed by std::map::operator[]
  DictEntry_()
    : item( boost::any() )
    , accessed( false )
  {
  }
  DictEntry_( const boost::any& item )
    : item( item )
    , accessed( false )
  {
  }

  boost::any item;       //!< actual item stored
  mutable bool accessed; //!< initally false, set to true once entry is accessed
};

class dictionary : public std::map< std::string, DictEntry_ >
{
  // TODO-PYNEST-NG: Meta-information about entries:
  //                   * Value type (enum?)
  //                   * Whether value is writable
  //                   * Docstring for each entry
private:
  // TODO: PYNEST-NG: maybe change to unordered map, as that provides
  // automatic hashing of keys (currently strings) which might make
  // lookups more efficient
  using maptype_ = std::map< std::string, DictEntry_ >;
  using maptype_::maptype_; // Inherit constructors

  /**
   * @brief Cast the specified value to the specified type.
   *
   * @tparam T Type of element. If the value is not of the specified type, a TypeMismatch error is thrown.
   * @param value the any object to cast.
   * @param key key where the value is located in the dictionary, for information upon cast errors.
   * @throws TypeMismatch if the value is not of specified type T.
   * @return value cast to the specified type.
   */
  template < typename T >
  T
  cast_value_( const boost::any& value, const std::string& key ) const
  {
    try
    {
      return boost::any_cast< T >( value );
    }
    catch ( const boost::bad_any_cast& )
    {
      std::string msg = std::string( "Failed to cast '" ) + key + "' from " + debug_type( value ) + " to type "
        + std::string( boost::core::demangle( typeid( T ).name() ) );
      throw nest::TypeMismatch( msg );
    }
  }

  template <>
  double
  cast_value_< double >( const boost::any& value, const std::string& key ) const
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
  cast_value_< std::vector< double > >( const boost::any& value, const std::string& key ) const
  {
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


  /**
   * @brief Cast the specified value to an integer.
   *
   * @param value the any object to cast.
   * @param key key where the value is located in the dictionary, for information upon cast errors.
   * @throws TypeMismatch if the value is not an integer.
   * @return value cast to an integer.
   */
  size_t // TODO: or template?
  cast_to_integer_( const boost::any& value, const std::string& key ) const
  {
    if ( is_type< size_t >( value ) )
    {
      return cast_value_< size_t >( value, key );
    }
    else if ( is_type< long >( value ) )
    {
      return cast_value_< long >( value, key );
    }
    else if ( is_type< int >( value ) )
    {
      return cast_value_< int >( value, key );
    }
    // Not an integer type
    std::string msg =
      std::string( "Failed to cast '" ) + key + "' from " + debug_type( at( key ) ) + " to an integer type ";
    throw nest::TypeMismatch( msg );
  }

  void register_access_( const DictEntry_& entry ) const;

public:
  /**
   * @brief Get the value at key in the specified type.
   *
   * @tparam T Type of the value. If the value is not of the specified type, a TypeMismatch error is thrown.
   * @param key key where the value is located in the dictionary.
   * @throws TypeMismatch if the value is not of specified type T.
   * @return the value at key cast to the specified type.
   */
  template < typename T >
  T
  get( const std::string& key ) const
  {
    return cast_value_< T >( at( key ), key );
  }

  /**
   * @brief Get the value at key as an integer.
   *
   * @param key key where the value is located in the dictionary.
   * @throws TypeMismatch if the value is not an integer.
   * @return the value at key cast to the specified type.
   */
  size_t // TODO: or template?
  get_integer( const std::string& key ) const
  {
    return cast_to_integer_( at( key ), key );
  }

  /**
   * @brief Update the specified value if there exists a value at key.
   *
   * @param key key where the value may be located in the dictionary.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not the same type as the value argument.
   * @return Whether value was updated.
   */
  template < typename T >
  bool
  update_value( const std::string& key, T& value ) const
  {
    auto it = find( key );
    if ( it != end() )
    {
      value = cast_value_< T >( it->second.item, key );
      return true;
    }
    return false;
  }

  /**
   * @brief Update the specified value if there exists an integer value at key.
   *
   * @param key key where the value may be located in the dictionary.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not an integer.
   * @return Whether the value was updated.
   */
  template < typename T >
  bool
  update_integer_value( const std::string& key, T& value ) const
  {
    auto it = find( key );
    if ( it != end() )
    {
      value = cast_to_integer_( it->second.item, key );
      return true;
    }
    return false;
  }

  /**
   * @brief Check whether there exists a value with specified key in the dictionary.
   *
   * @param key key where the value may be located in the dictionary.
   * @return true if there is a value with the specified key, false if not.
   *
   * @note This does **not** mark the entry, because we sometimes need to confirm
   * that a certain key is not in a dictionary.
   */
  bool
  known( const std::string& key ) const
  {
    // Bypass find() function to not set access flag
    return maptype_::find( key ) != end();
  }

  /**
   * @brief Mark entry with given key as accessed.
   */
  void
  mark_as_accessed( const std::string& key ) const
  {
    register_access_( maptype_::at( key ) );
  }

  /**
   * @brief Return true if entry has been marked as accessed.
   */
  bool
  has_been_accessed( const std::string& key ) const
  {
    return maptype_::at( key ).accessed;
  }

  /**
   * @brief Check whether the dictionary is equal to another dictionary.
   *
   * Two dictionaries are equal only if they contain the exact same entries with the same values.
   *
   * @param other dictionary to check against.
   * @return true if the dictionary is equal to the other dictionary, false if not.
   */
  bool operator==( const dictionary& other ) const;

  /**
   * @brief Check whether the dictionary is unequal to another dictionary.
   *
   * Two dictionaries are unequal if they do not contain the exact same entries with the same values.
   *
   * @param other dictionary to check against.
   * @return true if the dictionary is unequal to the other dictionary, false if not.
   */
  bool
  operator!=( const dictionary& other ) const
  {
    return not( *this == other );
  }

  /**
   * @brief Initializes or resets access flags for the current dictionary.
   *
   * @note The method assumes that the dictionary was defined in global scope, whence it should
   * only be called from a serial context. If the dict is in thread-specific, pass `true` to
   * allow call in parallel context.
   */
  void init_access_flags( const bool thread_local_dict = false ) const;

  /**
   * @brief Check that all elements in the dictionary have been accessed.
   *
   * @param where Which function the error occurs in.
   * @param what Which parameter triggers the error.
   * @param thread_local_dict See note below.
   * @throws UnaccessedDictionaryEntry if there are unaccessed dictionary entries.
   *
   * @note The method assumes that the dictionary was defined in global scope, whence it should
   * only be called from a serial context. If the dict is in thread-specific, pass `true` to
   * allow call in parallel context.
   */
  void
  all_entries_accessed( const std::string& where, const std::string& what, const bool thread_local_dict = false ) const;

  // Wrappers for access flags
  boost::any& operator[]( const std::string& key );
  boost::any& operator[]( std::string&& key );
  boost::any& at( const std::string& key );
  const boost::any& at( const std::string& key ) const;
  iterator find( const std::string& key );
  const_iterator find( const std::string& key ) const;
};

std::ostream& operator<<( std::ostream& os, const dictionary& dict );

#endif /* DICTIONARY_H_ */
