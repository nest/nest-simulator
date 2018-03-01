/*
 *  typechk.h
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

#ifndef TYPECHECK_H
#define TYPECHECK_H
/*
    classes for dynamic type checking in SLI
*/

/******************************************************************
Project:   SYNOD/SLI 2.0

Task:      With a TypeTrie it will be possible to perfrom a type
           check of (SLI) function input parameters. A TypeNode
           represents the position and the datatype of a single
           input parameter. The leaves of the tree will contain the
           interpreter function of correct input parameters.

           A simple add type tree:
           -----------------------

           root
            |
           long -----------------> double -|
            |                        |
           long -> double -|       long -> double -|
           (add)   (add)           (add)   (add)

Baseclass: None

Inherit :

History:  This is a revised version of the type checking mechanism based on
          tries.
Author:    Marc-Oliver Gewaltig, Thomas Matyak

Date:      18.11.95

*******************************************************************/


// C++ includes:
#include <iostream>
#include <typeinfo>
#include <vector>

// Includes from sli:
#include "slifunction.h"
#include "slinames.h"
#include "tokenarray.h"
#include "tokenstack.h"
#include "typearray.h"

class TypeTrie
{
private:
  class TypeNode
  {
  private:
    unsigned int refs; // number of references to this Node

  public:
    Name type;  // expected type at this stack level
    Token func; // points to the operator or an error func.

    TypeNode* alt;  // points to the next parameter alternative
    TypeNode* next; // points to the next stack level for this path


    void
    addreference( void )
    {
      ++refs;
    }

    void
    removereference( void )
    {
      if ( --refs == 0 )
      {
        delete this;
      }
    }

    TypeNode( const Name& n )
      : refs( 1 )
      , type( n )
      , func()
      , alt( NULL )
      , next( NULL )
    {
    }

    TypeNode( const Name& n, Token f )
      : refs( 1 )
      , type( n )
      , func( f )
      , alt( NULL )
      , next( NULL )
    {
    }

    ~TypeNode()
    {
      if ( next != NULL )
      {
        next->removereference();
      }
      if ( alt != NULL )
      {
        alt->removereference();
      }
    }
    void toTokenArray( TokenArray& ) const;
    void info( std::ostream&, std::vector< TypeNode const* >& ) const;
  };

  TypeNode* root;

  //    TypeTrie operator=(const TypeTrie &){}; // disable this operator
  TypeNode* getalternative( TypeNode*, const Name& );

  TypeNode* newnode( const TokenArray& ) const;

public:
  TypeTrie()
    : root( new TypeNode( Name() ) )
  {
  }

  TypeTrie( const TokenArray& ta )
    : root( NULL )
  {
    root = newnode( ta );
  }

  TypeTrie( const TypeTrie& tt )
    : root( tt.root )
  {
    if ( root != NULL )
    {
      root->addreference();
    }
  }

  ~TypeTrie();

  void insert_move( const TypeArray&, Token& );
  void
  insert( const TypeArray& a, const Token& t )
  {
    Token tmp( t );
    // We have no insert variant, that's why we copy the token
    // to a temporary and then move it to the trie.
    insert_move( a, tmp );
  }

  const Token& lookup( const TokenStack& st ) const;

  bool operator==( const TypeTrie& ) const;

  inline bool equals( const Name&, const Name& ) const;
  void toTokenArray( TokenArray& ) const;
  void info( std::ostream& ) const;
};

inline TypeTrie::~TypeTrie()
{
  if ( root != NULL )
  {
    root->removereference();
  }
}
/*_____ end ~TypeTrie() __________________________________________*/


// Typename comparison including /anytype which compares
// positively for all other typenames

inline bool
TypeTrie::equals( const Name& t1, const Name& t2 ) const
{
  return ( t1 == t2 || t2 == sli::any || t1 == sli::any );
}

inline const Token&
TypeTrie::lookup( const TokenStack& st ) const
{
  /*
  Task:      Tokens on stack 'st' will be compared with the TypeTrie.
             Each stack element must have an equivalent type on the
             current tree level. By reaching a leaf the interpreter
             function will be returned. If an error occurs the
             'ErrorFunction' will be returned.

  Author:    Marc Oliver Gewaltig

  Date:      18.11.95, rewritten on Apr. 16 1998

  Parameter: st = stack

  */
  const unsigned int load = st.load();
  unsigned int level = 0;

  TypeNode* pos = root;

  while ( level < load )
  {
    Name find_type = st.pick( level )->gettypename();

    // Step 1: find the type at the current stack level in the
    // list of alternatives. Unfortunately, this search is O(n).

    while ( not equals( find_type, pos->type ) )
    {
      if ( pos->alt != NULL )
      {
        pos = pos->alt;
      }
      else
      {
        throw ArgumentType( level );
      }
    }

    // Now go to the next argument.
    pos = pos->next;
    if ( pos->type == sli::object )
    {
      return pos->func;
    }

    ++level;
  }

  throw StackUnderflow( level + 1, load );
}


inline bool TypeTrie::operator==( const TypeTrie& tt ) const
{
  return ( root == tt.root );
}

#endif
