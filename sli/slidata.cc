/*
 *  slidata.cc
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
    SLI's data access functions
*/

#include "slidata.h"

// C++ includes:
#include <climits>
#include <sstream>
#include <vector>

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "iteratordatum.h"
#include "namedatum.h"
#include "stringdatum.h"
#include "tokenutils.h"

/** @BeginDocumentation
Name: allocations - Return the number of array reallocations.
Synopsis: - allocations -> int
Description: This function returns the total number of array-allocations
which have occured during the run-time of the SLI interpreter.
This number is important in the context of benchmarking and optimization.
*/
void
Allocations_aFunction::execute( SLIInterpreter* i ) const
{
  Token at( new IntegerDatum( TokenArrayObj::getallocations() ) );
  i->OStack.push_move( at );
  i->EStack.pop();
}

void
Get_aFunction::execute( SLIInterpreter* i ) const
{
  //  call:  array int get_a
  assert( i->OStack.load() > 1 );

  IntegerDatum* idx = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( idx != NULL );
  ArrayDatum* obj = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  assert( obj != NULL );


  if ( ( idx->get() >= 0 ) && ( ( size_t ) idx->get() < obj->size() ) )
  {
    i->EStack.pop();
    Token objT( obj->get( idx->get() ) );
    i->OStack.pop( 2 );
    i->OStack.push_move( objT );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Get_a_aFunction::execute( SLIInterpreter* i ) const
{
  //  call:  array int get_a
  assert( i->OStack.load() > 1 );

  ArrayDatum* idx = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  if ( idx == NULL )
  {
    i->message( SLIInterpreter::M_ERROR,
      "get_a_a",
      "Second argument must be an array of indices." );
    i->message( SLIInterpreter::M_ERROR,
      "get_a_a",
      "Usage: [a] [i1 .. in] get -> [a[i1] ... a[in]]" );
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  ArrayDatum* obj = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  if ( obj == NULL )
  {
    i->message( SLIInterpreter::M_ERROR,
      "get_a_a",
      "Usage: [a] [i1 .. in] get -> [a[i1] ... a[in]]" );
    i->message(
      SLIInterpreter::M_ERROR, "get_a_a", "First argument must be an array." );
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  std::vector< size_t > indices;
  indices.reserve( idx->size() );

  for ( Token* t = idx->begin(); t != idx->end(); ++t )
  {
    IntegerDatum* id = dynamic_cast< IntegerDatum* >( t->datum() );
    if ( id == NULL )
    {
      std::ostringstream sout;

      sout << "Index at position " << ( size_t )( t - idx->begin() )
           << " ignored." << std::ends;
      i->message( SLIInterpreter::M_INFO, "get_a_a", sout.str().c_str() );
      i->message(
        SLIInterpreter::M_INFO, "get_a_a", "Index must be an integer." );
      continue;
    }

    if ( not( ( id->get() >= 0 ) && ( ( size_t ) id->get() < obj->size() ) ) )
    {
      std::ostringstream sout;
      sout << "At position " << ( size_t )( t - idx->begin() ) << "."
           << std::ends;
      i->message( SLIInterpreter::M_ERROR, "get_a_a", sout.str().c_str() );
      i->message( SLIInterpreter::M_ERROR, "get_a_a", "Index out of range." );
      i->raiseerror( i->RangeCheckError );
      return;
    }
    indices.push_back( id->get() );
  }

  TokenArray result;
  result.reserve( idx->size() );

  for ( size_t j = 0; j < indices.size(); ++j )
  {
    result.push_back( obj->get( indices[ j ] ) );
  }

  assert( result.size() == indices.size() );

  i->OStack.pop( 2 );
  i->OStack.push( ArrayDatum( result ) );
  i->EStack.pop();
}

void
Get_pFunction::execute( SLIInterpreter* i ) const
{
  //  call:  array int get_a
  assert( i->OStack.load() > 1 );

  IntegerDatum* idx = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( idx != NULL );
  ProcedureDatum* obj =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 1 ).datum() );
  assert( obj != NULL );


  if ( ( idx->get() >= 0 ) && ( ( size_t ) idx->get() < obj->size() ) )
  {
    i->EStack.pop();
    Token objT( obj->get( idx->get() ) );
    i->OStack.pop( 2 );
    i->OStack.push_move( objT );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Get_lpFunction::execute( SLIInterpreter* i ) const
{
  //  call:  array int get_a
  assert( i->OStack.load() > 1 );

  IntegerDatum* idx = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( idx != NULL );
  LitprocedureDatum* obj =
    dynamic_cast< LitprocedureDatum* >( i->OStack.pick( 1 ).datum() );
  assert( obj != NULL );

  if ( ( idx->get() >= 0 ) && ( ( size_t ) idx->get() < obj->size() ) )
  {
    i->EStack.pop();
    Token objT( obj->get( idx->get() ) );
    i->OStack.pop( 2 );
    i->OStack.push_move( objT );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Append_aFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  ArrayDatum* obj = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  assert( obj != NULL );


  obj->push_back_move( i->OStack.top() );
  i->OStack.pop();
}

void
Append_pFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  ProcedureDatum* obj =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 1 ).datum() );
  assert( obj != NULL );


  obj->push_back_move( i->OStack.top() );
  i->OStack.pop();
}

/** @BeginDocumentation
Name: append - Append an object to a string or array.

Synopsis: (string) int append -> string
          [array] obj append  -> array

Examples: (hello) 44 append -> (hello,)  (44 is ASCII value for ,)
[1 2 3] (hello) append -> [1 2 3 (hello)]
[1 2 3] 44 append -> [1 2 3 44]

Author:  docu by Sirko Straube, Marc-Oliver Gewaltig

SeeAlso: prepend, insert
*/
void
Append_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer append_s string
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( sd != NULL && id != NULL );

  ( *sd ) += static_cast< char >( id->get() );

  i->OStack.pop();
}

/** @BeginDocumentation
Name: join - Join two strings or arrays.
Synopsis:
(string1) (string2) join -> (string1string2)
[array1] [array2] -> [array1 array2]
<< dict1 >> << dict2 >> -> << contents of dict2 assigned to dict1 >>
Examples:
(spike) (train) join -> (spiketrain)
[1 2] [3 4] join -> [1 2 3 4]
/j << /C_m 250.0 /Tau_m 10.0 >> def  j  << /Tau_m 25.0 /I_e 130.0 >> join j
                  -> << /C_m 250.0 /Tau_m 25.0 /I_e 130.0 >>
Author: docu edited by Sirko Straube
SeeAlso: append, getinterval, get

*/
void
Join_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string string join_s string
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* s2 = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );

  if ( s1 == NULL || s2 == NULL )
  {
    i->message(
      SLIInterpreter::M_ERROR, "join_s", "Usage: (string1) (string2) join_s" );
    i->raiseerror( i->ArgumentTypeError );
    return;
  }

  s1->append( *s2 );

  i->OStack.pop();
}

