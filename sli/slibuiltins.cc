/*
 *  slibuiltins.cc
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
    Interpreter builtins
*/

#include "slibuiltins.h"

// Includes from sli:
#include "arraydatum.h"
#include "callbackdatum.h"
#include "functiondatum.h"
#include "integerdatum.h"
#include "interpret.h"
#include "stringdatum.h"

void
IlookupFunction::execute( SLIInterpreter* i ) const
{
  i->EStack.pop( 2 );
}

void
IsetcallbackFunction::execute( SLIInterpreter* i ) const
{
  // move the hopefully present callback action
  // into the interpreters callback token.
  i->EStack.pop();
  assert( dynamic_cast< CallbackDatum* >( i->EStack.top().datum() ) != NULL );
  i->EStack.pop_move( i->ct );
}

void
IiterateFunction::backtrace( SLIInterpreter* i, int p ) const
{
  ProcedureDatum const* pd = dynamic_cast< ProcedureDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( pd != NULL );

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->EStack.pick( p + 1 ).datum() );
  assert( id != NULL );

  std::cerr << "In procedure:" << std::endl;

  pd->list( std::cerr, "   ", id->get() - 1 );
  std::cerr << std::endl;
}

void
IiterateFunction::execute( SLIInterpreter* i ) const
{
  /*
     This function is responsible for executing a procedure
     object. Iiterate expects the procedure to execute as first
     and the iteration counter as second argument.

     Like in all internal function, no error checking is done.

     */

  /* Stack Layout:
        3       2       1
     <proc>  <pos>   %iterate
  */

  ProcedureDatum const* pd = static_cast< ProcedureDatum* >( i->EStack.pick( 2 ).datum() );
  long& pos = static_cast< IntegerDatum* >( i->EStack.pick( 1 ).datum() )->get();

  while ( pd->index_is_valid( pos ) )
  {
    const Token& t = pd->get( pos );
    ++pos;

    i->code_executed++; // code coverage

    if ( t->is_executable() )
    {
      i->EStack.push( t );
      return;
    }
    i->OStack.push( t );
  }

  i->EStack.pop( 3 );
  i->dec_call_depth();
}

void
IloopFunction::execute( SLIInterpreter* i ) const
{
  // stack: mark procedure n   %loop
  // level:  4      3      2     1

  ProcedureDatum const* proc = static_cast< ProcedureDatum* >( i->EStack.pick( 2 ).datum() );
  long& pos = static_cast< IntegerDatum* >( i->EStack.pick( 1 ).datum() )->get();

  while ( proc->index_is_valid( pos ) )
  {
    const Token& t( proc->get( pos ) );
    ++pos;
    if ( t->is_executable() )
    {
      i->EStack.push( t );
      return;
    }

    i->OStack.push( t );
  }

  pos = 0;
}

void
IloopFunction::backtrace( SLIInterpreter* i, int p ) const
{
  ProcedureDatum const* pd = dynamic_cast< ProcedureDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( pd != NULL );

  IntegerDatum* id = dynamic_cast< IntegerDatum* >( i->EStack.pick( p + 1 ).datum() );
  assert( id != NULL );

  std::cerr << "During loop:" << std::endl;

  pd->list( std::cerr, "   ", id->get() - 1 );
  std::cerr << std::endl;
}


/**********************************************/
/* %repeat                                    */
/*  call: mark  count proc  n %repeat         */
/*  pick   5      4     3   2    1            */
/**********************************************/
void
IrepeatFunction::execute( SLIInterpreter* i ) const
{
  ProcedureDatum* proc = static_cast< ProcedureDatum* >( i->EStack.pick( 2 ).datum() );

  long& pos = static_cast< IntegerDatum* >( i->EStack.pick( 1 ).datum() )->get();
  while ( proc->index_is_valid( pos ) )
  {
    const Token& t = proc->get( pos );
    ++pos;
    if ( t->is_executable() )
    {
      i->EStack.push( t );
      return;
    }
    i->OStack.push( t );
  }

  long& lc = static_cast< IntegerDatum* >( i->EStack.pick( 3 ).datum() )->get();
  if ( lc > 0 )
  {
    pos = 0; // reset procedure iterator
    --lc;
  }
  else
  {
    i->EStack.pop( 5 );
    i->dec_call_depth();
  }
}

