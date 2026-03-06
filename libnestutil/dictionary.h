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

#include <concepts>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <boost/type_index.hpp>

#include "exceptions.h"

#include "logging.h"

class dictionary_;
class Dictionary;

namespace nest
{
class Parameter;
class NodeCollection;
}

// PyNEST passes vector with element type any if and only if it needs to pass an empty vector, because the element type
// of empty lists cannot be inferred at the Python level.
struct EmptyList
{
  friend std::ostream&
  operator<<( std::ostream& os, const EmptyList& )
  {
    return os << "[]";
  }
  bool operator==( const EmptyList& ) const = default;
};

template < typename... Scalars >
struct DictionarySchemaBuilder
{
  template < typename... Extras >
  using VariantType = std::variant< Scalars...,              // scalar types
    std::vector< Scalars >...,                               // vector variants of the scalar types
    std::vector< std::vector< Scalars > >...,                // vector-vector variants of the scalar types
    std::vector< std::vector< std::vector< Scalars > > >..., // vector-vector variants of the scalar types
    std::vector< std::vector< std::vector< std::vector< Scalars > > > >..., // vector-vector variants of the scalar
                                                                            // types
    Extras...                                                               // any extra types
    >;
};

using DictionarySchema = DictionarySchemaBuilder< long, double, bool, std::string, Dictionary >;

using any_type = DictionarySchema::VariantType< std::shared_ptr< nest::NodeCollection >,
  std::vector< std::shared_ptr< nest::NodeCollection > >,
  std::shared_ptr< nest::Parameter >,
  nest::VerbosityLevel,
  EmptyList >;


// Define a simple Concept for "Integer Integers" (excluding bool and char)
template < typename T >
concept Integer = std::integral< T > and not std::same_as< T, bool > and not std::same_as< T, char >;

// Define a concept that T must be holdable by any_type
template < typename T >
concept DictionaryType = std::is_constructible_v< any_type, T >;

template < typename T >
bool
is_type( const any_type& operand )
{
  return std::holds_alternative< T >( operand );
}


class Dictionary : public std::shared_ptr< dictionary_ >
{
public:
  Dictionary()
    : std::shared_ptr< dictionary_ >( std::make_shared< dictionary_ >() )
  {
  }

  operator dictionary_&() const
  {
    return **this;
  }

  any_type& operator[]( const std::string& key ) const;
  any_type& operator[]( std::string&& key ) const;
  any_type& at( const std::string& key );
  const any_type& at( const std::string& key ) const;

  auto begin() const;
  auto end() const;

  auto size() const;
  auto empty() const;
  void clear() const;
  auto find( const std::string& key ) const;

  bool known( const std::string& key ) const;
  void mark_as_accessed( const std::string& key ) const;
  bool has_been_accessed( const std::string& key ) const;
  void init_access_flags( const bool thread_local_dict = false ) const;
  void
  all_entries_accessed( const std::string& where, const std::string& what, const bool thread_local_dict = false ) const;

  template < DictionaryType T >
  T get( const std::string& key ) const;

  template < typename T >
  std::vector< T >& get_vector( const std::string& key );

  template < typename T >
  bool update_value( const std::string& key, T& value ) const;

  template < Integer T >
  bool update_integer_value( const std::string& key, T& value ) const;

  bool update_dictionary( dictionary_& dict_out ) const;
};

namespace nest
{
class Parameter;
class NodeCollection;
}

/**
 * @brief Get the typename of the operand.
 *
 * @param operand to get the typename of.
 * @return std::string of the typename.
 */
std::string debug_type( const any_type& operand );

std::string debug_dict_types( const Dictionary& dict );

/**
 * @brief A Python-like dictionary_, based on std::map.
 *
 * Values are stored as any_type objects, with std::string keys.
 */
struct DictEntry_
{
  //! Constructor without arguments needed by std::map::operator[]
  DictEntry_()
    : item( any_type() )
    , accessed( false )
  {
  }
  DictEntry_( const any_type& item )
    : item( item )
    , accessed( false )
  {
  }

  bool
  operator==( const DictEntry_& other ) const
  {
    return item == other.item;
  }

  any_type item;         //!< actual item stored
  mutable bool accessed; //!< initially false, set to true once entry is accessed
};

class dictionary_ : public std::map< std::string, DictEntry_ >
{
  // PYNEST-NG-FUTURE: Meta-information about entries:
  //                   * Value type (enum?)
  //                   * Whether value is writable
  //                   * Docstring for each entry
  // TODO: PYNEST-NG: maybe change to unordered map, as that provides
  // automatic hashing of keys (currently strings) which might make
  // lookups more efficient
  using maptype_ = std::map< std::string, DictEntry_ >;
  using maptype_::maptype_; // Inherit constructors

