/*
 *  tree_barrier.h
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

#ifndef TREE_BARRIER_H
#define TREE_BARRIER_H

// C++ includes:
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <thread>
#include <vector>

namespace nest
{

/**
 * Spinning barrier for the threads of the update loop.
 *
 * A combining tree serves the arrival of the threads and one global flag their release. The threads are divided into
 * groups of fan_in consecutive thread numbers, the groups into groups of groups, and so on. A thread increments the
 * counter of its group; the last thread of a group continues with the counter of the next level; the last thread of the
 * root flips the global sense flag on which all other threads are spinning. If consecutive thread numbers are pinned to
 * neighboring cores (e.g. OMP_PROC_BIND=close), the counters of the lowest level are shared within a core complex or
 * socket, so that the cache lines which every arriving thread has to modify do not travel across the chip, in contrast
 * to a central counter. All state is padded to cache lines.
 *
 * The threads can further be divided into groups of consecutive threads, given by their boundaries (the groups of the
 * collocation of the spikes, see EventDeliveryManager::get_collocation_group_bounds()). Each group has a tree
 * of its own; the last thread of a group arrives at a counter of the groups. The variant of wait() with steps uses this
 * to replace the two barriers of a cycle by one: the last thread of a group to arrive executes the step of the group,
 * the master thread waits until all groups are complete and executes the step of the master, and only then are the
 * threads released. The master step stays on the master thread, as MPI requires.
 *
 * Every thread of the team must call wait() with its own thread number for every barrier. The barrier is meant for
 * teams that are not oversubscribed; after a number of spins a waiting thread yields the processor.
 */
class TreeBarrier
{
public:
  /**
   * Prepare the barrier; not thread safe.
   *
   * Group g consists of the threads [ group_bounds[ g ], group_bounds[ g + 1 ] ); the last entry is the number of
   * threads.
   */
  void initialize( const size_t fan_in, const std::vector< size_t >& group_bounds );

  //! Wait until all threads have arrived.
  void wait( const size_t tid );

  /**
   * Wait until all threads have arrived and the steps are done.
   *
   * The last thread of group g to arrive calls on_group( g ), the master thread calls on_root() after all groups have
   * completed their steps; no thread leaves before on_root() has returned.
   */
  template < typename OnGroup, typename OnRoot >
  void wait( const size_t tid, OnGroup&& on_group, OnRoot&& on_root );

private:
  /**
   * Counter or flag of the barrier on a cache line of its own.
   *
   * Atomic, because several threads modify it concurrently (counters) or one thread writes it while the others
   * spin on it (flags); its own cache line keeps the spinning threads from invalidating the lines of other counters.
   * The 64 bytes are the cache line size of the server processors NEST is run on; on a processor with larger
   * cache lines, two counters could share a line and the padding would not separate them.
   */
  struct alignas( 64 ) Padded_
  {
    std::atomic< int > value_ = 0;
  };

  //! Tree of the arrival within one group of threads.
  struct Group_
  {
    size_t first_ = 0;                    //!< first thread of the group
    size_t size_ = 0;                     //!< number of threads of the group
    std::vector< size_t > level_nodes_;   //!< number of nodes per level
    std::vector< size_t > level_offset_;  //!< index of the first counter of the level
  };

  //! Hint to the processor that the thread is spinning.
  static void pause_();

  //! Spin until the flag has the given value.
  static void spin_( const Padded_& flag, const int value );

  //! Flip the sense of thread tid and return the new sense.
  int flip_sense_( const size_t tid );

  //! Arrival of thread tid in the tree of its group; true if it is the last thread of the group.
  bool arrive_in_group_( const size_t tid );

  size_t num_threads_ = 0;
  size_t fan_in_ = 8;
  size_t num_groups_ = 1;
  std::vector< size_t > group_of_;  //!< group of each thread
  std::vector< Group_ > groups_;
  std::vector< Padded_ > counters_;
  std::vector< Padded_ > local_sense_;
  Padded_ groups_arrived_;
  Padded_ global_sense_;
};

