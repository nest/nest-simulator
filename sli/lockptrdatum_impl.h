
/*
 *  lockptrdatum_impl.h
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

#ifndef LOCKPTRDATUMIMPL_H
#define LOCKPTRDATUMIMPL_H

#include "lockptrdatum.h"
#include "datumconverter.h"


template <class D, SLIType *slt>
bool lockPTRDatum<D, slt>::equals(const Datum *dat) const
{
  // The following construct works around the problem, that
  // a direct dynamic_cast<const GenericDatum<D> * > does not seem
  // to work.
  
  const lockPTRDatum<D,slt>
    *ddc=dynamic_cast<lockPTRDatum<D, slt> * >(const_cast< Datum *>(dat));
        
  if(ddc == NULL)
    return false;
  
  return  static_cast<lockPTR<D> >(*ddc) == static_cast<lockPTR<D> >(*this);
}
 
template <class D, SLIType *slt>
void lockPTRDatum<D, slt>::pprint(std::ostream &out) const
{
  out << "<lockPTR["<< this->references()<<"]->" 
      << this->gettypename()
      << '(' << static_cast<void *>(this->get()) <<")>";
  this->unlock();
}

template <class D, SLIType *slt>
void lockPTRDatum<D, slt>::print(std::ostream &out) const
{
  out << '<' << this->gettypename() << '>';   
}


template <class D, SLIType *slt>
void lockPTRDatum<D, slt>::info(std::ostream &out) const
{
//  out << *dynamic_cast<C *>(const_cast<lockPTR<C,slt> *>(this));    
   pprint(out);   
}

/**
 * Accept a DatumConverter as a visitor to this datum.
 * A visitor may be used to make a conversion to a type, which is not
 * known to NEST.  (visitor pattern).
 */
template<class D, SLIType *slt>
void lockPTRDatum<D, slt>::use_converter(DatumConverter &converter)
{
  // call convert_me with our own type here this will call the
  // approproate implementation of the derived class
  converter.convert_me(*this);
}

#endif