void
Join_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array array join_a array
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  ArrayDatum* a1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  ArrayDatum* a2 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 0 ).datum() );

  assert( a1 != NULL && a2 != NULL );

  a1->append_move( *a2 );

  i->OStack.pop();
}

void
Join_pFunction::execute( SLIInterpreter* i ) const
{
  // call: proc proc join_p proc
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  ProcedureDatum* a1 =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 1 ).datum() );
  ProcedureDatum* a2 =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 0 ).datum() );

  assert( a1 != NULL && a2 != NULL );

  a1->append_move( *a2 );

  i->OStack.pop();
}

/** @BeginDocumentation
Name: insert - Insert all elements of one container in another container.
Synopsis: (string1) n (string2) insert -> (string3)
           Inserts string2 into string1, starting at position n.

          [array1] n [array2] insert-> [array3]
          Inserts all elements of array2 into array1, starting at
          position n
Examples: (spikesimulation) 5 (train) insert -> (spiketrainsimulation)
[20 21 22 24 25 26] 3 [23] insert -> [20 21 22 23 24 25 26]
SeeAlso: join, insertelement, append, prepend
*/

void
Insert_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string index string insert_s string
  assert( i->OStack.load() > 2 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* s2 = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && s2 != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    i->EStack.pop();
    s1->insert( id->get(), *s2 );
    i->OStack.pop( 2 );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
Name: insertelement - insert an element to a container at a specific position
Synopsis: (string1) n c insertelement -> (string2)
           Inserts the character c into string1, starting at position n.

          [array1] n any insertelement -> [array3]
          Inserts element any into array1, starting at
          position n
Examples:(hello) 3 44 insertelement -> (hel,lo)
[1 2 3] 1 (hello) insertelement -> [1 (hello) 2 3]
SeeAlso: join, insert, append, prepend
*/
void
InsertElement_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer integer insertelement_s string
  assert( i->OStack.load() > 2 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* c =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && c != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    i->EStack.pop();
    s1->insert( id->get(), 1, static_cast< char >( c->get() ) );
    i->OStack.pop( 2 );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation:
Name: prepend - Attach an object to the front of an array or string.

Synopsis: (string) int prepend -> string
          [array] any  prepend -> array

Examples: (hello) 44 prepend -> (,hello) (44 is ASCII value for ,)
[1 2 3] (hello) prepend -> [(hello) 1 2 3]
[1 2 3] 44 prepend -> [44 1 2 3]

Author: docu edited by Sirko Straube

SeeAlso: append, insertelement
*/
void
Prepend_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer prepend_s string
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* c =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && c != NULL );

  s1->insert( ( size_t ) 0, 1, static_cast< char >( c->get() ) );

  i->OStack.pop( 1 );
}

