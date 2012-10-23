/*
 *  numericdatum.h
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

#ifndef NUMERICDATUM_H
#define NUMERICDATUM_H
/* 
    Datum template for numeric data types
*/

#include "genericdatum.h"
#include "allocator.h"



// prefixed all references to members of GenericDatum with this->,
// since HP's aCC otherwise complains about them not being declared
// according to ISO Standard Sec. 14.6.2(3) [temp.dep]
// HEP, 2001-08-08

template<class D, SLIType *slt>
class NumericDatum: public GenericDatum<D,slt>
{
 protected:
  static sli::pool memory;

 private:
  Datum *clone(void) const
    {
        return new NumericDatum<D,slt>(*this);    
    }

public:
    
    NumericDatum() { this->d = (D) 0;}
    NumericDatum(const D& d_s) {this->d=d_s;}
    virtual ~NumericDatum() {}
    
    
    void incr(void)
    {
        ++(this->d) ;
    }

    void decr(void )
    {
        --(this->d) ;
    }

    void add(const NumericDatum<D,slt> &nd ) 
    {
        this->d += nd.d;
    }
    
    void add(D n ) 
    {
        this->d += n;
    }

    void sub(const NumericDatum<D,slt> &nd ) 
    {
        this->d -= nd.d;
    }

    void sub(D n ) 
    {
        this->d -= n;
    }
    void sub_from(const NumericDatum<D,slt> &nd ) 
    {
        this->d = nd.d - this->d;
    }

    void sub_from(D n ) 
    {
        this->d = n - this->d;
    }

    void mul(const NumericDatum<D,slt> &nd ) 
    {
        this->d *= nd.d;
    }
    void mul(D n ) 
    {
        this->d *= n;
    }

    void div(const NumericDatum<D,slt> &nd ) 
    {
        this->d /= nd.d;
    }
    void div(D n ) 
    {
        this->d /= n ;
    }

    void div_by(D n ) 
    {
        this->d = n / this->d ;
    }   

  void  input_form(std::ostream &) const;
  void  pprint(std::ostream &) const;


  static void * operator new(size_t size)
    {
      if(size != memory.size_of())
	return ::operator new(size);
      return memory.alloc();
    }

  static void operator delete(void *p, size_t size)
    {
      if(p == NULL)
	return;
      if(size != memory.size_of())
      {
	::operator delete(p);
	return;
      }
      memory.free(p);
    }


  /**
   * Accept a DatumVisitor as a visitor to the datum (visitor pattern).
   * This member has to be overridden in the derived classes
   * to call visit and passing themselves as an argument.
   */
  void use_converter(DatumConverter &);

};



#endif
