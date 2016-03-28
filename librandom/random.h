/*
 *  random.h
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

#ifndef RANDOM_H
#define RANDOM_H

// Includes from librandom:
#include "random_datums.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "token.h"

namespace librandom
{

librandom::RngDatum create_rng( const long seed, const RngFactoryDatum& factory );

librandom::RdvDatum create_rdv( const RdvFactoryDatum& factory, const RngDatum& rng );

void set_status( const DictionaryDatum& dict, RdvDatum& rdv );
DictionaryDatum get_status( const RdvDatum& rdv );

void seed( const long seed, RngDatum& rng );
unsigned long irand( const long N, RngDatum& rng );
double drand( RngDatum& rng );

ArrayDatum random_array( RdvDatum& rdv, const size_t n );
Token random( RdvDatum& rdv );
}

#endif /* RANDOM_H */
