/*
 *  token.h
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

#ifndef TOKEN_H
#define TOKEN_H
/* 
    token.h defines the base objects used by the SLI interpreter.
*/

#include <typeinfo>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <valarray>
#include "config.h"

#include "datum.h"

class Name;
class Token;
class TokenArray;
class TokenArrayObj;



/***********************************************************/
/* Token                                               */
/* ---------                                               */
/*  Token class for all Data Objects                   */
/*

const Datum* p;   makes p a pointer to a const. Any change to the
                  object p points to is prevented.
Datum *const p;   makes p a const pointer to a Datum. Any change to the
                  pointer is prevented.

 It is not necessary to declare the pointer itself const, because the
 return value is copied anyway. Only if the return value can be used as
 an lvalue do we need to protect it.   

  A member function declared const does not change any member object
  of the class, it can be called for const class objects.

*/

/***********************************************************/

/** A type-independent container for C++-types.
 *  @ingroup TokenHandling
 */
class Token
{ 
  friend class Datum; 
  friend class TokenArrayObj;	
  
private:
  Datum *p;

  /** Flag for access control.
   * Is set by getValue() and setValue() via datum().
   */
  mutable bool  accessed_;   

  void SetDatum(Datum *p_s)
    {
      delete p;
      p = p_s;
    }

public:

   ~Token()
    {
      delete p;
    }

  Token(const Token& c_s)
    :p(NULL)
    {
      if(c_s.p !=NULL)
        p=c_s.p->clone(); // we can savely assume, that this object 
    }                              // does not point to any Datum object
  
    
  Token(Datum *p_s = NULL) //!< use existing pointer to datum, token takes responsibulity of the pointer.
    :p(p_s){}
    
  Token(const Datum &d )   //!< copy datum object and store its pointer.
    { p=d.clone();}

  Token(int);
  Token(unsigned int);
  Token(long);
  Token(bool);
  Token(unsigned long);
  Token(double);
  Token(const char*);
  Token(std::string);
  Token(const std::vector<double>&);
  Token(const std::valarray<double>&);
  Token(const std::vector<long>&);
  Token(const std::vector<size_t>&);
  Token(const std::ostream&);
  Token(const std::istream&);

  operator Datum*() const; 
  operator size_t() const;
  operator long() const;
  operator double() const;
  operator float() const;
  operator bool() const;
  operator std::string() const;
//  operator vector<double> const;
//  operator vector<long> const;

  
  void move( Token &c)
    {
      delete p;
      p=c.p;
      c.p = NULL;
    }
  
  void swap(Token &c)
    {
      std::swap(p,c.p);
    }
  
  void clear(void)
    {
      delete p;
      p = NULL;
    }
  
  bool contains(const Datum &d) const
    {
      return (p!=NULL) ? p->equals(&d) : false;
    }
  
  bool empty(void) const
    {
      return p == NULL;
    }
  
  bool operator!(void) const
    {
      return p == NULL;
    }
  
  Datum* datum(void) const
    {
      accessed_ = true;
      return p;
    }
  
  
  Datum* operator->() const
    {
      assert(p!= NULL);
      return p;
    }
  
  
  Datum& operator*() const
    {
      assert(p != NULL);
      return *p;
    }
  
  const std::type_info& type(void) const
    {
      return typeid(*p);
    }
    
  
  Token& operator=(const Token& c_s)
    {
      
      if (c_s.p != NULL)
      {
        if (p!=c_s.p)               // protection against a=a !
	  {
	    delete p;
	    p=c_s.p->clone();
	  }
      }
      else
        clear();
      
      return *this; 
    }   

  Token& operator=(Datum *p_s)
    {
      if(p != p_s)
      {
	delete p;
	p=p_s;
      }
      
      return *this; 
    }   


  bool operator==(const Token &t) const
    {
      if(p == t.p)
        return true;
      if( p == NULL || t.p == NULL )
        return false;
      
      return p->equals(t.p);
      
    }

  // define != explicitly --- HEP 2001-08-09
  bool operator!=(const Token &t) const
    {
      return !( *this == t );
    }

  void info(std::ostream &) const;

  void pprint(std::ostream &) const;
	
  /** Clear accessed flag. */
  void clear_access_flag() { accessed_ = false; }
  void set_access_flag() const { accessed_ = true; }
  
  /** Check for access.
   * Access control does not differentiate between read and write
   * access, and relies on getValue<>, setValue<> to use datum()
   * to access the data in the Token. The access flag should be
   * cleared before entering the code for which access is to be
   * checked.
   */
  bool accessed() const { return accessed_; }

  /**
   * Check whether Token contains a Datum of a given type.
   * @return true if Token is of type given by template parameter.
   */
  template <typename DatumType>
  bool is_a() const 
  {
    return dynamic_cast<DatumType*>(p);
  }
  
};


/************* Misc functions ********************/

std::ostream& operator<<(std::ostream&, const Token&);

typedef  unsigned long Index;

#endif
