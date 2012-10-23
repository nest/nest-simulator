/*
 *  connection_id.cpp
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

#include "connection_id.h"

namespace nest
{

  ConnectionID::ConnectionID(long source_gid, long target_thread, long synapse_typeid, long port)
          : source_gid_(source_gid),
            target_thread_(target_thread),
            synapse_typeid_(synapse_typeid),
            port_(port)
  {}
  
  DictionaryDatum ConnectionID::get_dict()
  {
    DictionaryDatum dict(new Dictionary);

    def<long>(dict, nest::names::source, source_gid_);             // The gid of the presynaptic node
    def<long>(dict, nest::names::synapse_typeid, synapse_typeid_); // The id of the synapse model
    def<long>(dict, nest::names::target_thread, target_thread_);   // The thread of the postsynaptic node
    def<long>(dict, nest::names::port, port_);                     // The index in the list 

    return dict;
  }

  bool ConnectionID::operator==(const ConnectionID& c)
  {
    return (source_gid_ == c.source_gid_)
        && (target_thread_ == c.target_thread_)
        && (port_ == c.port_)
        && (synapse_typeid_ == c.synapse_typeid_);
  }

  std::ostream & ConnectionID::print_me(std::ostream& out) const
  {
    out << "<connectiontype>";
    return out;
  }

std::ostream & operator<<(std::ostream& out, const nest::ConnectionID& c)
{
  return c.print_me(out);
}  

} // namespace


