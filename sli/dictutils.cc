/*
 *  dictutils.cc
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

#include "dictutils.h"

void initialize_property_array(DictionaryDatum &d, Name propname)
{
  Token t = d->lookup(propname);
  if (t == d->getvoid())
  {
    ArrayDatum arrd;
    def<ArrayDatum>(d, propname, arrd);
  }
}

void initialize_property_doublevector(DictionaryDatum &d, Name propname)
{
  Token t = d->lookup(propname);
  if (t == d->getvoid())
  {
    DoubleVectorDatum arrd(new std::vector<double>);
    def<DoubleVectorDatum>(d, propname, arrd);
  }
}

void initialize_property_intvector(DictionaryDatum &d, Name propname)
{
  Token t = d->lookup(propname);
  if (t == d->getvoid())
  {
    IntVectorDatum arrd(new std::vector<long>);
    def<IntVectorDatum>(d, propname, arrd);
  }
}



