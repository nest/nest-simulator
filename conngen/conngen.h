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

// C++ includes:
#include <vector>

// Includes from conngen:
#include "conngendatum.h"

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "stringdatum.h"

namespace nest
{

void cg_connect( ConnectionGeneratorDatum& cg,
  const index source_id,
  const index target_id,
  const DictionaryDatum& params_map,
  const Name& synmodel_name );

void cg_connect( ConnectionGeneratorDatum& cg,
  IntVectorDatum& source_id,
  IntVectorDatum& target_id,
  const DictionaryDatum& params_map,
  const Name& synmodel_name );

ConnectionGeneratorDatum cg_parse( const StringDatum& xml );

ConnectionGeneratorDatum cg_parse_file( const StringDatum& xml );

void cg_select_implementation( const StringDatum& library,
  const StringDatum& tag );

void cg_set_masks( ConnectionGeneratorDatum& cg,
  IntVectorDatum& sources,
  IntVectorDatum& targets );
void cg_start( ConnectionGeneratorDatum& cgd );
bool cg_next( ConnectionGeneratorDatum& cgd,
  int& src,
  int& tgt,
  std::vector< double >& values );
}

#endif /* CONNGEN_H */
