/*
 *  typechk.cc
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

/*
    typechk.cc
*/

/******************************************************************
Project:   SLI-2.0, taken from: SLIDE - GUI for SLI

Task:      With a TypeTrie it will be possible to perfrom a type
           check of (SLI) function input parameters. A TypeNode
           represents the position and the datatype of a single
           input parameter. The leaves of the tree will contain the
           interpreter function of correct input parameters.

           A simple add type tree:
           -----------------------

           *root
            |
           long -----------------> double ->0
            |                        |
           long ->  double->0      long  ->  double -> 0
            |         |             |          |
           (add)->0 (add)->0      (add)->0  (add)->0
            |        |             |         |
            0        0             0         0

Baseclass: None

Inherit :

Author:    Marc-Oliver Gewaltig, Thomas Matyak

Date:      18.11.95

*******************************************************************/


#include "typechk.h"

// Includes from sli:
#include "arraydatum.h"
#include "namedatum.h"
#include "sliexceptions.h"

void
TypeTrie::TypeNode::toTokenArray( TokenArray& a ) const
{
  assert( a.size() == 0 );
  if ( next == NULL && alt == NULL ) // Leaf node
  {
    a.push_back( func );
  }
  else
  {
    assert( next != NULL );
    a.push_back( LiteralDatum( type ) );
    TokenArray a_next;
    next->toTokenArray( a_next );
    a.push_back( ArrayDatum( a_next ) );
    if ( alt != NULL )
    {
      TokenArray a_alt;
      alt->toTokenArray( a_alt );
      a.push_back( ArrayDatum( a_alt ) );
    }
  }
  assert( a.size() != 0 );
}

void
TypeTrie::TypeNode::info( std::ostream& out, std::vector< TypeNode const* >& tl ) const
{
  if ( next == NULL && alt == NULL ) // Leaf node
  {
    // print type list then function
    for ( int i = tl.size() - 1; i >= 0; --i )
    {
      out << std::setw( 15 ) << std::left << LiteralDatum( tl[ i ]->type );
    }
    out << "calls " << func << std::endl;
  }
  else
  {
    assert( next != NULL );
    tl.push_back( this );
    next->info( out, tl );
    tl.pop_back();
    if ( alt != NULL )
    {
      alt->info( out, tl );
    }
  }
}


TypeTrie::TypeNode*
TypeTrie::newnode( const TokenArray& ta ) const
{
  assert( ta.size() > 0 );
  assert( ta.size() <= 3 );
  TypeNode* n = NULL;
  if ( ta.size() == 1 ) // leaf
  {
    n = new TypeNode( sli::object, ta[ 0 ] );
  }
  else
  {
    // first object in the array must be a literal, indicating the type
    // the second and third object must be an array.
    LiteralDatum* typed = dynamic_cast< LiteralDatum* >( ta[ 0 ].datum() );
    assert( typed != NULL );
    ArrayDatum* nextd = dynamic_cast< ArrayDatum* >( ta[ 1 ].datum() );
    assert( nextd != NULL );
    n = new TypeNode( *typed );
    n->next = newnode( *nextd );
    if ( ta.size() == 3 )
    {
      ArrayDatum* altd = dynamic_cast< ArrayDatum* >( ta[ 2 ].datum() );
      assert( altd != NULL );
      n->alt = newnode( *altd );
    }
  }
  return n;
}

/*
Task:      Destructor removes the complete tree.

Author:    Marc Oliver Gewaltig

Date:      18.11.95

Parameter: None
*/


TypeTrie::TypeNode*
TypeTrie::getalternative( TypeTrie::TypeNode* pos, const Name& type )
{
  // Finds Node for the current type in the alternative List,
  // starting at pos. If the type is not already present, a new
  // Node will be created.
  const Name empty;

  if ( pos->type == empty )
  {
    pos->type = type;
    //    assert(pos->next == NULL);
    return pos;
  }

  while ( type != pos->type )
  {
    if ( pos->alt == NULL )
    {
      pos->alt = new TypeNode( type );
    }

    if ( pos->type == sli::any )
    {
      // any must have been the tail and the previous
      // if must have added an extra Node, thus the following
      // assertion must hold:
      // assert(pos->alt->alt == NULL);

      TypeNode* new_tail = pos->alt;

      // Move the wildcard to the tail Node.

      pos->type = type;
      new_tail->type = sli::any;
      new_tail->func.swap( pos->func );
      new_tail->next = pos->next;
      pos->next = NULL;

      // this  while() cycle will terminate, as
      // pos->type==type by assignment.
    }
    else
    {
      pos = pos->alt; // pos->alt is always defined here.
    }
  }

  return pos;
}

void
TypeTrie::insert_move( const TypeArray& a, Token& f )
{
  /*
  Task:      Array 'a' adds a correct parameter list into the
             'TypeTrie'. Function 'f' will manage the handling
             of a correct parameter list. If 'array' is empty,
             function 'f' will handle a SLI procedure without
             input parameter.
             Insert will overwrite a function with identical parameter
             list which might be already in the trie.

  Bugs:      If a represents a parameter-list which is already
             present, nothing happens, just a warning is
             issued.

  Author:    Marc Oliver Gewaltig

  Date:      15.Apr.1998, 18.11.95
              completely rewritten, 16.Apr. 1998

  Parameter: a = array of datatypes
             f = interpreter function

  */
  TypeNode* pos = root;
  const Name empty;

  assert( root != NULL );

  // Functions with no parameters are possible, but useless in trie
  // structures, so it is best to forbid them!
  assert( not a.empty() );

  for ( unsigned int level = 0; level < a.size(); ++level )
  {

    pos = getalternative( pos, a[ level ] );
    if ( pos->next == NULL )
    {
      pos->next = new TypeNode( empty );
    }
    pos = pos->next;
  }

  /* Error conditions:
     1. If pos->next!=NULL, the parameter list overlaps with
     an existing function definition.
     2. If pos->alt != NULL, something undefined must have happened.
     This should be impossible.
  */
  if ( pos->next == NULL )
  {
    pos->type = sli::object;
    pos->func.move( f );
  }
  else
  {
    std::cout << "Method 'TypeTrie::InsertFunction'" << std::endl
              << "Warning! Ambigous Function Definition ." << std::endl
              << "A function with longer, but identical initial parameter "
              << "list is already present!" << std::endl
              << "Nothing was changed." << std::endl;
  }
}

/*_____ end InsertFunction() _____________________________________*/


void
TypeTrie::toTokenArray( TokenArray& a ) const
{
  a.clear();
  if ( root != NULL )
  {
    root->toTokenArray( a );
  }
}

void
TypeTrie::info( std::ostream& out ) const
{
  std::vector< TypeNode const* > tl;
  tl.reserve( 5 );
  if ( root != NULL )
  {
    root->info( out, tl );
  }
}
