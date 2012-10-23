/*
 *  dictstack.cc
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

#include "dictstack.h"

DictionaryStack::DictionaryStack(const Token &t )
        : VoidToken(t)
{ 
}

DictionaryStack::DictionaryStack(const DictionaryStack &ds )
  : VoidToken(ds.VoidToken), d(ds.d)
{ 
}

DictionaryStack::~DictionaryStack()
{
  // We have to clear the dictionary before we delete it, otherwise the
  // dictionary references will prevent proper deletion.
  for(std::list<DictionaryDatum>::iterator i=d.begin(); i != d.end(); ++i)
    (*i)->clear();
}

/*
const Token &  DictionaryStack::baselookup(const Name &n) const
{
        // just lookup in the bottom (base-) dictionary
        // This method expects the base-dictionary to be
        // present.
    
//    assert(d.end()!=d.begin());
        
    list<DictionaryDatum>::const_iterator i(--d.end());
    return (*i)->lookup(n);
}
*/
/*
const Token &  DictionaryStack::lookup(const Name &n) const
{
  //
  // search from top to bottom for a definition of n
  // return VoidToken if no definition can be found.
  // Note: need const_iterator here because lookup is
  //       declared const.
  // a refernce can be returned because we return a 
  // refernce to a member of the class (not of the lookup
  // function) which is destroyed late enough. 

  list<DictionaryDatum>::const_iterator i(d.begin());

  while (i!=d.end() && (*i)->lookup(n)==VoidToken)
   ++i;
 
  return (i==d.end()) ? VoidToken : (*i)->lookup(n);
}
*/

void DictionaryStack::def(const Name &n, const Token &t)
{
  //
  // insert (n,t) in top level dictionary
  // dictionary stack must contain at least one dictionary
  // VoidToken is an illegal value for t.
  //
 
 assert(d.empty()==false);
 assert(t!=VoidToken);
 
 (*d.begin())->insert(n,t);
}

void DictionaryStack::undef(const Name &n)
{
  assert(d.empty()==false);

  size_t num_erased = 0;
  for (std::list<DictionaryDatum>::iterator it = d.begin();
       it != d.end();
       it++)    
    num_erased += (*it)->erase(n);    

  if (num_erased == 0)
    throw UndefinedName(n.toString());
}

void DictionaryStack::basedef(const Name &n, const Token &t)
{
  //
  // insert (n,t) in bottom level dictionary
  // dictionary stack must contain at least one dictionary
  // VoidToken is an illegal value for t.
  //
 
 assert(d.empty()==false);
 assert(t!=VoidToken);
 
 std::list<DictionaryDatum>::const_iterator i(--d.end());
 (*i)->insert(n,t);
}


void DictionaryStack::def_move(const Name &n, Token &t)
{
  //
  // insert (n,t) in top level dictionary
  // dictionary stack must contain at least one dictionary
  // VoidToken is an illegal value for t.
  // def_move returns t as the VoidToken.  
  //

 assert(d.empty()== false);
 assert(t!=VoidToken);

 (*d.begin())->insert_move(n,t);
}

void DictionaryStack::basedef_move(const Name &n, Token &t)
{
  assert(d.empty()== false);
  assert(t!=VoidToken);

  std::list<DictionaryDatum>::const_iterator i(--d.end());
  (*i)->insert_move(n, t);
}


void DictionaryStack::pop(void)
{ 
  //
  // remove top dictionary from stack
  // dictionary stack must contain at least one dictionary 
  //

 assert(d.empty()==false);

 d.pop_front();
}

void DictionaryStack::clear(void)
{ 
  d.erase(d.begin(),d.end());
}


void DictionaryStack::top(Token &t) const
{
  //
  // create a copy of the top level dictionary
  // and move it into Token t.
  // new should throw an exception if it fails
  //

  DictionaryDatum *dd= new DictionaryDatum(*(d.begin()));
  assert(dd!=NULL);

  Token dt( dd);
  t.move(dt);
}

void DictionaryStack::toArray(TokenArray &ta) const
{
  //
  // create a copy of the top level dictionary
  // and move it into Token t.
  // new should throw an exception if it fails
  //

  ta.erase();
  
  std::list<DictionaryDatum>::const_reverse_iterator i(d.rbegin());

  while (i!=d.rend())
  {
    ta.push_back((*i));
   ++i;
  }
}
  
void DictionaryStack::push(const Token& t)
{
  //
  // extract Dictionary from Token envelope
  // and push it on top of the stack.
  // a non dictionary datum at this point is a program bug.
  //

  DictionaryDatum *pd= dynamic_cast<DictionaryDatum *>(t.datum());
  assert(pd!=NULL);

  d.push_front(*pd);
}


size_t DictionaryStack::size(void) const
{
  //
  // return number of dictionaries on stack
  //

  return d.size();
}


void DictionaryStack::info(std::ostream& o) const
{
//  for_each(d.rbegin(), d.rend(), bind_2nd(mem_fun(&Dictionary::info),o));

  std::list<DictionaryDatum>::const_reverse_iterator i(d.rbegin());

  o << "DictionaryStack::info" << std::endl;
  o << "Size = " << d.size() << std::endl;
  while (i!=d.rend())
  {
    (*i)->info(o);
   ++i;
  }
}

void DictionaryStack::top_info(std::ostream& o) const
{
    (*d.begin())->info(o);
}
 
const DictionaryStack& DictionaryStack::operator=(const DictionaryStack& ds)
{
  if(&ds != this)
  {
    d=ds.d;
  }
  return *this;
}
