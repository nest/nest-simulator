/*
 *  model_manager.h
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

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

// C++ includes:
#include <string>

// Includes from nestkernel:
#include "connector_model.h"
#include "genericmodel.h"
#include "manager_interface.h"
#include "model.h"
#include "nest.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
class ModelManager : public ManagerInterface
{
public:
  ModelManager();
  ~ModelManager() override;

  void initialize() override;
  void finalize() override;
  void change_number_of_threads() override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  /**
   * Resize the structures for the Connector objects if necessary.
   * This function should be called after number of threads, min_delay,
   * max_delay, and time representation have been changed in the scheduler.
   * The TimeConverter is used to convert times from the old to the new
   * representation. It is also forwarding the calibration
   * request to all ConnectorModel objects.
   */
  void calibrate( const TimeConverter& );


  /**
   * Return a proxynode configured for thread tid and the given
   * node_id.
   */
  Node* get_proxy_node( thread tid, index node_id );

  /**
   * Return pointer to protoype for given synapse id.
   * @throws UnknownSynapseType
   *
   * @todo: make the return type const, after the increment of
   *        num_connections and the min_ and max_delay setting in
   *        ConnectorBase was moved out to the ConnectionManager
   */
  ConnectorModel& get_connection_model( synindex syn_id, thread t = 0 );

  const std::vector< ConnectorModel* >& get_connection_models( thread tid );

  /**
   * Register a node-model prototype.
   * This function must be called exactly once for each model class to make
   * it known in the simulator. The natural place for a call to this function
   * is in a *module.cpp file.
   * @param name of the new node model.
   * @param deprecation_info  If non-empty string, deprecation warning will
   *                          be issued for model with this info to user.
   * @return ID of the new model object.
   * @see register_prototype_connection
   */
  template < class ModelT >
  index register_node_model( const Name& name, std::string deprecation_info = std::string() );

  /**
   * Copy an existing model and register it as a new model.
   * This function allows users to create their own, cloned models.
   * @param old_name name of existing model.
   * @param new_name name of new model.
   * @param params default parameters of new model.
   * @return model ID of new Model object.
   * @see copy_node_model_, copy_connection_model_
   */
  index copy_model( Name old_name, Name new_name, DictionaryDatum params );

  /**
   * Set the default parameters of a model.
   * @param name of model.
   * @param params default parameters to be set.
   * @return true if the operation succeeded, else false
   * @see set_node_defaults_, set_synapse_defaults_
   */
  bool set_model_defaults( Name name, DictionaryDatum params );

  /**
   * Register a synape model with a custom Connector model and without any
   * common properties.
   *
   * "hpc synapses" use `TargetIdentifierIndex` for `ConnectionT` and store
   * the target neuron in form of a 2 Byte index instead of an 8 Byte pointer.
   * This limits the number of thread local neurons to 65,536. No support for
   * different receptor types. Otherwise identical to non-hpc version.
   *
   * When called, this function should be specialised by a class template,
   * e.g. `bernoulli_synapse< targetidentifierT >`
   *
   * @param name The name under which the ConnectorModel will be registered.
   */
  template < template < typename targetidentifierT > class ConnectionT >
  void register_connection_model( const std::string& name,
    const RegisterConnectionModelFlags flags = default_connection_model_flags );

  template < template < typename targetidentifierT > class ConnectionT >
  void register_secondary_connection_model( const std::string& name,
    const RegisterConnectionModelFlags flags = default_secondary_connection_model_flags );

  /**
   * @return The model ID for a Model with a given name
   * @throws UnknownModelName if the model is not available
   */
  index get_node_model_id( const Name ) const;

  /**
   * @return The Model registered with the given model ID
   */
  Model* get_node_model( index ) const;

  /**
   * @return The numeric ID of a given synapse model
   * @throws UnknownSynapseType if the model is not available
   */
  index get_synapse_model_id( std::string model_name );

  DictionaryDatum get_connector_defaults( synindex syn_id ) const;

  /**
   * Checks, whether synapse type requires symmetric connections
   */
  bool connector_requires_symmetric( const synindex syn_id ) const;

  /**
   * Checks, whether synapse type requires Clopath archiving
   */
  bool connector_requires_clopath_archiving( const synindex syn_id ) const;

  /**
   * Checks, whether synapse type requires Urbanczik archiving
   */
  bool connector_requires_urbanczik_archiving( const synindex syn_id ) const;

  void set_connector_defaults( synindex syn_id, const DictionaryDatum& d );

  /**
   * Asserts validity of synapse index, otherwise throws exception.
   * @throws UnknownSynapseType
   */
  void assert_valid_syn_id( synindex syn_id, thread t = 0 ) const;

  bool are_model_defaults_modified() const;

  size_t get_num_connection_models() const;

  /**
   * Print out the memory information for each node model.
   */
  void memory_info() const;

  SecondaryEvent& get_secondary_event_prototype( const synindex syn_id, const thread tid );

