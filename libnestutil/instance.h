/*
 *  instance.h
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

#ifndef INSTANCE_H
#define INSTANCE_H
#include "nest.h"
#include "allocator.h"


namespace nest
{
  /**
   * @addtogroup MemoryManagement Memory management
   * Classes which are involved in Memory management.
   */
                                                                               
  /**
   * @defgroup InstanceTemplate Instance template
   * The Instance template is used to extend normal classes with a pool based
   * memory management. It provides a framework of functions which allow the kernel
   * to efficiently create or delete nodes. Also functions for getting information
   * about the allocated/used memory are included. For example the function allocate
   * in an arbitrary template class can be written as
   * @code
   * template<typename ElementT>
   * inline Node* GenericModel<ElementT>::allocate(void)
   * {
   *   Node *n=new Instance<ElementT>;
   *   assert(n !=NULL);
   *   return n;
   * }
   * @endcode
   * to take advantage of the pool based allocator. Likewise the function reserve()
   * looks like
   * @code
   * template< typename ElementT>
   * inline void GenericModel<ElementT>::reserve(size_t s)
   * {
   *   Instance<ElementT>::reserve(s);
   * }
   * @endcode
   * @note
   * Probably the best way to see how Instance is used is to look into the code where
   * it is used. One Place for this is GenericModel-Template
   * @note   
   * J. M. Eppler, 2003-11-12, 02:49
   * @ingroup MemoryManagement
   */
  
  /**
   * Instance is a template is used to provide a pool-based management for arbitrary
   * classes.
   * @ingroup MemoryManagement
   * @ingroup InstanceTemplate
   */
  template<typename ClassT>
  class Instance: public ClassT
  {
  
  public:
    Instance()
      :ClassT(){}

    Instance(const Instance<ClassT>&c)
      :ClassT(c){}

    Instance(const ClassT &c)
      :ClassT(c){}

    ClassT *clone() const;
    
    static void reserve(size_t);

    static void * operator new(size_t size);
    static void   operator delete(void *p, size_t size);

    static size_t memory_used();
    static size_t memory_capacity();

    static size_t instantiations();
    
  private:
    static sli::pool memory_;
  };
  
  template<typename ClassT>
  inline
  ClassT * Instance<ClassT>::clone() const
  {
    return new Instance<ClassT>(*this);
  }

  template<typename ClassT>
  inline
  void * Instance<ClassT>::operator new(size_t size)
  {
    if(size != sizeof(Instance<ClassT>))
      {
	/**
	 * If a derived class does not possess its own
	 * new/delete operators, they will end up here.
	 * Thus, we forward their request to the standard
	 * new/delete operators.
	 */
	return ::operator new(size);
      }
    return memory_.alloc();
  }

  template< typename ClassT>
  inline
  void Instance<ClassT>::operator delete(void *p, size_t size)
  {
    if(p == NULL)
      return;
    
    if(size != sizeof(Instance<ClassT>))
      {
	/*
	 * If a derived class does not possess its own
	 * new/delete operators, they will end up here.
	 * Thus, we forward their request to the standard
	 * new/delete operators.
	 */
	::operator delete(p);
	return;
      }
    memory_.free(p);
  }

  template <typename ClassT>
  inline
  void Instance<ClassT>::reserve(size_t s)
  {
    memory_.reserve(s);
  }
  
  template <typename ClassT>
  inline
  size_t Instance<ClassT>::memory_used()
  {
    return memory_.get_instantiations() * memory_.get_el_size();
  }
  
  template <typename ClassT>
  inline
  size_t Instance<ClassT>::memory_capacity()
  {
    return memory_.get_total() * memory_.get_el_size();
  }

  template <typename ClassT>
  inline
  size_t Instance<ClassT>::instantiations()
  {
    return memory_.get_instantiations();
  }


  /** Declaration of static memory object */
  template <typename ClassT>
  sli::pool Instance<ClassT>::memory_(sizeof(Instance<ClassT>), 1024, 1);  
  
} // namespace nest
#endif