void
Insert_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array index array insert_a array
  assert( i->OStack.load() > 2 );

  ArrayDatum* a1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  ArrayDatum* a2 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 0 ).datum() );

  assert( a1 != NULL && id != NULL && a2 != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < a1->size() ) )
  {
    i->EStack.pop();
    a1->insert_move( id->get(), *a2 ); // ArrayDatum is a TokenArray.
    i->OStack.pop( 2 );                // insert_move empties TokenArray *a2
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
InsertElement_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array index any insertelement_a array
  assert( i->OStack.load() > 2 );

  ArrayDatum* a1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );

  assert( a1 != NULL && id != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < a1->size() ) )
  {
    i->EStack.pop();
    a1->insert_move( id->get(), i->OStack.top() );
    i->OStack.pop( 2 );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Prepend_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array any prepend_a array
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  ArrayDatum* a1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );

  assert( a1 != NULL );

  a1->insert_move( 0, i->OStack.top() );

  i->OStack.pop( 1 );
}

void
Prepend_pFunction::execute( SLIInterpreter* i ) const
{
  // call: array any prepend_a array
  i->EStack.pop();

  assert( i->OStack.load() > 1 );

  ProcedureDatum* a1 =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 1 ).datum() );

  assert( a1 != NULL );

  a1->insert_move( 0, i->OStack.top() );

  i->OStack.pop( 1 );
}

/** @BeginDocumentation
Name: replace - Replace a section of a string or array by a new sequence.

Synopsis: (string1) a b (string2) replace -> (string3)
          [array1]  a b [array2]  replace -> [array3]

Description: Replaces the elements a through b in container1 by
             container2.

Examples: [1 2 3 4 5 6 7] 2 3 [99 99 99 99] replace -> [1 2 99 99 99 99 6 7]
(computer) 1 5 (are) replace ->(career)

SeeAlso: ReplaceOccurrences
*/

void
Replace_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer integer string replace_s string
  assert( i->OStack.load() > 3 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 3 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* n =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* s2 = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && n != NULL && s2 != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    if ( n->get() >= 0 )
    {
      i->EStack.pop();
      s1->replace( id->get(), n->get(), *s2 );
      i->OStack.pop( 3 );
    }
    else
    {
      i->raiseerror( i->PositiveIntegerExpectedError );
    }
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Replace_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array integer integer array replace_a array
  assert( i->OStack.load() > 3 );

  ArrayDatum* s1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 3 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* n =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  ArrayDatum* s2 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && n != NULL && s2 != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    if ( n->get() >= 0 )
    {
      i->EStack.pop();
      s1->replace_move( id->get(), n->get(), *s2 );
      i->OStack.pop( 3 );
    }
    else
    {
      i->raiseerror( i->PositiveIntegerExpectedError );
    }
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
Name: erase - Deletes a subsequece of a string or array.
Synopsis: (string1) a n erase -> (string2)
          [array1] a n erase -> [array2]
Parameters:
a - index of the first element to be removed, starting with 0.
n - number of element to be removed.
Description:
Erases n elements from the container, starting with element a.
Examples:
SLI ] [1 2 3 4 5 6] 5 1 erase ==
[1 2 3 4 5]
SeeAlso: getinterval
*/
void
Erase_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer integer erase_s string
  assert( i->OStack.load() > 2 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* n =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && n != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    if ( n->get() >= 0 )
    {
      i->EStack.pop();
      s1->erase( id->get(), n->get() );
      i->OStack.pop( 2 );
    }
    else
    {
      i->raiseerror( i->PositiveIntegerExpectedError );
    }
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Erase_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array integer integer erase_a array
  assert( i->OStack.load() > 2 );

  ArrayDatum* s1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* n =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && n != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    if ( n->get() >= 0 )
    {
      i->EStack.pop();
      s1->erase( id->get(), n->get() );
      i->OStack.pop( 2 );
    }
    else
    {
      i->raiseerror( i->PositiveIntegerExpectedError );
    }
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

void
Erase_pFunction::execute( SLIInterpreter* i ) const
{
  // call: proc integer integer erase_p proc
  assert( i->OStack.load() > 2 );

  ProcedureDatum* s1 =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* n =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && n != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    if ( n->get() >= 0 )
    {
      i->EStack.pop();
      s1->erase( id->get(), n->get() );
      i->OStack.pop( 2 );
    }
    else
    {
      i->raiseerror( i->PositiveIntegerExpectedError );
    }
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}


void
Put_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string index integer put_s string
  assert( i->OStack.load() > 2 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* cd =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && id != NULL && cd != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < s1->size() ) )
  {
    i->EStack.pop();
    ( *s1 )[ id->get() ] = static_cast< char >( cd->get() );
    i->OStack.pop( 2 );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}


