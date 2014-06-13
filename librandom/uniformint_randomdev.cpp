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
#include "sliexceptions.h"
#include "compose.hpp"

#include <cmath>
#include <limits>

// by default, init as exponential density with mean 1
librandom::UniformIntRandomDev::UniformIntRandomDev(RngPtr r_source)
  : RandomDev(r_source), 
    nmin_(0),
    nmax_(0),
    range_(nmax_-nmin_+1)
{ 
}

librandom::UniformIntRandomDev::UniformIntRandomDev() 
: RandomDev(), 
    nmin_(0),
    nmax_(0),
    range_(nmax_-nmin_+1)
{ 
}

void librandom::UniformIntRandomDev::set_status(const DictionaryDatum& d)
{
  long new_nmin = nmin_;
  long new_nmax = nmax_;

  updateValue<long>(d, "low", new_nmin);
  updateValue<long>(d, "high", new_nmax);

  if ( new_nmax < new_nmin )
    throw BadParameterValue("Uniformint RDV: low <= high required.");
  
  if ( new_nmax - new_nmin < 0 )
    throw BadParameterValue(String::compose("Uniformint RDV: high - low < %1 required.",
					    static_cast<double>(std::numeric_limits<long>::max())));

  nmin_ = new_nmin;
  nmax_ = new_nmax;
  range_ = nmax_ - nmin_ + 1;
} 

void librandom::UniformIntRandomDev::get_status(DictionaryDatum &d) const 
{
  def<long>(d, "low", nmin_);
  def<long>(d, "high", nmax_);
}
