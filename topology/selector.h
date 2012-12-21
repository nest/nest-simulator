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

#include "nest.h"
#include "dictdatum.h"

namespace nest
{

  /**
   * Contains rules for selecting nodes from a layer when connecting.
   */
  struct Selector {
    Selector(): model(-1), depth(-1)
      {}
    Selector(const DictionaryDatum &);
    bool select_model() const
      { return model>=0; }
    bool select_depth() const
      { return depth>=0; }
    bool operator==(const Selector & other)
      { return (other.model==model) and (other.depth==depth); }
    long_t model;
    long_t depth;
  };

} // namespace nest

#endif