void
Put_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array index any put_a array
  assert( i->OStack.load() > 2 );

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );

  assert( ad != NULL && id != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < ad->size() ) )
  {
    i->EStack.pop();
    ad->assign_move(
      id->get(), i->OStack.top() ); // its safe to empty top() because
    i->OStack.pop( 2 );             // it will be poped.
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}


void
Put_pFunction::execute( SLIInterpreter* i ) const
{
  // call: array index any put_a array
  assert( i->OStack.load() > 2 );

  ProcedureDatum* ad =
    dynamic_cast< ProcedureDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );

  assert( ad != NULL && id != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < ad->size() ) )
  {
    i->EStack.pop();
    ad->assign_move(
      id->get(), i->OStack.top() ); // its safe to empty top() because
    i->OStack.pop( 2 );             // it will be poped.
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}


void
Put_lpFunction::execute( SLIInterpreter* i ) const
{
  // call: array index any put_a array
  assert( i->OStack.load() > 2 );

  LitprocedureDatum* ad =
    dynamic_cast< LitprocedureDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );

  assert( ad != NULL && id != NULL );

  if ( ( id->get() >= 0 ) && ( ( size_t ) id->get() < ad->size() ) )
  {
    i->EStack.pop();
    ad->assign_move(
      id->get(), i->OStack.top() ); // its safe to empty top() because
    i->OStack.pop( 2 );             // it will be poped.
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
 Name: length_s - counts elements of a string
 Synopsis: string length -> int

 Examples:
   (Hello world!)      length --> 12
 Author: docu by Sirko Straube
 Remarks: Use length if you are not sure of the data type.
 SeeAlso: length
*/

void
Length_sFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  StringDatum* s = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->length() ) );

  i->OStack.pop();
  i->OStack.push_move( t );
}

/** @BeginDocumentation
 Name: length_a - counts elements of an array
 Synopsis: array length_a -> int

 Examples:
   [1 2 3 4 6 7] length_a --> 6
 Author: docu by Sirko Straube
 Remarks: Use length if you are not sure of the data type.
 SeeAlso: length
*/

void
Length_aFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ArrayDatum* s = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->size() ) );

  i->OStack.pop();
  i->OStack.push_move( t );
}

/** @BeginDocumentation
 Name: length_p - counts elements of a procedure
 Synopsis: procedure length_p -> int

 Examples:
   {mul dup} length_p --> 2
 Author: docu by Sirko Straube
 Remarks: Use length if you are not sure of the data type.
 SeeAlso: length
*/

void
Length_pFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ProcedureDatum* s =
    dynamic_cast< ProcedureDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->size() ) );

  i->OStack.pop();
  i->OStack.push_move( t );
}

/** @BeginDocumentation
 Name: length_lp - counts elements of a literal procedure
 Synopsis: literal procedure length_lp -> int

 Examples: { {1 2 3} } 0 get length_lp -> 3

 Remarks: 0 get is needed to make sure that procedure is still
literal procedure, when it is evaluated. Use length if you are
not sure of the data type.

 Author: docu by Sirko Straube

 SeeAlso: length
*/

void
Length_lpFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  LitprocedureDatum* s =
    dynamic_cast< LitprocedureDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->size() ) );

  i->OStack.pop();
  i->OStack.push_move( t );
}

/** @BeginDocumentation
  Name: capacity - Returns the capacity of an array.
  Synopsis: array capacity -> n
  Description: Returns the number of elements that a given array
  can hold without being re-sized.
  Examples: [1] capacity -> 100
            101 array capacity -> 200
  Remarks: The default size for an array is 100. If it is bigger,
  its size is readjusted to a factor of 100. The number of elements
  in an array is given by length.
  Author: docu by Sirko Straube
  SeeAlso: reserve, shrink, allocations, length, size
*/

void
Capacity_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array capacity_a array integer
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ArrayDatum* s = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->capacity() ) );

  i->OStack.push_move( t );
}

/** @BeginDocumentation
  Name: size - Returns the size of an array/string.
  Synopsis: array size -> n array
  string size -> n string
  Description: Returns the number of elements (similar to length) of
  an object and additionally the object itself.
  Examples: [1 2] size -> 2 [1 2]
  (hello) size -> 5 (hello)
  Author: docu by Sirko Straube
  SeeAlso: reserve, shrink, allocations, length, capacity
*/

void
Size_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array size_a array integer
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ArrayDatum* s = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->size() ) );

  i->OStack.push_move( t );
}

