/*
 *  conngen.h
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

#ifndef CONNGEN_H
#define CONNGEN_H

#include "nest_types.h"
#include "nest_names.h"

namespace nest
{
class ConnectionGeneratorDatum;
class IntVectorDatum;
class DictionaryDatum;
class StringDatum;


void CGConnect( const ConnectionGeneratorDatum& cg,
  const index source_id,
  const index target_id,
  const DictionaryDatum& params_map,
  const Name& synmodel_name );

void CGConnect( const ConnectionGeneratorDatum& cg,
  const IntVectorDatum& source_id,
  const IntVectorDatum& target_id,
  const DictionaryDatum& params_map,
  const Name& synmodel_name );

ConnectionGeneratorDatum CGParse( const StringDatum& xml );

ConnectionGeneratorDatum CGParseFile( const StringDatum& xml );

void CGSelectImplementation( const StringDatum& library, const StringDatum& tag );
}

#endif /* CONNGEN_H */
