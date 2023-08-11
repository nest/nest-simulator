/*
 *  histentry.cpp
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

#include "histentry.h"

nest::histentry::histentry( double t, double Kminus, double Kminus_triplet, size_t access_counter )
  : t_( t )
  , Kminus_( Kminus )
  , Kminus_triplet_( Kminus_triplet )
  , access_counter_( access_counter )
{
}

nest::histentry_extended::histentry_extended( double t, double dw, size_t access_counter )
  : t_( t )
  , dw_( dw )
  , access_counter_( access_counter )
{
}
