/*
 *  datum.h
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

#ifndef DATUM_H
#define DATUM_H

#include "slitype.h"
class DatumConverter;


/***********************************************************/
/* Datum                                                   */
/* -----                                                   */
/*  base class for all Data Objects                        */
/***********************************************************/
class Datum
{
  
  friend class Token;
  
  virtual Datum * clone(void) const = 0;


  /**
   * Returns a reference counted pointer to the datum, or a new pointer, if the
   * type does not support reference counting.
   * The prefix const indicates that the pointer should be trated as const
   * because changes affect all other references as well.
   */ 

  virtual Datum * get_ptr() 
  {
    return clone();
  }

 protected:
  // Putting the following variables here, avoids a number of virtual
  // functions.

  const SLIType     *type;   //!< Pointer to type object.
  const SLIFunction *action; //!< Shortcut to the SLIType default action.
  mutable
    unsigned int reference_count_;
  bool   executable_;

 Datum() :
  type(NULL), 
    action(NULL),
    reference_count_(1),
    executable_(true)
      {}

  
 Datum(const SLIType *t) : 
  type(t), 
    action(t->getaction()),
    reference_count_(1),
    executable_(true)
      { }

 Datum(const Datum &d) : type(d.type), action(d.action),reference_count_(1),executable_(d.executable_){}


 public:

  virtual ~Datum() {};

  void addReference() const
  {
    ++reference_count_;
  }

  void removeReference()
  {
    --reference_count_;
    if(reference_count_==0)
      delete this;
  }

  size_t numReferences() const
  {
    return reference_count_;
  }

  bool is_executable() const
  {
    return executable_;
  }

  void set_executable()
  {
    executable_=true;
  }

  void unset_executable()
  {
    executable_=false;
  }

  virtual void  print(std::ostream & ) const =0;
  virtual void  pprint(std::ostream &) const =0;

  virtual void  list(std::ostream &o, std::string prefix, int l) const
  {
    if(l==0)
      prefix="-->"+prefix;
    else
      prefix="   "+prefix;
    o << prefix;
    print(o);
  }

  virtual void  input_form(std::ostream &o) const
  {
    pprint(o);
  }
  
  virtual bool  equals(const Datum *d) const
  {
    return this == d;
  }
  
  virtual void  info(std::ostream &) const;
  
  const Name & gettypename(void) const
    {
      return type->gettypename();
    }

  bool isoftype(SLIType const &t) const
    {
      return (type==&t);  // or: *type==t, there is only one t with same contents !
    }
    
  void execute(SLIInterpreter *i)
    {
      action->execute(i);
    }

  /**
   * Accept a DatumConverter as a visitor to this datum for conversion.
   * (visitor pattern).
   */
  virtual void use_converter(DatumConverter &v);
  
};

template<SLIType * slt >
class TypedDatum: public Datum
{
 public:
 TypedDatum(void)
   :Datum(slt)
  { }
  
 TypedDatum(const TypedDatum<slt> &d) :Datum(d){}
  const TypedDatum<slt>& operator=(const TypedDatum<slt>&);
  
};

template<SLIType * slt >
inline 
const TypedDatum<slt>& TypedDatum<slt>::operator=(const TypedDatum<slt>&)
{
  //  assert( type == d.type );
  return *this;
}
  


#endif

