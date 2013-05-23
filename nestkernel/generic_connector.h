/*
 *  generic_connector.h
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

#ifndef GENERICCONNECTOR_H
#define GENERICCONNECTOR_H

#include "dictutils.h"
#include "nest_time.h"
#include "connector.h"
#include "node.h"
#include "event.h"
#include "generic_connector_model.h"
#include <algorithm>
#include <vector>

#include "spikecounter.h"
#include "nest_names.h"
#include "connectiondatum.h"
namespace nest {

/**
 * Template for the default implementation of a Connector. This template may be used to
 * create a Connector for a given connection of type ConnectionT.
 * It allows to create new connections and to store them in a vector.
 * This template can serve as a base class.
 * 
 * ConnectionT:       type of connections to store
 * CommonPropertiesT: type of common propeerties object storing parameters which are common to all synapses
 * ConnectorModelT:   type of ConnectorModel which is the factory of these Connectors
 *
 * @note This class uses a deque to store synapses, since deques can adapt their size without need of copying
 *       their content, as is required for vectors. See http://www.codeproject.com/vcpp/stl/vector_vs_deque.asp
 *       for a discussion.
 */
template <typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT> 
class GenericConnectorBase : public Connector
{

 public:
  // explicit type declaration needed by compiler
  typedef typename std::vector< ConnectionT, std::allocator< ConnectionT > >::iterator ConnIter;
  typedef typename std::vector< ConnectionT, std::allocator< ConnectionT > >::const_iterator ConnConstIter;

  /**
   * Default constructor.
   * \param cm ConnectorModel, which created this Connector.
   */
  GenericConnectorBase(ConnectorModelT &cm);

  GenericConnectorBase(GenericConnectorBase &rhs)
    : connections_(),
    connector_model_(rhs.connector_model_)
      {}

  /**
   * Default destructor.
   */
  virtual ~GenericConnectorBase() {}

  /**
   * Register a new connection at the sender side.
   */ 
  void register_connection(Node&, Node&);

  /**
   * Register a new connection at the sender side.
   * Use given weight and delay.
   */ 
  void register_connection(Node&, Node&, double_t, double_t);

  /**
   * Register a new connection at the sender side. 
   * Use given dictionary for parameters.
   */ 
  void register_connection(Node&, Node&, DictionaryDatum&);
  
  /**
   * Register a new connection at the sender side.
   */ 
  void register_connection(Node&, Node&, ConnectionT&, port);
 
 /**
   * Register many connections in bulk. 
   * The dictionary contains all parameters as array.
   */ 
  void register_connections(DictionaryDatum&);

  /**
   * Return a list of ports (legacy version)
   */
  std::vector<long>* find_connections(DictionaryDatum params) const;

  /**
   * Return the list of ports at which post_gid is connected. 
   * Return the list of ports that connect to the provided post_gid.
   */
  void get_connections(size_t source_gid, size_t thrd, size_t synapse_id, ArrayDatum &conns) const;
  void get_connections(size_t source_gid, size_t target_gid, size_t thrd, size_t synapse_id, ArrayDatum &conns) const;

  size_t get_num_connections() const
  {
    return connections_.size();
  }



  /**
   * Get properties for all connections handled by this connector.
   * \param d dictionary to store properties in.
   */ 
  void get_status(DictionaryDatum & d) const;

  /**
   * Set properties for all connections handled by this connector.
   * \param d dictionary which contains values for the properties.
   */ 
  void set_status(const DictionaryDatum & d);

  /**
   * Get properties of synapse p of this connector.
   * \param d dictionary to store properties in.
   */ 
  void get_synapse_status(DictionaryDatum & d, port p) const;

  /**
   * Set properties of synapse p of this connector.
   * \param d dictionary which contains values for the properties.
   */ 
  void set_synapse_status(const DictionaryDatum & d, port p);

  /**
   * Send an event to this connector.
   * The connector will propagate it to all its targets.
   */
  void send(Event& e);

  /*
   * Re-calibrate the delays in all connections
   */
  void calibrate(const TimeConverter &);

  /**
   * Check, whether a connection to this node already exists.
   * Prerequisite: operator== must be defined for ConnectionT with
   * const Node & on the right hand side 
   * \see StaticConnection
   */
  port connection_exists(const Node &r) const;


  /** triggers a weight update in neuromodulated synapses
   * based on spike times and multiplicity of a neuron 
   * population releasing a neuromodulator; the method is called
   * by the volume transmitter (attention: neuromodulated synapses 
   * and volume transmitter are not in the current release version
   */
  void trigger_update_weight(const std::vector<spikecounter>& neuromodulator_spikes, double_t t_trig);

 protected:
  std::vector<ConnectionT> connections_;
  ConnectorModelT &connector_model_;

  // point in time of last spike transmitted
  double_t t_lastspike_;

};


