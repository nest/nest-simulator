/*
 *  nodelist.cpp
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

#include "nodelist.h"

namespace nest{

  NodeList::iterator NodeList::begin() const
  {
    if (empty())
      return end();

    Compound *r=root_;
    vector<Node*>::iterator n;

    while( r!=NULL && !r->empty() )
    {
      n=r->begin(); //!< Move down in tree
      if( (r=dynamic_cast<Compound*>(*n)) == NULL)
	break;
    }
    /** We have reached the end of tree */
    return iterator(n);
  }



  /** 
   * NodeList::iterator::operator++()
   * Operator++ advances the iterator to the right neighbor
   * in a post-order tree traversal, including the non-leaf
   * nodes.
   *
   * The formulation is iterative. Maybe a recursive 
   * formulation would be faster, however, we will also
   * supply a chached-iterator, which does this work only once.
   */
    
  NodeList::iterator NodeList::iterator::operator++()
  {
    /**
     * We must assume that this operator is not called on
     * end(). For this case, the result is undefined!
     */

    /** This compound is the container to which e belongs!
     *  If c yields NULL, the tree is ill-formed!
     */
    Compound *c=(*p_)->get_parent();
    assert(c != NULL);

    /**
     * 1. Find the right neighbor
     * 2.   Traverse the left-most branch
     * 3.   return leaf of leftmost branch
     * 4. If no right neigbor exists, go up one level
     * 5.   return element.
     * 6. If we cannot go up, return end() of local compound
     */

    /** Goto right neighbor */
    ++p_;
    
    if(p_ != c->end())
    {
      Compound *r=dynamic_cast<Compound *>(*p_);

      while(r != NULL && ! r->empty())
      {
	p_=r->begin();
	r=dynamic_cast<Compound *>(*p_);
      }
      
      return *this;
    }
    
    
    /** This is the case where no right neighbor exists.
     * We have to go up and return the parent
     */
    Compound *p=c->get_parent();
    if(p==NULL)
    {
      /** We are already at the root container and
       * there is no right neighbor. Thus, we
       * have reached the end.
       */
      p_=c->end();
      return *this;
    }
    /**
     * We are at the end of a local container
     * thus, we have to ascend, so that we can proceed
     * with its right neigbor in the next round
     */

    /** 
     * Compute the iterator which points to c
     */
    p_= p->begin()+c->get_lid();
    return *this;
  }

  void NodeList::set_root(Compound &r)
  {
    root_=&r;
  }

  Compound & NodeList::get_root() const
  {
    assert(root_ != NULL);
    return *root_;
  }
}
	
	
	
