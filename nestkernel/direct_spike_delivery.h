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
#include <vector>

namespace nest
{

/**
 * Address of the spike input buffer of a node, see Node::get_direct_spike_input().
 *
 * The buffer is a ring of slots; slot s starts at `slot0 + stride * s` and holds one double per
 * input channel. A spike with weight w is added to the excitatory channel if w > 0 and to the
 * inhibitory channel otherwise. A default-constructed address describes a node that does not
 * support direct delivery; its spikes take the regular path through handle( SpikeEvent& ).
 */
class DirectSpikeInputAddr
{
public:
  //! Address of a node that does not support direct delivery.
  DirectSpikeInputAddr()
    : slot0_( nullptr )
    , stride_( 0 )
    , excitatory_channel_( 0 )
    , inhibitory_channel_( 0 )
  {
  }

  /**
   * Address of the input buffer `buffer` of a node.
   *
   * BufferT provides slot_data() and slot_stride, as MultiChannelInputBuffer does. The address is
   * valid until the buffer is resized, that is until the next call to prepare().
   */
  template < typename BufferT >
  DirectSpikeInputAddr( BufferT& buffer,
    const unsigned char excitatory_channel,
    const unsigned char inhibitory_channel )
    : slot0_( buffer.slot_data() )
    , stride_( BufferT::slot_stride )
    , excitatory_channel_( excitatory_channel )
    , inhibitory_channel_( inhibitory_channel )
  {
    static_assert( BufferT::slot_stride < 256, "stride must fit into an unsigned char" );
  }

  //! Whether spikes can be added directly to the buffer.
  bool
  is_supported() const
  {
    return slot0_ != nullptr;
  }

  //! Add weight w to the channel of its sign in the given slot of the ring.
  void
  add( const long slot, const double w ) const
  {
    slot0_[ stride_ * slot + ( w > 0 ? excitatory_channel_ : inhibitory_channel_ ) ] += w;
  }

private:
  double* slot0_;                     //!< first double of slot 0; nullptr if direct delivery is not supported
  unsigned char stride_;              //!< doubles per slot
  unsigned char excitatory_channel_;  //!< channel for spikes with positive weight
  unsigned char inhibitory_channel_;  //!< channel for spikes with weight <= 0
};

/**
 * Table of the input-buffer addresses of the targets of one Connector.
 *
 * The table is indexed by the thread-local node index stored in hpc connections
 * (TargetIdentifierIndex). Entries are resolved lazily, on the first spike delivered to a target
 * after the table was cleared. The table only has as many entries as the largest target index
 * seen, i.e., it is proportional to the number of nodes on the thread, not to the number of
 * connections. It is owned by a Connector and therefore only accessed by a single thread.
 *
 * SimulationManager::prepare() clears all tables (ConnectionManager::clear_direct_spike_tables()),
 * because prepare() is the only place where input buffers may be reallocated.
 */
class DirectSpikeTable
{
public:
  //! Discard all entries; the memory is kept for the next simulation.
  void
  clear()
  {
    entries_.clear();
  }

  /**
   * Address of the input buffer of the target of connection `conn`.
   *
   * Resolves the entry if needed: the target node is only accessed on the first spike to it.
   */
  template < typename ConnectionT >
  const DirectSpikeInputAddr&
  get( const ConnectionT& conn, const size_t tid )
  {
    const size_t idx = conn.get_target_index();
    if ( idx >= entries_.size() )
    {
      entries_.resize( idx + 1 );
    }
    Entry& entry = entries_[ idx ];
    if ( not entry.resolved )
    {
      entry.addr = conn.get_target( tid )->get_direct_spike_input();
      entry.resolved = true;
    }
    return entry.addr;
  }

private:
  //! Address of one target and whether it has been resolved since the last clear().
  struct Entry
  {
    Entry()
      : addr()
      , resolved( false )
    {
    }

    DirectSpikeInputAddr addr;
    bool resolved;
  };

  std::vector< Entry > entries_;
};

}  // namespace nest

#endif /* #ifndef DIRECT_SPIKE_DELIVERY_H */
