/*
 *  update_block.h
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

#ifndef UPDATE_BLOCK_H
#define UPDATE_BLOCK_H

// C++ includes:
#include <cassert>
#include <iterator>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace nest
{
class Node;
class Time;

/**
 * Neurons of one model on one thread that are updated by a single call.
 *
 * A model may keep the state of a block of its neurons in arrays, so that the update of the block runs as one loop
 * over contiguous memory. The neurons of a block are consecutive among the local nodes of their thread; the
 * simulation loop then calls update() of the block once per slice instead of Node::update() of each neuron
 * (see group_update_units()). Frozen neurons of a block are skipped by the block itself. A block belongs to the model
 * of its neurons, which creates it, and is used only by the thread of its neurons, so it needs no synchronization.
 */
class UpdateBlock
{
public:
  virtual ~UpdateBlock() = default;

  //! Update all neurons of the block for the steps from to to - 1 of the slice that begins at origin.
  virtual void update( const Time& origin, const long from, const long to ) = 0;
};

//! One unit of the update of a thread: a block (node is the first node of the block) or a single node (block nullptr).
template < typename NodeT >
using UpdateUnit = std::pair< UpdateBlock*, NodeT* >;

/**
 * Group the local nodes of a thread into the units of the update, in their order.
 *
 * Consecutive nodes for which block_of() returns the same block form one unit; every node without a block is a unit
 * of its own. The nodes of a block must be consecutive: a block that appeared in two units would be updated twice per
 * slice, which the assertion at the end excludes. A pure function over iterators to node pointers, so that the rule
 * can be tested without the kernel; the simulation loop uses it with Node and Node::get_update_block().
 */
template < typename Iterator, typename BlockOf >
auto
group_update_units( Iterator begin, const Iterator end, BlockOf block_of )
{
  using NodeT = std::remove_pointer_t< std::iter_value_t< Iterator > >;
  std::vector< UpdateUnit< NodeT > > units;
  for ( ; begin != end; ++begin )
  {
    NodeT* node = *begin;
    UpdateBlock* block = block_of( node );
    if ( block and not units.empty() and units.back().first == block )
    {
      continue;  // a further node of the block of the previous unit
    }
    units.emplace_back( block, node );
  }
#ifndef NDEBUG
  std::unordered_set< const UpdateBlock* > blocks;
  for ( const auto& [ block, node ] : units )
  {
    assert( not block or blocks.insert( block ).second );  // the nodes of a block are consecutive
  }
#endif
  return units;
}

}  // namespace nest

#endif /* #ifndef UPDATE_BLOCK_H */
