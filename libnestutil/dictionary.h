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

#include "config.h"
#include "exceptions.h"
#include "logging.h"

#ifdef HAVE_BOOST
#include <boost/type_index.hpp>
#endif

namespace nest
{
class Parameter;
class NodeCollection;
}

/**
 * @defgroup nestdict Data types related to the @ref Dictionary class in NEST
 *
 * The @ref Dictionary class provides the data exchange interface between the NEST kernel
 * and the Python level as well as with C++ applications. This group collects data types
 * related to the implementation of the @Dictionary class.
 *
 * The @ref Dictionary class provides a map-like interface with access checking to allow
 * detection of misspelled or unknown entries passed in to NEST. Since dictionaries need to
 * be able to contain dictionaries, the outward-facing @ref Dictionary class acts as a handle
 * to the @ref dictionary_ instance actually implementing the dictionary as a @c std::map<>.
 *
 * The dictionary entries are of type @c DictEntry_ combining the actual data and the access flag.
 * The actual data are represented as instances of @ref any_type , which is a C++20 @c std::variant .
 */
class dictionary_;
class Dictionary;
class AnyVector;
struct EmptyList;


/**
 * Add a set of standard datatypes to @ref any_type variant.
 *
 * The main purpose of this type is to automatically generate variants representing the basic scalar types,
 * strings, and dictionaries and one-dimensional @c std::vector<> of these. More specific types are added
 * later in the actual declaration of @ref any_type.
 *
 * The first entry in the @c std::variant<> must be @c std::monostate , so that default-initialized @ref any_value
 * instances will have this type. This is needed to fill the entries corresponding to non-local nodes when status
 * information for a @ref NodeCollection is returned.
 *
 * @note Conversion from @ref Dictionary elements to and from Python datatypes is done by
 * @c nestkernel_api.pyx:any_to_pyobj and @c nestkernel_api.pyx:pydict_to_Dictionary, respectively. If you add a
 * type to the variant, you most likely need to add corresponding conversions to those functions.
 *
 * @ingroup nestdict
 */
template < typename... Scalars >
struct DictionarySchemaBuilder
{
  template < typename... Extras >
  using VariantType = std::variant< std::monostate, Scalars..., std::vector< Scalars >..., Extras... >;
};

/**
 * Basic scalar data types, strings and dictionaries to be included in variant.
 *
 * This schema includes all types that should enter the variant as themselves and as vectors.
 * Only @c long is included as integer type, because Python only knows signed integers.
 *
 * @ingroup nestdict
 */
using DictionarySchema = DictionarySchemaBuilder< double, long, bool, std::string, Dictionary >;

/**
 * Complete @c any_type by adding specific data types.
 *
 * Add here data types that are required but were not added through the schema. The aim is to have
 * as few types in the variant as possible and we therefore limit multi-dimensional vectors to those
 * element types where they are needed (3-dim vectors are required for correlo-dectectors).
 *
 * @ref AnyVector is essentially a flexible Python list type. It is needed to return status data
 * combining actual data from local nodes with @c std::monostate values marking the absence of data. These
 * are converted to @c None at the Python level to maintain the NEST 3.9 user interface.
 *
 * @ingroup nestdict
 */
using any_type = DictionarySchema::VariantType< std::shared_ptr< nest::NodeCollection >,
  std::vector< std::vector< double > >,
  std::vector< std::vector< std::vector< double > > >,
  std::vector< std::vector< std::vector< long > > >,
  std::shared_ptr< nest::Parameter >,
  AnyVector,
  nest::VerbosityLevel,
  EmptyList >;

/**
 * Vector of @ref any_type
 *
 * This type is required to collect status data from elements of a @ref NodeCollection .
 *
 * The main need for this vector type is to combine actual data from local nodes with
 * @c std::monostate entries for non-local nodes when returning status data from @ref NodeCollection .
 *
 * We define it as a class of its own to get around circular definition problems.
 *
 * @ingroup nestdict
 */
class AnyVector : public std::vector< any_type >
{
  using vectype_ = std::vector< any_type >;
  using vectype_::vectype_;  // Inherit constructors
};

/**
 * Datatype to allow empty Python lists to be represented without defaulting to a datatype.
 *
 * Python lists are converted to @c std::vector, which requires chosing a data type. For non-empty types the
 * type can be inferred from the elements, but this is not possible for empty lists. Here, only the C++ level
 * knows what type would be expected. To avoid type collisions, the Cython interface represents empty lists
 * as @c EmptyList which can be cast to vectors of any element data type at the C++ level.
 *
 * @ingroup nestdict
 */
struct EmptyList
{
  bool operator==( const EmptyList& ) const = default;
};

/**
 * Print @ref EmptyList.
 *
 * @ingroup nestdict
 */
std::ostream& operator<<( std::ostream& os, const EmptyList& );

/**
 * Print @c std@::monostate as "None".
 *
 * @ingroup nestdict
 */
std::ostream& operator<<( std::ostream& os, const std::monostate& );

