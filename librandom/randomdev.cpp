/*
 *  randomdev.cpp
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

#include "randomdev.h"

// These two functions must not be inlined
// Otherwise, the Compaq C++ Compiler goes haywire
// HEP, 2004-06-29
unsigned long librandom::RandomDev::uldev(void)
{
  assert(false);
  return 0;
}

unsigned long librandom::RandomDev::uldev(RngPtr) const
{
  assert(false);
  return 0;
}