/** @BeginDocumentation
Name: reserve - Prepare an array or string to hold a given number of elements.
Synopsis: array n reserve -> array
Description: reserve makes sure that the array can hold at least n objects.
If the current capacity of the array is lower than n, it will be resized.
Note that if the current capacity is larger than n, no resize will take place.
SeeAlso: capacity, shrink, allocations
*/

void
Reserve_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array integer reserve_a array
  assert( i->OStack.load() > 1 );

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( ad != NULL && id != NULL );
  if ( id->get() >= 0 )
  {
    i->EStack.pop();
    ad->reserve( id->get() );
    i->OStack.pop();
  }
  else
  {
    i->raiseerror( i->PositiveIntegerExpectedError );
  }
}

/** @BeginDocumentation
Name: :resize - Change the internal size of an array.
Synopsis: array n resize -> array.
Description: resize changes the size of the supplied array
independent of the current capacity.
resize is used to free memory by shrinking arrays whose capacity
has grown too large.

If the new size is smaller than the array, the trailing elements
 are lost.
SeeAlso: capacity, reserve, shrink
*/

void
Resize_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array integer resize_a array
  assert( i->OStack.load() > 1 );

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( ad != NULL && id != NULL );
  if ( id->get() >= 0 )
  {
    i->EStack.pop();
    ad->resize( id->get() );
    i->OStack.pop();
  }
  else
  {
    i->raiseerror( i->PositiveIntegerExpectedError );
  }
}

void
Empty_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array empty_a array bool
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );

  assert( ad != NULL );

  if ( ad->empty() )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  }
}

void
References_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array references_a array integer
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );

  assert( ad != NULL );

  Token t( new IntegerDatum( ad->references() ) );

  i->OStack.push_move( t );
}

/** @BeginDocumentation
Name: shrink - Reduce the capacity of an array or string to its minimum.
Synopsis: array shrink -> array bool
Description: Shrink reduces the capacity of an array or string to its minimum.
The boolean return value indicates whether a re-sizing of the array was done.
SeeAlso: capacity, reserve, allocations
*/
void
Shrink_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array shrink_a array bool
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );

  assert( ad != NULL );

  if ( ad->shrink() )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  }
}

void
Capacity_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string capacity_s string integer
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  StringDatum* s = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->capacity() ) );

  i->OStack.push_move( t );
}

void
Size_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string size_a string integer
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  StringDatum* s = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( s != NULL );

  Token t( new IntegerDatum( s->size() ) );

  i->OStack.push_move( t );
}

void
Reserve_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer reserve_a string
  assert( i->OStack.load() > 1 );

  StringDatum* ad = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( ad != NULL && id != NULL );

  if ( id->get() >= 0 )
  {
    i->EStack.pop();
    ad->reserve( id->get() );
    i->OStack.pop();
  }
  else
  {
    i->raiseerror( i->PositiveIntegerExpectedError );
  }
}

void
Resize_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string integer resize_a string
  assert( i->OStack.load() > 1 );

  StringDatum* ad = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );

  assert( ad != NULL && id != NULL );

  if ( id->get() >= 0 )
  {
    i->EStack.pop();
    ad->resize( id->get(), ' ' ); // space as default char
    i->OStack.pop();
  }
  else
  {
    i->raiseerror( i->PositiveIntegerExpectedError );
  }
}


void
Empty_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string empty_a string bool
  i->EStack.pop();
  assert( i->OStack.load() > 0 );

  StringDatum* ad = dynamic_cast< StringDatum* >( i->OStack.top().datum() );

  assert( ad != NULL );

  if ( ad->empty() )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  }
}

/** @BeginDocumentation
Name: getinterval - Return a subsequence of a string or array.
Synopsis: (string1) a b getinterval -> (string2)
[array1]  a b getinterval -> [array2]
Description:
getinterval returns a new container with b elements
starting at element a
Note that getinterval can only handle indices from 0 to N-1
where N is the length of the original array

If other values are given (i.e. indices which do not exist in the array), the
function throws a RangeCheckError

If negative values are given, getinterval throws a PostiveIntegerExpectedError

If b = 0, getinterval returns an empty array

Examples: (spiketrainsimulation) 5 5 getinterval -> train
(spiketrainsimulation) 0 5 getinterval -> spike
[23 24 25 26 27 30] 0 2 getinterval -> [23 24]
[23 24 25 26 27 30] 2 3 getinterval -> [25 26 27]
[23 24 25 26 27 30] 0 6 getinterval -> [Error]: RangeCheck
Author: docu edited by Sirko Straube
SeeAlso: get, put, putinterval, Take
*/

