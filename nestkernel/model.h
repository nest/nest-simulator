/*
 *  model.h
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

#ifndef MODEL_H
#define MODEL_H

// C++ includes:
#include <memory>
#include <new>
#include <string>
#include <vector>

// Includes from nestkernel:
#include "node.h"
#include "update_block.h"


namespace nest
{
class TimeConverter;

/**
 * Base class for all Models.
 *
 * Each Node class is associated with a corresponding Model
 * class. The Model class is responsible for the creation and class
 * wide parametrisation of its associated Node objects.
 *
 * class Model manages the thread-sorted memory pool of the model.
 * The default constructor uses one thread as default. Use set_threads() to
 * use more than one thread.
 * @ingroup user_interface
 * @see Node
 */
class Model
{
public:
  Model( const std::string& name );
  Model( const Model& m )
    : name_( m.name_ )
    , type_id_( m.type_id_ )
    , memory_( m.memory_ )
    , blocks_()
  {
  }

  virtual ~Model()
  {
  }

  /**
   * Create clone with new name.
   */
  virtual Model* clone( const std::string& ) const = 0;

  /**
   * Set number of threads based on number set in network.
   *
   * As long as no nodes of the model have been allocated, the number
   * of threads may be changed.
   * @note Requires that network pointer in NestModule is initialized.
   */
  void set_threads();

  /**
   * Allocate new Node and return its pointer.
   *
   * create() is not const, because it
   * is allowed to modify the Model object for
   * 'administrative' purposes.
   */
  Node* create( size_t t );

  /**
   * Deletes all nodes which belong to this model.
   */

  void clear();

  /**
   * Reserve space for n additional Nodes.
   */
  void reserve_additional( size_t t, size_t n );

  /**
   * Let the model form a block of the neurons nodes it has just created on thread t.
   *
   * nodes are the neurons of one Create() call on thread t. If the model forms a block (make_update_block_()), the
   * block makes Node::get_update_block() of each of its neurons return the block, so that the simulation loop updates
   * them through it (see UpdateBlock), and the model keeps the block until its neurons are destroyed.
   *
   * Only thread t calls this function, inside the parallel region of NodeManager::add_neurons_(), and it writes only
   * blocks_[ t ], which set_threads_() has sized before.
   */
  void create_update_block( size_t t, const std::vector< Node* >& nodes );

  /**
   * Return name of the Model.
   *
   * This function returns the name of the Model as C++ string. The
   * name is defined by the constructor. The result is identical to the value
   * of Node::get_name();
   * @see Model::Model()
   * @see Node::get_name()
   */
  std::string get_name() const;

  /**
   * Return the available memory. The result is given in number of elements,
   * not in bytes.
   *
   * Note that this function reports a sum over all threads.
   */
  size_t mem_available();

  /**
   * Return the memory capacity. The result is given in number of elements,
   * not in bytes.
   *
   * Note that this function reports a sum over all threads.
   */
  size_t mem_capacity();

  virtual bool has_proxies() = 0;
  virtual bool local_receiver() = 0;
  virtual bool one_node_per_process() = 0;
  virtual bool is_off_grid() = 0;
  virtual void calibrate_time( const TimeConverter& tc ) = 0;

  /**
   * Change properties of the prototype node according to the
   * entries in the dictionary.
   *
   * @param d Dictionary with named parameter settings.
   * @ingroup status_interface
   */
  void set_status( const Dictionary& );

  /**
   * Export properties of the prototype node by setting
   * entries in the status dictionary.
   *
   * @param d Dictionary.
   * @ingroup status_interface
   */
  Dictionary get_status();

  virtual size_t send_test_event( Node&, size_t, synindex, bool ) = 0;

  virtual void sends_secondary_event( GapJunctionEvent& ge ) = 0;
  virtual void sends_secondary_event( InstantaneousRateConnectionEvent& re ) = 0;
  virtual void sends_secondary_event( DiffusionConnectionEvent& de ) = 0;
  virtual void sends_secondary_event( DelayedRateConnectionEvent& re ) = 0;
  virtual void sends_secondary_event( LearningSignalConnectionEvent& re ) = 0;
  virtual void sends_secondary_event( SICEvent& sic ) = 0;

  /**
   * Check what type of signal this model is sending.
   *
   * Required so that proxynode can formward this call
   * to model that in turn delegates the call to the underlying
   * prototype.
   */
  virtual SignalType sends_signal() const = 0;

  /**
   * Return the size of the prototype.
   */
  virtual size_t get_element_size() const = 0;

  /**
   * Return const reference to the prototype.
   */
  virtual Node const& get_prototype() const = 0;

  /**
   * Set the model id on the prototype.
   */
  virtual void set_model_id( int ) = 0;

  /**
   * Get the model id from the prototype.
   */
  virtual int get_model_id() = 0;

  /**
   * Issue deprecation warning on first call if model is deprecated.
   *
   * @param calling function
   */
  virtual void deprecation_warning( const std::string& ) = 0;

  /**
   * Set the model id on the prototype.
   */
  void
  set_type_id( size_t id )
  {
    type_id_ = id;
  }

  size_t
  get_type_id() const
  {
    return type_id_;
  }

private:
  virtual void set_status_( const Dictionary& ) = 0;

  virtual Dictionary get_status_() = 0;


  /**
   * Set the number of threads.
   * @see set_threads()
   */
  void set_threads_( size_t t );

  /**
   * Create a new object.
   */
  virtual Node* create_() = 0;

  //! Return a block of the neurons nodes, or nullptr if the model forms no block.
  virtual std::unique_ptr< UpdateBlock > make_update_block_( const std::vector< Node* >& nodes ) = 0;

  /**
   * Name of the Model.
   *
   * This name will be used to identify all Nodes which are
   * created by this model object.
   */
  std::string name_;

  /**
   * Identifier of the model C++ type.
   *
   * For pristine models, the type_id equals the model_id.
   * For copied models, the type_id equals the type_id of the base model.
   * This number is needed to automatically save and restore copied models.
   */
  size_t type_id_;

  /**
   * Memory for all nodes sorted by threads.
   */
  std::vector< std::vector< Node* > > memory_;

  /**
   * Blocks of the neurons of the model, sorted by threads (see create_update_block()).
   *
   * The blocks must outlive their neurons. They do: ResetKernel finalizes the managers in reverse order, so the
   * NodeManager deletes the neurons before the ModelManager clears the models and with them the blocks.
   */
  std::vector< std::vector< std::unique_ptr< UpdateBlock > > > blocks_;
};


inline Node*
Model::create( size_t t )
{
  assert( t < memory_.size() );
  Node* n = create_();
  memory_[ t ].emplace_back( n );
  return n;
}

inline std::string
Model::get_name() const
{
  return name_;
}
}
#endif
