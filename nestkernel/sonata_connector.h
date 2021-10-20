/*
 *  conn_builder_sonata.h
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

#ifndef SONATA_CONNECTOR_H
#define SONATA_CONNECTOR_H

#include "config.h"

// C++ includes:
#include <map>
#include <vector>

// Includes from nestkernel:
#include "nest_datums.h"

#include "H5Cpp.h"

namespace nest
{

class SonataConnector
{

public:
  SonataConnector( const DictionaryDatum& sonata_dynamics );

  void connect();

private:
  hsize_t get_num_elements_( H5::DataSet& dataset );
  int* read_data_( H5::DataSet dataset, int num_elements );
  int* get_data_( H5::Group group, std::string name );
  void get_attributes_( std::string& attribute_value, H5::DataSet dataset, std::string attribute_name );
  void create_type_id_2_syn_spec_( DictionaryDatum edge_dict );
  void get_synapse_params_( DictionaryDatum syn_params, index snode_id, Node& target, thread target_thread, RngPtr rng );

  DictionaryDatum sonata_dynamics_;
  std::map< int, DictionaryDatum > type_id_2_syn_spec_;
  DictionaryDatum param_dict_;

};

} // namespace nest


#endif /* ifdef SONATA_CONNECTOR_H */