private:
  void clear_node_models_();

  void clear_connection_models_();

  index register_node_model_( Model* model );

  synindex register_connection_model_( ConnectorModel* );

  /**
   * Copy an existing node model and register it as a new model.
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @return model ID of new Model object.
   * @see copy_model(), copy_connection_model_()
   */
  index copy_node_model_( index old_id, Name new_name );

  /**
   * Copy an existing synapse model and register it as a new model.
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @return model ID of new Model object.
   * @see copy_model(), copy_node_model_()
   */
  index copy_connection_model_( index old_id, Name new_name );

  /**
   * Set the default parameters of a model.
   * @param model_id of model.
   * @param params default parameters to be set.
   * @see set_model_defaults, set_synapse_defaults_
   */
  void set_node_defaults_( index model_id, const DictionaryDatum& params );

  /**
   * Set the default parameters of a model.
   * @param name of model.
   * @param params default parameters to be set.
   * @see set_model_defaults, set_node_defaults_
   */
  void set_synapse_defaults_( index model_id, const DictionaryDatum& params );

  //! Compares model ids for sorting in memory_info
  static bool compare_model_by_id_( const int a, const int b );

  /**
   * List of clean built-in node models.
   */
  std::vector< Model* > builtin_node_models_;

  /**
   * List of usable node models. This list is cleared and repopulated
   * upon application startup and calls to ResetKernel. It contains
   * copies of the built-in models, models registered from extension
   * modules, and models created by calls to CopyModel(). The elements
   * of this list also keep the user-modified defaults.
   */
  std::vector< Model* > node_models_;

  /**
   * List of built-in clean connection models.
   */
  std::vector< ConnectorModel* > builtin_connection_models_;

  /**
   * The list of usable connection models. The first dimension keeps
   * one entry per thread, the second dimension has the actual models.
   * This list is cleared and repopulated upon application startup and
   * calls to ResetKernel. The inner list contains copies of the
   * built-in models, models registered from extension modules, and
   * models created by calls to CopyModel(). The elements of the list
   * also keep the user-modified defaults.
   */
  std::vector< std::vector< ConnectorModel* > > connection_models_;

  DictionaryDatum modeldict_;   //!< Dictionary of all node models
  DictionaryDatum synapsedict_; //!< Dictionary of all synapse models

  Model* proxynode_model_;

  Node* create_proxynode_( thread t, int model_id );

  //! Placeholders for remote nodes, one per thread
  std::vector< std::vector< Node* > > proxy_nodes_;
  //! True if any model defaults have been modified
  bool model_defaults_modified_;
};


inline Model*
ModelManager::get_node_model( index m ) const
{
  assert( m < node_models_.size() );
  return node_models_[ m ];
}

inline bool
ModelManager::are_model_defaults_modified() const
{
  return model_defaults_modified_;
}

inline ConnectorModel&
ModelManager::get_connection_model( synindex syn_id, thread t )
{
  assert_valid_syn_id( syn_id );
  return *( connection_models_[ t ][ syn_id ] );
}

inline const std::vector< ConnectorModel* >&
ModelManager::get_connection_models( thread tid )
{
  return connection_models_[ tid ];
}

inline size_t
ModelManager::get_num_connection_models() const
{
  assert( connection_models_[ 0 ].size() <= invalid_synindex );
  return connection_models_[ 0 ].size();
}

inline void
ModelManager::assert_valid_syn_id( synindex syn_id, thread t ) const
{
  if ( syn_id >= connection_models_[ t ].size() or not connection_models_[ t ][ syn_id ] )
  {
    throw UnknownSynapseType( syn_id );
  }
}

inline SecondaryEvent&
ModelManager::get_secondary_event_prototype( const synindex syn_id, const thread tid )
{
  assert_valid_syn_id( syn_id );
  return *get_connection_model( syn_id, tid ).get_event();
}

} // namespace nest

#endif /* MODEL_MANAGER_H */
