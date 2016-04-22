/*
 *  random_datums.h
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

/*
 * SLI Datum types related to the NEST random library.
 *
 */

#ifndef RANDOM_DATUMS_H
#define RANDOM_DATUMS_H

// Includes from librandom:
#include "random_numbers.h"
#include "randomdev.h"
#include "randomgen.h"

// Includes from sli:
#include "lockptrdatum.h"
#include "lockptrdatum_impl.h"

namespace librandom
{

/** Encapsulates random number generators in SLI.
 *  @ingroup RandomNumberGenerators
 */
typedef lockPTRDatum< librandom::RandomGen, &RandomNumbers::RngType > RngDatum;

/** Encapsulates random number generator factories in SLI.
 *  @ingroup RandomNumberGenerators
 */
typedef lockPTRDatum< librandom::GenericRNGFactory,
  &RandomNumbers::RngFactoryType >
  RngFactoryDatum;

/** Encapsulates random deviate generators in SLI.
 *  @ingroup RandomNumberGenerators
 */
typedef lockPTRDatum< librandom::RandomDev, &RandomNumbers::RdvType > RdvDatum;

/** Encapsulates random deviate generator factories in SLI.
 *  @ingroup RandomNumberGenerators
 */
typedef lockPTRDatum< librandom::GenericRandomDevFactory,
  &RandomNumbers::RdvFactoryType >
  RdvFactoryDatum;
}

#endif
