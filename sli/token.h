/*
 *  token.h
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

#ifndef TOKEN_H
#define TOKEN_H
/*
    token.h defines the base objects used by the SLI interpreter.
*/

// C++ includes:
#include <iomanip>
#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "datum.h"

class Name;
class Token;
class TokenArray;
class TokenArrayObj;

/***********************************************************/
/* Token                                               */
/* ---------                                               */
/*  Token class for all Data Objects                   */
/*

const Datum* p;   makes p a pointer to a const. Any change to the
                  object p points to is prevented.

Datum *const p;   makes p a const pointer to a Datum. Any change to the
                  pointer is prevented.

 It is not necessary to declare the pointer itself const, because the
 return value is copied anyway. Only if the return value can be used as
 an lvalue do we need to protect it.

  A member function declared const does not change any member object
  of the class, it can be called for const class objects.

*/

/***********************************************************/

/**
 * A type-independent container for C++-types
 * ==========================================
 *
 * Class Token is a wrapper class around Datum pointers and non-Datum
 * objects. In fact, since Datum objects have a memory manager, we should
 * avoid creating Datum objects on the stack as local variables. Class
 * Token takes ownership of the Datum pointers and will properly delete
 * them when they are no longer needed. Thus, use one of the following
 * idioms:
 *
 * Construction
 * ------------
 *
 * .. code-block:: C++
 *    Token t( new IntergerDatum( 5 ) );
 *    Token t = 5;
 *    Token t = new IntegerDatum( 5 );
 *
 * The object constructor `Token(Datum&)` is historic and should not be
 * used anymore.
 *
 * Assignment
 * ----------
 *
 * .. code-block:: C++
 *    Token t1 = t;
 *    t1.move( t ); // move Datum from t to t1
 *
 * ``TokenArrays``, ``TokenStack``, and ``Dictionary`` are token
 * containers. Their assignment interface takes
 *
 * 1. Datum pointers
 * 2. Token references
 *
 * Thus, the recommended assignments are
 *
 * .. code-block:: C++
 *    array.push_back( new IntegerDatum( 5 ) );
 *
 * It directly passes the Datum pointer to the location in the
 * array. Some convenient ways to write assignments are actually
 * inefficient
 *
 * Examples
 * --------
 *
 * 1. ``a.push_back(5);``
 *
 *    This is convenient notation, but is much more expensive, because it is
 *    equivalent to the following code:
 *
 *    .. code-block:: C++
 *       IntegerDatum tmp1( 5 );
 *       Token tmp2( new IntegerDatum( mp1 ) );
 *       Token tmp3( tmp2 );  // one more Datum copy
 *       a.push_back_move( tmp3 );
 *
 *    Some of this, the compiler can optimize away, but benchmarks showed a
 *    big residual overhead compared to directly assigning the Datum
 *    pointer.
 *
 * 2. ``a.push_back(IntegerDatum(5));``
 *
 *    This looks efficient, but in fact it is not, because it is equivalent
 *    to:
 *    .. code-block:: C++
 *       Token tmp1( new IntegerDatum( IntegerDatum( 5 ) );
 *       a.push_back_move( tmp1 );
 *
 * 3. ``a.push_back(t);``
 *
 *    Involves one Datum copy
 *
 * 4. ``a.push_back_move(t);``
 *
 *    Moves the pointer and leaves a void token behind.
 *
 * @ingroup TokenHandling
 */
class Token
{
  friend class Datum;
  friend class TokenArrayObj;

private:
  Datum* p;

  /** Flag for access control.
   * Is set by getValue() and setValue() via datum().
   */
  mutable bool accessed_;

public:
  ~Token()
  {
    if ( p )
    {
      p->removeReference();
    }
    p = 0;
  }

  Token( const Token& c_s )
    : p( NULL )
  {
    if ( c_s.p )
    {
      p = c_s.p->get_ptr();
    }
  }


  /**
   * use existing pointer to datum, token takes responsibility of the pointer.
   */
  Token( Datum* p_s = NULL )
    : p( p_s )
  {
  }

  Token( const Datum& d ) //!< copy datum object and store its pointer.
  {
    p = d.clone();
  }

  Token( int );
  Token( unsigned int );
  Token( long );
  Token( bool );
  Token( unsigned long );
#ifdef HAVE_32BIT_ARCH
  Token( uint64_t );
#endif
  Token( double );
  Token( const char* );
  Token( std::string );
  Token( const std::vector< double >& );
  Token( const std::vector< long >& );
  Token( const std::vector< size_t >& );
  Token( const std::ostream& );
  Token( const std::istream& );
  operator Datum*() const;
  operator size_t() const;
  operator long() const;
  operator double() const;
  operator float() const;
  operator bool() const;
  operator std::string() const;
  //  operator vector<double> const;
  //  operator vector<long> const;

