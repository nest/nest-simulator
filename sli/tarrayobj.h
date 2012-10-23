/*
 *  tarrayobj.h
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

#ifndef TARRAYOBJ_H
#define TARRAYOBJ_H
/* 
    Array of Tokens
*/

#include <typeinfo>
#include <cstddef>
#include "token.h"

#define ARRAY_ALLOC_SIZE 128

class Token;

class TokenArrayObj
{
 private:
  Token* p;
  Token* begin_of_free_storage;
  Token* end_of_free_storage;
  size_t alloc_block_size;
  
//  bool homogeneous;

  void allocate(size_t, size_t, size_t, const Token & = Token());
    
  static size_t allocations;
 public:
    unsigned int refs;
    
    TokenArrayObj(void)
            :p(NULL),begin_of_free_storage(NULL),
             end_of_free_storage(NULL),
             alloc_block_size(ARRAY_ALLOC_SIZE), 
//	     homogeneous(true),
	     refs(1)
    {};
    
    TokenArrayObj(size_t , const Token & = Token(), size_t =  0);
    TokenArrayObj(const TokenArrayObj &);

    virtual ~TokenArrayObj();
    
    Token * begin() const
    {
        return p;
    }
    
    Token * end() const
    {
        return begin_of_free_storage;
    }

    size_t size(void) const
    {
        return (size_t)(begin_of_free_storage-p);
    }
        
    size_t capacity(void) const
    {
        return (size_t)(end_of_free_storage - p);
    }
    
    Token & operator[](size_t i) { return p[i]; }
    
    const Token & operator[](size_t i) const
    { return p[i]; }

  void rotate(Token *, Token *, Token *);


        // Memory allocation
    
    bool shrink(void);
    bool reserve(size_t);
    unsigned int references(void)
    {
      return refs;
    }

    void resize(size_t, size_t, const Token & = Token());
    void resize(size_t, const Token & = Token());

        // Insertion, deletion
    void push_back(const Token &t)
    {
      if(capacity()<size()+1)
	reserve(size()+alloc_block_size);
      *begin_of_free_storage++ = t;
    }

    void push_back_move(Token &t)
    {
      if(capacity()<size()+1)
	reserve(size()+alloc_block_size);

      begin_of_free_storage->move(t);
      ++begin_of_free_storage;
    }


    void assign_move(Token *tp, Token &t)
    {
      tp->move(t);
    }

    void pop_back(void)
    {
//    assert(size()>0);
      (--begin_of_free_storage)->clear();

    }

  // Erase the range given by the iterators.
    void erase(size_t , size_t);   
    void erase(Token *, Token *);
    void erase(Token *tp)
    {
        erase(tp,tp+1);
    }

  // Reduce the array to the range given by the iterators
    void reduce(Token *, Token *);
    void reduce(size_t, size_t);

    void insert(size_t, size_t = 1, const Token& = Token());
    void insert(size_t i, const Token& t)
    {
        insert(i,1,t);
    }
    
    void insert_move(size_t, TokenArrayObj &);
    void insert_move(size_t, Token &);

    void assign_move(TokenArrayObj &, size_t, size_t);
    void assign(const TokenArrayObj &, size_t, size_t);

    void replace_move(size_t, size_t, TokenArrayObj &);
 
    void append_move(TokenArrayObj &);

    void clear(void);
        
        
    const TokenArrayObj & operator=(const TokenArrayObj &);
    
    bool operator==(const TokenArrayObj &) const;

    bool empty(void) const
    {
        return size()==0;
    }
    
    void info(std::ostream &) const;

  static size_t getallocations(void)
    { return allocations;}

  bool valid(void) const; // check integrity
    
};

std::ostream & operator<<(std::ostream& , const TokenArrayObj&);



#endif


