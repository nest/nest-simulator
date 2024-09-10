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

/**
 * Create the masks for sources and targets and set them on the
 * Connection Generator.
 *
 * The masks are based on the contiguous ranges present in the given
 * sources and targets. We need to do some index translation here, as
 * the CG expects indices from 0..n for both source and target
 * populations, while the corresponding RangeSets for sources and
 * targets contain NEST global indices (node IDs).
 *
 * The masks for the sources must contain all nodes (local+remote). To
 * achieve this, the skip of the mask is set to 1 and the same source
 * mask is stored n_proc times on each process.
 *
 * The masks for the targets must only contain local nodes. This is
 * achieved by first setting skip to num_processes upon creation of
 * the mask, and second by the fact that for each contiguous range of
 * nodes in a mask, each of them contains the index-translated id of
 * the first local neuron as the first entry. If this renders the
 * range empty (i.e. because the first local id is beyond the last
 * element of the range), the range is not added to the mask.
 *
 * \note Each process computes the full set of source and target
 * masks, i.e. one mask per rank will be created on each rank.
 *
 * \note Setting the masks for all processes on each process might
 * become a memory bottleneck when going to very large numbers of
 * processes. Especially so for the source masks, which are all the
 * same. This could be solved by making the ConnectionGenerator
 * interface MPI aware and communicating the masks during connection
 * setup.
 */
class ConnectionGeneratorBuilder : public BipartiteConnBuilder
{
  typedef std::vector< ConnectionGenerator::ClosedInterval > RangeSet;
  typedef ConnectionGenerator::ClosedInterval Range;

public:
  ConnectionGeneratorBuilder( NodeCollectionPTR,
    NodeCollectionPTR,
    ThirdOutBuilder*,
    const DictionaryDatum&,
    const std::vector< DictionaryDatum >& );

protected:
  void connect_();
  void cg_set_masks();
  size_t cg_get_right_border( size_t left, size_t step, const NodeCollectionPTR nodes );

  /**
   * Determine all contiguous ranges found in a given vector of node IDs
   * and add the ranges to the given RangeSet.
   *
   * \param ranges A reference to the RangeSet to add to
   * \param nodes A reference to a std::vector<long> of node IDs
   *
   * \note We do not store the indices into the given range, but
   * instead we store the actual node IDs. This allows us to use CG
   * generated indices as indices into the ranges spanned by the
   * RangeSet. Index translation is done in cg_create_masks().
   */
  void cg_get_ranges( RangeSet& ranges, const NodeCollectionPTR nodes );

private:
  ConnectionGeneratorDatum cg_;
  DictionaryDatum params_map_;
};

} // namespace nest

#endif /* ifdef HAVE_LIBNEUROSIM */

#endif /* ifdef CONN_BUILDER_CONNGEN_H */
