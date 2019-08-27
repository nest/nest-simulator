/*
 *  dict.h
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

#ifndef DICT_H
#define DICT_H
/*
    SLI's dictionary class
*/
// C++ includes:
#include <map>

// Includes from sli:
#include "name.h"
#include "sliexceptions.h"
#include "token.h"


typedef std::map< Name, Token, std::less< Name > > TokenMap;

inline bool operator==( const TokenMap& x, const TokenMap& y )
{
  return ( x.size() == y.size() ) && equal( x.begin(), x.end(), y.begin() );
}

/** A class that associates names and tokens.
 *  @ingroup TokenHandling
 */
class Dictionary : private TokenMap
{
  /**
   * Helper class for lexicographical sorting of dictionary entries.
   * Provides comparison operator for ascending, case-insensitive
   * lexicographical ordering.
   * @see This is a simplified version of the class presented in
   * N.M.Josuttis, The C++ Standard Library, Addison Wesley, 1999,
   * ch. 6.6.6.
   */
  class DictItemLexicalOrder
  {
  private:
    static bool nocase_compare( char c1, char c2 );

  public:
    bool operator()( const std::pair< Name, Token >& lhs, const std::pair< Name, Token >& rhs ) const
    {
      const std::string& ls = lhs.first.toString();
      const std::string& rs = rhs.first.toString();

      return std::lexicographical_compare( ls.begin(), ls.end(), rs.begin(), rs.end(), nocase_compare );
    }
  };

public:
  Dictionary()
    : refs_on_dictstack_( 0 )
  {
  }
  Dictionary( const Dictionary& d )
    : TokenMap( d )
    , refs_on_dictstack_( 0 )
  {
  }
  ~Dictionary();

  using TokenMap::erase;
  using TokenMap::size;
  using TokenMap::begin;
  using TokenMap::end;
  using TokenMap::iterator;
  using TokenMap::find;

  void clear();

  /**
   * Lookup and return Token with given name in dictionary.
   * If the name is not found, an empty token is returned.
   * This version of lookup is deprecated and will disappear in future versions.
   * Please use lookup2() instead.
   * @note The token returned should @b always  be stored as a
   *       <tt>const \&</tt>, so that the control flag for
   *       dictionary read-out is set on the Token in the dictionary,
   *       not its copy.
   */

  const Token& lookup( const Name& n ) const;

  /**
   * lookup a name in the dictionary. If the name is not found an UndefinedName
   * exception is thrown.
   * lookup2 is the preferred way to retrieve entries from the dictionary.
   * @note The token returned should @b always  be stored as a
   *       <tt>const \&</tt>, so that the control flag for
   *       dictionary read-out is set on the Token in the dictionary,
   *       not its copy.
   */
  const Token& lookup2( const Name& n ) const; // throws UndefinedName
  bool known( const Name& ) const;

  //! Returns true if name is known but token has not been accessed
  bool known_but_not_accessed( const Name& ) const;

  Token& insert( const Name& n, const Token& t );
  Token& insert_move( const Name&, Token& );

  //! Remove entry from dictionary
  void remove( const Name& n );

  const Token& operator[]( const Name& ) const;
  Token& operator[]( const Name& );
  const Token& operator[]( const char* ) const;
  Token& operator[]( const char* );

  bool
  empty( void ) const
  {
    return TokenMap::empty();
  }

  void info( std::ostream& ) const;

  bool operator==( const Dictionary& d ) const
  {
    return ::operator==( *this, d );
  }

  /**
   * Add the contents of this dictionary to another.
   * The target dictionary is given by names and must be retrieved via
   * the interpreter.
   * @todo Allow for free formatting of target dictionary entries
   * via functor, and add traits to allow duplicates.
   * @see remove_dict
   */
  void add_dict( const std::string&, SLIInterpreter& );

  /**
   * Remove entries found in another dictionary from this.
   * @see add_dict
   */
  void remove_dict( const std::string&, SLIInterpreter& );

  /**
   * Clear access flags on all dictionary elements.
   * Flags for nested dictionaries are cleared recursively.
   * @see all_accessed()
   */
  void clear_access_flags();

