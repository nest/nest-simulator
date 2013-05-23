/*
 *  generic_connector_model.h
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

/*
 * first version
 * date: march 2006
 * author: Moritz Helias
 *
 * modified for new Connetion system
 * Jochen Martin Eppler, Moritz Helias
 * date march 2007
 */

#ifndef GENERICCONNECTORMODEL_H
#define GENERICCONNECTORMODEL_H

#include<vector>
#include<cstdlib>


#include "network.h"
#include "connector_model.h"
#include "common_synapse_properties.h"

namespace nest
{

/**
 * Template base class for ConnectorModels.
 * An actual ConnectorModel for a specific connector of type ConnectorT is obtained by deriving from the 
 * specialization to ConnectionT, CommonPropertiesT and ConnectorT and overriding the
 * pure virtual function double_t get_default_delay_().
 * ConnectionT is the class representing a single sonnection (=synapse).
 * CommonProperties conntains properties that are shared among all Connections of type ConnectionT.
 * ConnectorT is the connector class that contains the connections.
 */
template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
class GenericConnectorModelBase : public ConnectorModel
{
 public:

  /** Standard constructor. */
  GenericConnectorModelBase(Network& net, std::string name);

  /** Copy constructor. */
  GenericConnectorModelBase(const GenericConnectorModelBase& f, std::string name);
  
  ///////////////////////////////////////////////////////
  // Implementation of interface to ConnectionManager. //
  // virtual functions defined in ConnectorModel       //
  ///////////////////////////////////////////////////////

  /** see ConnectorModel::get_connector() */
  ConnectorT* get_connector();

  /** see ConnectorModel::reset() */ 
  void reset();
   
  /** see ConnectorModel::calibrate(const TimeConverter &) */ 
  void calibrate(const TimeConverter &);
   
  /** see ConnectorModel::get_status() */ 
  void get_status(DictionaryDatum& d) const;

  /** see ConnectorModel::set_status() */
  void set_status(const DictionaryDatum& d);

  /** needed for heterosynaptic connections */
  Node* get_registering_node();

  


  ///////////////////////////////////////////////////////
  // Public member functions for the interface to      //
  // Connection and or Connector.                      //
  // These functions do not need to be virtual         //
  ///////////////////////////////////////////////////////

  
  /**
   * Tell the connector model that the default delay has been used to create a synapse.
   * This is needed by the delay checking.
   */
  inline
  void used_default_delay();

  /**
   * Return the receptor type for this connection.
   */
  inline
  port get_receptor_type() const;
 
  /** 
   * Return the default connection which serves as as prototype to create a new
   * connection from it. This is done via ConnectionT's copy constructor,
   * which therefore has to be defined propertly
   */
  inline
  const ConnectionT & get_default_connection() const;

  /**
   * Returns the comom properties for all synapses.
   * These contain parameters which are the same for all synapses.
   * We cannot return a const reference here, since some connections
   * want to store information in the CommonProperties object.
   */
  inline
  CommonPropertiesT & get_common_properties();
  

 protected:

  ///////////////////////////////
  // private member functions  //
  ///////////////////////////////
  
  /**
   * This function has to be overridden by the derived class in order to return the default delay. The default delay may either reside
   * in the default connection or in the common properties object. This allows us to write generic code for connections with
   * homogeneous delays and heterogeneous delays.
   */
  virtual double_t get_default_delay_() = 0;


