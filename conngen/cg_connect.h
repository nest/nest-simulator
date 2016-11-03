/*
 *  cg_connect.h
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

#ifndef CG_CONNECT_H
#define CG_CONNECT_H

// C++ includes:
#include <vector>

// Includes from conngen:
#include "conngendatum.h"

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
void cg_connect( ConnectionGeneratorDatum& cg,
  RangeSet& sources,
  index source_offset,
  RangeSet& targets,
  index target_offset,
  DictionaryDatum params_map,
  index syn );
void cg_connect( ConnectionGeneratorDatum& cg,
  RangeSet& sources,
  std::vector< long >& source_gids,
  RangeSet& targets,
  std::vector< long >& target_gids,
  DictionaryDatum params_map,
  index syn );

void cg_set_masks( ConnectionGeneratorDatum& cg,
  RangeSet& sources,
  RangeSet& targets );
void cg_create_masks( std::vector< ConnectionGenerator::Mask >* masks,
  RangeSet& sources,
  RangeSet& targets );

index cg_get_right_border( index left, size_t step, std::vector< long >& gids );
void cg_get_ranges( RangeSet& ranges, std::vector< long >& gids );
}

#endif /* #ifndef CG_CONNECT_H */
