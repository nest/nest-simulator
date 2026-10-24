/*
 *  direct_spike_delivery.h
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


#ifndef DIRECT_SPIKE_DELIVERY_H
#define DIRECT_SPIKE_DELIVERY_H

// C++ includes:
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nest
{

/**
 * Description of the spike input buffer of a node, see Node::get_direct_spike_input().
 *
 * The buffer is a ring of slots; slot s starts at `slot0 + stride * s` and holds one double per
 * input channel. A spike with weight w is added to channel `excitatory_channel` if w > 0 and to
 * `inhibitory_channel` otherwise.
 */
struct DirectSpikeInput
{
  double* slot0 = nullptr;  //!< first double of slot 0; nullptr if the node does not support direct delivery
  uint32_t stride = 0;      //!< doubles per slot; 0 marks a table entry that has not been resolved yet
  uint8_t excitatory_channel = 0;
  uint8_t inhibitory_channel = 0;
};

/**
 * Table of the spike input buffers of the targets of one Connector, indexed by the thread-local
 * node index stored in hpc connections (TargetIdentifierIndex).
 *
 * Entries are resolved lazily, on the first spike delivered to a target after the table was
 * invalidated. The table only has as many entries as the largest target index seen, i.e., it is
 * proportional to the number of nodes on the thread, not to the number of connections.
 * It is owned by a Connector and therefore only accessed by a single thread.
 *
 * All entries are discarded when the generation counter of the EventDeliveryManager changes, which
 * happens in SimulationManager::prepare(), the only place where input buffers may be reallocated.
 */
class DirectSpikeTable
{
public:
  //! Discard all entries if the generation has changed.
  void
  validate( const size_t generation )
  {
    if ( generation != generation_ )
    {
      entries_.assign( entries_.size(), DirectSpikeInput() );
      generation_ = generation;
    }
  }

  /**
   * Entry for the target with thread-local index idx. `node_of` is called to obtain the node if the
   * entry is not yet resolved, it returns the node, whose get_direct_spike_input() is queried.
   */
  template < typename NodeOf >
  const DirectSpikeInput&
  get( const size_t idx, NodeOf&& node_of )
  {
    if ( idx >= entries_.size() )
    {
      entries_.resize( idx + 1 );
    }
    DirectSpikeInput& entry = entries_[ idx ];
    if ( entry.stride == 0 )
    {
      entry = node_of()->get_direct_spike_input();
      if ( entry.slot0 == nullptr )
      {
        entry.stride = 1;  // resolved: node uses the regular path
      }
    }
    return entry;
  }

private:
  std::vector< DirectSpikeInput > entries_;
  size_t generation_ = static_cast< size_t >( -1 );
};

}  // namespace nest

#endif /* #ifndef DIRECT_SPIKE_DELIVERY_H */