void
Getinterval_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string index count getinterval_s string
  assert( i->OStack.load() > 1 );

  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* cd =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  assert( sd != NULL && id != NULL && cd != NULL );

  if ( cd->get() >= 0 )
  {
    if ( id->get() >= 0 && static_cast< size_t >( id->get() ) < sd->size()
      && static_cast< size_t >( id->get() + cd->get() ) <= sd->size() )
    {
      i->EStack.pop();
      sd->assign( *sd, id->get(), cd->get() );
      i->OStack.pop( 2 );
    }
    else
    {
      i->raiseerror( i->RangeCheckError );
    }
  }
  else
  {
    i->raiseerror( i->PositiveIntegerExpectedError );
  }
}


void
Getinterval_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array index count getinterval_a array
  assert( i->OStack.load() > 1 );

  ArrayDatum* sd = dynamic_cast< ArrayDatum* >( i->OStack.pick( 2 ).datum() );
  IntegerDatum* id =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  IntegerDatum* cd =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 0 ).datum() );
  assert( sd != NULL && id != NULL && cd != NULL );

  if ( cd->get() >= 0 )
  {

    if ( id->get() >= 0 && static_cast< size_t >( id->get() ) < sd->size()
      && static_cast< size_t >( id->get() + cd->get() ) <= sd->size() )
    {
      i->EStack.pop();
      sd->reduce( id->get(), cd->get() );
      i->OStack.pop( 2 );
    }
    else
    {
      i->raiseerror( i->RangeCheckError );
    }
  }
  else
  {
    i->raiseerror( i->PositiveIntegerExpectedError );
  }
}


void
Cvx_aFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();

  assert( i->OStack.load() > 0 );

  ArrayDatum* obj = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new ProcedureDatum( *obj ) );
  t->set_executable();
  i->OStack.top().swap( t );
}

void
Cvlit_nFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  NameDatum* obj = dynamic_cast< NameDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new LiteralDatum( *obj ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}

void
Cvn_lFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  LiteralDatum* obj = dynamic_cast< LiteralDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new NameDatum( *obj ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}

void
Cvn_sFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  StringDatum* obj = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new NameDatum( *obj ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}

void
Cvlit_pFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  ProcedureDatum* obj =
    dynamic_cast< ProcedureDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new ArrayDatum( *obj ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}

//
// { } cvlp /{ }
//
void
Cvlp_pFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  ProcedureDatum* obj =
    dynamic_cast< ProcedureDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new LitprocedureDatum( *obj ) );
  t->set_executable();
  i->OStack.top().swap( t );
  i->EStack.pop();
}

// ---- begin iterator experimental section
void
RangeIterator_aFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  ArrayDatum* a = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( a != NULL );

  const long start = getValue< long >( a->get( 0 ) );
  const long stop = getValue< long >( a->get( 1 ) );
  const long di = getValue< long >( a->get( 2 ) );

  Token t( new IteratorDatum( start, stop, di ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}


void
IteratorSize_iterFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  IteratorDatum* iter =
    dynamic_cast< IteratorDatum* >( i->OStack.top().datum() );
  assert( iter != NULL );

  Token t( new IntegerDatum( iter->size() ) );
  i->OStack.push_move( t );
  i->EStack.pop();
}


// ---- end iterator experimental section


void
Cvi_sFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  StringDatum* obj = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new IntegerDatum( std::atoi( obj->c_str() ) ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}


void
Cvd_sFunction::execute( SLIInterpreter* i ) const
{
  assert( i->OStack.load() > 0 );

  StringDatum* obj = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( obj != NULL );
  Token t( new DoubleDatum( std::atof( obj->c_str() ) ) );
  i->OStack.top().swap( t );
  i->EStack.pop();
}


void
Get_sFunction::execute( SLIInterpreter* i ) const
{
  // call:  string int get_s
  assert( i->OStack.load() > 1 );

  IntegerDatum* idx = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( idx != NULL );

  StringDatum* obj =
    dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  assert( obj != NULL );


  if ( ( idx->get() >= 0 ) && ( ( size_t ) idx->get() < obj->size() ) )
  {
    i->EStack.pop();
    Token objT( new IntegerDatum( ( *obj )[ idx->get() ] ) );
    i->OStack.pop( 2 );
    i->OStack.push_move( objT );
  }
  else
  {
    i->raiseerror( i->RangeCheckError );
  }
}

/** @BeginDocumentation
Name: search - Search for a sequence in an array or string.
Synopsis: (string) (seek) search -> (post) (match) (pre) true
                                 -> (string) false
          [array] [seek]  search -> [post] [match] [pre] true
                                 -> [array] false

Examples: (hamburger) (burg) search -> true (ham) (burg) (er)

SeeAlso: searchif
*/

