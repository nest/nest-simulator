/*
 *  slistack.cc
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
    slistack.cc
*/

#include "slistack.h"

// C++ includes:
#include <typeinfo>

// Includes from sli:
#include "arraydatum.h"
#include "integerdatum.h"

//******************* Stack Functions
/** @BeginDocumentation
Name: pop - Pop the top object off the stack

Description: Alternatives: You can use ; (undocumented),
which is the same as pop.

Diagnostics: Raises StackUnderflow error if the stack is empty

SeeAlso: npop
 */

void
PopFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }
  i->EStack.pop();
  i->OStack.pop();
}

/** @BeginDocumentation
Name: npop - Pop n object off the stack
Synopsis: obj_k ... obj_n+1 ojb_n ... obj_0 n pop -> obj_k ... obj_n
Diagnostics: Raises StackUnderflow error if the stack contains less
 than n elements.
SeeAlso: pop
 */

void
NpopFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( id != NULL );
  size_t n = id->get();
  if ( n < i->OStack.load() )
  {
    i->EStack.pop();
    i->OStack.pop( n + 1 ); // pop one more and also remove the argument
  }
  else
  {
    i->raiseerror( i->StackUnderflowError );
  }
}

/** @BeginDocumentation
Name: dup - Duplicate the object which is on top of the stack
Synopsis: any dup -> any any
Diagnostics: Raises StackUnderflow error if the stack is empty.
Examples: 2 dup -> 2 2
(hello) dup -> (hello) (hello)
Author: docu edited by Sirko Straube
SeeAlso: over, index, copy
*/
void
DupFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }
  i->EStack.pop();
  i->OStack.index( 0 );
}

/** @BeginDocumentation
Name: over - Copy object at stack level 1
Synopsis: any obj over -> any obj any
Diagnostics: Raises StackUnderflow error if there are less than two objects on
  the stack.
Examples: 1 2 3 over -> 2
1 2 3 4 5 over -> 4
SeeAlso: dup, index, copy
*/
void
OverFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() < 2 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }
  i->EStack.pop();
  i->OStack.index( 1 );
}

/** @BeginDocumentation
Name: exch - Exchange the order of the first two stack objects.
Synopsis: obj1 obj2 exch -> obj2 obj1
Diagnostics: Raises StackUnderflow error if there are less than two objects on
  the stack.
SeeAlso: roll, rollu, rolld, rot
*/

void
ExchFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() < 2 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }
  i->EStack.pop();
  i->OStack.swap();
}

/** @BeginDocumentation
Name: index - Copy object at stack level n
Synopsis: ... obj_n ... obj0 n index -> ... obj_n ... obj0 obj_n
Diagnostics: Raises StackUnderflow error if there are less than n+1 objects on
  the stack.
SeeAlso: dup, over, copy
*/
void
IndexFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }
  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( id != NULL );
  size_t pos = id->get();

  if ( pos + 1 < i->OStack.load() )
  {
    i->EStack.pop();
    i->OStack.pop();
    i->OStack.index( pos );
  }
  else
  {
    i->raiseerror( i->StackUnderflowError );
  }
}

/** @BeginDocumentation
Name: copy - Copy the top n stack objects
Synopsis: ... obj_n ... obj1 n copy -> ... obj_n ... obj1 obj_n ... obj1
Examples: 1 2 3 4 2 copy
-> after this execution 1 2 3 4 3 4 lies on the stack (the last two elements
were copied).
Diagnostics: Raises StackUnderflow error if there are less than n+1 objects on
  the stack.
Author: docu edited by Sirko Straube
SeeAlso: dup, over, index
*/
void
CopyFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }
  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  assert( id != NULL );
  size_t n = id->get();
  if ( n < i->OStack.load() )
  {
    i->EStack.pop();
    i->OStack.pop();
    for ( size_t p = 0; p < n; ++p )
    {
      //  Since the stack is growing, the argument to index is constant.
      i->OStack.index( n - 1 );
    }
  }
  else
  {
    i->raiseerror( i->StackUnderflowError );
  }
}

/** @BeginDocumentation
Name: roll - Roll a portion n stack levels k times
Synopsis: objn ... obj1 n k roll
Description:
roll performs a circular shift of the first n stack levels
by k positions. Before this is done, roll removes its arguments
from the stack.

If k is positive, each shift consists of moving the contents of level
0 to level n-1, thereby moving elements at levels 1 through n-1 up one
stack level.

If k is negative, each shift consists of moving the contents of level
n-1 to level 0, thereby moving elements at levels 1 through n-1 down
one stack level.

Examples:
    (a) (b) (c) 3 1  roll -> (c) (a) (b)
    (a) (b) (c) 3 -1 roll -> (b) (c) (a)
    (a) (b) (c) 3 0  roll -> (a) (b) (c)
Diagnostics: Raises StackUnderflow error if there are less than n+2 objects
on the stack.
SeeAlso: exch, rollu, rolld, rot
*/
void
RollFunction::execute( SLIInterpreter* i ) const
{
  const size_t load = i->OStack.load();
  if ( load < 2 )
  {
    throw StackUnderflow( 2, load );
  }

  IntegerDatum* idn =
    dynamic_cast< IntegerDatum* >( i->OStack.pick( 1 ).datum() );
  if ( idn == NULL )
  {
    throw ArgumentType( 1 );
  }

  IntegerDatum* idk = dynamic_cast< IntegerDatum* >( i->OStack.top().datum() );
  if ( idk == NULL )
  {
    throw ArgumentType( 0 );
  }

  long& n = idn->get();
  if ( n < 0 )
  {
    i->raiseerror( i->RangeCheckError );
    return;
  }
  if ( static_cast< size_t >( n + 2 ) > load )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  i->EStack.pop();
  i->OStack.pop( 2 );

  i->OStack.roll( n, idk->get() );
}

