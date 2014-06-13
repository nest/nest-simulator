/*
 *  dictdatum.h
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

#ifndef DICTDATUM_H
#define DICTDATUM_H
/* 
    Defines class DictionaryDatum
*/
#include "interpret.h"
#include "dict.h"
#include "lockptrdatum.h"

typedef lockPTRDatum<Dictionary,&SLIInterpreter::Dictionarytype> DictionaryDatum;


// MD experimental, selfreferences. need to extend to recursive nesting of arrays and dicts
template<>
inline
size_t lockPTRDatum<Dictionary,&SLIInterpreter::Dictionarytype>::selfreferences(void)
{
 size_t r=0;

 if (valid())
 {
  Dictionary *d=get();

  for(Dictionary::iterator i = d->begin(); i != d->end(); ++i)
  {
   DictionaryDatum *d=dynamic_cast<DictionaryDatum*>(i->second.datum_without_tagging_as_accessed());
   if(d !=0)
   {
  
     if (equals(d)) 
      ++r;
  
   }
  }
    
  unlock();
 }

 return r;
}


// MD experimental
template<>
inline
lockPTRDatum<Dictionary,&SLIInterpreter::Dictionarytype>::~lockPTRDatum<Dictionary,&SLIInterpreter::Dictionarytype>()
{
 
  if (exists()) // otherwise we have already detached
  {
  size_t s=selfreferences();
  
  if (s>0) // otherwise just directly hand over to destructors of parent classes
   {

     if (references()>s+1)
     { // the object survives, we just need to detach and correct the reference count
      detach(); // assuming also decrements by 1
     }
     

     else // references == selfreferences + 1
     { // we need to destroy, before that remove self references   

      Dictionary *my_d=get();  

      for(Dictionary::iterator i = my_d->begin(); i != my_d->end(); ++i)
      {
       DictionaryDatum *d=dynamic_cast<DictionaryDatum*>(i->second.datum_without_tagging_as_accessed());
       if(d !=0)
       {
	 if (this->equals(d)) 
	  d->detach();

       }
      }
    
      // at this point we have: 
      //   references == 1
      //   selfreferences == 0
      // therefore we can unlock() before handing over to ~lockPTR()
   
      unlock();
     }
   }
  }
}


#endif
