/*
 *  allocator.h
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

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

// Generated includes:
#include "config.h"

// C++ includes:
#include <cassert>
#include <cstdlib>
#include <string>

namespace sli
{

/**
 * @addtogroup MemoryManagement Memory management
 * Classes which are involved in Memory management.
 */

/**
 * @defgroup PoolAllocator Pool allocator
 * The pool allocator is specialized for creating many small identical objects.
 * @ingroup MemoryManagement
 */

/**
 * pool is a specialized allocator class for many identical small
 * objects. It targets a performance close to the optimal performance
 * which is achieved by allocating all needed objects at once.
 * @ingroup MemoryManagement
 * @ingroup PoolAllocator
 */

class pool
{
  struct link
  {
    link* next;
  };

  class chunk
  {
    const size_t csize;
    chunk( const chunk& );            //!< not implemented
    chunk& operator=( const chunk& ); //!< not implemented

  public:
    chunk* next;
    char* mem;

    chunk( size_t s )
      : csize( s )
      , next( 0 )
      , mem( new char[ csize ] )
    {
    }

    ~chunk()
    {
      delete[] mem;
      mem = NULL;
    }

    size_t
    size( void )
    {
      return csize;
    }
  };

  size_t initial_block_size;
  size_t growth_factor;

  size_t block_size;     //!< number of elements per chunk
  size_t el_size;        //!< sizeof an element
  size_t instantiations; //!< number of instantiated elements
  size_t total;          //!< total number of allocated elements
  size_t capacity;       //!< number of free elements
  chunk* chunks;         //!< linked list of memory chunks
  link* head;            //!< head of free list

  bool initialized_; //!< True if the pool is initialized.

  void grow( size_t ); //!< make pool larger by n elements
  void grow();         //!< make pool larger


public:
  /** Create pool for objects of size n. Initial is the initial allocation
   *  block size, i.e. the number of objects per block.
   *  growth is the factor by which the allocations block increases after
   *  each growth.
   */
  pool();
  pool( const pool& );
  pool& operator=( const pool& );

  pool( size_t n, size_t initial = 100, size_t growth = 1 );
  void init( size_t n, size_t initial = 100, size_t growth = 1 );

  ~pool(); //!< deallocate ALL memory

  /**
    Increase the pools capacity (free slots) by n.

        reserve_additional() ensures that the pool has at least n empty slots,
        i.e., that the pool can store at least n additional elements
        before more memory needs to be allocated from the operating
        system.
  */
  void reserve_additional( size_t n );

  size_t
  available( void ) const
  {
    return total - instantiations;
  }

  inline void* alloc( void );  //!< allocate one element
  inline void free( void* p ); //!< put element back into the pool
  size_t
  size_of( void ) const
  {
    return el_size;
  }

  inline size_t get_el_size() const;
  inline size_t get_instantiations() const;
  inline size_t get_total() const;
};

inline void*
pool::alloc( void )
{

  if ( head == 0 )
  {
    grow( block_size );
    block_size *= growth_factor;
  }

  link* p = head;

  head = head->next;
  ++instantiations;

  return p;
}

inline void
pool::free( void* elp )
{
  link* p = static_cast< link* >( elp );
  p->next = head;
  head = p;
  --instantiations;
}

inline size_t
pool::get_el_size() const
{
  return el_size;
}

inline size_t
pool::get_instantiations() const
{
  return instantiations;
}

inline size_t
pool::get_total() const
{
  return total;
}
}

#ifdef USE_PMA

const int MAX_THREAD = 128;

/**
 * The poor man's allocator is a simple pool-based allocator, used to
 * allocate storage for connections in the limit of large machines.
 *
 * The allocator only supports allocation, but no freeing.  In the
 * limit of large machines this is sufficient, because we rarely need
 * to grow synapse lists, as the majority of neurons have at most one
 * target per machine.
 * The allocator manages the pool of free memory in chunks that form a
 * linked list.  A head pointer keeps track of the next position in a
 * chunk to be handed to the caller of allocate. Once head reaches the
 * end of the current chunk, a new chunk is allocated from the OS and
 * appends it to the linked list of chunks.
 */
class PoorMansAllocator
{
private:
  /**
   * A chunk of memory, one element in the linked list of the memory
   * pool.
   */
  struct chunk
  {
    chunk( char* mem, chunk* next )
      : mem_( mem )
      , next_( next ){};
    char* mem_;
    chunk* next_;
  };

public:
  /**
   * No constructors, as this would be counted as a 'use' of the
   * pool before declaring it thread-private by the compiler.
   * Therefore we have our own init() and destruct() functions.
   */
  void init( size_t chunk_size = 1048576 );
  void destruct();
  void* alloc( size_t obj_size );

private:
  /**
   * Append a new chunk of memory to the list forming the memory
   * pool.
   */
  void new_chunk();

  /**
   * The size of each chunk to be allocated. This size must be
   * chosen as a tradeoff between few allocations of chunks (large
   * chunk) and little memory overhead due to unused chunks (small
   * chunks).
   */
  size_t chunk_size_;

  /**
   * Points to the next free location in the current chunk. This
   * location will be handed to the caller in the next call of
   * allocate. The type is char*, so that pointer arithmetic allows
   * adding sizeof(object) directly to advance the pointer to the
   * next free location.
   */
  char* head_;

  /**
   * First element of the linked list of chunks.
   */
  chunk* chunks_;

  /**
   * Remaining capacity of the current chunk.
   */
  size_t capacity_;
};

#ifdef IS_K
/**
 * The Fujitsu compiler on K cannot handle OpenMP thread-private
 * properly. We therefore need to emulate the thread-private storage
 * by padding the allocator objects to the size of a cache line so
 * that the instantiations for different threads lie on different
 * cache lines.
 */
class PaddedPMA : public PoorMansAllocator
{
  // Only works for sizeof(PoorMansAllocator) < 64
  char padding[ 64 - sizeof( PoorMansAllocator ) ];
};
#endif /* #ifdef IS_K */

#endif /* #ifdef USE_PMA */

#endif /* #ifndef ALLOCATOR_H */
