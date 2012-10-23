/*
 *  dictstack.h
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

#ifndef DICTIONARYSTACK_H
#define DICTIONARYSTACK_H
/* 
    SLI's dictionary stack
*/
#include <typeinfo>
#include "dictdatum.h"
#include <list>



/***************************************************************

Problems:
     
- is ist better to uses dictionaries as references to common
  objects like in PS. What is the exact meaning of undef and
  where in our current situation (read RedBook).     
- more efficient implementation exploiting 
  the name ids (documented elsewhere).



    History:
            (1) using list<Dictionary> 
               MD, 23.6.1, Freiburg
            (0) first version (single dictionary)
               MOG, MD, June 1997, Freiburg
***************************************************************
*/

class DictionaryStack
{
private:
    const Token VoidToken;
    std::list<DictionaryDatum> d;
    
public:
  DictionaryStack(const Token & = Token());
  DictionaryStack(const DictionaryStack&);
  ~DictionaryStack();
    
  /** Lookup a name searching all dictionaries on the stack.
   *  The first occurrence is reported. If the Name is not found,
   *  @a VoidToken is returned.
   */
  const Token & lookup(const Name &n) const
    {  
      std::list<DictionaryDatum>::const_iterator i(d.begin());

      while (i!=d.end())
      {
	const Token &t=(*i)->lookup(n);
	if(!(t==VoidToken))
	  return t;
	++i;
      }
      return VoidToken;
    }
  
  /** Lookup a name searching only the bottom level dictionary.
   *  If the Name is not found,
   *  @a VoidToken is returned.
   */
  const Token & baselookup(const Name &n) const // lookup in a specified
    {                                                  // base dictionary
      std::list<DictionaryDatum>::const_iterator i(--d.end());
      return (*i)->lookup(n);
    }

  /** Test for a name searching all dictionaries on the stack.
   */
  bool known(const Name &n) const
    {  
      std::list<DictionaryDatum>::const_iterator i(d.begin());

      while (i!=d.end())
      {
	if((*i)->known(n))
	  return true;
	++i;
      }
      return false;
    }
  
  /** Test for a name in the bottom level dictionary.
   */
  bool baseknown(const Name &n) const // lookup in a specified
    {                                                  // base dictionary
      std::list<DictionaryDatum>::const_iterator i(--d.end());
      return (*i)->known(n);
    }

   //
   // def and def_move operate on the top level dictionary.
   // undef is not defined for the dictionary stack.

  /** Bind a Token to a Name in the top level dictionary.
   *  The Token is copied.
   */
  void def(const Name &, const Token &);

  /** Unbind a previously defined Name from its token. Seach in all dictionaries.
   */
  void undef(const Name &);

  /** Bind a Token to a Name in the bottom level dictionary.
   *  The Token is copied.
   */
  void basedef(const Name &n, const Token &t);

  /** Bind a Token to a Name in the top level dictionary.
   *  The Token is moved.
   */
  void def_move(const Name&, Token &);
  
  /** Bind a Token to a Name in the bottom level dictionary.
   *  The Token is moved.
   */
  void basedef_move(const Name &n, Token &t);

  bool where(const Name&, Token&);
    
  void pop(void);


   //
   // a Dictionary is always wrapped in a Token
   //
   void top(Token &) const;
   void push(const Token &);

  void clear(void);
  void toArray(TokenArray &) const;
   //
   // move is efficient for interaction with operand and execution
   // stack, but can not be implemented in this version
   //
//    void pop_move(Token &) const;
//    void push_move(const Token &);

    // 
    // number of dictionaries currently on the stack
    //
  size_t size(void) const;


    //
    // info for debugging purposes. 
    // calls info(ostream&) for all dictionaries 
    //
  void info(std::ostream&) const;
  void top_info(std::ostream &) const; // calls info of top dictionary
  const DictionaryStack& operator=(const DictionaryStack&); 
};

#endif