void
IrepeatFunction::backtrace( SLIInterpreter* i, int p ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( p + 3 ).datum() );
  assert( count != NULL );

  ProcedureDatum const* pd = static_cast< ProcedureDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( pd != NULL );

  IntegerDatum* id = static_cast< IntegerDatum* >( i->EStack.pick( p + 1 ).datum() );
  assert( id != NULL );

  std::cerr << "During repeat with " << count->get() << " iterations remaining." << std::endl;

  pd->list( std::cerr, "   ", id->get() - 1 );
  std::cerr << std::endl;
}

/*****************************************************/
/* %for                                              */
/*  call: mark incr limit count proc  n  %for        */
/*  pick   6     5    4     3    2    1    0         */
/*****************************************************/
void
IforFunction::execute( SLIInterpreter* i ) const
{

  IntegerDatum* proccount = static_cast< IntegerDatum* >( i->EStack.pick( 1 ).datum() );

  ProcedureDatum const* proc = static_cast< ProcedureDatum* >( i->EStack.pick( 2 ).datum() );

  long& pos = proccount->get();

  while ( proc->index_is_valid( pos ) )
  {
    const Token& t = proc->get( pos );
    ++pos;
    if ( t->is_executable() )
    {
      i->EStack.push( t );
      return;
    }
    i->OStack.push( t );
  }

  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( 3 ).datum() );

  IntegerDatum* lim = static_cast< IntegerDatum* >( i->EStack.pick( 4 ).datum() );

  IntegerDatum* inc = static_cast< IntegerDatum* >( i->EStack.pick( 5 ).datum() );


  if ( ( ( inc->get() > 0 ) && ( count->get() <= lim->get() ) )
    || ( ( inc->get() < 0 ) && ( count->get() >= lim->get() ) ) )
  {
    pos = 0; // reset procedure interator

    i->OStack.push( i->EStack.pick( 3 ) ); // push counter to user
    ( count->get() ) += ( inc->get() );    // increment loop counter
  }
  else
  {
    i->EStack.pop( 7 );
    i->dec_call_depth();
  }
}

void
IforFunction::backtrace( SLIInterpreter* i, int p ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( p + 3 ).datum() );
  assert( count != NULL );
  ProcedureDatum const* pd = static_cast< ProcedureDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( pd != NULL );
  IntegerDatum* id = static_cast< IntegerDatum* >( i->EStack.pick( p + 1 ).datum() );
  assert( id != NULL );

  std::cerr << "During for at iterator value " << count->get() << "." << std::endl;

  pd->list( std::cerr, "   ", id->get() - 1 );
  std::cerr << std::endl;
}

/*********************************************************/
/* %forallarray                                          */
/*  call: mark object count proc n %forallarray      */
/*  pick    5     4    3     2    1    0               */
/*********************************************************/
void
IforallarrayFunction::execute( SLIInterpreter* i ) const
{

  IntegerDatum* proccount = static_cast< IntegerDatum* >( i->EStack.pick( 1 ).datum() );

  ProcedureDatum const* proc = static_cast< ProcedureDatum* >( i->EStack.pick( 2 ).datum() );

  long& pos = proccount->get();

  while ( proc->index_is_valid( pos ) )
  {
    const Token& t = proc->get( pos );
    ++pos;
    if ( t->is_executable() )
    {
      i->EStack.push( t );
      return;
    }
    i->OStack.push( t );
  }

  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( 3 ).datum() );
  ArrayDatum* ad = static_cast< ArrayDatum* >( i->EStack.pick( 4 ).datum() );

  long& idx = count->get();

  if ( ad->index_is_valid( idx ) )
  {
    pos = 0; // reset procedure interator

    i->OStack.push( ad->get( idx ) ); // push counter to user
    ++idx;
  }
  else
  {
    i->EStack.pop( 6 );
    i->dec_call_depth();
  }
}


