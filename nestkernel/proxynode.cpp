/*
 *  proxynode.cpp
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


#include "network.h"
#include "dictutils.h"
#include "proxynode.h"
#include "connection.h"

namespace nest
{

proxynode::proxynode(index gid, index parent_gid, index model_id, index vp) :
    Node()
{
  set_gid_(gid);
  Subnet* parent = dynamic_cast<Subnet*>(network()->get_node(parent_gid));
  assert(parent);
  set_parent_(parent);
  set_model_id(model_id);
  set_vp(vp);
  set(frozen);
}

port proxynode::check_connection(Connection& c, port receptor_type)
{
  return network()->get_model(get_model_id())->check_connection(c, receptor_type);
}

} // namespace
