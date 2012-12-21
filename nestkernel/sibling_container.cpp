/*
 *  sibling_container.cpp
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

#include "event.h"
#include "sibling_container.h"
#include "dictdatum.h"
#include "arraydatum.h"
#include "dictutils.h"
#include "network.h"
#include <string>

#ifdef N_DEBUG
#undef N_DEBUG
#endif

nest::SiblingContainer::SiblingContainer()
  :Node(),
   nodes_()
{
  set(frozen);  // freeze SiblingContainer by default
}

nest::SiblingContainer::SiblingContainer(const SiblingContainer &c)
  :Node(c),
   nodes_(c.nodes_)
{}

