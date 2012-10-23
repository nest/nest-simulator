#ifndef SELECTOR_H
#define SELECTOR_H

/*
 *  selector.h
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

/*
  This file is part of the NEST topology module.
  Author: Kittel Austvoll

*/

#include "nest.h"
#include "dictdatum.h"
#include "compound.h"

/** @file selector.h
 *  Implements the slicing functionality.
 */

namespace nest{

  /**
   * The Selector class extracts nodes from a node or a compound 
   * structure. The nodes are extracted based upon the entries set
   * in the layer slice dictionaries. Nodes can be extracted based
   * upon modeltype and subnet depth.
   */
  class Selector{
  public:
    /**
     * Create an empty Selector.
     */
    Selector();

    /**
     * Create a Selector based upon an input dictionary.
     * @param dictionary with slicing information
     */
    Selector(const DictionaryDatum&);
    
    /**
     * @param subnet Compound where nodes extracted by slicing are stored
     * @param node   Layer node that will be sliced
     */
    void slice_node(Compound& subnet, Node* node);
    
  private:
    // Class variables. slice_depth_ of 0 indicates that all depths
    // should be connected to. modeltype_ of -1 indicates that all
    // modeltypes should be connected.
    index slice_depth_;
    long_t modeltype_;
    
    /**
     * Recursive function called by slice_node(Compound&, Node*).
     *
     * @param subnet Compound where nodes extracted by slicing are stored
     * @param node   Node that will be sliced
     * @param slice_depth 
     */
    void slice_node(Compound& subnet, Node* node,
		    index slice_depth);

  };

}

#endif

