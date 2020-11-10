/*
 *  conn_builder_conngen.h
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

#ifndef CONN_BUILDER_CONNGEN_H
#define CONN_BUILDER_CONNGEN_H

#include "config.h"
#ifdef HAVE_LIBNEUROSIM

// C++ includes:
#include <map>
#include <vector>

// Includes from nestkernel:
#include "conn_builder.h"
#include "nest_datums.h"

namespace nest
{

class ConnectionGeneratorBuilder : public ConnBuilder
{
  typedef std::vector< ConnectionGenerator::ClosedInterval > RangeSet;
  typedef ConnectionGenerator::ClosedInterval Range;

public:
  ConnectionGeneratorBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_();
  void cg_set_masks();
  index cg_get_right_border( index left, size_t step, const NodeCollectionPTR nodes );
  void cg_get_ranges( RangeSet& ranges, const NodeCollectionPTR nodes );

private:
  ConnectionGeneratorDatum cg_;
  DictionaryDatum params_map_;
};

} // namespace nest

#endif /* ifdef HAVE_LIBNEUROSIM */

#endif /* ifdef CONN_BUILDER_CONNGEN_H */
