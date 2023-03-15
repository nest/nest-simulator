/*
 *  nest_object_interface.h
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


#ifndef NEST_OBJECT_INTERFACE_H
#define NEST_OBJECT_INTERFACE_H

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_types.h"


// Includes from sli:
#include "dictdatum.h"


namespace nest
{

class NESTObjectInterface
{

public:
  NESTObjectInterface();
  NESTObjectInterface( const NESTObjectInterface& );
  virtual ~NESTObjectInterface();
  /**
   * Return class name.
   * Returns name of node model (e.g. "iaf_psc_alpha") as string.
   * This name is identical to the name that is used to identify
   * the model in the interpreter's model dictionary.
   */
  std::string get_name() const;

  /**
   * Return the element type of the node.
   * The returned Name is a free label describing the class of network
   * elements a node belongs to. Currently used values are "neuron",
   * "recorder", "stimulator", and "other", which are all defined as
   * static Name objects in the names namespace.
   * This function is overwritten with a corresponding value in the
   * derived classes
   */
  virtual Name get_element_type() const = 0;


  /**
   * Return global Network ID.
   * Returns the global network ID of the Node.
   * Each node has a unique network ID which can be used to access
   * the Node comparable to a pointer.
   *
   * The smallest valid node ID is 1.
   */
  index get_node_id() const;


  /**
   * Return model ID of the node.
   * Returns the model ID of the model for this node.
   * Model IDs start with 0.
   * @note The model ID is not stored in the model prototype instance.
   *       It is only set when actual nodes are created from a prototype.
   */
  int get_model_id() const;

  /**
   * @defgroup status_interface Configuration interface.
   * Functions and infrastructure, responsible for the configuration
   * of Nodes from the SLI Interpreter level.
   *
   * Each node can be configured from the SLI level through a named
   * parameter interface. In order to change parameters, the user
   * can specify name value pairs for each parameter. These pairs
   * are stored in a data structure which is called Dictionary.
   * Likewise, the user can query the configuration of any node by
   * requesting a dictionary with name value pairs.
   *
   * The configuration interface consists of four functions which
   * implement storage and retrieval of named parameter sets.
   */

  /**
   * Change properties of the node according to the
   * entries in the dictionary.
   * @param d Dictionary with named parameter settings.
   * @ingroup status_interface
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * Export properties of the node by setting
   * entries in the status dictionary.
   * @param d Dictionary.
   * @ingroup status_interface
   */
  virtual void get_status( DictionaryDatum& ) const = 0;


  /**
   * Store the number of the thread to which the node is assigned.
   * The assignment is done after node creation by the Network class.
   * @see: NodeManager::add_node().
   */
  void set_thread( thread );

  /**
   * Retrieve the number of the thread to which the node is assigned.
   */
  thread get_thread() const;

  /**
   * Store the number of the virtual process to which the node is assigned.
   * This is assigned to the node in NodeManager::add_node().
   */
  void set_vp( thread );

  /**
   * Retrieve the number of the virtual process to which the node is assigned.
   */
  thread get_vp() const;

  /** Set the model id.
   * This method is called by NodeManager::add_node() when a node is created.
   * @see get_model_id()
   */
  void set_model_id( int );


  /**
   *  Return a dictionary with the node's properties.
   *
   *  get_status_base() first gets a dictionary with the basic
   *  information of an element, using get_status_dict_(). It then
   *  calls the custom function get_status(DictionaryDatum) with
   *  the created status dictionary as argument.
   */
  DictionaryDatum get_status_base();

  /**
   * Set status dictionary of a node.
   *
   * Forwards to set_status() of the derived class.
   * @internal
   */
  void set_status_base( const DictionaryDatum& );


  /**
   * Returns true if node is model prototype.
   */
  bool is_model_prototype() const;

  /**
   * set thread local index

   */
  void set_thread_lid( const index );

  /**
   * get thread local index
   */
  index get_thread_lid() const;

  /**
   * Sets the local device id.
   * Throws an error if used on a non-device node.
   * @see get_local_device_id
   */
  virtual void set_local_device_id( const index lsdid );

  /**
   * Gets the local device id.
   * Throws an error if used on a non-device node.
   * @see set_local_device_id
   */
  virtual index get_local_device_id() const;

  void set_node_id_( index ); //!< Set global node id


private:
  /** Return a new dictionary datum .
   *
   * This function is called by get_status_base() and returns a new
   * empty dictionary by default.  Some nodes may contain a
   * permanent status dictionary which is then returned by
   * get_status_dict_().
   */
  virtual DictionaryDatum get_status_dict_();


private:
  /**
   * Global Element ID (node ID).
   *
   * The node ID is unique within the network. The smallest valid node ID is 1.
   */
  index node_id_;

  /**
   * Local id of this node in the thread-local vector of nodes.
   */
  index thread_lid_;

  /**
   * Model ID.
   * It is only set for actual node instances, not for instances of class Node
   * representing model prototypes. Model prototypes always have model_id_==-1.
   * @see get_model_id(), set_model_id()
   */
  int model_id_;

  thread thread_; //!< thread node is assigned to
  thread vp_;     //!< virtual process node is assigned to
};


inline index
NESTObjectInterface::get_node_id() const
{
  return node_id_;
}


inline void
NESTObjectInterface::set_node_id_( index i )
{
  node_id_ = i;
}


inline int
NESTObjectInterface::get_model_id() const
{
  return model_id_;
}


inline void
NESTObjectInterface::set_model_id( int i )
{
  model_id_ = i;
}


inline bool
NESTObjectInterface::is_model_prototype() const
{
  return vp_ == invalid_thread;
}

inline void
NESTObjectInterface::set_thread( thread t )
{
  thread_ = t;
}

inline thread
NESTObjectInterface::get_thread() const
{
  return thread_;
}

inline void
NESTObjectInterface::set_vp( thread vp )
{
  vp_ = vp;
}

inline thread
NESTObjectInterface::get_vp() const
{
  return vp_;
}


inline void
NESTObjectInterface::set_thread_lid( const index tlid )
{
  thread_lid_ = tlid;
}

inline index
NESTObjectInterface::get_thread_lid() const
{
  return thread_lid_;
}


}
#endif