/**
 * Print @ref AnyVector.
 *
 * @ingroup nestdict
 */
std::ostream& operator<<( std::ostream& os, const AnyVector& av );

/**
 * Print @ref Dictionary.
 *
 * @ingroup nestdict
 */
std::ostream& operator<<( std::ostream& os, const Dictionary& dict );

/**
 * A concept for "integer integers", excluding @c bool and @c char.
 *
 * @ingroup nestdict
 */
template < typename T >
concept Integer = std::integral< T > and not std::same_as< T, bool > and not std::same_as< T, char >;

/**
 * A concept for data types that can be stored in a @ref Dictionary as @ref any_type .
 *
 * @ingroup nestdict
 */

template < typename T >
concept DictionaryEntryType = std::is_constructible_v< any_type, T >;

/**
 * Dictionary class for interface to Python and C++ API.
 *
 * Only this class should be used in code outside @c dictionary.{h,cpp}.
 *
 * Methods that behave as in @c std::map are not documented explicitly.
 *
 * @ingroup nestdict
 */
class Dictionary : public std::shared_ptr< dictionary_ >
{
public:
  Dictionary()
    : std::shared_ptr< dictionary_ >( std::make_shared< dictionary_ >() )
  {
  }

  any_type& operator[]( const std::string& key );
  any_type& operator[]( std::string&& key );
  any_type& at( const std::string& key );
  const any_type& at( const std::string& key ) const;

  /**
   * @brief Check whether the dictionary is equal to another dictionary.
   *
   * Two dictionaries are equal if and only if they contain the exact same entries with the same values.
   *
   * @param other dictionary to check against.
   * @return true if the dictionary is equal to the other dictionary, false if not.
   */
  bool operator==( const Dictionary& other ) const;

  auto begin() const;
  auto end() const;

  auto size() const;
  auto empty() const;
  void clear();
  auto find( const std::string& key ) const;

  /**
   * Check whether there exists a value with specified key in the dictionary.
   *
   * @param key key where the value may be located in the dictionary.
   * @return @c true if there is a value with the specified key, @c false if not.
   *
   * @note This does @b not mark the entry as accessed, because we sometimes need to confirm
   * that a certain key is not in a dictionary.
   */
  bool known( const std::string& key ) const;

  /**
   * Mark to support checking for unaccessed entries
   *
   * @todo Should be removed, see #3791
   */
  void mark_as_accessed( const std::string& key ) const;

  /**
   * Return @c true if @ref mark_as_accessed has been called at least once for @ref key
   *
   * @todo Should be removed, see #3791
   */
  bool has_been_accessed( const std::string& key ) const;

  /**
   * Initialize access flags to prepare for later error checking with @ref all_entries_accessed.
   *
   * @param thread_local_dict Pass @c true if calling this method on a thread-specific dictionary. Used for internal
   * error checking (assert).
   */
  void init_access_flags( const bool thread_local_dict = false ) const;

  /**
   * Confirm that all entries in dictionary have been accessed.
   *
   * Requires that @ref init_access_flags has been called previously and does nothing if @c dict_miss_is_error has been
   * set to @c false.
   *
   * @throws @ref nest::UnaccessedDictionaryEntry if at least one dictionary entry has not been marked as accessed
   * @param where Information about calling location, will be included in error message
   * @param what Information about dictionary inspected, will be included in error message
   * @param thread_local_dict Pass @c true if calling this method on a thread-specific dictionary. Used for internal
   * error checking (assert).
   */
  void
  all_entries_accessed( const std::string& where, const std::string& what, const bool thread_local_dict = false ) const;

  /**
   * @brief Get the value at key in the specified type.
   *
   * @tparam T Type of the value. If the value is not of the specified type, a TypeMismatch error is thrown.
   * @param key key where the value is located in the dictionary.
   * @throws @c std::out_of_range if @c key is not on dictionary
   * @throws @ref TypeMismatch if the value is not of specified type T.
   * @return the value at key cast to the specified type.
   */
  template < DictionaryEntryType T >
  T get( const std::string& key ) const;

  /**
   * Return reference to vector of type T stored under key.
   *
   * If key does not exist in dict, create empty vector<T> and return it.
   */
  template < typename T >
  std::vector< T >& get_or_create_vector( const std::string& key );

  /**
   * @brief Update the specified non-vector value if there exists a value at key.
   *
   * @param key key where the value may be located in the dictionary.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not the same type as the value argument.
   * @return @c true if @c key was found in dictionary and @c value was updated, else @c false
   *
   * @note Only use this where the user is not allowed to use random or spatial parameters.
   *       Otherwise, use @ref update_value_param .
   */
  template < typename T >
  bool update_value( const std::string& key, T& value ) const;

  /**
   * @brief Update the specified value if there exists an integer value at key.
   *
   * @param key key where the value may be located in the dictionary.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not an integer.
   * @return @c true if @c key was found in dictionary and @c value was updated, else @c false
   */

  template < Integer T >
  bool update_integer_value( const std::string& key, T& value ) const;

