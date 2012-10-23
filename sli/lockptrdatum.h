/*
 *  lockptrdatum.h
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

#ifndef LOCKPTRDATUM_H
#define LOCKPTRDATUM_H

#include "datum.h"
#include "lockptr.h"

class DatumConverter;

// prefixed all references to members of lockPTR, TypedDatum with this->,
// since HP's aCC otherwise complains about them not being declared
// according to ISO Standard Sec. 14.6.2(3) [temp.dep]
// HEP, 2001-08-09

// this class must not be a base class
template <class D, SLIType *slt>
class lockPTRDatum: public lockPTR<D>, public TypedDatum<slt>
{
  Datum * clone(void) const
    {
      return new lockPTRDatum<D,slt>(*this);
    }

  public:
  
  lockPTRDatum() {}
  
//   template<SLIType *st>
//   lockPTRDatum(const lockPTRDatum<D,st> &d):lockPTR<D>(d), TypedDatum<slt>(){}
  
  lockPTRDatum(const lockPTR<D> d): lockPTR<D>(d), TypedDatum<slt>() {}    
  lockPTRDatum(D *d): lockPTR<D>(d), TypedDatum<slt>() {}    
  lockPTRDatum(D& d): lockPTR<D>(d), TypedDatum<slt>() {}    
  
  ~lockPTRDatum() {} // this class must not be a base class
       
  void print(std::ostream &) const;
  void pprint(std::ostream &) const;
  void info(std::ostream &) const;


  bool equals(const Datum *) const;

   /**
   * Accept a DatumConverter as a visitor to the datum (visitor pattern).
   * This member has to be overridden in the derived classes
   * to call visit and passing themselves as an argument.
   */
  void use_converter(DatumConverter &);

};


/******************************************/

#endif








