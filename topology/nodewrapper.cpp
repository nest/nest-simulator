/*
 *  nodewrapper.cpp
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
  Authors: Kittel Austvoll, Håkon Enger
*/

#include <vector>

#include "nodewrapper.h"
#include "compound.h"

namespace nest
{

  lockPTR<std::vector<NodeWrapper> >
  NodeWrapper::get_nodewrappers(Node* n, const Position<double_t>& pos, std::vector<double_t> *extent)
  {
    Compound *subnet = dynamic_cast<Compound*>(n);
    assert(subnet != 0);
    // Slicing of layer before calling ConnectLayer function
    // assures that the subnet isn't nested.

    lockPTR<std::vector<NodeWrapper> > nodewrappers(new std::vector<NodeWrapper>());    

    for(std::vector<Node*>::iterator it = subnet->begin();
	it != subnet->end(); ++it)
      {
	nodewrappers->push_back(NodeWrapper(*it, pos, extent));
      }

    return nodewrappers;
  }

}
