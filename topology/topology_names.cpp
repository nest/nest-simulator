/*
 *  topology_names.cpp
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

#include "topology_names.h"

namespace nest
{
  namespace names
  {
    const Name rows("rows");
    const Name columns("columns");
    const Name row("row");
    const Name column("column");
    const Name depth("depth");
    const Name extent("extent");
    const Name center("center");
    const Name edge_wrap("edge_wrap");
    const Name anchor("anchor");
    //const Name x("x");
    const Name y("y");
    const Name positions("positions");
    const Name topology("topology");
    const Name points("points");
    const Name sources("sources");
    const Name mask("mask");
    const Name lid("lid");
    const Name elements("elements");
    const Name allow_oversized_mask("allow_oversized_mask");
    const Name connection_type("connection_type");
    const Name number_of_connections("number_of_connections");
    const Name convergent("convergent");
    const Name divergent("divergent");
    const Name kernel("kernel");
    const Name lower_left("lower_left");
    const Name upper_right("upper_right");
    const Name radius("radius");
    const Name outer_radius("outer_radius");
    const Name inner_radius("inner_radius");
    const Name tau("tau");
    const Name p_center("p_center");
    const Name sigma("sigma");
    const Name min("min");
    const Name max("max");
    const Name mean_x("mean_x");
    const Name mean_y("mean_y");
    const Name sigma_x("sigma_x");
    const Name sigma_y("sigma_y");
    const Name rho("rho");
    const Name layers("layers");
    const Name layer("layer");
    const Name allow_autapses("allow_autapses");
    const Name allow_multapses("allow_multapses");
    const Name circular("circular");
    const Name spherical("spherical");
    const Name rectangular("rectangular");
    const Name box("box");
    const Name volume("volume");
    const Name doughnut("doughnut");
    const Name grid("grid");
    const Name grid3d("grid3d");
    const Name cutoff("cutoff");
    const Name mu("mu");
  }
}