  /**
   * @brief Cast the specified non-vector value to the specified type.
   *
   * @tparam T Type of element. If the value is not of the specified type, a TypeMismatch error is thrown.
   * @param value the any object to cast.
   * @param key key where the value is located in the Dictionary, for information upon cast errors.
   * @throws TypeMismatch if the value is not of specified type T.
   * @return value cast to the specified type.
   */
  template < typename T >
  T
  cast_value_( const any_type& value, const std::string& key ) const
  {
    try
    {
      return std::get< T >( value );
    }
    catch ( const std::bad_variant_access& )
    {
      std::string msg = String::compose( "Failed to cast '%1' from %2 to type %3",
        key,
        debug_type( value ),
        boost::typeindex::type_id< T >().pretty_name() );
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
  template < Integer RetT >
  RetT
  cast_to_integer_( const any_type& value, const std::string& key ) const
  {
    return std::visit(
      [ key ]( auto&& arg ) -> RetT
      {
        using T = std::decay_t< decltype( arg ) >;

        // Compile-time check: Is it an Integer?
        if constexpr ( Integer< T > )
        {
          // Runtime check: Does the value fit safely?
          if ( std::in_range< RetT >( arg ) )
          {
            return static_cast< RetT >( arg );
          }

          throw std::out_of_range( "Value causes data loss or overflow." );
        }
        else
        {
          throw std::runtime_error(
            String::compose( "The dictionary value with key %1 does not hold a numeric integer type.", key ) );
        }
      },
      value );
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
  template < DictionaryType T >
  T
  get( const std::string& key ) const
  {
    return cast_value_< T >( at( key ), key );
  }

  /**
   * Return reference to vector of type T stored under key.
   *
   * * If key does not exist in dict, create empty vector<T> and return it.
   */
  template < typename T >
  std::vector< T >&
  get_vector( const std::string& key )
  {
    // try_emplace will only insert if key doesn't exist.
    auto [ iter, success ] = maptype_::try_emplace( key, std::vector< T >() );

    return std::get< std::vector< T > >( iter->second.item );
  };

  /**
   * @brief Update the specified non-vector value if there exists a value at key.
   *
   * @param key key where the value may be located in the dictionary.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not the same type as the value argument.
   * @return Whether value was updated.
   *
   * @note Only use this where the user is not allowed to use random or spatial parameters.
   *       Otherwise, use update_value_param().
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
  template < Integer T >
  bool
  update_integer_value( const std::string& key, T& value ) const
  {
    auto it = find( key );
    if ( it != end() )
    {
      value = cast_to_integer_< T >( it->second.item, key );
      return true;
    }
    return false;
  }

  /**
   * @brief Update the provided dictionary with all key-value pairs in this dictionary.
   *
   * @param dict_out the dictionary to be updated.
   * @return Whether any values were updated or added to the provided dictionary.
   */
  bool
  update_dictionary( dictionary_& dict_out ) const
  {
    for ( auto [ key, value ] : *this )
    {
      dict_out[ key ] = value.item;
    }
    return size() > 0;
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
    return maptype_::contains( key );
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
  bool operator==( const dictionary_& other ) const;

  /**
   * @brief Check whether the dictionary is unequal to another dictionary.
   *
   * Two dictionaries are unequal if they do not contain the exact same entries with the same values.
   *
   * @param other dictionary to check against.
   * @return true if the dictionary is unequal to the other dictionary, false if not.
   */
  bool
  operator!=( const Dictionary& other ) const
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
  any_type& operator[]( const std::string& key );
  any_type& operator[]( std::string&& key );
  any_type& at( const std::string& key );
  const any_type& at( const std::string& key ) const;
  iterator find( const std::string& key );
  const_iterator find( const std::string& key ) const;
};

std::ostream& operator<<( std::ostream& os, const dictionary_& dict );

//! Specialization that allows passing long where double is expected
template <>
double dictionary_::cast_value_< double >( const any_type& value, const std::string& key ) const;

inline auto
Dictionary::begin() const
{
  return ( *this )->begin();
}

inline auto
Dictionary::end() const
{
  return ( *this )->end();
}

/**
 * Specialization that allows passing long vectors where double vectors are expected.
 *
 * Also handles empty vectors.
 *
 * @note This specialization forwards to cast_vector_value_<double>, but is required explicitly,
 *       because, e.g., get(), calls cast_value_() directly even if the argument is a vector.
 */
template <>
std::vector< double > dictionary_::cast_value_< std::vector< double > >( const any_type& value,
  const std::string& key ) const;

/**
 * Specialization that allows passing empty lists.
 */
template <>
std::vector< std::string > dictionary_::cast_value_< std::vector< std::string > >( const any_type& value,
  const std::string& key ) const;

inline auto
Dictionary::size() const
{
  return ( *this )->size();
}

inline auto
Dictionary::empty() const
{
  return ( *this )->empty();
}

inline void
Dictionary::clear() const
{
  ( *this )->clear();
}

inline auto
Dictionary::find( const std::string& key ) const
{
  return ( *this )->find( key );
}

inline bool
Dictionary::known( const std::string& key ) const
{
  return ( *this )->known( key );
}

inline void
Dictionary::mark_as_accessed( const std::string& key ) const
{
  ( *this )->mark_as_accessed( key );
}

inline bool
Dictionary::has_been_accessed( const std::string& key ) const
{
  return ( *this )->has_been_accessed( key );
}

inline void
Dictionary::init_access_flags( const bool thread_local_dict ) const
{
  ( *this )->init_access_flags( thread_local_dict );
}

inline void
Dictionary::all_entries_accessed( const std::string& where,
  const std::string& what,
  const bool thread_local_dict ) const
{
  ( *this )->all_entries_accessed( where, what, thread_local_dict );
}

template < DictionaryType T >
inline T
Dictionary::get( const std::string& key ) const
{
  return ( *this )->get< T >( key );
}
template < typename T >
std::vector< T >&
Dictionary::get_vector( const std::string& key )
{
  return ( *this )->get_vector< T >( key );
}

template < typename T >
inline bool
Dictionary::update_value( const std::string& key, T& value ) const
{
  return ( *this )->update_value( key, value );
}

template < Integer T >
inline bool
Dictionary::update_integer_value( const std::string& key, T& value ) const
{
  return ( *this )->update_integer_value( key, value );
}

inline bool
Dictionary::update_dictionary( dictionary_& out_dict ) const
{
  return ( *this )->update_dictionary( out_dict );
}

#endif /* DICTIONARY_H */
