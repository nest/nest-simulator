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
  ~ModelManager();

  /**
   *
   */
  virtual void initialize();

  /**
   *
   */
  virtual void finalize();

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
   *
   */
  virtual void set_status( const DictionaryDatum& );

  /**
   *
   */
  virtual void get_status( DictionaryDatum& );

  /**
   *
   */
  Node* get_proxy_node( thread tid, index gid );

  /**
   *
   */
  Node* get_dummy_spike_source( thread );

  /**
   * Return pointer to protoype for given synapse id.
   * @throws UnknownSynapseType
   */

  //  TODO: make the return type const, after the increment of
  //  num_connections and the min_ and max_delay setting in
  //  ConnectorBase was moved out to the ConnectionManager
  ConnectorModel& get_synapse_prototype( synindex syn_id, thread t = 0 );

  const std::vector< ConnectorModel* >& get_synapse_prototypes( thread tid );

  /**
   * Register a node-model prototype.
   * This function must be called exactly once for each model class to make
   * it known in the simulator. The natural place for a call to this function
   * is in a *module.cpp file.
   * @param name of the new node model.
   * @param private_model if true, don't add model to modeldict.
   * @param deprecation_info  If non-empty string, deprecation warning will
   *                          be issued for model with this info to user.
   * @return ID of the new model object.
   * @see register_private_prototype_model, register_preconf_node_model,
   * register_prototype_connection
   */
  template < class ModelT >
  index
  register_node_model( const Name& name, bool private_model = false, std::string deprecation_info = std::string() );

  /**
   * Register a pre-configured model prototype with the network.
   * This function must be called exactly once for each model class to make
   * it known to the network. The natural place for a call to this function
   * is in a *module.cpp file.
   *
   * Pre-configured models are models based on the same class, as
   * another model, but have different parameter settings; e.g.,
   * voltmeter is a pre-configured multimeter.
   *
   * @param name of the new node model.
   * @param private_model if true, don't add model to modeldict.
   * @param dictionary to use to pre-configure model
   * @param deprecation_info  If non-empty string, deprecation warning will
   *                          be issued for model with this info to user.
   *
   * @return ID of the new model object.
   * @see register_private_prototype_model, register_node_model,
   * register_prototype_connection
   */
  template < class ModelT >
  index register_preconf_node_model( const Name& name,
    DictionaryDatum& conf,
    bool private_model = false,
    std::string deprecation_info = std::string() );

  /**
   * Copy an existing model and register it as a new model.
   * This function allows users to create their own, cloned models.
   * @param old_name name of existing model.
   * @param new_name name of new model.
   * @param params default parameters of new model.
   * @return model ID of new Model object.
   * @see copy_node_model_, copy_synapse_model_
   */
  index copy_model( Name old_name, Name new_name, DictionaryDatum params );

  /**
   * Set the default parameters of a model.
   * @param name of model.
   * @param params default parameters to be set.
   * @see set_node_defaults_, set_synapse_defaults_
   */
  void set_model_defaults( Name name, DictionaryDatum params );

  /**
   * Register a synape model with default Connector and without any common
   * properties. Convenience function that used the default Connector model
   * GenericConnectorModel.
   * @param name The name under which the ConnectorModel will be registered.
   */
  template < typename ConnectionT >
  void register_connection_model( const std::string& name,
    const bool requires_symmetric = false,
    const bool requires_clopath_archiving = false );

  /**
   * Register a synape model with a custom Connector model and without any
   * common properties.
   * @param name The name under which the ConnectorModel will be registered.
   */
  template < typename ConnectionT, template < typename > class ConnectorModelT >
  void register_connection_model( const std::string& name,
    const bool requires_symmetric = false,
    const bool requires_clopath_archiving = false );

  template < typename ConnectionT >
  void register_secondary_connection_model( const std::string& name,
    const bool has_delay = true,
    const bool requires_symmetric = false,
    const bool supports_wfr = true );

  /**
   * @return The model id of a given model name
   */
  int get_model_id( const Name ) const;

  /**
   * @return The Model of a given model ID
   */
  Model* get_model( index ) const;

  DictionaryDatum get_connector_defaults( synindex syn_id ) const;

  /**
   * Checks, whether synapse type requires symmetric connections
   */
  bool connector_requires_symmetric( const synindex syn_id ) const;

  /**
   * Checks, whether synapse type requires Clopath archiving
   */
  bool connector_requires_clopath_archiving( const synindex syn_id ) const;

  void set_connector_defaults( synindex syn_id, const DictionaryDatum& d );

  /**
   * Check, if there are instances of a given model.
   * @param i the index of the model to check for.
   * @return True, if model is instantiated at least once.
   */
  bool is_model_in_use( index i );

  /**
   * Checks, whether connections of the given type were created
   */
  bool synapse_prototype_in_use( synindex syn_id );

  /**
   * Asserts validity of synapse index, otherwise throws exception.
   * @throws UnknownSynapseType
   */
  void assert_valid_syn_id( synindex syn_id, thread t = 0 ) const;

  /**
   * @return Reference to the model dictionary
   */
  DictionaryDatum& get_modeldict();

  /**
   * @return Reference to the synapse dictionary
   */
  DictionaryDatum& get_synapsedict();

  /**
   * Does the network contain copies of models created using CopyModel?
   */
  bool has_user_models() const;

  bool has_user_prototypes() const;

  bool are_model_defaults_modified() const;

  const std::vector< ConnectorModel* >& get_prototypes( const thread t ) const;

  size_t get_num_node_models() const;

  size_t get_num_synapse_prototypes() const;

  /**
   * Print out the memory information for each node model.
   * @see sli::pool
   */
  void memory_info() const;

  void create_secondary_events_prototypes();

  void delete_secondary_events_prototypes();

  SecondaryEvent& get_secondary_event_prototype( const synindex syn_id, const thread tid ) const;

