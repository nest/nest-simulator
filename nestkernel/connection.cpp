/*
 *  connection.cpp
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

#include "common_synapse_properties.h"
#include "connection.h"
#include "connector_model.h"
#include "network.h"

namespace nest
{

Connection::Connection()
        : target_(0),
          rport_(0)
{}

Connection::Connection(const Connection& c)
        : target_(c.target_),
          rport_(c.rport_)
{}

void Connection::get_status(DictionaryDatum & d) const
{
  if (target_ != 0)
  {
    def<long>(d, names::rport, rport_);
    def<long>(d, names::target, target_->get_gid());
  }

  (*d)[names::type] = LiteralDatum(names::synapse);
}


void Connection::initialize_property_arrays(DictionaryDatum & d) const
{
  initialize_property_array(d, names::targets);
  initialize_property_array(d, names::rports);
}

void Connection::append_properties(DictionaryDatum & d) const
{
  append_property<index>(d, names::targets, target_->get_gid());
  append_property<long_t>(d, names::rports, rport_);
}

} // namespace nest
