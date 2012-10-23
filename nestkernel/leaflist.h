/*
 *  leaflist.h
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

#ifndef LEAFLIST_H
#define LEAFLIST_H

#include "nodelist.h"

namespace nest{

  /** 
   * List interface to a network tree's leaves.
   * Class LeafList is an adaptor class which turns a Network object
   * into a list.
   * Class LeafList provides an iterator which can be used to traverse
   * the leaves of the network tree in post-order.
   * Note that this is also the standard NEST counting order for
   * traversing multidimensional subnets.
   * Only leavess are returned, not the non-leaf-nodes.
   * For a list interface that also accesses the intermediate
   * compounds, see class NodeList and its iterator.
   */

  class LeafList: public NodeList
  {
  public:


    class iterator: public NodeList::iterator
    {
      friend class LeafList;
      
    public:
      iterator(): NodeList::iterator(), the_container_(LeafList()) {}
    private:
      iterator(NodeList::iterator const &p, LeafList const &c): NodeList::iterator(p), the_container_(c) {}

    public:
      iterator operator++();

      // next two inherited from NodeList::iterator
      //      Node*        operator*();
      //      Node const*  operator*() const;

      // next two inherited from NodeList::iterator
      //      bool operator==(iterator const &) const;
      //      bool operator!=(iterator const &) const;

    private:
      LeafList const& the_container_; //!< a reference to "our" container.
    };


    // otherwise LeafList::is_leaf_() in
    // LeafList::iterator::operator++() is not accessible.
    // Discovered with Alpha cxx. 
    // 24.4.03, Diesmann
    //
    friend class iterator;

    LeafList(): NodeList() {}
    explicit LeafList(Compound &c): NodeList(c) {};

    iterator begin()  const; //!< iterator to the first leaf
    iterator end()    const;

    bool   empty()   const; //!< true, if network tree contains no leaves
    size_t size()    const; //!< number of leaves in the network tree

    // next two inherited from NodeList
    // Compound& get_root() const;
    // void set_root(Compound &);

  private:
    /** Return true if Element is a leaf, false otherwise.
     */
    static bool is_leaf_(Node const *); 
  };


  inline
    bool LeafList::is_leaf_(Node const * n)
    {
      //check if it is derived from Compound:
      return dynamic_cast<Compound const *>(n) == NULL; //yes, its no compund
    }

  inline
    LeafList::iterator LeafList::end() const
    {
      return iterator(NodeList::end(), *this);
    }
  
}
#endif
