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
 * neighbouring cores (e.g. OMP_PROC_BIND=close), the counters of the lowest level are shared within a core complex or
 * socket, so that the cache lines which every arriving thread has to modify do not travel across the chip, in contrast
 * to a central counter. All state is padded to cache lines.
 *
 * Every thread of the team must call wait() with its own thread number for every barrier. The barrier is meant for
 * teams that are not oversubscribed; after a number of spins a waiting thread yields the processor.
 */
class TreeBarrier
{
public:
  //! Barrier for no threads; initialize() prepares it for use.
  TreeBarrier()
    : num_threads_( 0 )
    , fan_in_( 8 )
    , level_nodes_()
    , level_offset_()
    , counters_()
    , local_sense_()
    , global_sense_()
  {
  }

  //! Prepare the barrier for num_threads threads; not thread safe.
  void
  initialize( const size_t num_threads, const size_t fan_in )
  {
    num_threads_ = num_threads;
    fan_in_ = fan_in < 2 ? 2 : fan_in;
    level_nodes_.clear();
    level_offset_.clear();
    size_t nodes_in_level = num_threads_;
    size_t offset = 0;
    while ( nodes_in_level > 1 )
    {
      const size_t groups = ( nodes_in_level + fan_in_ - 1 ) / fan_in_;
      level_nodes_.push_back( groups );
      level_offset_.push_back( offset );
      offset += groups;
      nodes_in_level = groups;
    }
    counters_ = std::vector< Padded >( offset > 0 ? offset : 1 );
    local_sense_ = std::vector< Padded >( num_threads_ );
    for ( auto& c : counters_ )
    {
      c.value.store( 0, std::memory_order_relaxed );
    }
    for ( auto& s : local_sense_ )
    {
      s.value.store( 0, std::memory_order_relaxed );
    }
    global_sense_.value.store( 0, std::memory_order_relaxed );
  }

  //! Wait until all threads have arrived.
  void
  wait( const size_t tid )
  {
    const int sense = local_sense_[ tid ].value.load( std::memory_order_relaxed ) ^ 1;
    local_sense_[ tid ].value.store( sense, std::memory_order_relaxed );

    size_t index = tid;
    size_t below = num_threads_;
    bool last = true;
    for ( size_t level = 0; level < level_nodes_.size(); ++level )
    {
      const size_t group = index / fan_in_;
      const size_t group_size = std::min( fan_in_, below - group * fan_in_ );
      Padded& counter = counters_[ level_offset_[ level ] + group ];
      if ( counter.value.fetch_add( 1, std::memory_order_acq_rel ) != static_cast< int >( group_size ) - 1 )
      {
        last = false;
        break;
      }
      counter.value.store( 0, std::memory_order_relaxed );
      index = group;
      below = level_nodes_[ level ];
    }

    if ( last )
    {
      global_sense_.value.store( sense, std::memory_order_release );
      return;
    }
    size_t spins = 0;
    while ( global_sense_.value.load( std::memory_order_acquire ) != sense )
    {
      pause_();
      if ( ++spins > 20000 )
      {
        std::this_thread::yield();
        spins = 0;
      }
    }
  }

private:
  /**
   * Counter or flag of the barrier on a cache line of its own.
   *
   * Atomic, because several threads modify it concurrently (counters) or one thread writes it while the others
   * spin on it (flags); its own cache line keeps the spinning threads from invalidating the lines of other counters.
   */
  struct alignas( 64 ) Padded
  {
    Padded()
      : value( 0 )
    {
    }

    std::atomic< int > value;
  };

  static inline void
  pause_()
  {
#if defined( __x86_64__ ) || defined( __i386__ )
    __builtin_ia32_pause();
#elif defined( __aarch64__ )
    asm volatile( "yield" ::: "memory" );
#endif
  }

  size_t num_threads_;
  size_t fan_in_;
  std::vector< size_t > level_nodes_;   //!< number of groups per level
  std::vector< size_t > level_offset_;  //!< index of the first counter of the level
  std::vector< Padded > counters_;
  std::vector< Padded > local_sense_;
  Padded global_sense_;
};

}  // namespace nest

#endif /* TREE_BARRIER_H */
