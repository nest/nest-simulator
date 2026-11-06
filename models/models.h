/*
 *  models.h
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

#ifndef MODELS_H
#define MODELS_H

// Includes from nestkernel:
#include "nest_impl.h"

namespace nest
{
/**
 * Function to register all node and connection models that were
 * selected for compilation either by using the cmake switch
 * -Dwith-models=<model;list> or as specified in the modelset given to
 * the option -Dwith-modelset=<modelset>
 */
void register_models();
}

#endif
