/*
 *  sliactions.cc
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
    Actions associated with SLI types.
*/
#include "sliactions.h"
#include "interpret.h"
#include "namedatum.h"
#include "arraydatum.h"
#include "functiondatum.h"
#include "callbackdatum.h"
#include "integerdatum.h"
#include "dictstack.h"
#include "iostreamdatum.h"
#include "triedatum.h"

#define SLIDEBUG 1
#undef SLIDEBUG
// DatatypeFunction: For all 'ordinary' data objects which end up on
// the operand stack
void DatatypeFunction::execute(SLIInterpreter *i) const
{
    i->OStack.push_move(i->EStack.top());
    i->EStack.pop();
}

void NametypeFunction::execute(SLIInterpreter *i) const
{
  
    NameDatum *nd= static_cast<NameDatum *>(i->EStack.top().datum());
//    assert(nd != NULL);
    
    Token contents(i->lookup(*nd));
    if(contents.datum()!=NULL)
    {
//#ifdef SLIDEBUG
//        i->EStack.push(i->Ilookup());
//#else
//        i->EStack.pop();
//#endif
//        i->EStack.push_move(contents);
	i->EStack.top().swap(contents);
    }
    else
        i->raiseerror(*nd, i->UndefinedNameError);    
}

void ProceduretypeFunction::execute(SLIInterpreter *i) const
{
    i->EStack.push(new IntegerDatum(0));
    i->EStack.push(i->Iiterate());
    i->inc_call_depth();
}

void LitproceduretypeFunction::execute(SLIInterpreter *i) const
{
        // Literal procedures are procedures which come straight from the
        // parser. In this state they must not be executed, but must instead be
        // moved to the operand stack. After this, the literal procedure becomes
        // an executable procedure and will be treated as such.
    
    LitprocedureDatum
        *lpd = static_cast<LitprocedureDatum *>(i->EStack.top().datum());
//    assert(lpd != NULL);

    i->OStack.push(new ProcedureDatum(*lpd));              //
    i->EStack.pop();                   // pop empty token
}

void FunctiontypeFunction::execute(SLIInterpreter *i) const
{
        // The Function is left on the estack
    
    FunctionDatum
        *fd = static_cast<FunctionDatum *>(i->EStack.top().datum());
//    assert(fd != NULL);
    if(i->step_mode())
    {
      std::cerr << "Calling builtin function: ";
      if(fd !=NULL)
      {
	fd->pprint(std::cerr);
      }
      else
      {
	std::cerr << "NULL" << std::endl;
	i->EStack.pop();
	return;
      }
    }

    fd->execute(i);  
}

void TrietypeFunction::execute(SLIInterpreter *i) const
{
    
    TrieDatum
      *tried = static_cast<TrieDatum *>(i->EStack.top().datum());
    Token tmp;
    tmp=tried->lookup(i->OStack);
    
    // This call must be outside the catch clause,
    // since it returns to the normal control flow 
    // for which we don't want to handle errors.
    i->EStack.top().swap(tmp);
    i->EStack.top()->execute(i);

    return;
}

void CallbacktypeFunction::execute(SLIInterpreter *i) const
{
//    assert(i->ct.datum() != NULL); // we wouldn't be here otherwise
    
    CallbackDatum *cb= static_cast<CallbackDatum *>(i->ct.datum());
    
    // Note, although cb is a pointer to a class derived from Datum,
    // it also has the properties of a token, since it is derived from both.
    

    i->EStack.push_move(i->ct);
    // This moves the complete callback datum to the EStack.
    // Now, the pointer in ct is set to NULL !!
    
    // Now push command to restore the callback, once the action has
    // been finished
    i->EStack.push(i->baselookup(i->isetcallback_name));
    i->EStack.push(*(cb->get()));
}

void XIstreamtypeFunction::execute(SLIInterpreter *i) const
{
        // The EStack contains an open ifstream object,
        // which can be executed by calling
        // ::parse
    i->EStack.push(i->baselookup(i->iparse_name));
}

void XIfstreamtypeFunction::execute(SLIInterpreter *i) const
{
        // The EStack contains an open ifstream object,
        // which can be executed by calling
        // ::parse
    i->EStack.push(i->baselookup(i->iparse_name));
}


