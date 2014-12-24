/*
 *  sibling_container.h
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

#ifndef SIBLING_CONTAINER_H
#define SIBLING_CONTAINER_H

#include <vector>
#include <string>
#include "node.h"
#include "dictdatum.h"

namespace nest{

  using std::vector;

  class Node;
  class Network;
  class Scheduler;

  /**
   * SiblingContainer class.
   * This class is used to group the replicas of nodes on different
   * threads into one entity. It is derived from node in order to take
   * advantage of the pool allocator, which has only very little
   * overhead compared to a normal std::vector.
   */
  class SiblingContainer: public Node
  {
    friend class Network;
    friend class Scheduler;
    friend class Subnet;

  public:

    SiblingContainer();   

    SiblingContainer(const SiblingContainer &);

    virtual ~SiblingContainer(){}
   
    void set_status(const DictionaryDatum&) { assert(false); }
    void get_status(DictionaryDatum&) const { assert(false); }

    bool is_subnet() const;

    bool has_proxies() const;
          
    bool   empty() const;
    void   reserve(size_t);

    void push_back(Node*);
    
    /**
     * Return iterator to the first child node.
     */
    vector<Node*>::iterator begin();

    /**
     * Return iterator to the end of the child-list.
     */
    vector<Node*>::iterator end();

    /**
     * Return const iterator to the first child node.
     */
    vector<Node*>::const_iterator begin() const;

    /**
     * Return const iterator to the end of the child-list.
     */
    vector<Node*>::const_iterator end() const;

  protected:
    size_t num_thread_siblings_() const;
    Node* get_thread_sibling_(index) const;
    Node* get_thread_sibling_safe_(index) const;
    void init_node_(const Node&) {}
    void init_state_(const Node&) {}
    void init_buffers_() {}

    void calibrate() {}
    void update(Time const &, const long_t, const long_t) {}
    
    /**
     * Pointer to child nodes.
     * This vector contains the pointers to the child nodes.
     * Since deletion of Nodes is possible, entries in this
     * vector may be NULL. Note that all code must handle
     * this case gracefully.
     */
    vector<Node *> nodes_;       //!< Pointer to child nodes.
  };


  inline
  void SiblingContainer::push_back(Node *n)
  {
    nodes_.push_back(n);
  }  
  
  inline
  Node* SiblingContainer::get_thread_sibling_safe_(index i) const
  {
    return nodes_.at(i); //with range check
  }

  inline
  Node* SiblingContainer::get_thread_sibling_(index i) const
  {
    return nodes_[i]; //without range check
  }

  inline
  vector<Node*>::iterator SiblingContainer::begin()
  {
    return nodes_.begin();
  }

  inline
  vector<Node*>::iterator SiblingContainer::end()
  {
    return nodes_.end();
  }

  inline
  vector<Node*>::const_iterator SiblingContainer::begin() const
  {
    return nodes_.begin();
  }

  inline
  vector<Node*>::const_iterator SiblingContainer::end() const
  {
    return nodes_.end();
  }

  inline
  bool SiblingContainer::empty() const
  {
    return nodes_.empty();
  }

  inline
  size_t SiblingContainer::num_thread_siblings_() const
  {
    return nodes_.size();
  }

  inline 
  void SiblingContainer::reserve(size_t n)
  {
    nodes_.reserve(n);
  }

  inline
  bool SiblingContainer::has_proxies() const
  {
    return false;
  }

  inline
  bool SiblingContainer::is_subnet() const
  {
	 return empty() ? false : nodes_[0]->is_subnet();
  }

} // namespace

#endif
