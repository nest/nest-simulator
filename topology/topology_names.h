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

#ifndef TOPOLOGY_NAMES_H
#define TOPOLOGY_NAMES_H

/*
    This file is part of the NEST topology module.
*/

// Includes from sli:
#include "name.h"

namespace nest
{

/**
 * This namespace contains Name objects that are used by the topology
 * module. See nest_names.h for more info.
 */
namespace names
{

extern const Name allow_autapses;
extern const Name allow_multapses;
extern const Name allow_oversized_mask;
extern const Name anchor;
extern const Name azimuth_angle;
extern const Name box;
extern const Name center;
extern const Name circular;
extern const Name connection_type;
extern const Name edge_wrap;
extern const Name elements;
extern const Name ellipsoidal;
extern const Name elliptical;
extern const Name extent;
extern const Name grid;
extern const Name grid3d;
extern const Name inner_radius;
extern const Name kernel;
extern const Name lower_left;
extern const Name major_axis;
extern const Name mask;
extern const Name max;
extern const Name min;
extern const Name minor_axis;
extern const Name number_of_connections;
extern const Name offset;
extern const Name outer_radius;
extern const Name pairwise_bernoulli_on_source;
extern const Name pairwise_bernoulli_on_target;
extern const Name polar_angle;
extern const Name polar_axis;
extern const Name positions;
extern const Name radius;
extern const Name rectangular;
extern const Name shape;
extern const Name spherical;
extern const Name theta;
extern const Name upper_right;

} // namespace names

} // namespace nest

#endif