/** @BeginDocumentation
Name: rollu - Roll the three top stack elements upwards
Synopsis: obj1 obj2 obj3 rollu -> obj3 obj1 obj2
Description: rollu is equivalent to 3 1 roll
Diagnostics: Raises StackUnderflow error if there are less
             than 3 objects on the stack.
SeeAlso: roll, rolld, rot
*/
void
RolluFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() < 3 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  i->EStack.pop();

  i->OStack.roll( 3, 1 );
}

/** @BeginDocumentation
Name: rolld - Roll the three top stack elements downwards
Synopsis: obj1 obj2 obj3 rolld -> obj2 obj3 obj1
Description: rolld is equivalent to 3 -1 roll
Diagnostics: Raises StackUnderflow error if there are less
             than 3 objects on the stack.
SeeAlso: roll, rollu, rot
*/
void
RolldFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() < 3 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  i->EStack.pop();

  i->OStack.roll( 3, -1 );
}

/** @BeginDocumentation
Name: rot - Rotate entire stack contents
Synopsis: obj_n ... obj1 obj0 rot -> obj0 obj_n ... obj1
SeeAlso: roll, rollu, rolld
*/

void
RotFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();

  i->OStack.roll( i->OStack.load(), 1 );
}

/** @BeginDocumentation
Name: count - Count the number of objects on the stack.
Synopsis: obj_n-1 ... obj0 count -> obj_n-1 ... obj0 n
*/
void
CountFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  Token load( new IntegerDatum( i->OStack.load() ) );

  i->OStack.push_move( load );
}

/** @BeginDocumentation
Name: clear - Clear the entire stack.
SeeAlso: pop, npop
*/
void
ClearFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();
  i->OStack.clear();
}

/** @BeginDocumentation
Name: execstack - Return the contents of the execution stack as array.
Synopsis: - execstack -> array
Description: execstack converts the current contents of the execution stack
into an array. The first array element corrensponds to the bottom and the
last array element to the top of the execution stack.
SeeAlso: restoreestack, operandstack
*/
void
ExecstackFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();

  Token st( new ArrayDatum( i->EStack.toArray() ) );
  i->OStack.push_move( st );
}

/** @BeginDocumentation
Name: restoreestack - Restore the execution stack from an array.
Synopsis: array restoreexecstack -> -

Description: restoreexecstack is used to restore the execution stack
from an array. Most probably this array was obtained by saving a previous
state of the execution stack with execstack.

Be very careful with this command, as it can easily damage or terminate
the SLI interpreter.
Diagnostics: ArgumentType, StackUnderflow
SeeAlso: execstack
*/
void
RestoreestackFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( ad != NULL );
  TokenArray ta = *ad;
  i->OStack.pop();
  i->EStack = ta;
}

/** @BeginDocumentation
Name: restoreostack - Restore the stack from an array.
Synopsis: [any0 ... any_n] restoreexecstack -> any0 ... any_n

Description: restoroexecstack is used to replace the contents of
the stack with the contents of the supplied array. The first element
of the array will become the bottom of the stack and the last element
of the array will become the top of the stack.

Diagnostics: ArgumentType, StackUnderflow
SeeAlso: operandstack, arrayload, arraystore
*/
void
RestoreostackFunction::execute( SLIInterpreter* i ) const
{
  if ( i->OStack.load() == 0 )
  {
    i->raiseerror( i->StackUnderflowError );
    return;
  }

  i->EStack.pop();

  ArrayDatum* ad = dynamic_cast< ArrayDatum* >( i->OStack.top().datum() );
  assert( ad != NULL );
  TokenArray ta = *ad;
  i->OStack = ta;
}

/** @BeginDocumentation
Name: operandstack - Return the contents of the stack as array.
Synopsis: anyn ... any0 operandstack -> [anyn ... any0]
SeeAlso: restoreostack, arrayload, arraystore
*/
void
OperandstackFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop();

  Token st( new ArrayDatum( i->OStack.toArray() ) );
  i->OStack.push_move( st );
}

const PopFunction popfunction;
const NpopFunction npopfunction;
const ExchFunction exchfunction;
const DupFunction dupfunction;
const IndexFunction indexfunction;
const CopyFunction copyfunction;
const RollFunction rollfunction;
const CountFunction countfunction;
const ClearFunction clearfunction;

const RotFunction rotfunction;
const RolluFunction rollufunction;
const RolldFunction rolldfunction;
const OverFunction overfunction;

const ExecstackFunction execstackfunction;
const RestoreestackFunction restoreestackfunction;
const RestoreostackFunction restoreostackfunction;
const OperandstackFunction operandstackfunction;

void
init_slistack( SLIInterpreter* i )
{
  // Stack routines
  i->createcommand( "pop", &popfunction );
  i->createcommand( "npop", &npopfunction );
  i->createcommand( ";", &popfunction );
  i->createcommand( "dup", &dupfunction );
  i->createcommand( "exch", &exchfunction );
  i->createcommand( "index", &indexfunction );
  i->createcommand( "copy", &copyfunction );
  i->createcommand( "roll", &rollfunction );
  i->createcommand( "count", &countfunction );
  i->createcommand( "clear", &clearfunction );

  i->createcommand( "rollu", &rollufunction );
  i->createcommand( "rolld", &rolldfunction );
  i->createcommand( "rot", &rotfunction );
  i->createcommand( "over", &overfunction );

  i->createcommand( "execstack", &execstackfunction );
  i->createcommand( "restoreestack", &restoreestackfunction );
  i->createcommand( "restoreostack", &restoreostackfunction );
  i->createcommand( "operandstack", &operandstackfunction );
}