  ConnectionT defaults_;        //!< Connection object to store default parameters for one synapse
  CommonPropertiesT common_props_; //!< Common proerties to all synapses
  port receptor_type_;          //!< The default receptor used for new connections.
};

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
    GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::GenericConnectorModelBase(Network& net, std::string name)
    : ConnectorModel(net, name),
      defaults_(),
    common_props_(),
    receptor_type_(0)
{
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
    GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::GenericConnectorModelBase(const GenericConnectorModelBase& f, std::string name)
    : ConnectorModel(f, name),
      defaults_(f.defaults_),
    common_props_(f.common_props_),
    receptor_type_(f.receptor_type_)
{ 
}

///////////////////////////////////////////////////////
// Imlementation of ConnectorModel virtual functions //
///////////////////////////////////////////////////////

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
void GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::reset()
{
  min_delay_ = Time::pos_inf();
  max_delay_ = Time::neg_inf();
    
  // create a new default Connection with default parameters
  defaults_ = ConnectionT();
  // create new commom properties with default values
  common_props_ = CommonPropertiesT();
}


template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
void GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::get_status(DictionaryDatum& d) const
{
  
  // first get properties common to all synapses
  // these are stored only once (not within each Connection)
  common_props_.get_status(d);
  (*d)["property_object"]=  (size_t) &common_props_;
  // then get default properties for individual synapses
  defaults_.get_status(d);

  (*d)["min_delay"] = get_min_delay().get_ms();
  (*d)["max_delay"] = get_max_delay().get_ms();
  (*d)[names::receptor_type] = receptor_type_;
  (*d)["num_connections"] = get_num_connections();
  (*d)["num_connectors"] = get_num_connectors();
  (*d)["synapsemodel"] = LiteralDatum(get_name());
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
void GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::set_status(const DictionaryDatum& d)
{

  double_t min_delay, max_delay, new_delay;
  bool min_delay_updated = updateValue<double_t>(d, "min_delay", min_delay);
  bool max_delay_updated = updateValue<double_t>(d, "max_delay", max_delay);

  // the delay might also be updated, so check new_min_delay and new_max_delay against new_delay, if given
  if ( ! updateValue<double_t>(d, "delay", new_delay) )
      new_delay = get_default_delay_(); // call overridden virtual function
                                // depending on its implementation, it returns the default delay from 
                                // the default connection or from common properties object

  if (min_delay_updated xor max_delay_updated)
   net_.message(SLIInterpreter::M_ERROR,
		"SetDefaults", "Both min_delay and max_delay have to be specified");

  if (min_delay_updated && max_delay_updated)
  {
    if (num_connections_ > 0)      
      net_.message(SLIInterpreter::M_ERROR, 
		   "SetDefaults", 
		   "Connections already exist. Please call ResetKernel first");
    else if ( min_delay > new_delay )
      net_.message(SLIInterpreter::M_ERROR,
		   "SetDefaults", 
		   "min_delay is not compatible with default delay");
    else if ( max_delay < new_delay )
      net_.message(SLIInterpreter::M_ERROR,
		   "SetDefaults", 
		   "max_delay is not compatible with default delay");
    else if ( min_delay < Time::get_resolution().get_ms() )
      net_.message(SLIInterpreter::M_ERROR,
		   "SetDefaults", 
		   "min_delay must be greater than or equal to resolution");
    else if ( max_delay < Time::get_resolution().get_ms() )
      net_.message(SLIInterpreter::M_ERROR,
		   "SetDefaults", 
		   "max_delay must be greater than or equal to resolution");
    else
    {
      min_delay_ = Time(Time::ms(min_delay));
      max_delay_ = Time(Time::ms(max_delay));
      user_set_delay_extrema_ = true;
    }
  }

  // common_props_.set_status(d, *this) AND defaults_.set_status(d, *this);
  // has to be done after adapting min_delay / max_delay, since Connection::set_status
  // and CommonProperties::set_status might want to check the delay
  
  // store min_delay_, max_delay_
  // calling set_status will check the delay.
  // and so may modify min_delay, max_delay, if the specified delay exceeds one of these bounds
  // we have to save min/max_delay because we dont know, if the default will ever be used
  Time min_delay_tmp = min_delay_;
  Time max_delay_tmp = max_delay_;

  common_props_.set_status(d, *this);
  defaults_.set_status(d, *this);

  // restore min_delay_, max_delay_
  min_delay_ = min_delay_tmp;
  max_delay_ = max_delay_tmp;
  
  // we've possibly just got a new default delay. So enforce checking netxt time it is used
  default_delay_needs_check_ = true; 

#ifdef HAVE_MUSIC
  // We allow music_channel as alias for receptor_type during connection setup
  updateValue<long_t>(d, names::music_channel, receptor_type_);
#endif
  updateValue<long_t>(d, names::receptor_type, receptor_type_);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
ConnectorT * GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::get_connector()
{
  num_connectors_++;
  return new ConnectorT(*this);
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
void GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::calibrate(const TimeConverter &tc)
{
  // calibrate the dalay of the default properties here
  defaults_.calibrate(tc);
  
  // Calibrate will be called after a change in resolution, when there are no network elements present.

  // calibrate any time objects that might reside in CommonProperties
  common_props_.calibrate(tc);

  min_delay_ = tc.from_old_steps(min_delay_.get_steps());
  max_delay_ = tc.from_old_steps(max_delay_.get_steps());
}



template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
Node* GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::get_registering_node() 
{
  return common_props_.get_node(); //should't work w/o return should it?
}



template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
inline
const ConnectionT & GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::get_default_connection() const
{
  return defaults_;
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
inline
CommonPropertiesT & GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::get_common_properties()
{
  return common_props_;
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
inline
port GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::get_receptor_type() const
{
  return receptor_type_;
}


template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
inline
void GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::used_default_delay()
{
  // if not used before, check now. Solves bug #138, MH 08-01-08
  // replaces whole delay checking for the default delay, see bug #217, MH 08-04-24
  // get_default_delay_ must be overridded by derived class to return the correct default delay
  // (either from commonprops or default connection)
  if (default_delay_needs_check_)
    {
      if ( !check_delay( get_default_delay_() ) ) 
	throw BadDelay(get_default_delay_());
      default_delay_needs_check_ = false;
    }

}




/**
 * Template class for ConnectorModels with heterogeneous delays.
 * An actual ConnectorModel for a specific connector of type ConnectorT is obtained by
 * specialization to ConnectionT, CommonPropertiesT and ConnectorT.
 * ConnectionT is the class representing a single sonnection (=synapse).
 * CommonProperties conntains properties that are shared among all Connections of type ConnectionT.
 * ConnectorT is the connector class that contains the connections.
 */
template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
class GenericConnectorModel 
  : public GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >
{

 public:

  /** Standard constructor. */
  GenericConnectorModel(Network& net, std::string name) 
   : GenericConnectorModelBase< ConnectionT, 
                                CommonPropertiesT, 
                                ConnectorT
                              > (net, name)
 { }

  /** Copy constructor. */
  GenericConnectorModel(const GenericConnectorModel& f, std::string name)
   :  GenericConnectorModelBase< ConnectionT, 
                                 CommonPropertiesT, 
                                 ConnectorT 
                               > (f, name)
 { }

  ConnectorModel* clone(std::string) const;

 private:
  /**
   * Returns the default delay from the prototype connection.
   */   
  double_t get_default_delay_();

};

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
ConnectorModel* GenericConnectorModel< ConnectionT, CommonPropertiesT, ConnectorT >::clone(std::string name) const
{
  return new GenericConnectorModel(*this, name); // calls copy construtor
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
double_t  GenericConnectorModel<  ConnectionT, CommonPropertiesT, ConnectorT >::get_default_delay_()
 {
   return GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::defaults_.get_delay();
 }

/**
 * Template class for ConnectorModels with homogeneous delays.
 * An actual ConnectorModel for a specific connector of type ConnectorT is obtained by
 * specialization to ConnectionT, CommonPropertiesT and ConnectorT.
 * ConnectionT is the class representing a single sonnection (=synapse).
 * CommonProperties conntains properties that are shared among all Connections of type ConnectionT.
 * ConnectorT is the connector class that contains the connections.
 */
template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
class GenericConnectorModelHomD
  : public GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >
{

 public:
 
  /** Standard constructor. */
  GenericConnectorModelHomD(Network& net, std::string name)
   : GenericConnectorModelBase< ConnectionT, 
                                CommonPropertiesT, 
                                ConnectorT 
                              > (net, name)
 { }

  /** Copy constructor. */
  GenericConnectorModelHomD(const GenericConnectorModelHomD& f, std::string name)
   :  GenericConnectorModelBase< ConnectionT, 
                                 CommonPropertiesT, 
                                 ConnectorT 
                               > (f, name)
 { }
  

  ConnectorModel* clone(std::string) const;

 private:
  /**
   * Return the default delay from the common property object.
   */
  double_t get_default_delay_();

};

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
ConnectorModel* GenericConnectorModelHomD< ConnectionT, CommonPropertiesT, ConnectorT >::clone(std::string name) const
{
  return new GenericConnectorModelHomD(*this, name); // calls copy construtor
}

template< typename ConnectionT, typename CommonPropertiesT, typename ConnectorT >
double_t GenericConnectorModelHomD<  ConnectionT, CommonPropertiesT, ConnectorT >::get_default_delay_()
{
  return GenericConnectorModelBase< ConnectionT, CommonPropertiesT, ConnectorT >::common_props_.get_delay();
}

 

/////////////////////////////////////////////////////////////////////////////////
// Convenient versions of template functions for registering new synapse types //
// by modules                                                                  //
/////////////////////////////////////////////////////////////////////////////////

// synapses wit heterogeneous delays

/**
 * registering new synapse with heterogeneous delay
 */
template <class ConnectionT, class ConnectorT >
index register_prototype_connection_connector(Network& net, const std::string &name)
{
  ConnectorModel* prototype = new GenericConnectorModel < ConnectionT,
                                                          CommonSynapseProperties,
                                                          ConnectorT
                                                        > (net, name);
 
  return net.register_synapse_prototype(prototype);
}

template <class ConnectionT, class ConnectorT, class CommonPropertiesT>
index register_prototype_connection_connector_commonproperties(Network& net, const std::string &name)
{
  ConnectorModel* prototype = new GenericConnectorModel < ConnectionT,
                                                          CommonPropertiesT,
                                                          ConnectorT
                                                        > (net, name);
  return net.register_synapse_prototype(prototype);  
}

// synapses with homogeneous delays

/**
 * registering new synapse with homogeneous delay
 */
template< typename ConnectionT, typename ConnectorT, typename CommonPropertiesT >
index register_prototype_connection_connector_commonproperties_hom_d(Network& net, const std::string &name)
{
  ConnectorModel* prototype = new GenericConnectorModelHomD < ConnectionT,
                                                              CommonPropertiesT,
                                                              ConnectorT
                                                            > (net, name);
  return net.register_synapse_prototype(prototype);
}


} // namespace nest

#endif /* #ifndef GENERICCONNECTORMODEL_H */