  /**
   * If the contained datum has more than one reference, clone it, so it can
   * be modified.
   */
  void
  detach()
  {
    if ( p and p->numReferences() > 1 )
    {
      p->removeReference();
      p = p->clone();
    }
  }

  void
  move( Token& c )
  {
    if ( p )
    {
      p->removeReference();
    }
    p = c.p;
    c.p = NULL;
  }


  /**
   * Initialize the token by moving a datum from another token.
   * This function assumes that the token does not
   * point to a valid datum and that the argument token
   * does point to a valid datum.
   * This function does not change the reference count of the datum.
   */
  void
  init_move( Token& rhs )
  {
    p = rhs.p;
    rhs.p = NULL;
  }

  /**
   * Initialize the token by moving a datum from another token.
   * This function assumes that the token does not
   * point to a valid datum and that the argument token
   * does point to a valid datum.
   * This function does not change the reference count of the datum.
   */
  void
  init_by_copy( const Token& rhs )
  {
    p = rhs.p->get_ptr();
  }

  /**
   * Initialize the token with a reference.
   * This function assumes that the token does not
   * point to a valid datum and that the argument token
   * does point to a valid datum.
   * This function increases the reference count of the argument.
   */

  void
  init_by_ref( const Token& rhs )
  {
    rhs.p->addReference();
    p = rhs.p;
  }

  /**
   * Initialize the token with a datum pointer.
   * This function assumes that the token does not point to
   * a valid datum.
   * The function assumes that the datum is new and DOES NOT increases its
   * reference count.
   */
  void
  init_by_pointer( Datum* rhs )
  {
    p = rhs;
  }

  void
  assign_by_ref( const Token& rhs )
  {
    //    assert(rhs.p !=NULL);
    if ( p != rhs.p )
    {
      if ( p )
      {
        p->removeReference();
      }
      p = rhs.p->get_ptr();
    }
  }

  void
  assign_by_pointer( Datum* rhs )
  {
    assert( rhs != NULL );
    rhs->addReference();
    if ( p )
    {
      p->removeReference();
    }
    p = rhs;
  }


  void
  swap( Token& c )
  {
    std::swap( p, c.p );
  }

  void
  clear( void )
  {
    if ( p )
    {
      p->removeReference();
    }
    p = NULL;
  }

  bool
  contains( const Datum& d ) const
  {
    return ( p != NULL ) ? p->equals( &d ) : false;
  }

  bool
  empty( void ) const
  {
    return p == NULL;
  }

  bool operator not( void ) const
  {
    return p == NULL;
  }

  Datum*
  datum( void ) const
  {
    accessed_ = true;
    return p;
  }


  bool
  valid() const
  {
    return not empty();
  }

  Datum* operator->() const
  {
    //      assert(p!= NULL);
    return p;
  }


  Datum& operator*() const
  {
    //      assert(p != NULL);
    return *p;
  }


  const std::type_info&
  type( void ) const
  {
    return typeid( *p );
  }


  Token& operator=( const Token& c_s )
  {
    if ( c_s.p == p )
    {
      return *this;
    }

    if ( c_s.p == NULL )
    {
      clear();
      return *this;
    }
    if ( p )
    {
      p->removeReference();
    }
    p = c_s.p->get_ptr();

    return *this;
  }

  Token& operator=( Datum* p_s )
  {
    if ( p != p_s )
    {
      if ( p )
      {
        p->removeReference();
      }
      p = p_s;
    }

    return *this;
  }


  bool operator==( const Token& t ) const
  {
    if ( p == t.p )
    {
      return true;
    }

    return p and p->equals( t.p );
  }

  // define != explicitly --- HEP 2001-08-09
  bool operator!=( const Token& t ) const
  {
    return not( *this == t );
  }

  void info( std::ostream& ) const;

  void pprint( std::ostream& ) const;

  /** Clear accessed flag. */
  void
  clear_access_flag()
  {
    accessed_ = false;
  }
  void
  set_access_flag() const
  {
    accessed_ = true;
  }

  /** Check for access.
   * Access control does not differentiate between read and write
   * access, and relies on getValue<>, setValue<> to use datum()
   * to access the data in the Token. The access flag should be
   * cleared before entering the code for which access is to be
   * checked.
   */
  bool
  accessed() const
  {
    return accessed_;
  }


  /**
   * Check whether Token contains a Datum of a given type.
   * @return true if Token is of type given by template parameter.
   */
  template < typename DatumType >
  bool
  is_a() const
  {
    return dynamic_cast< DatumType* >( p );
  }


  /**
   * Returns true if token equals rhs as string.
   *
   * The main purpose of this method is to allow seamless
   * comparison of LiteralDatum and StringDatum tokens.
   */
  bool matches_as_string( const Token& rhs ) const;
};


/************* Misc functions ********************/

std::ostream& operator<<( std::ostream&, const Token& );

typedef unsigned long Index;

#endif
