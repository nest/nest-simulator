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
  void set_status( const Dictionary& );

  /**
   *
   */
  void get_status( Dictionary& );

  /**
   * Register a node model.
   * Also enters the model in modeldict, unless private_model is true.
   * @param m Model object to be registered.
   * @param private_model if true, don't add model to modeldict.
   * @return ID of the new model object.
   */
  index register_model( Model& m, bool private_model = false );

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

  Dictionary get_connector_defaults( synindex syn_id ) const;
  void set_connector_defaults( synindex syn_id, const Dictionary& d );

  /**
   * Check, if there are instances of a given model.
   * @param i the index of the model to check for.
   * @return True, if model is instantiated at least once.
   */
  bool is_model_in_use( index i );

  /**
   * @return Reference to the model dictionary
   */
  const Dictionary& get_modeldict();

  /**
   * @return Reference to the synapse dictionary
   */
  const Dictionary& get_synapsedict() const;

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

  /* BeginDocumentation
     Name: modeldict - dictionary containing all devices and models of NEST
     Description:
     'modeldict info' shows the contents of the dictionary
     SeeAlso: info, Device, RecordingDevice, iaf_neuron, subnet
  */
  Dictionary* modeldict_;    //!< Dictionary of all models

  /* BeginDocumentation
     Name: synapsedict - Dictionary containing all synapse models.
     Description:
     'synapsedict info' shows the contents of the dictionary
     FirstVersion: October 2005
     Author: Jochen Martin Eppler
     SeeAlso: info
  */
  Dictionary* synapsedict_;  //!< Dictionary of all synapse models

  bool model_defaults_modified_;  //!< True if any model defaults have been modified

  /**  */
  void clear_models_( bool called_from_destructor = false );
};

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

inline const Dictionary&
ModelManager::get_modeldict()
{
  assert( modeldict_ != 0 );
  return *modeldict_;
}

inline const Dictionary&
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
