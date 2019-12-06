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
#include "nest_datums.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "stringdatum.h"

namespace nest
{

void cg_connect( ConnectionGeneratorDatum& cg,
  const NodeCollectionPTR source_node_ids,
  const NodeCollectionPTR target_node_ids,
  const DictionaryDatum& params_map,
  const Name& synmodel_name );

void cg_set_masks( ConnectionGeneratorDatum& cg, const NodeCollectionPTR sources, const NodeCollectionPTR targets );

void cg_create_masks( std::vector< ConnectionGenerator::Mask >& masks, RangeSet& sources, RangeSet& targets );

index cg_get_right_border( index left, size_t step, const NodeCollectionPTR node_ids );

void cg_get_ranges( RangeSet& ranges, const NodeCollectionPTR node_ids );
}

#endif /* CONNGEN_H */
