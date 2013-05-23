#ifndef TOPOLOGY_NAMES_H
#define TOPOLOGY_NAMES_H

/*
 *  topology_names.h
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

#include "name.h"

namespace nest
{
  /**
   * This namespace contains Name objects that are used by the topology
   * module. See nest_names.h for more info.
   */
  namespace names
  {
    extern const Name rows;    //!< Number of rows in a layer or mask
    extern const Name columns;
    extern const Name row;
    extern const Name column;
    extern const Name depth;
    extern const Name extent;
    extern const Name center;
    extern const Name edge_wrap;
    extern const Name anchor;
    //    extern const Name x; // this name is already defined in nest_names
    extern const Name y;
    extern const Name positions;
    extern const Name topology;
    extern const Name points;
    extern const Name sources;
    extern const Name mask;
    extern const Name lid;
    extern const Name elements;
    extern const Name allow_oversized_mask;
    extern const Name connection_type;
    extern const Name number_of_connections;
    extern const Name convergent;
    extern const Name divergent;
    extern const Name kernel;
    extern const Name lower_left;
    extern const Name upper_right;
    extern const Name radius;
    extern const Name outer_radius;
    extern const Name inner_radius;
    extern const Name tau;
    extern const Name p_center;
    extern const Name sigma;
    extern const Name min;
    extern const Name max;
    extern const Name mean_x;
    extern const Name mean_y;
    extern const Name sigma_x;
    extern const Name sigma_y;
    extern const Name rho;
    extern const Name layers;
    extern const Name layer;
    extern const Name allow_autapses;
    extern const Name allow_multapses;
    extern const Name circular;
    extern const Name spherical;
    extern const Name rectangular;
    extern const Name box;
    extern const Name volume;
    extern const Name doughnut;
    extern const Name grid;
    extern const Name grid3d;
    extern const Name cutoff;
    extern const Name mu;
  }
}

#endif
