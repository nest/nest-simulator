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

#include "dict.h"
#include "nest_names.h"

namespace nest
{
class ModelManager:ManagerInterface
{
public:
  /**
   *
   */
  void init();

  /**
   *
   */
  void reset();

  /**
   *
   */
  void set_status( const DictionaryDatum& );

  /**
   *
   */
  void get_status( DictionaryDatum& );

  /**
   *
   */
  Node* get_proxy_nodes(const index model_id);

  /**
   *
   */
  Model* get_subnet_model();

  /**
   * Register a node-model prototype.
   * This function must be called exactly once for each model class to make
   * it known in the simulator. The natural place for a call to this function
   * is in a *module.cpp file.
   * @param name of the new node model.
   * @param private_model if true, don't add model to modeldict.
   * @return ID of the new model object.
   * @see register_private_prototype_model, register_preconf_node_model, register_prototype_connection
   */
  template < class ModelT >
  index
  register_node_model( const Name& name, bool private_model = false );
  
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
   * @return ID of the new model object.
   * @see register_private_prototype_model, register_node_model, register_prototype_connection
   */
  template < class ModelT >
  index
  register_preconf_node_model( const Name& name, DictionaryDatum& conf, bool private_model = false )

  /**
   * Copy an existing model and register it as a new model.
   * This function allows users to create their own, cloned models.
   * @param old_id ID of existing model.
   * @param new_name name of new model.
   * @return model ID of new Model object.
   * @see copy_synapse_prototype()
   */
  index copy_model( index old_id, Name new_name );

  /**
   * Register a synapse type.
   * @param cm ConnectorModel to be registered.
   * @return an ID for the synapse prototype.
   */
  synindex register_synapse_prototype( ConnectorModel* cf );

  /**
   * Copy an existing synapse type.
   * @see copy_model(), ConnectionManager::copy_synapse_prototype()
   * @param old_id ID of synapse model to copy.
   * @param new_name name of new synapse model.
   * @return ID of new synapse model.
   */
  synindex copy_synapse_prototype( synindex old_id, Name new_name );

  /**
   * @return The model id of a given model name
   */
  int get_model_id( const Name ) const;

  /**
   * @return The Model of a given model ID
   */
  Model* get_model( index ) const;

  DictionaryDatum get_connector_defaults( synindex syn_id ) const;
  void set_connector_defaults( synindex syn_id, const DictionaryDatum& d );

  /**
   * Check, if there are instances of a given model.
   * @param i the index of the model to check for.
   * @return True, if model is instantiated at least once.
   */
  bool is_model_in_use( index i );

  /**
   * @return Reference to the model dictionary
   */
  const DictionaryDatum& get_modeldict();

  /**
   * @return Reference to the synapse dictionary
   */
  const DictionaryDatum& get_synapsedict() const;

  /**
   * Does the network contain copies of models created using CopyModel?
   */
  bool has_user_models() const;

  bool is_model_defaults_modified() const;
  void set_model_defaults_modified();

private:
  /**
   * The list of clean models. The first component of the pair is a
   * pointer to the actual Model, the second is a flag indicating if
   * the model is private. Private models are not entered into the
   * modeldict.
   */
  std::vector< std::pair< Model*, bool > > pristine_models_;

  std::vector< Model* > models_;  //!< List of available models

  std::vector< std::vector< Node* > >
    proxy_nodes_; //!< Placeholders for remote nodes, one per thread

  /* BeginDocumentation
     Name: modeldict - dictionary containing all devices and models of NEST
     Description:
     'modeldict info' shows the contents of the dictionary
     SeeAlso: info, Device, RecordingDevice, iaf_neuron, subnet
  */
  DictionaryDatum* modeldict_;    //!< DictionaryDatum of all models

  /* BeginDocumentation
     Name: synapsedict - DictionaryDatum containing all synapse models.
     Description:
     'synapsedict info' shows the contents of the dictionary
     FirstVersion: October 2005
     Author: Jochen Martin Eppler
     SeeAlso: info
  */
  DictionaryDatum* synapsedict_;  //!< DictionaryDatum of all synapse models

  bool model_defaults_modified_;  //!< True if any model defaults have been modified

  /**  */
  void clear_models_( bool called_from_destructor = false );

  /**  */
  index
  register_node_model_( const Model* model, bool private_model = false );
};

inline
Node*
get_proxy_nodes(const index model_id)
{
  thread t = kernel().vm_manager.get_thread_id();
  return proxy_nodes_[ t ][ model_id ];
}

inline
Model*
get_subnet_model()
{
  Model* subnet_model = pristine_models_[0].first;
  assert( subnet_model != 0 );
  return subnet_model;
}

template < class ModelT >
index
register_node_model( const Name& name, bool private_model = false )
{
  if ( !private_model && modeldict_->known( name ) )
  {
    throw NamingConflict("A model called '" + name + "' already exists.\n"
        "Please choose a different name!");
  }

  Model* model = new GenericModel< ModelT >( name );
  return register_node_model_( model, private_model );
}

template < class ModelT >
index
register_preconf_node_model( const Name& name, DictionaryDatum& conf, bool private_model = false )
{
  if ( !private_model && modeldict_->known( name ) )
  {
    throw NamingConflict("A model called '" + name + "' already exists.\n"
        "Please choose a different name!");
  }

  Model* model = new GenericModel< ModelT >( name );
  conf.clear_access_flags();
  model->set_status( conf );
  Name missed;
  assert( conf.all_accessed( missed ) ); // we only get here from C++ code, no need for exception
  return register_node_model_( model, private_model );
}

index
register_node_model_( const Model* model, bool private_model = false )
{
  const index id = models_.size();
  model->set_model_id( id );
  model->set_type_id( id );

  pristine_models_.push_back( std::pair< Model*, bool >( model, private_model ) );
  models_.push_back( model->clone( name ) );
  int proxy_model_id = get_model_id( "proxynode" );
  assert( proxy_model_id > 0 );
  Model* proxy_model = models_[ proxy_model_id ];
  assert( proxy_model != 0 );

  for ( thread t = 0; t < get_num_threads(); ++t )
  {
    Node* newnode = proxy_model->allocate( t );
    newnode->set_model_id( id );
    proxy_nodes_[ t ].push_back( newnode );
  }

  if ( !private_model )
    modeldict_->insert( name, id );

  return id;
}

inline
Model* ModelManager::get_model( index m ) const
{
  if ( m >= models_.size() || models_[ m ] == 0 )
    throw UnknownModelID( m );

  return models_[ m ];
}

inline
bool ModelManager::is_model_defaults_modified() const
{
  return model_defaults_modified_;
}

inline
void ModelManager::set_model_defaults_modified()
{
  model_defaults_modified_ = true;
}

bool
ModelManager::model_in_use( index i )
{
  return node_model_ids_.model_in_use( i );
}

inline const DictionaryDatum&
ModelManager::get_modeldict()
{
  assert( modeldict_ != 0 );
  return *modeldict_;
}

inline const DictionaryDatum&
ModelManager::get_synapsedict() const
{
  assert( synapsedict_ != 0 );
  return *synapsedict_;
}

inline bool
ModelManager::has_user_models() const
{
  return models_.size() > pristine_models_.size();
}

}

#endif /* MODEL_MANAGER_H */