  /**
   * Check whether all elements have been accessed.
   * Checks nested dictionaries recursively.
   * @param std::string& contains string with names of non-accessed entries
   * @returns true if all dictionary elements have been accessed
   * @note this is just a wrapper, all_accessed_() does the work, hides
   * recursion
   * @see clear_access_flags(), all_accessed_()
   */
  bool
  all_accessed( std::string& missed ) const
  {
    return all_accessed_( missed );
  }

  friend std::ostream& operator<<( std::ostream&, const Dictionary& );

  /**
   * Constant iterator for dictionary.
   * Dictionary inherits privately from std::map to hide implementation
   * details. To allow for inspection of all elements in a dictionary,
   * we export the constant iterator type and begin() and end() methods.
   */
  typedef TokenMap::const_iterator const_iterator;

  /**
   * First element in dictionary.
   * Dictionary inherits privately from std::map to hide implementation
   * details. To allow for inspection of all elements in a dictionary,
   * we export the constant iterator type and begin() and end() methods.
   */
  //  const_iterator begin() const;

  /**
   * One-past-last element in dictionary.
   * Dictionary inherits privately from std::map to hide implementation
   * details. To allow for inspection of all elements in a dictionary,
   * we export the constant iterator type and begin() and end() methods.
   */
  // const_iterator end() const;

  /**
   *
   */
  void initialize_property_array( Name propname );

  /**
   * This function is called when a dictionary is pushed to the dictionary
   * stack. The dictioray stack must keep track about which dictioraries are on
   * the dictionary stack. If a dictionary is modified and it is on the
   * dictionary stack, the cache of the dictionary stack must
   * be adjusted. This is e.g. the case for the systemdict or the errordict.
   */
  void
  add_dictstack_reference()
  {
    ++refs_on_dictstack_;
  }

  /**
   * This function is called when the dictionary is popped from the dictionary
   * stack.
   */
  void
  remove_dictstack_reference()
  {
    --refs_on_dictstack_;
  }

  /**
   * Returns true, if the dictionary has references on the dictionary stack.
   */
  bool
  is_on_dictstack() const
  {
    return refs_on_dictstack_ > 0;
  }


private:
  /**
   * Worker function checking whether all elements have been accessed.
   * Checks nested dictionaries recursively.
   * @param std::string& contains string with names of non-accessed entries
   * @param std::string prefix for nested dictionary entries, built during
   * recursion
   * @returns true if all dictionary elements have been accessed
   * @note this is just the worker for all_accessed()
   * @see clear_access_flags(), all_accessed()
   */

  int refs_on_dictstack_;
  bool all_accessed_( std::string&, std::string prefix = std::string() ) const;
  static const Token VoidToken;
};

inline const Token&
Dictionary::lookup( const Name& n ) const
{
  TokenMap::const_iterator where = find( n );
  if ( where != end() )
  {
    return ( *where ).second;
  }
  else
  {
    return Dictionary::VoidToken;
  }
}

inline const Token&
Dictionary::lookup2( const Name& n ) const
{
  TokenMap::const_iterator where = find( n );
  if ( where != end() )
  {
    return ( *where ).second;
  }
  else
  {
    throw UndefinedName( n.toString() );
  }
}

inline bool
Dictionary::known( const Name& n ) const
{
  TokenMap::const_iterator where = find( n );
  if ( where != end() )
  {
    return true;
  }
  else
  {
    return false;
  }
}

inline bool
Dictionary::known_but_not_accessed( const Name& n ) const
{
  TokenMap::const_iterator where = find( n );
  if ( where != end() )
  {
    return not where->second.accessed();
  }
  else
  {
    return false;
  }
}

inline Token&
Dictionary::insert( const Name& n, const Token& t )
{
  return TokenMap::operator[]( n ) = t;
}


inline const Token& Dictionary::operator[]( const Name& n ) const
{
  TokenMap::const_iterator where = find( n );
  if ( where != end() )
  {
    return ( *where ).second;
  }
  else
  {
    throw UndefinedName( n.toString() );
  }
}


inline Token& Dictionary::operator[]( const Name& n )
{
  return TokenMap::operator[]( n );
}

inline Token&
Dictionary::insert_move( const Name& n, Token& t )
{
  Token& result = TokenMap::operator[]( n );
  result.move( t );
  return result;
}


#endif