void
IforallarrayFunction::backtrace( SLIInterpreter* i, int p ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( p + 3 ).datum() );
  assert( count != NULL );

  std::cerr << "During forall (array) at iteration " << count->get() << "." << std::endl;
}


/*********************************************************/
/* %forallindexedarray                                   */
/*  call: mark object limit count proc forallindexedarray  */
/*  pick   5      4    3     2    1      0         */
/*********************************************************/
void
IforallindexedarrayFunction::execute( SLIInterpreter* i ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( 2 ).datum() );
  IntegerDatum* limit = static_cast< IntegerDatum* >( i->EStack.pick( 3 ).datum() );

  long& cnt = count->get();
  if ( cnt < limit->get() )
  {
    ArrayDatum* obj = static_cast< ArrayDatum* >( i->EStack.pick( 4 ).datum() );

    i->OStack.push( obj->get( cnt ) );                    // push element to user
    i->OStack.push_by_pointer( new IntegerDatum( cnt ) ); // push index to user
    ++cnt;
    i->EStack.push( i->EStack.pick( 1 ) );
  }
  else
  {
    i->EStack.pop( 6 );
    i->dec_call_depth();
  }
}

void
IforallindexedarrayFunction::backtrace( SLIInterpreter* i, int p ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( count != NULL );

  std::cerr << "During forallindexed (array) at iteration " << count->get() - 1 << "." << std::endl;
}

void
IforallindexedstringFunction::backtrace( SLIInterpreter* i, int p ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( count != NULL );

  std::cerr << "During forallindexed (string) at iteration " << count->get() - 1 << "." << std::endl;
}

/*********************************************************/
/* %forallindexedarray                                   */
/*  call: mark object limit count proc forallindexedarray  */
/*  pick   5      4    3     2    1      0         */
/*********************************************************/
void
IforallindexedstringFunction::execute( SLIInterpreter* i ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( 2 ).datum() );
  IntegerDatum* limit = static_cast< IntegerDatum* >( i->EStack.pick( 3 ).datum() );

  if ( count->get() < limit->get() )
  {
    StringDatum const* obj = static_cast< StringDatum* >( i->EStack.pick( 4 ).datum() );

    i->OStack.push( ( *obj )[ count->get() ] ); // push element to user
    i->OStack.push( count->get() );             // push index to user
    ++( count->get() );
    i->EStack.push( i->EStack.pick( 1 ) );
    if ( i->step_mode() )
    {
      std::cerr << "forallindexed:"
                << " Limit: " << limit->get() << " Pos: " << count->get() << " Iterator: ";
      i->OStack.pick( 1 ).pprint( std::cerr );
      std::cerr << std::endl;
    }
  }
  else
  {
    i->EStack.pop( 6 );
    i->dec_call_depth();
  }
}

void
IforallstringFunction::backtrace( SLIInterpreter* i, int p ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( p + 2 ).datum() );
  assert( count != NULL );

  std::cerr << "During forall (string) at iteration " << count->get() - 1 << "." << std::endl;
}
/*********************************************************/
/* %forallstring                                         */
/*  call: mark object limit count proc %forallarray  */
/*  pick   5      4    3     2    1      0         */
/*********************************************************/
void
IforallstringFunction::execute( SLIInterpreter* i ) const
{
  IntegerDatum* count = static_cast< IntegerDatum* >( i->EStack.pick( 2 ).datum() );
  IntegerDatum* limit = static_cast< IntegerDatum* >( i->EStack.pick( 3 ).datum() );

  if ( count->get() < limit->get() )
  {
    StringDatum const* obj = static_cast< StringDatum* >( i->EStack.pick( 4 ).datum() );
    i->OStack.push_by_pointer( new IntegerDatum( ( *obj )[ count->get() ] ) ); // push element to user
    ++( count->get() );
    i->EStack.push( i->EStack.pick( 1 ) );
    if ( i->step_mode() )
    {
      std::cerr << "forall:"
                << " Limit: " << limit->get() << " Pos: " << count->get() << " Iterator: ";
      i->OStack.top().pprint( std::cerr );
      std::cerr << std::endl;
    }
  }
  else
  {
    i->EStack.pop( 6 );
    i->dec_call_depth();
  }
}
