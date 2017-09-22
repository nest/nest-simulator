/*
 *  nodelist.h
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

#ifndef NODELIST_H
#define NODELIST_H

// Includes from nestkernel:
#include "node.h"
#include "subnet.h"

namespace nest
{

/**
 * Template for list interface to network tree.
 *
 * LocalNodeListBase provides a template for classes providing
 * iterator interfaces to subnets. These iterators traverse only
 * local nodes.
 *
 * Concrete classes for local lists are created by instantiating
 * LocalNodeListBase with suitable iterator classes and specializing
 * the begin() method for those template instances.
 *
 * @note All iterators provided must either perform post-order iteration
 *       or specialize the end() method suitably, as the end() method
 *       provided by LocalNodeListBase assumes post-order traversal.
 *
 * @note The list iterators provided here are not thread-safe. If you need
 *       to use then in threaded code, you must make sure that (a) each
 *       thread has its own list and iterator instances and (b) that no two
 *       threads manipulate the same node simultaneously.
 */
template < typename ListIterator >
class LocalNodeListBase
{
public:
  typedef ListIterator iterator;

  explicit LocalNodeListBase( Subnet& subnet )
    : subnet_( subnet )
  {
  }

  /**
   * Return iterator pointing to first node in subnet.
   * @node Must be defined by all derived classes.
   */
  iterator begin() const;

  /**
   * Return iterator pointing to node past last node.
   *
   * @note Since traversal is post-order, the local_end() of the top-level
   *       subnet is the end also for Leaf and Child lists.
   */
  iterator
  end() const
  {
    return iterator( subnet_.local_end(), subnet_.local_end() );
  }

  //! Returns true if no local nodes
  bool
  empty() const
  {
    return subnet_.local_empty();
  }

  //! Returns subnet wrapped by NodeList
  Subnet&
  get_subnet() const
  {
    return subnet_;
  }

private:
  Subnet& subnet_; //!< root of the network
};


// ----------------------------------------------------------------------------

/**
 * Iterator for post-order traversal of all local nodes in a subnet.
 */
class LocalNodeListIterator
{
  friend class LocalNodeListBase< LocalNodeListIterator >;
  friend class LocalLeafListIterator;

private:
  //! Create iterator from pointer to Node in subnet
  LocalNodeListIterator( std::vector< Node* >::iterator const& node,
    std::vector< Node* >::iterator const& list_end )
    : current_node_( node )
    , list_end_( list_end )
  {
  }
  bool
  is_end_() const
  {
    return current_node_ == list_end_;
  }

public:
  LocalNodeListIterator operator++();

  Node* operator*()
  {
    return *current_node_;
  }
  Node const* operator*() const
  {
    return *current_node_;
  }

  bool operator==( const LocalNodeListIterator& i ) const
  {
    return current_node_ == i.current_node_;
  }
  bool operator!=( const LocalNodeListIterator& i ) const
  {
    return not( *this == i );
  }

private:
  //! iterator to the current node in subnet
  std::vector< Node* >::iterator current_node_;
  std::vector< Node* >::iterator list_end_;
};


// ----------------------------------------------------------------------------

template <>
LocalNodeListBase< LocalNodeListIterator >::iterator
LocalNodeListBase< LocalNodeListIterator >::begin() const;

/**
 * List interface to subnet providing iteration over all local nodes.
 */
typedef LocalNodeListBase< LocalNodeListIterator > LocalNodeList;

// ----------------------------------------------------------------------------

/**
 * Iterator for traversal of all local immediate child nodes in a subnet.
 */
class LocalChildListIterator
{
  friend class LocalNodeListBase< LocalChildListIterator >;

private:
  //! Create iterator from pointer to Node in subnet
  LocalChildListIterator( std::vector< Node* >::iterator const& node,
    std::vector< Node* >::iterator const& list_end )
    : current_node_( node )
    , list_end_( list_end )
  {
  }

public:
  LocalChildListIterator operator++();

  Node* operator*()
  {
    return *current_node_;
  }
  Node const* operator*() const
  {
    return *current_node_;
  }

  bool operator==( const LocalChildListIterator& i ) const
  {
    return current_node_ == i.current_node_;
  }
  bool operator!=( const LocalChildListIterator& i ) const
  {
    return not( *this == i );
  }

private:
  //! iterator to the current node in subnet
  std::vector< Node* >::iterator current_node_;
  std::vector< Node* >::iterator list_end_;
};

template <>
LocalNodeListBase< LocalChildListIterator >::iterator
LocalNodeListBase< LocalChildListIterator >::begin() const;

/**
 * List interface to subnet providing iteration over immediate local child
 * nodes.
 */
typedef LocalNodeListBase< LocalChildListIterator > LocalChildList;

// ----------------------------------------------------------------------------

/**
 * Iterator for traversal of only local leaf nodes in a subnet.
 * @note Leaf nodes are those children that are not subnets. Empty subnets
 *       are not considered leaves.
 */
class LocalLeafListIterator
{
  friend class LocalNodeListBase< LocalLeafListIterator >;

private:
  //! Create iterator from pointer to Node in subnet
  LocalLeafListIterator( std::vector< Node* >::iterator const& node,
    std::vector< Node* >::iterator const& list_end )
    : base_it_( node, list_end )
  {
    while ( not base_it_.is_end_() && not is_leaf_( *base_it_ ) )
    {
      ++base_it_;
    }
  }

public:
  LocalLeafListIterator operator++();

  Node* operator*()
  {
    return *base_it_;
  }
  Node const* operator*() const
  {
    return *base_it_;
  }

  bool operator==( const LocalLeafListIterator& i ) const
  {
    return base_it_ == i.base_it_;
  }
  bool operator!=( const LocalLeafListIterator& i ) const
  {
    return not( *this == i );
  }

private:
  LocalNodeListIterator base_it_; //<! we use this one for the basic iteration

  static bool
  is_leaf_( Node* n )
  {
    return not dynamic_cast< Subnet* >( n );
  }
};

template <>
LocalNodeListBase< LocalLeafListIterator >::iterator
LocalNodeListBase< LocalLeafListIterator >::begin() const;

/**
 * List interface to subnet providing iteration over local leaf nodes.
 * @note Leaf nodes are those children that are not subnets. Empty subnets
 *       are not considered leaves.
 */
typedef LocalNodeListBase< LocalLeafListIterator > LocalLeafList;
}
#endif
