/*
 *  connection_hom_wd.h
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

#ifndef CONNECTION_HOM_WD_H
#define CONNECTION_HOM_WD_H

/* BeginDocumentation
   Name: static_synapse_hom_wd - Static synapse type
         using homogeneous weight and delay, i.e. all synapses
         feature the same w and d.

   FirstVersion: April 2008
   Author: Moritz Helias, Susanne Kunkel
   SeeAlso: synapsedict, static_synapse
*/

#include "connection.h"
#include "archiving_node.h"
#include <cmath>

namespace nest
{

  /**
   * Class containing the common properties for all synapses of type ConnectionHomWD.
   */
  class CommonPropertiesHomWD : public CommonSynapseProperties
    {
      friend class ConnectionHomWD;

      public:

      /**
       * Default constructor.
       * Sets all property values to defaults.
       */
      CommonPropertiesHomWD();

      /**
       * Get all properties and put them into a dictionary.
       */
      void get_status(DictionaryDatum & d) const;

      /**
       * Return the delay of all connections.
       */
      double_t get_delay() const;

      /**
       * Set properties from the values given in dictionary.
       */
      void set_status(const DictionaryDatum & d, ConnectorModel& cm);

    private:

      // data members common to all connections
      double_t weight_;
      long_t delay_;
    };

inline
double_t CommonPropertiesHomWD::get_delay() const
{
  return Time(Time::step(delay_)).get_ms();
}

  /**
   * Class representing an STDP connection with homogeneous parameters, i.e. parameters are the same for all synapses.
   */
  class ConnectionHomWD : public Connection
  {

  public:
  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  ConnectionHomWD();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  ConnectionHomWD(const ConnectionHomWD& c);

  /**
   * Default Destructor.
   */
  ~ConnectionHomWD() {}

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status(DictionaryDatum & d) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status(const DictionaryDatum &, ConnectorModel &) { }

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */
  void set_status(const DictionaryDatum &, index, ConnectorModel &) { }

  /**
   * No weight and delay in this base class,
   * but generic_connector expects set_weight() and set_delay().
   * IllegalConnection exception is thrown.
   */
  //@{
  void set_delay(double_t);
  void set_weight(double_t);
  //@}

  /**
   * Needed by Generic connector.
   */	
  void calibrate(const TimeConverter &) {};

  /**
   * Create new empty arrays for the properties of this connection in the given
   * dictionary. It is assumed that they are not existing before.
   */
  void initialize_property_arrays(DictionaryDatum & d) const;

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void append_properties(DictionaryDatum & d) const;

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike Point in time of last spike sent.
   */
  void send(Event& e, double_t t_lastspike, const CommonPropertiesHomWD &);

};

inline
void ConnectionHomWD::set_delay(double_t)
{
  throw IllegalConnection();
}

inline
void ConnectionHomWD::set_weight(double_t)
{
  throw IllegalConnection();
}

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
inline
void ConnectionHomWD::send(Event& e, double_t, const CommonPropertiesHomWD &cp)
{
  e.set_weight(cp.weight_);
  e.set_delay(cp.delay_);
  e.set_receiver(*target_);
  e.set_rport(rport_);
  e();
}

} // namespace nest

#endif // CONNECTION_HOM_WD_H
