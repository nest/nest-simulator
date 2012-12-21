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
#include <cassert>
#include <cstdlib>
#include <string>

namespace sli {

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
    struct link { link *next; };

    class chunk 
    {
      const size_t csize;
      chunk(const chunk&);            //!< not implemented
      chunk& operator=(const chunk&); //!< not implemented

     public:
      chunk *next;
      char  *mem;

      chunk(size_t s)
	:csize(s),
	 mem(new char[csize])
	{}

      ~chunk()
	{ 
	  delete [] mem;
	  mem=NULL;
	}

      size_t size(void)
	{ return csize;}

    };

    size_t initial_block_size;
    size_t growth_factor;

    size_t block_size;     //!< number of elements per chunk
    size_t el_size;        //!< sizeof an element
    size_t instantiations; //!< number of instatiated elements
    size_t total;          //!< total number of allocated elements
    size_t capacity;       //!< number of free elements
    chunk *chunks;         //!< linked list of memory chunks
    link  *head;           //!< head of free list

    bool  initialized_;    //!< True if the pool is initialized.

    void grow(size_t);     //!< make pool larger by n elements
    void grow();           //!< make pool larger

    
   public:
    /** Create pool for objects of size n. Initial is the inital allocation 
     *  block size, i.e. the number of objects per block.
     *  growth is the factor by which the allocations block increases after
     *  each growth.
     */
    pool();
    pool(const pool &);
    pool& operator=(const pool&);

    pool(size_t n, size_t initial=100, size_t growth=1); 
    void init(size_t n, size_t initial=100, size_t growth=1); 

    ~pool();               //!< deallocate ALL memory
    
    /** Increase the pools capacity (free slots) to at least n.
	Reserve() ensures that the pool has at least n empty slots,
	i.e., that the pool can store at least n additional elements
	before more memory needs to be allocated from the operating
	system.  
        @note The semantics of pool::reserve(n) differ from the semantics
	of reserve(n) for STL containers: for STL containers, n is the total
        number of elements after the reserve() call, while for pool it is the
        number of @b free @b elements.
	@todo Adapt the semantics of capacity() and reserve() to STL semantics.
    */
    void reserve(size_t n);

    size_t available(void) const
      { return total-instantiations;}

    inline void *alloc(void);     //!< allocate one element
    inline void free(void* p);    //!< put element back into the pool
    size_t size_of(void) const
      { return el_size;}

    inline size_t get_el_size() const;
    inline size_t get_instantiations() const;
    inline size_t get_total() const;
  };

  inline
  void * pool::alloc(void)
  {

    if(head==0)
    {
      grow(block_size);
      block_size *= growth_factor;
    }

    link *p=head;

    head = head->next;
    ++instantiations;
    
    return p;
  }

  inline
  void pool::free(void *elp)
  {
    link *p= static_cast<link *>(elp);
    p->next= head;
    head = p;
    --instantiations;
  }

  inline
  size_t pool::get_el_size() const
  {
    return el_size;
  }

  inline
  size_t pool::get_instantiations() const
  {
    return instantiations;
  }
 
  inline
  size_t pool::get_total() const
  {
    return total;
  }

}
#endif
