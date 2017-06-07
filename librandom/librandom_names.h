/*
 *  librandom_names.h
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

#ifndef LIBRANDOM_NAMES_H
#define LIBRANDOM_NAMES_H

// Includes from sli:
#include "name.h"

namespace librandom
{
/**
 * This namespace contains Name objects that are used by the librandom
 * libraries. See nest_names.h for more info.
 */
namespace names
{
extern const Name high;
extern const Name is_discrete;
extern const Name lambda;
extern const Name low;
extern const Name mu;
extern const Name n;
extern const Name order;
extern const Name p;
extern const Name scale;
extern const Name sigma;
}
}

#endif