inline void
TreeBarrier::initialize( const size_t fan_in, const std::vector< size_t >& group_bounds )
{
  // every group has at least one thread, otherwise no thread would complete it
  assert( group_bounds.size() >= 2 and group_bounds.front() == 0
    and std::adjacent_find(
          group_bounds.begin(), group_bounds.end(), []( const size_t a, const size_t b ) { return a >= b; } )
      == group_bounds.end() );
  num_threads_ = group_bounds.back();
  fan_in_ = fan_in < 2 ? 2 : fan_in;
  num_groups_ = group_bounds.size() - 1;
  group_of_.assign( num_threads_, 0 );
  groups_.assign( num_groups_, Group_() );
  size_t offset = 0;
  for ( size_t g = 0; g < num_groups_; ++g )
  {
    Group_& group = groups_[ g ];
    group.first_ = group_bounds[ g ];
    group.size_ = group_bounds[ g + 1 ] - group.first_;
    std::fill_n( group_of_.begin() + group.first_, group.size_, g );
    size_t nodes_in_level = group.size_;
    while ( nodes_in_level > 1 )
    {
      const size_t nodes = ( nodes_in_level + fan_in_ - 1 ) / fan_in_;
      group.level_nodes_.push_back( nodes );
      group.level_offset_.push_back( offset );
      offset += nodes;
      nodes_in_level = nodes;
    }
  }
  counters_ = std::vector< Padded_ >( std::max< size_t >( offset, 1 ) );
  local_sense_ = std::vector< Padded_ >( num_threads_ );
  groups_arrived_.value_.store( 0, std::memory_order_relaxed );
  global_sense_.value_.store( 0, std::memory_order_relaxed );
}

inline int
TreeBarrier::flip_sense_( const size_t tid )
{
  const int sense = local_sense_[ tid ].value_.load( std::memory_order_relaxed ) ^ 1;
  local_sense_[ tid ].value_.store( sense, std::memory_order_relaxed );
  return sense;
}

inline bool
TreeBarrier::arrive_in_group_( const size_t tid )
{
  const Group_& group = groups_[ group_of_[ tid ] ];
  size_t index = tid - group.first_;
  size_t below = group.size_;
  for ( size_t level = 0; level < group.level_nodes_.size(); ++level )
  {
    const size_t node = index / fan_in_;
    const size_t node_size = std::min( fan_in_, below - node * fan_in_ );
    Padded_& counter = counters_[ group.level_offset_[ level ] + node ];
    if ( counter.value_.fetch_add( 1, std::memory_order_acq_rel ) != static_cast< int >( node_size ) - 1 )
    {
      return false;
    }
    counter.value_.store( 0, std::memory_order_relaxed );
    index = node;
    below = group.level_nodes_[ level ];
  }
  return true;
}

inline void
TreeBarrier::wait( const size_t tid )
{
  const int sense = flip_sense_( tid );
  // The last thread of the last group to arrive releases all threads.
  if ( arrive_in_group_( tid )
    and ( num_groups_ == 1
      or groups_arrived_.value_.fetch_add( 1, std::memory_order_acq_rel ) == static_cast< int >( num_groups_ ) - 1 ) )
  {
    groups_arrived_.value_.store( 0, std::memory_order_relaxed );
    global_sense_.value_.store( sense, std::memory_order_release );
    return;
  }
  spin_( global_sense_, sense );
}

template < typename OnGroup, typename OnRoot >
inline void
TreeBarrier::wait( const size_t tid, OnGroup&& on_group, OnRoot&& on_root )
{
  const int sense = flip_sense_( tid );
  if ( arrive_in_group_( tid ) )
  {
    on_group( group_of_[ tid ] );
    groups_arrived_.value_.fetch_add( 1, std::memory_order_acq_rel );
  }
  if ( tid == 0 )
  {
    spin_( groups_arrived_, static_cast< int >( num_groups_ ) );
    groups_arrived_.value_.store( 0, std::memory_order_relaxed );
    on_root();
    global_sense_.value_.store( sense, std::memory_order_release );
    return;
  }
  spin_( global_sense_, sense );
}

inline void
TreeBarrier::spin_( const Padded_& flag, const int value )
{
  size_t spins = 0;
  while ( flag.value_.load( std::memory_order_acquire ) != value )
  {
    pause_();
    if ( ++spins > 20000 )
    {
      std::this_thread::yield();
      spins = 0;
    }
  }
}

inline void
TreeBarrier::pause_()
{
#if defined( __x86_64__ ) || defined( __i386__ )
  __builtin_ia32_pause();
#elif defined( __aarch64__ )
  asm volatile( "yield" ::: "memory" );
#endif
}

}  // namespace nest

#endif /* TREE_BARRIER_H */
