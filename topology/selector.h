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

#ifndef SELECTOR_H
#define SELECTOR_H

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest {

/**
 * Contains rules for selecting nodes from a layer when connecting. Users
 * may select by model or by depth.
 */
struct Selector {
  /**
   * The default constructor creates a Selector with no specific rules,
   * so all nodes are selected.
   */
  Selector()
    : model( -1 )
    , depth( -1 )
  {
  }
  /**
   * The dictionary used to initialize the Selector may contain
   * parameters 'model', which is the name of a model to select by,
   * and/or 'lid', which is the depth to select by (an integer >= 1).
   * @param d Dictionary containing parameters for the Selector.
   */
  Selector( const DictionaryDatum& d );
  /**
   * @returns true if this Selector selects by model
   */
  bool
  select_model() const
  {
    return model >= 0;
  }
  /**
   * @returns true if this Selector selects by depth
   */
  bool
  select_depth() const
  {
    return depth >= 0;
  }
  /**
   * Test if two selectors are equal, i.e. contain the same rules.
   * @returns true if both selectors are equal.
   */
  bool operator==( const Selector& other )
  {
    return ( other.model == model ) and ( other.depth == depth );
  }
  /**
   * The model to select, or -1 if all models are allowed.
   */
  long model;
  /**
   * The depth to select, or -1 if all depths are allowed.
   */
  long depth;
};

} // namespace nest

#endif