private:
  /**  */
  void clear_models_( bool called_from_destructor = false );

  /**  */
  void clear_prototypes_();

  /**  */
  index register_node_model_( Model* model, bool private_model = false );

  synindex register_connection_model_( ConnectorModel* );

  /**
   * Copy an existing node model and register it as a new model.
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @return model ID of new Model object.
   * @see copy_model(), copy_synapse_model_()
   */
  index copy_node_model_( index old_id, Name new_name );

  /**
   * Copy an existing synapse model and register it as a new model.
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @return model ID of new Model object.
   * @see copy_model(), copy_node_model_()
   */
  index copy_synapse_model_( index old_id, Name new_name );

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
   * The list of clean node models. The first component of the pair is a
   * pointer to the actual Model, the second is a flag indicating if
   * the model is private. Private models are not entered into the
   * modeldict.
   */
  std::vector< std::pair< Model*, bool > > pristine_models_;

  std::vector< Model* > models_; //!< List of available models


  /**
   * The list of clean synapse models. The first component of the pair is a
   * pointer to the actual Model, the second is a flag indicating if
   * the model is private. Private models are not entered into the
   * modeldict.
   */
  std::vector< ConnectorModel* > pristine_prototypes_;

  /**
   * The list of available synapse prototypes: first dimension one
   * entry per thread, second dimension for each synapse type
   */
  std::vector< std::vector< ConnectorModel* > > prototypes_;

  /**
   * prototypes of events
   */
  std::vector< Event* > event_prototypes_;

  std::vector< ConnectorModel* > secondary_connector_models_;
  std::vector< std::map< synindex, SecondaryEvent* > > secondary_events_prototypes_;

  /** @BeginDocumentation
   Name: modeldict - dictionary containing all devices and models of NEST

   Description:
   'modeldict info' shows the contents of the dictionary

   SeeAlso: info, Device, RecordingDevice
   */
  DictionaryDatum modeldict_; //!< Dictionary of all models

  /** @BeginDocumentation
   Name: synapsedict - Dictionary containing all synapse models.

   Description:
   'synapsedict info' shows the contents of the dictionary
   Synapse model names ending with '_hpc' provide minimal memory requirements by
   using thread-local target neuron IDs and fixing the `rport` to 0.
   Synapse model names ending with '_lbl' allow to assign an individual integer
   label (`synapse_label`) to created synapses at the cost of increased memory
   requirements.

   FirstVersion: October 2005

   Author: Jochen Martin Eppler

   SeeAlso: info
   */
  DictionaryDatum synapsedict_; //!< Dictionary of all synapse models

  Model* proxynode_model_;

  Node* create_proxynode_( thread t, int model_id );

  //! Placeholders for remote nodes, one per thread
  std::vector< std::vector< Node* > > proxy_nodes_;
  //! Placeholders for spiking remote nodes, one per thread
  std::vector< Node* > dummy_spike_sources_;
  //! True if any model defaults have been modified
  bool model_defaults_modified_;
};


inline Model*
ModelManager::get_model( index m ) const
{
  if ( m >= models_.size() or models_[ m ] == 0 )
  {
    throw UnknownModelID( m );
  }

  return models_[ m ];
}

inline Node*
ModelManager::get_dummy_spike_source( thread tid )
{
  return dummy_spike_sources_[ tid ];
}

inline bool
ModelManager::are_model_defaults_modified() const
{
  return model_defaults_modified_;
}

inline DictionaryDatum&
ModelManager::get_modeldict()
{
  return modeldict_;
}

inline DictionaryDatum&
ModelManager::get_synapsedict()
{
  return synapsedict_;
}

inline bool
ModelManager::has_user_models() const
{
  return models_.size() > pristine_models_.size();
}

inline ConnectorModel&
ModelManager::get_synapse_prototype( synindex syn_id, thread t )
{
  assert_valid_syn_id( syn_id );
  return *( prototypes_[ t ][ syn_id ] );
}

inline const std::vector< ConnectorModel* >&
ModelManager::get_synapse_prototypes( thread tid )
{
  return prototypes_[ tid ];
}

inline size_t
ModelManager::get_num_node_models() const
{
  return models_.size();
}

inline size_t
ModelManager::get_num_synapse_prototypes() const
{
  assert( prototypes_[ 0 ].size() <= invalid_synindex );
  return prototypes_[ 0 ].size();
}

inline void
ModelManager::assert_valid_syn_id( synindex syn_id, thread t ) const
{
  if ( syn_id >= prototypes_[ t ].size() or prototypes_[ t ][ syn_id ] == 0 )
  {
    throw UnknownSynapseType( syn_id );
  }
}

inline bool
ModelManager::has_user_prototypes() const
{
  return prototypes_[ 0 ].size() > pristine_prototypes_.size();
}

inline void
ModelManager::delete_secondary_events_prototypes()
{
  for ( std::vector< std::map< synindex, SecondaryEvent* > >::iterator it = secondary_events_prototypes_.begin();
        it != secondary_events_prototypes_.end();
        ++it )
  {
    for ( std::map< synindex, SecondaryEvent* >::iterator iit = it->begin(); iit != it->end(); ++iit )
    {
      ( *iit->second ).reset_supported_syn_ids();
      delete iit->second;
    }
  }
  secondary_events_prototypes_.clear();
}

inline SecondaryEvent&
ModelManager::get_secondary_event_prototype( const synindex syn_id, const thread tid ) const
{
  assert_valid_syn_id( syn_id );
  // Using .at() because operator[] does not guarantee constness.
  return *( secondary_events_prototypes_[ tid ].at( syn_id ) );
}

} // namespace nest

#endif /* MODEL_MANAGER_H */