void
Search_sFunction::execute( SLIInterpreter* i ) const
{
  // call: string seek search_s post match pre true
  //                            string false
  i->EStack.pop();
  assert( i->OStack.load() > 1 );

  StringDatum* s1 = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* s2 = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && s2 != NULL );

  size_t p = s1->find( *s2 );


  if ( p == ULONG_MAX ) // what we realy want is MAX of size_type
  {                     // as soon as C++ limits are supported
    i->OStack.pop();    // (see Stroustrup 3rd ed. p. 586)
    i->OStack.push( i->baselookup( i->false_name ) );
  }
  else
  {
    StringDatum* s3 = new StringDatum();

    size_t n = p; // number of pre elements
    s3->assign( *s1, ( size_t ) 0, n );
    s1->erase( 0, n + s2->size() );

    Token pre( s3 );
    i->OStack.push_move( pre );
    i->OStack.push( i->baselookup( i->true_name ) );
  }
}

void
Search_aFunction::execute( SLIInterpreter* i ) const
{
  // call: array array search_a post match pre true
  //                            array false
  i->EStack.pop();
  assert( i->OStack.load() > 1 );

  ArrayDatum* s1 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 1 ).datum() );
  ArrayDatum* s2 = dynamic_cast< ArrayDatum* >( i->OStack.pick( 0 ).datum() );

  assert( s1 != NULL && s2 != NULL );

  Token* p = std::search( s1->begin(), s1->end(), s2->begin(), s2->end() );


  if ( p == s1->end() )
  {
    i->OStack.pop();
    i->OStack.push( i->baselookup( i->false_name ) );
  }
  else
  {
    ArrayDatum* s3 = new ArrayDatum();

    size_t n = p - s1->begin();              // number of pre elements
    s3->assign_move( *s1, ( size_t ) 0, n ); // _move members may invalidate
    s1->erase( 0, n + s2->size() );          // argument iterators

    Token pre( s3 );
    i->OStack.push_move( pre );
    i->OStack.push( i->baselookup( i->true_name ) );
  }
}

/**********************************************/
/* %repeatany                               */
/*  call: mark  count any  %repeatany       */
/*  pick    3    2     1     0       */
// This version repeats any object n times.
/**********************************************/
void
IrepeatanyFunction::execute( SLIInterpreter* i ) const
{

  IntegerDatum* loopcount =
    static_cast< IntegerDatum* >( i->EStack.pick( 2 ).datum() );

  if ( loopcount->get() > 0 )
  {
    i->EStack.push( i->EStack.pick( 1 ) );
    --( loopcount->get() );
  }
  else
  {
    i->EStack.pop( 4 );
  }
}

/** @BeginDocumentation

Name: repeatany - Place any object n times on stack.

Synopsis: n obj repeatany -> obj obj ... obj (n times)

Examples: 3 (foo) repeatany -> (foo) (foo) (foo)

SeeAlso: repeat, Table
*/
void
RepeatanyFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  // level  1  0
  // stack: n proc repeat
  i->EStack.pop();

  i->EStack.push( i->baselookup( i->mark_name ) );
  i->EStack.push_move( i->OStack.pick( 1 ) );
  i->EStack.push_move( i->OStack.pick( 0 ) );
  i->EStack.push( i->baselookup( Name( "::repeatany" ) ) );

  i->OStack.pop( 2 );
}

const Allocations_aFunction allocations_afunction;
const Get_a_aFunction get_a_afunction;
const Get_aFunction get_afunction;
const Get_pFunction get_pfunction;
const Get_lpFunction get_lpfunction;

const Append_aFunction append_afunction;
const Append_pFunction append_pfunction;
const Append_sFunction append_sfunction;
const Prepend_aFunction prepend_afunction;
const Prepend_pFunction prepend_pfunction;
const Prepend_sFunction prepend_sfunction;
const Join_sFunction join_sfunction;
const Join_aFunction join_afunction;
const Join_pFunction join_pfunction;
const Insert_sFunction insert_sfunction;
const Insert_aFunction insert_afunction;
const InsertElement_aFunction insertelement_afunction;
const InsertElement_sFunction insertelement_sfunction;
const Replace_sFunction replace_sfunction;
const Replace_aFunction replace_afunction;
const Erase_sFunction erase_sfunction;
const Erase_aFunction erase_afunction;
const Erase_pFunction erase_pfunction;

const Length_sFunction length_sfunction;
const Length_aFunction length_afunction;
const Length_lpFunction length_lpfunction;
const Length_pFunction length_pfunction;

const Getinterval_sFunction getinterval_sfunction;
const Getinterval_aFunction getinterval_afunction;

const Cvx_aFunction cvx_afunction;
const Cvlit_nFunction cvlit_nfunction;
const Cvlit_pFunction cvlit_pfunction;
const Cvlp_pFunction cvlp_pfunction;
const RangeIterator_aFunction rangeiterator_afunction;
const IteratorSize_iterFunction iteratorsize_iterfunction;
const Cvn_lFunction cvn_lfunction;
const Cvn_sFunction cvn_sfunction;
const Cvi_sFunction cvi_sfunction;
const Cvd_sFunction cvd_sfunction;

