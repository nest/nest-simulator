/*
 *  literaldatum.cc
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

#include "namedatum.h"
#include "datumconverter.h"

void LiteralDatum::pprint(std::ostream &out) const
{
  out << '/';
  print(out);   
}

void LiteralDatum::use_converter(DatumConverter &converter)
{
  converter.convert_me(*this); // call visit with our own type here
                  // this will call the approproate implementation of the derived class
}

