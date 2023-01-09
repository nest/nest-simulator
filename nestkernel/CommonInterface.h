/*
 *  CommonInterface.h
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

#ifndef COMMON_INTERFACE_H
#define COMMON_INTERFACE_H


// Includes from nestkernel:
#include "deprecation_warning.h"
#include "event.h"
#include "histentry.h"
#include "nest_names.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node_collection.h"
#include "secondary_event.h"

// Includes from sli:
#include "dictdatum.h"

// Includes from boot
#include <boost/any.hpp>

#include <map>
#include <string>


namespace nest
{
class TimeConverter;

class CommonInterface
{
public:
  CommonInterface();
  CommonInterface( CommonInterface const& )
  {
  }
  virtual ~CommonInterface()
  {
  }


  /**
   * Returns true if the node has proxies on remote threads. This is
   * used to discriminate between different types of nodes, when adding
   * new nodes to the network.
   */
  virtual bool has_proxies() const;

  /**
   * Returns true if the node exists only once per process, but does
   * not have proxies on remote threads. This is used to
   * discriminate between different types of nodes, when adding new
   * nodes to the network.
   *
   * TODO: Is this true for *any* model at all? Maybe MUSIC related?
   */
  virtual bool one_node_per_process() const;


  /**
   * Returns true if the node sends/receives off-grid events. This is
   * used to discriminate between different types of nodes when adding
   * new nodes to the network.
   */
  virtual bool is_off_grid() const;

  /**
   * Return class name.
   * Returns name of node model (e.g. "iaf_psc_alpha") as string.
   * This name is identical to the name that is used to identify
   * the model in the interpreter's model dictionary.
   */
  virtual std::string get_name() const = 0;


  /**
   * Return model ID of the node.
   * Returns the model ID of the model for this node.
   * Model IDs start with 0.
   * @note The model ID is not stored in the model prototype instance.
   *       It is only set when actual nodes are created from a prototype.
   */

  virtual int get_model_id() const = 0;

  /**
   * Re-calculate time-based properties of the node.
   * This function is called after a change in resolution.
   */
  virtual void
  calibrate_time( const TimeConverter& )
  {
  }

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
   * Send an event to the receiving_node passed as an argument.
   * This is required during the connection handshaking to test,
   * if the receiving_node can handle the event type and receptor_type sent
   * by the source node.
   *
   * If dummy_target is true, this indicates that receiving_node is derived from
   * ConnTestDummyNodeBase and used in the first call to send_test_event().
   * This can be ignored in most cases, but Nodes sending DS*Events to their
   * own event hooks and then *Events to their proper targets must send
   * DS*Events when called with the dummy target, and *Events when called with
   * the real target, see #478.
   */
  virtual port send_test_event( Node& receiving_node, rport receptor_type, synindex syn_id, bool dummy_target );

public:
  /**
   * @defgroup event_interface Communication.
   * Functions and infrastructure, responsible for communication
   * between Nodes.
   *
   * Nodes communicate by sending an receiving events. The
   * communication interface consists of two parts:
   * -# Functions to handle incoming events.
   * -# Functions to check if a connection between nodes is possible.
   *
   * @see Event
   */

  /**
   * Check if the node can handle a particular event and receptor type.
   * This function is called upon connection setup by send_test_event().
   *
   * handles_test_event() function is used to verify that the receiver
   * can handle the event. It can also be used by the receiver to
   * return information to the sender in form of the returned port.
   * The default implementation throws an IllegalConnection
   * exception.  Any node class should define handles_test_event()
   * functions for all those event types it can handle.
   *
   * See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.
   *
   * @note The semantics of all other handles_test_event() functions is
   * identical.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual port handles_test_event( SpikeEvent&, rport receptor_type );
  virtual port handles_test_event( WeightRecorderEvent&, rport receptor_type );
  virtual port handles_test_event( RateEvent&, rport receptor_type );
  virtual port handles_test_event( DataLoggingRequest&, rport receptor_type );
  virtual port handles_test_event( CurrentEvent&, rport receptor_type );
  virtual port handles_test_event( ConductanceEvent&, rport receptor_type );
  virtual port handles_test_event( DoubleDataEvent&, rport receptor_type );
  virtual port handles_test_event( DSSpikeEvent&, rport receptor_type );
  virtual port handles_test_event( DSCurrentEvent&, rport receptor_type );
  virtual port handles_test_event( GapJunctionEvent&, rport receptor_type );
  virtual port handles_test_event( InstantaneousRateConnectionEvent&, rport receptor_type );
  virtual port handles_test_event( DiffusionConnectionEvent&, rport receptor_type );
  virtual port handles_test_event( DelayedRateConnectionEvent&, rport receptor_type );


  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( GapJunctionEvent& ge );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( InstantaneousRateConnectionEvent& re );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DiffusionConnectionEvent& de );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DelayedRateConnectionEvent& re );


  /** Set the model id.
   * This method is called by NodeManager::add_node() when a node is created.
   * @see get_model_id()
   */
  void set_model_id( int );


protected:
  virtual void populate_data() = 0;

  /** Return a new dictionary datum .
   *
   * This function is called by get_status_base() and returns a new
   * empty dictionary by default.  Some nodes may contain a
   * permanent status dictionary which is then returned by
   * get_status_dict_().
   */
  virtual DictionaryDatum get_status_dict_() const;

private:
  std::map< std::string, boost::any > data;
};

inline bool
CommonInterface::is_off_grid() const
{
  return false;
}


inline bool
CommonInterface::has_proxies() const
{
  return true;
}

inline bool
CommonInterface::one_node_per_process() const
{
  return false;
}

}


#endif