const Get_sFunction get_sfunction;
const Put_sFunction put_sfunction;
const Put_aFunction put_afunction;
const Put_pFunction put_pfunction;
const Put_lpFunction put_lpfunction;

const Search_sFunction search_sfunction;
const Search_aFunction search_afunction;

const Capacity_aFunction capacity_afunction;
const Size_aFunction size_afunction;
const Reserve_aFunction reserve_afunction;
const Resize_aFunction resize_afunction;
const Empty_aFunction empty_afunction;
const References_aFunction references_afunction;
const Shrink_aFunction shrink_afunction;

const Capacity_sFunction capacity_sfunction;
const Size_sFunction size_sfunction;
const Reserve_sFunction reserve_sfunction;
const Resize_sFunction resize_sfunction;
const Empty_sFunction empty_sfunction;
const IrepeatanyFunction irepeatanyfunction;
const RepeatanyFunction repeatanyfunction;

void
init_slidata( SLIInterpreter* i )
{
  i->createcommand( "allocations", &allocations_afunction );
  i->createcommand( "get_s", &get_sfunction );
  i->createcommand( "get_a", &get_afunction );
  i->createcommand( "get_a_a", &get_a_afunction );
  i->createcommand( "get_p", &get_pfunction );
  i->createcommand( "get_lp", &get_lpfunction );
  i->createcommand( "append_a", &append_afunction );
  i->createcommand( "append_p", &append_pfunction );
  i->createcommand( "append_s", &append_sfunction );
  i->createcommand( "prepend_a", &prepend_afunction );
  i->createcommand( "prepend_p", &prepend_pfunction );
  i->createcommand( "prepend_s", &prepend_sfunction );
  i->createcommand( "join_s", &join_sfunction );
  i->createcommand( "join_a", &join_afunction );
  i->createcommand( "join_p", &join_pfunction );
  i->createcommand( "insert_s", &insert_sfunction );
  i->createcommand( "insert_a", &insert_afunction );
  i->createcommand( "insertelement_s", &insertelement_sfunction );
  i->createcommand( "insertelement_a", &insertelement_afunction );
  i->createcommand( "replace_s", &replace_sfunction );
  i->createcommand( "replace_a", &replace_afunction );
  i->createcommand( "erase_s", &erase_sfunction );
  i->createcommand( "erase_a", &erase_afunction );
  i->createcommand( "erase_p", &erase_pfunction );

  i->createcommand( "length_s", &length_sfunction );
  i->createcommand( "length_a", &length_afunction );
  i->createcommand( "length_p", &length_pfunction );
  i->createcommand( "length_lp", &length_lpfunction );
  i->createcommand( "getinterval_s", &getinterval_sfunction );
  i->createcommand( "getinterval_a", &getinterval_afunction );
  i->createcommand( "cvx_a", &cvx_afunction );
  i->createcommand( "cvlit_n", &cvlit_nfunction );
  i->createcommand( "cvlit_p", &cvlit_pfunction );
  i->createcommand( "cvlp_p", &cvlp_pfunction );
  i->createcommand( "RangeIterator_a", &rangeiterator_afunction );
  i->createcommand( "size_iter", &iteratorsize_iterfunction );
  i->createcommand( "cvn_l", &cvn_lfunction );
  i->createcommand( "cvn_s", &cvn_sfunction );
  i->createcommand( "cvi_s", &cvi_sfunction );
  i->createcommand( "cvd_s", &cvd_sfunction );
  i->createcommand( "put_s", &put_sfunction );
  i->createcommand( "put_a", &put_afunction );
  i->createcommand( "put_p", &put_pfunction );
  i->createcommand( "put_lp", &put_lpfunction );

  i->createcommand( "search_s", &search_sfunction );
  i->createcommand( "search_a", &search_afunction );

  i->createcommand( "capacity_a", &capacity_afunction );
  i->createcommand( "size_a", &size_afunction );
  i->createcommand( "reserve_a", &reserve_afunction );
  i->createcommand( ":resize_a", &resize_afunction );
  i->createcommand( "empty_a", &empty_afunction );
  i->createcommand( "references_a", &references_afunction );
  i->createcommand( "shrink_a", &shrink_afunction );

  i->createcommand( "capacity_s", &capacity_sfunction );
  i->createcommand( "size_s", &size_sfunction );
  i->createcommand( "reserve_s", &reserve_sfunction );
  i->createcommand( ":resize_s", &resize_sfunction );
  i->createcommand( "empty_s", &empty_sfunction );
  i->createcommand( "::repeatany", &irepeatanyfunction );
  i->createcommand( "repeatany", &repeatanyfunction );
}
