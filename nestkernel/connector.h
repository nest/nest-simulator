/*
 *  connector.h
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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "node.h"
#include "event.h"
#include "exceptions.h"
#include "spikecounter.h"

class Dictionary;

namespace nest
{

class TimeConverter;

/**
 * Pure abstract base class for all Connectors. It constitutes
 * the interface between the ConnectionManager and a Connector.
 */
class Connector
{
 public:
  virtual ~Connector() {}
  virtual void register_connection(Node&, Node&) = 0;
  virtual void register_connection(Node&, Node&, double_t, double_t) = 0;
  virtual void register_connection(Node&, Node&, DictionaryDatum&) = 0;
  virtual std::vector<long>* find_connections(DictionaryDatum) const = 0;
  /**
   * Return a list of all connections. 
   * Return the list of ports that connect to the provided.
   */
  virtual void get_connections(size_t source_gid, size_t thrd, size_t synapse_id, ArrayDatum &conns) const=0;

  /**
   * Return a list of ports. 
   * Return the list of ports that connect to the provided post_gid.
   */
  virtual void get_connections(size_t source_gid, size_t target_gid, size_t thrd, size_t synapse_id, ArrayDatum &conns) const=0;

  virtual size_t get_num_connections() const =0;
  virtual void get_status(DictionaryDatum & d) const = 0;
  virtual void set_status(const DictionaryDatum & d) = 0;
  virtual void get_synapse_status(DictionaryDatum & d, port p) const = 0;
  virtual void set_synapse_status(const DictionaryDatum & d, port p) = 0;
  virtual void send(Event& e) = 0;
  virtual void calibrate(const TimeConverter &) = 0;
  virtual void trigger_update_weight(const std::vector<spikecounter>&, double_t){};
};
 

}

#endif /* #ifndef CONNECTOR_H */