  /**
   * @brief Update the provided dictionary with all key-value pairs in this dictionary.
   *
   * @param dict_out the dictionary to be updated.
   * @return @c true if any values were updated in or added to @c dict_out .
   */
  bool update_dictionary( Dictionary& dict_out ) const;
};

/**
 * Return typename boost-prettified if available, otherwise as returned by typename().
 *
 * @note This function isolates the boost-dependency of pretty-printing the type.
 * @ingroup nestdict
 */
template < typename T >
std::string
pretty_typename()
{
#ifdef HAVE_BOOST
  return boost::typeindex::type_id< T >().pretty_name();
#else
  return typeid( T ).name();
#endif
}

/**
 * @brief Return typename of @c operand.
 *
 * @ingroup nestdict
 */
std::string get_typename( const any_type& operand );

/**
 * @brief Return multi-line string displaying dictionary content.
 *
 * @ingroup nestdict
 */
std::string get_dict_typenames( const Dictionary& dict );

/**
 * Represent value in dictionary entry with access information.
 *
 * @c accessed flag is mutable so access (error) checking does not interfere with logcial const-ness.
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

  any_type item;          //!< actual item stored
  mutable bool accessed;  //!< initially false, set to true once entry is accessed
};

/**
 * @brief A Python-like dictionary_, based on @c std::map.
 *
 * Values are stored as @ref DictEntry_ items providing access checking.
 */
class dictionary_ : public std::map< std::string, DictEntry_ >
{
  using maptype_ = std::map< std::string, DictEntry_ >;
  using maptype_::maptype_;  // Inherit constructors

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
      std::string msg =
        String::compose( "Failed to cast '%1' from %2 to type %3", key, get_typename( value ), pretty_typename< T >() );
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

  //! Mark dictionary entry as accessed
  void register_access_( const DictEntry_& entry ) const;

public:
  /**
   * See @ref Dictionary::get
   */
  template < DictionaryEntryType T >
  T
  get( const std::string& key ) const
  {
    return cast_value_< T >( at( key ), key );
  }

  /**
   * See @ref Dictionary::get_or_create_vector
   */
  template < typename T >
  std::vector< T >&
  get_or_create_vector( const std::string& key )
  {
    // try_emplace will only insert if key doesn't exist.
    auto [ iter, success ] = maptype_::try_emplace( key, std::vector< T >() );

    return std::get< std::vector< T > >( iter->second.item );
  }

  /**
   * See @ref Dictionary::update_value
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
   * See @ref Dictionary::update_integer_value
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
   * See @ref Dictionary::update_dictionary
   */
  bool
  update_dictionary( Dictionary& dict_out ) const
  {
    for ( const auto& [ key, value ] : *this )
    {
      dict_out[ key ] = value.item;
    }
    return size() > 0;
  }

  /**
   * See @ref Dictionary::known
   */
  bool
  known( const std::string& key ) const
  {
    return maptype_::contains( key );
  }

  /**
   * See @ref Dictionary::marked_as_accessed
   */
  void
  mark_as_accessed( const std::string& key ) const
  {
    register_access_( maptype_::at( key ) );
  }

  /**
   * See @ref Dictionary::has_been_accessed
   */
  bool
  has_been_accessed( const std::string& key ) const
  {
    return maptype_::at( key ).accessed;
  }

  /**
   * See @ref Dictionary::operator==
   */
  bool operator==( const dictionary_& other ) const;

  /**
   * See @ref Dictionary::operator!=
   */
  bool
  operator!=( const dictionary_& other ) const
  {
    return not( *this == other );
  }

  /**
   * See @ref Dictionary::init_access_flags
   */
  void init_access_flags( const bool thread_local_dict = false ) const;

  /**
   * See @ref Dictionary::all_entries_accessedinit_access_flags
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
 * Specialization that allows passing long or empty vectors where double vectors are expected.
 *
 * @note This specialization forwards to cast_vector_value_<double>, but is required explicitly,
 *       because, e.g., @c get() calls @c 0cast_value_() directly even if the argument is a vector.
 */
template <>
std::vector< double > dictionary_::cast_value_< std::vector< double > >( const any_type& value,
  const std::string& key ) const;

/**
 * Specialization that allows passing empty lists where a list of strings is expected.
 */
template <>
std::vector< std::string > dictionary_::cast_value_< std::vector< std::string > >( const any_type& value,
  const std::string& key ) const;

inline bool
Dictionary::operator==( const Dictionary& other ) const
{
  return **this == *other;
}

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
Dictionary::clear()
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

template < DictionaryEntryType T >
inline T
Dictionary::get( const std::string& key ) const
{
  return ( *this )->get< T >( key );
}
template < typename T >
std::vector< T >&
Dictionary::get_or_create_vector( const std::string& key )
{
  return ( *this )->get_or_create_vector< T >( key );
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
Dictionary::update_dictionary( Dictionary& out_dict ) const
{
  return ( *this )->update_dictionary( out_dict );
}

#endif /* DICTIONARY_H */