template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::GenericConnectorBase(ConnectorModelT &cm)
  : connector_model_(cm),
    t_lastspike_(0.0)
{
  //  std::cerr << "Connector with common object at "<< (size_t) &connector_model_.get_common_properties() << '\n';
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::register_connection(Node& s, Node& r)
{
  // create a new instance of the default connection
  ConnectionT cn = ConnectionT( connector_model_.get_default_connection() );

  // tell the connector model, that we used the default delay
  connector_model_.used_default_delay();

  register_connection(s, r, cn, connector_model_.get_receptor_type());
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::register_connection(Node& s, Node& r, double_t w, double_t d)
{

  // We have to convert the delay in ms to a Time object then to steps and back the ms again
  // in order to get the value in ms which can be represented with an integer number of steps
  // in the currently chosen Time representation.
  // See also bug #217, MH 08-04-23
  if ( !connector_model_.check_delay( Time(Time::step(Time(Time::ms(d)).get_steps())).get_ms() ) )
      throw BadDelay(d);

  // create a new instance of the default connection
  ConnectionT cn = ConnectionT( connector_model_.get_default_connection() );
  cn.set_weight(w);
  cn.set_delay(d);

  register_connection(s, r, cn, connector_model_.get_receptor_type());
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::register_connection(Node& s, Node& r, DictionaryDatum& d)
{
  // check delay
  double_t delay = 0.0;
  if ( updateValue<double_t>(d, names::delay, delay) )
  {
    if ( !connector_model_.check_delay( Time(Time::step(Time(Time::ms(delay)).get_steps())).get_ms() ) )
      throw BadDelay(delay);
  }
  else
    connector_model_.used_default_delay();

  // create a new instance of the default connection
  ConnectionT cn = ConnectionT( connector_model_.get_default_connection() );
  cn.set_status(d, connector_model_);
  
  port receptor_type = connector_model_.get_receptor_type();

#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue<long_t>(d, names::music_channel, receptor_type);
#endif
  updateValue<long_t>(d, names::receptor_type, receptor_type);

  register_connection(s, r, cn, receptor_type);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
inline
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::register_connection(Node& s, Node& r, ConnectionT &cn, port receptor_type)
{
  cn.check_connection(s, r, receptor_type, t_lastspike_);
  Node* n = connector_model_.get_registering_node(); //if the connection is a heterosynatpic one, it gets the node which contributes to heterosynaptic plasticity 

  connections_.push_back(cn);
  if(n!=0 && connections_.size()==1) //register for first connection connector in registered node  
  {
    n->register_connector(*this); //register node in heterosynapse
  }
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
std::vector<long>* GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::find_connections(DictionaryDatum params) const
{
  long postgid = -1;
  bool use_postgid = updateValue<long>(params, names::target, postgid);
  
  std::vector<long>* p  = new std::vector<long>;
  for (size_t i = 0; i < connections_.size(); ++i)
    if (!use_postgid || 
	(use_postgid && connections_[i].get_target()->get_gid() == static_cast<index>(postgid)))
      p->push_back(i);        
  return p;
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::get_connections(size_t source_gid, size_t thrd, size_t synapse_id, ArrayDatum &conns) const
{
  for (size_t prt = 0; prt < connections_.size(); ++prt)
      conns.push_back(new ConnectionDatum(ConnectionID(source_gid, connections_[prt].get_target()->get_gid() , thrd, synapse_id, prt)));        
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
  void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::get_connections(size_t source_gid, size_t target_gid, size_t thrd, size_t synapse_id, ArrayDatum &conns) const
{
  for (size_t prt = 0; prt < connections_.size(); ++prt)
    if (connections_[prt].get_target()->get_gid() == target_gid)
      conns.push_back(new ConnectionDatum(ConnectionID(source_gid, target_gid, thrd, synapse_id, prt)));        
}


template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::get_status(DictionaryDatum & d) const
{
  // Initializes empty arrays in the dictionary
  connections_.begin()->initialize_property_arrays(d);
  
  // here we get the properties for every connection and append them to the
  // appropriate array in the dictionary
  for (ConnConstIter iter = connections_.begin(); iter != connections_.end(); ++iter)
    iter->append_properties(d);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::set_status(const DictionaryDatum & d)
{
  // traverse the dictionary and check if all contained arrays are of length
  // connections_.size(), else throw DimensionMismatch();
  TokenMap::iterator iter;
  for (iter = d->begin(); iter != d->end(); ++iter)
  {
    ArrayDatum* ad = dynamic_cast<ArrayDatum*>((iter->second).datum());
    if (ad != 0)
      if (ad->size() != connections_.size())
        throw DimensionMismatch(connections_.size(), ad->size());
  }
  
  // here we set properties for every connection. The parameters are stored
  // in a dictionary containing an array for each parameter
  size_t i = 0;
  for (ConnIter iter = connections_.begin(); iter != connections_.end(); ++iter, ++i)
    iter->set_status(d, i, connector_model_);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::get_synapse_status(DictionaryDatum & d, port p) const
{
  assert (p >= 0 && static_cast<size_t>(p) < connections_.size());
  connections_[p].get_status(d);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::set_synapse_status(const DictionaryDatum & d, port p)
{
  assert (p >= 0 && static_cast<size_t>(p) < connections_.size());
  connections_[p].set_status(d, connector_model_);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::send(Event& e)
{
  /* Indexed lookup in deque's is slow. We therefore use iterators to 
     go through the connections and use an additional index variable
     to keep track of the port number.
     The iterator cannot be const, since send() may modify connection properties.
   */  
  ConnIter conn_it;
  size_t i;
  for ( conn_it = connections_.begin(), i = 0; 
        conn_it != connections_.end();
        ++conn_it, ++i )
  {
    e.set_port(i);
    conn_it->send(e, t_lastspike_, connector_model_.get_common_properties());
  }

  t_lastspike_ = e.get_stamp().get_ms();
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 

void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::calibrate(const TimeConverter &tc)
{  
  for (ConnIter it = connections_.begin(); it < connections_.end(); ++it)
    it->calibrate(tc);
}



template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
void GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::trigger_update_weight(const std::vector<spikecounter>& neuromodulator_spikes, double_t t_trig)
{  
  for (ConnIter it = connections_.begin(); it < connections_.end(); ++it)
    it->trigger_update_weight(neuromodulator_spikes, t_trig, connector_model_.get_common_properties());
}



template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorModelT > 
port GenericConnectorBase< ConnectionT, CommonPropertiesT, ConnectorModelT >::connection_exists(const Node &r) const
{
  ConnConstIter it = connections_.begin();
  while ( it->get_target() != &r && it != connections_.end() )
    ++it;
  
  if (it != connections_.end())
    return it->get_rport();
  else
    return invalid_port_;
}




/**
 * GenericConnector. Specialization which uses the GenericConnector and the GenericConnectorModel directly.
 * Only ConnectionT and CommonPropertiesT have to be specified.
 * This is the generic connector for most connections.
 * A prerequisite to use this template is that the dynamics of a connection must be defined locally,
 * i.e. it must be independent of the dynamics of all other connections.
 */
template <typename ConnectionT, typename CommonPropertiesT> 
class GenericConnector : public GenericConnectorBase< ConnectionT,
						      CommonPropertiesT,
						      GenericConnectorModelBase< ConnectionT,
									     CommonPropertiesT,
									     GenericConnector<ConnectionT, CommonPropertiesT>
									   >
						    >
{
  typedef GenericConnectorModelBase< ConnectionT, CommonPropertiesT, GenericConnector > GCMT;

 public:
  GenericConnector(GCMT &cm) :
    GenericConnectorBase< ConnectionT, CommonPropertiesT, GCMT >(cm)
    {}
};


/////////////////////////////////////////////////////////////////////////////////
// Convenient versions of template functions for registering new synapse types //
// by modules                                                                  //
/////////////////////////////////////////////////////////////////////////////////

/**
 * Register a synape with default Connector and without any common properties.
 */
template <class ConnectionT>
index register_prototype_connection(Network& net, const std::string &name)
{
  ConnectorModel* prototype = new GenericConnectorModel < ConnectionT,
                                                          CommonSynapseProperties,
                                                          GenericConnector < ConnectionT, 
                                                                             CommonSynapseProperties
                                                                           >
                                                        > (net, name);
  
  return net.register_synapse_prototype(prototype);
}

/**
 * Register a synapse with a given Connection containing the delay, 
 * using the default Connector and given common properties, which do not contain the delay.
 */
template <class ConnectionT, class CommonPropertiesT>
index register_prototype_connection_commonproperties(Network& net, const std::string &name)
{
  ConnectorModel* prototype = new GenericConnectorModel < ConnectionT,
                                                          CommonPropertiesT,
                                                          GenericConnector < ConnectionT, 
                                                                             CommonPropertiesT
                                                                           >
                                                          > (net, name);
  return net.register_synapse_prototype(prototype);
}

/**
 * Register a synapse with a given Connection not containing the delay,
 * using the default Connector and given common properties that contain the delay.
 */
template <class ConnectionT, class CommonPropertiesT>
index register_prototype_connection_commonproperties_hom_d(Network& net, const std::string &name)
{
  ConnectorModel* prototype = new GenericConnectorModelHomD < ConnectionT,
                                                              CommonPropertiesT,
                                                              GenericConnector < ConnectionT, 
                                                                                 CommonPropertiesT
                                                                                >
                                                            > (net, name);
  return net.register_synapse_prototype(prototype);
}

} // namespace

#endif
