/*
 *  slitypecheck.cc
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

#include "slitypecheck.h"

// C++ includes:
#include <sstream>

// Includes from sli:
#include "arraydatum.h"
#include "interpret.h"
#include "iostreamdatum.h"
#include "namedatum.h"
#include "triedatum.h"

/** @BeginDocumentation
Name: trie - Create a new type-trie object
Synopsis: /name -> /name typetrie
Description: Create a new typetrie with internal
name /name. This object is not bound to /name in the
current dictionary. This has to be done by an explicit def.
Examples: /square trie
          [/doubletype] { dup mul } addtotrie def

Author: Marc-Oliver
SeeAlso: addtotrie, cva_t
*/
void
TrieFunction::execute( SLIInterpreter* i ) const
{

  if ( i->OStack.load() < 1 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  LiteralDatum* name = dynamic_cast< LiteralDatum* >( i->OStack.top().datum() );

  if ( name == NULL )
  {
    i->raiseerror( i->ArgumentTypeError );
    return;
  }
  i->EStack.pop();

  TrieDatum* trie = new TrieDatum( *name );

  Token tmp( trie );
  i->OStack.push_move( tmp );
}


/** @BeginDocumentation
Name: addtotrie - Add a function variant to a trie-object
Synopsis: trie [type-list] obj addtotrie -> trie
Parameters:
trie        - a trie object, obtained by a call to trie.
[type-list] - an array with type-names, corresponding to the
              types of the parameters, expected by the function.
Description:
addtotrie adds a new variant to the type-trie. Note, the type-list must
contain at least one type. (Functions without parameters cannot be
overloaded.

Author: Marc-Oliver Gewaltig
SeeAlso: trie
*/

void
AddtotrieFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() < 3 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  TrieDatum* trie = dynamic_cast< TrieDatum* >( i->OStack.pick( 2 ).datum() );

  if ( trie == NULL )
  {
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  TypeArray a;

  // Construct a TypeArray from the TokenArray
  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );

  if ( ad == NULL )
  {
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  if ( ad->size() == 0 )
  {
    i->message(
      SLIInterpreter::M_ERROR, "addtotrie", "type-array must not be empty." );
    i->message(
      SLIInterpreter::M_ERROR, "addtotrie", "No change was made to the trie." );
    i->raiseerror( i->ArgumentTypeError );
    return;
  }


  for ( Token* t = ad->end() - 1; t >= ad->begin(); --t )
  {
    LiteralDatum* nd = dynamic_cast< LiteralDatum* >( t->datum() );

    if ( nd == NULL )
    {
      std::ostringstream message;
      message << "In trie " << trie->getname() << ". "
              << "Error at array position " << t - ad->begin() << '.'
              << std::ends;
      i->message( SLIInterpreter::M_ERROR, "addtotrie", message.str().c_str() );
      i->message( SLIInterpreter::M_ERROR,
        "addtotrie",
        "Array must contain typenames as literals." );
      i->message( SLIInterpreter::M_ERROR,
        "addtotrie",
        "No change was made to the trie." );

      i->raiseerror( i->ArgumentTypeError );
      return;
    }

    a.push_back( *nd );
  }

  trie->insert_move( a, i->OStack.top() );
  i->OStack.pop( 2 );
  i->EStack.pop();
}

/** @BeginDocumentation

   Name: cva_t - Converts a type trie to an equivalent array

   Synopsis: trie cva_t -> /name array

   Description:
   cva_t maps the tree structure of the trie-object to an array.
   The first return value is the name of the trie object.
   The second value is an array, representing the trie.

   The layout of a trie node is represented as:
   [/type [next] [alt]] for non-leaf nodes and
   [object]            for leaf nodes.

   /type is  a literal, representing the expected type.
   [next] is an array, representig the next parameter levels.
   [alt] is an array, representig parameter alternatives
         at the current level.

   This definitions recursively define the type-trie.

   Examples:
   /pop load cva_t -> /pop [/anytype [-pop-]]

   Diagnostics:
   This operation is rather low level and does not raise
   errors
   Bugs:

   Author:
   Marc-Oliver Gewaltig

   FirstVersion:
   May 20 1999

   Remarks:
   cva_t is the inverse function to cvt_a.
   If cva_t is applied to the result of cvt_a, it yields
   the original argument:
   aTrie cva_t cvt_a -> aTrie

   SeeAlso: cvt_a, trie, addtotrie, type, cva
*/

void
Cva_tFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.size() > 0 );

  i->EStack.pop();

  Token trietoken;
  trietoken.move( i->OStack.top() );
  i->OStack.pop();

  TrieDatum* trie = dynamic_cast< TrieDatum* >( trietoken.datum() );
  assert( trie != NULL );

  Name triename( trie->getname() );
  i->OStack.push( LiteralDatum( triename ) );
  TokenArray a;
  trie->get().toTokenArray( a );
  i->OStack.push( ArrayDatum( a ) );
}

