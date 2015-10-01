/*
 *  conngendatum.h
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

#ifndef CONNGENDATUM_H
#define CONNGENDATUM_H

#include "slitype.h"
#include "lockptrdatum.h"

#include <neurosim/connection_generator.h>

typedef std::vector< ConnectionGenerator::ClosedInterval > RangeSet;
typedef ConnectionGenerator::ClosedInterval Range;

namespace nest
{

extern SLIType ConnectionGeneratorType;

typedef lockPTRDatum< ConnectionGenerator, &ConnectionGeneratorType > ConnectionGeneratorDatum;

} // namespace nest

#endif /* CONNGENDATUM_H */
