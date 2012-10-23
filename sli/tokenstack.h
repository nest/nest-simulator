/*
 *  tokenstack.h
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

#ifndef TOKENSTACK_H
#define TOKENSTACK_H
/* 
    SLI's stack for tokens
*/

#include "token.h"
#include "tarrayobj.h"
#include "tokenarray.h"

/* This stack implementation assumes that functions are only called,
   if the necessary pre-requisites are fulfilled. The code will break
   otherwise.
*/

class TokenStack : private TokenArrayObj
{

    
public:
    TokenStack(Index n ) 
            : TokenArrayObj(0,Token(),n) {}
    TokenStack(const TokenArray& ta) 
            : TokenArrayObj(ta) {}

  using TokenArrayObj::reserve;

    void clear(void)
    {
        erase(begin(),end());
    }
    
    void push(const Token& e)
    {
        push_back(e);
    }

    void push_move(Token& e)
    {
        push_back_move(e);
//        (end()-1)->move(e);
    }
        
    void pop(void)
    {
//        assert(load() > 0);
        pop_back();
    }
 
    void pop_move(Token& e)   // new one 5.5.97
    {
//        assert(load() > 0);
        e.move(*(end()-1));
        pop_back();
    }
    
    void pop(size_t n)
    {
//        assert(load() >= n);
        erase(end()-n, end());
    }
 
    
    Token& top(void)
    {
        return *(end()-1);
    }
    const Token& top(void) const
    {
        return *(end()-1);
    }

    const Token& pick(size_t i) const
    {
        return *(end()-i-1);           
    }
    
    Token& pick(size_t i)
    {
        return *(end()-i-1);
    }

  using TokenArrayObj::empty;
//  using TokenArray::operator=;
    
        
  void swap(void)
    {
//        assert(load()>1);
        (end()-1)->swap(*(end()-2));
    }

  void swap(Token &e)
    {
//      assert(load()>1);
        (end()-1)->swap(e);
    }

  void index(Index i)
    {
//      assert(load()>i);
      push(pick(i));
    }

  void roll(size_t n, long k)
    {
//      assert(load() >= n);
      
      if (k>=0)
      {
	rotate(end()-n,end()-(k%n),end());
      }
      else 
      {
	rotate(end()-n,end()-(n+k)%n,end());
      }
    }            
            
  Index size(void) const        {return TokenArrayObj::capacity();}
  Index load(void) const        {return TokenArrayObj::size();} 

  void dump(std::ostream &) const;

  TokenArray toArray(void) const
    {
      return TokenArray(*this);
    }
    
};

#endif