void
TrieInfoFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.size() > 1 );

  i->EStack.pop();

  OstreamDatum* osd =
    dynamic_cast< OstreamDatum* >( i->OStack.pick( 1 ).datum() );
  assert( osd != 0 );

  Token trietoken;
  trietoken.move( i->OStack.top() );

  TrieDatum* trie = dynamic_cast< TrieDatum* >( trietoken.datum() );
  assert( trie != NULL );

  trie->get().info( **osd );
  i->OStack.pop( 2 );
}

/** @BeginDocumentation

   Name: cvt_a - Converts an array to the equivalent type trie.

   Synopsis:  /name array cvt_a -> trie

   Description:
   cvt_a tries to construct a type-trie object from a given array.
   The supplied literal is used as name for the trie-object.

   WARNING:
   Be very careful when using this function. If the supplied
   array is not well formed, the interpreter will abort
   ungracefully!

   Tries should not be constructed fron scratch using cvt_a.
   Use the operators trie and addtotrie for this purpose.
   Rather, cvt_a is provided to correct minor errors in tries
   with the help of cva_t.

   Parameters:

   The supplied array is the root trie node.
   The layout of each trie node must conform to the following
   pattern:
   [/type [next] [alt]] for non-leaf nodes and
   [object]            for leaf nodes.

   /type is  a literal, representing the expected type.
   object is any type of token. It is returned when this leaf of
          the trie is reached.
   [next] is an array, representig the next parameter levels.
   [alt] is an array, representig parameter alternatives
         at the current level.

   This pattern recursively defines a type-trie. Note, however,
   that violations of this definition are handled ungracefully.

   Examples:
   /pop [/anytype [-pop-]]  cvt_a -> trie

   Diagnostics:
   This operation is low level and does not raise
   errors. If the array is ill-formed, the interpreter will
   abort!

   Bugs:
   Errors should be handled gracefully.

   Author:
   Marc-Oliver Gewaltig

   FirstVersion:
   May 20 1999

   Remarks:
   cvt_a is the inverse function to cva_t.
   If cvt_a is applied to the result of cva_t, it yields
   the original argument:
   /name [array] cvt_a cva_t -> /name [array]

   SeeAlso: cva_t, trie, addtotrie, type, cst, cva, cv1d, cv2d, cvd, cvi, cvlit,
   cvn, cvs
*/
void
Cvt_aFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.size() > 1 );

  LiteralDatum* name =
    dynamic_cast< LiteralDatum* >( i->OStack.pick( 1 ).datum() );
  assert( name != NULL );
  ArrayDatum* arr = dynamic_cast< ArrayDatum* >( i->OStack.pick( 0 ).datum() );
  assert( arr != NULL );

  TrieDatum* trie = new TrieDatum( *name, *arr );
  assert( trie != NULL );
  Token tmp( trie );
  i->OStack.pop();
  i->OStack.push_move( tmp );
}

/** @BeginDocumentation
Name: type - Return the type of an object
Synopsis: obj type -> /typename
Examples: 1 type -> /integertype
*/
void
TypeFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  i->EStack.pop();
  Token tmp;
  tmp.move( i->OStack.top() );
  i->OStack.pop();
  Token n( new LiteralDatum( tmp->gettypename() ) );
  i->OStack.push_move( n );
}

const TrieFunction triefunction;
const TrieInfoFunction trieinfofunction;
const AddtotrieFunction addtotriefunction;
const Cva_tFunction cva_tfunction;
const Cvt_aFunction cvt_afunction;
const TypeFunction typefunction;

void
init_slitypecheck( SLIInterpreter* i )
{
  i->createcommand( "trie", &triefunction );
  i->createcommand( "addtotrie", &addtotriefunction );
  i->createcommand( "trieinfo_os_t", &trieinfofunction );
  i->createcommand( "cva_t", &cva_tfunction );
  i->createcommand( "cvt_a", &cvt_afunction );
  i->createcommand( "type", &typefunction );
}
