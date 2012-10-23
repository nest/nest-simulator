/*
 *  uniformint_randomdev.cpp
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

#include "uniformint_randomdev.h"
#include "dictutils.h"

#include <cmath>
#include <stdexcept>

// by default, init as exponential density with mean 1
librandom::UniformIntRandomDev::UniformIntRandomDev(RngPtr r_source)
  : RandomDev(r_source), 
    nmin_(0),
    nmax_(0),
    range_(nmax_-nmin_+1)
{ 
  assert(range_ > 0);
}

librandom::UniformIntRandomDev::UniformIntRandomDev() 
: RandomDev(), 
    nmin_(0),
    nmax_(0),
    range_(nmax_-nmin_+1)
{ 
  assert(range_ > 0);
}


void librandom::UniformIntRandomDev::set_status(const DictionaryDatum& d)
{
  updateValue<long>(d, "nmin", nmin_);
  updateValue<long>(d, "nmax", nmax_);
  range_ = nmax_ - nmin_ + 1;

  if ( range_ < 1 )
    throw std::out_of_range("UniformIntRandomDev::set_status: range >= 1 required.");
} 

void librandom::UniformIntRandomDev::get_status(DictionaryDatum &d) const 
{
  def<long>(d, "nmin", nmin_);
  def<long>(d, "nmax", nmax_);
}
