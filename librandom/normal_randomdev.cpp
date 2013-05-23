/*
 *  normal_randomdev.cpp
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

#include <cmath>
#include "config.h"
#include "normal_randomdev.h"

// by default, init as exponential density with mean 1
librandom::NormalRandomDev::NormalRandomDev(RngPtr r_source) 
: RandomDev(r_source)
{}

// threaded
librandom::NormalRandomDev::NormalRandomDev() 
: RandomDev()
{}

double librandom::NormalRandomDev::operator()(RngPtr r) const
{
  // Box-Muller algorithm, see Knuth TAOCP, vol 2, 3rd ed, p 122
  // we waste one number
  double V1;
  double V2;
  double S;
  
  do {
    V1 = 2 * r->drand() - 1;
    V2 = 2 * r->drand() - 1;
    S  = V1*V1 + V2*V2;
  } while ( S >= 1 );
  
  if ( S == 0 )
    return 0;
  else
    return V1 * std::sqrt(-2 * std::log(S)/S);  
}
