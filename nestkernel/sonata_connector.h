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
#include "conn_parameter.h"
#include "nest_datums.h"

#include "H5Cpp.h"

namespace nest
{

class SonataConnector
{

  /*
   * HDF5 do NOT support MPI parallel and thread parallel runs simultaneously, see
   * https://portal.hdfgroup.org/display/knowledge/Questions+about+thread-safety+and+concurrent+access
   */
public:
  SonataConnector( const DictionaryDatum& sonata_dynamics );
  ~SonataConnector();

  void connect();

private:
  hsize_t get_num_elements_( H5::DataSet& dataset );
  int* read_data_( H5::DataSet dataset, int num_elements );
  int* get_data_( H5::Group group, std::string name );
  void get_attributes_( std::string& attribute_value, H5::DataSet dataset, std::string attribute_name );
  void weight_and_delay_from_dataset_( H5::Group group);
  void create_type_id_2_syn_spec_( DictionaryDatum edge_dict );
  void set_synapse_params(DictionaryDatum syn_dict, index synapse_model_id, int type_id);
  void get_synapse_params_( index snode_id, Node& target, thread target_thread, RngPtr rng, int edge_type_id );
  void reset_params();

  DictionaryDatum sonata_dynamics_;
  bool weight_dataset_;
  bool delay_dataset_;
  double* syn_weight_data_;
  double* delay_data_;
  std::map< int, index > type_id_2_syn_model_;
  std::map< int, std::vector< std::map< Name, ConnParameter* > > > type_id_2_syn_spec_;
  std::map< int, std::vector< DictionaryDatum > > type_id_2_param_dicts_;

};

} // namespace nest


#endif /* ifdef SONATA_CONNECTOR_H */
