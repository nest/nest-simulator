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

  void initialize( const bool ) override;
  void finalize( const bool ) override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  /**
   * Resize the structures for the Connector objects if necessary.
   *
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
  Node* get_proxy_node( size_t tid, size_t node_id );

  /**
   * Return pointer to protoype for given synapse id.
   *
   * @throws UnknownSynapseType
   *
   * @todo: make the return type const, after the increment of
   *        num_connections and the min_ and max_delay setting in
   *        ConnectorBase was moved out to the ConnectionManager
   */
  ConnectorModel& get_connection_model( synindex syn_id, size_t thread_id );

  const std::vector< ConnectorModel* >& get_connection_models( size_t tid );

  /**
   * Register a node-model prototype.
   *
   * This function must be called exactly once for each model class to make
   * it known in the simulator. The natural place for a call to this function
   * is in a *module.cpp file.
   * @param name of the new node model.
   * @param deprecation_info  If non-empty string, deprecation warning will
   *                          be issued for model with this info to user.
   * @return ID of the new model object.
   * @see register_connection_model
   */
  template < class ModelT >
  size_t register_node_model( const Name& name, std::string deprecation_info = std::string() );

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
  void register_connection_model( const std::string& name );

  /**
   * Copy an existing model and register it as a new model.
   *
   * This function allows users to create their own, cloned models.
   * @param old_name name of existing model.
   * @param new_name name of new model.
   * @param params default parameters of new model.
   * @see copy_node_model_, copy_connection_model_
   */
  void copy_model( Name old_name, Name new_name, DictionaryDatum params );

  /**
   * Set the default parameters of a model.
   *
   * @param name of model.
   * @param params default parameters to be set.
   * @return true if the operation succeeded, else false
   * @see set_node_defaults_, set_synapse_defaults_
   */
  bool set_model_defaults( Name name, DictionaryDatum params );

  /**
   * @return The model ID for a Model with a given name
   * @throws UnknownModelName if the model is not available
   */
  size_t get_node_model_id( const Name ) const;

  /**
   * @return The Model registered with the given model ID
   */
  Model* get_node_model( size_t ) const;

  /**
   * @return The numeric ID of a given synapse model
   * @throws UnknownSynapseType if the model is not available
   */
  size_t get_synapse_model_id( std::string model_name );

  DictionaryDatum get_connector_defaults( synindex syn_id ) const;

  void set_connector_defaults( synindex syn_id, const DictionaryDatum& d );

  /**
   * Asserts validity of synapse index, otherwise throws exception.
   * @throws UnknownSynapseType
   */
  void assert_valid_syn_id( synindex syn_id, size_t t ) const;

  bool are_model_defaults_modified() const;

  size_t get_num_connection_models() const;

  /**
   * Print out the memory information for each node model.
   */
  void memory_info() const;

  std::unique_ptr< SecondaryEvent > get_secondary_event_prototype( const synindex syn_id, const size_t tid );

private:
  /**
   * Delete all models and clear the modeldict
   *
   * This function deletes all models, which will as a side-effect also
   * delete all nodes. The built-in models will be re-registered in
   * initialize()
   */
  void clear_node_models_();

  void clear_connection_models_();

  size_t register_node_model_( Model* model );

  template < typename CompleteConnecionT >
  void register_specific_connection_model_( const std::string& name );

  /**
   * Copy an existing node model and register it as a new model.
   *
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @see copy_model(), copy_connection_model_()
   */
  void copy_node_model_( const size_t old_id, Name new_name, DictionaryDatum params );

  /**
   * Copy an existing synapse model and register it as a new model.
   *
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @see copy_model(), copy_node_model_()
   */
  void copy_connection_model_( const size_t old_id, Name new_name, DictionaryDatum params );

  /**
   * Set the default parameters of a model.
   *
   * @param model_id of model.
   * @param params default parameters to be set.
   * @see set_model_defaults, set_synapse_defaults_
   */
  void set_node_defaults_( size_t model_id, const DictionaryDatum& params );

  /**
   * Set the default parameters of a model.
   *
   * @param name of model.
   * @param params default parameters to be set.
   * @see set_model_defaults, set_node_defaults_
   */
  void set_synapse_defaults_( size_t model_id, const DictionaryDatum& params );

  //! Compares model ids for sorting in memory_info
  static bool compare_model_by_id_( const int a, const int b );

  /**
   * List of node models.
   *
   * The list contains all built-in node models requested for
   * compilation using -Dwith-models or -Dwith-modelset, models
   * registered from within extension modules, and models created by
   * calls to CopyModel().
   *
   * ResetKernel() clears this list and rebuilds it with built-in
   * models only.
   *
   * The elements of this list are used to create instances and are
   * responsible for the storage of model defaults.
   */
  std::vector< Model* > node_models_;

  /**
   * The list of connection models.
   *
   * This list contains all built-in connection models requested for
   * compilation using -Dwith-models or -Dwith-modelset, models
   * registered from within extension modules, and models created by
   * calls to CopyModel().
   *
   * The first dimension is thread-optimitzed by containing one vector
   * with all models per thread, as to avoid colliding memory accesses
   * during connection creation.
   *
   * This list is cleared and built-in models are re-registered upon
   * calls to ResetKernel, while those registered from user-modules
   * and copies are not.
   *
   * The elements of this list are used to create instances and are
   * responsible for the storage of model defaults.
   */
  std::vector< std::vector< ConnectorModel* > > connection_models_;

  DictionaryDatum modeldict_;   //!< Dictionary of all node models
  DictionaryDatum synapsedict_; //!< Dictionary of all synapse models

  Model* proxynode_model_;

  Node* create_proxynode_( size_t t, int model_id );

  //! Placeholders for remote nodes, one per thread
  std::vector< std::vector< Node* > > proxy_nodes_;
  //! True if any model defaults have been modified
  bool model_defaults_modified_;
};


inline Model*
ModelManager::get_node_model( size_t m ) const
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
ModelManager::get_connection_model( synindex syn_id, size_t thread_id )
{
  assert_valid_syn_id( syn_id, thread_id );
  return *( connection_models_[ thread_id ][ syn_id ] );
}

inline const std::vector< ConnectorModel* >&
ModelManager::get_connection_models( size_t tid )
{
  return connection_models_[ tid ];
}

inline void
ModelManager::assert_valid_syn_id( synindex syn_id, size_t t ) const
{
  if ( syn_id >= connection_models_[ t ].size() or not connection_models_[ t ][ syn_id ] )
  {
    throw UnknownSynapseType( syn_id );
  }
}

inline std::unique_ptr< SecondaryEvent >
ModelManager::get_secondary_event_prototype( const synindex syn_id, const size_t tid )
{
  assert_valid_syn_id( syn_id, tid );
  return get_connection_model( syn_id, tid ).get_secondary_event();
}

} // namespace nest

#endif /* MODEL_MANAGER_H */
