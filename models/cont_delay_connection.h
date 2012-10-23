/*
 *  cont_delay_connection.h
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

#ifndef CONT_DELAY_CONNECTION_H
#define CONT_DELAY_CONNECTION_H

/* BeginDocumentation
  Name: cont_delay_synapse - Synapse type for continuous delays

  Description:
  cont_delay_synapse relaxes the condition that NEST only implements delays
  which are an integer multiple of the time step h. A continuous delay is 
  decomposed into an integer part (delay_) and a double (delay_offset_) so
  that the actual delay is given by  delay_*h - delay_offset_. This can be
  combined with off-grid spike times.

  Transmits: SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent, DoubleDataEvent

  References: none
  FirstVersion: June 2007
  Author: Abigail Morrison
  SeeAlso: synapsedict, static_synapse, iaf_psc_alpha_canon
*/

#include "connection_het_wd.h"
#include "generic_connector.h"
#include <cmath>

namespace nest
{
  //class CommonProperties;

  class ContDelayConnection : public ConnectionHetWD {

  public:
  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  ContDelayConnection();
  
  /**
   * Copy constructor.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  ContDelayConnection(const ContDelayConnection &);

  /**
   * Default Destructor.
   */
  ~ContDelayConnection() {}

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status(DictionaryDatum & d) const;
  
  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status(const DictionaryDatum & d, ConnectorModel &cm);

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void set_status(const DictionaryDatum & d, index p, ConnectorModel &cm);

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void append_properties(DictionaryDatum & d) const;

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike Point in time of last spike sent.
   * \param cp common properties of all synapses (empty).
   */
  void send(Event& e, double_t t_lastspike, const CommonSynapseProperties &cp);

  // overloaded for all supported event types
  using Connection::check_event;
  void check_event(SpikeEvent&) {}
  void check_event(RateEvent&) {}
  void check_event(CurrentEvent&) {}
  void check_event(ConductanceEvent&) {}
  void check_event(DoubleDataEvent&) {}
  
 private:

  // data members of each connection
  double_t delay_offset_;              // fractional delay < h, total delay = delay_ - delay_offset_

  };

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
inline
void ContDelayConnection::send(Event& e, double_t, const CommonSynapseProperties &)
{
  e.set_receiver(*target_);
  e.set_weight(weight_);
  e.set_rport(rport_);
  double orig_event_offset = e.get_offset();
  double total_offset = orig_event_offset + delay_offset_;
  if (total_offset  < Time::get_resolution().get_ms())
    {
      e.set_delay(delay_);
      e.set_offset(total_offset);
    }
  else
    {
      e.set_delay(delay_ - 1);
      e.set_offset(total_offset - Time::get_resolution().get_ms());
    }
  e();
  //reset offset to original value
  e.set_offset(orig_event_offset);
}

} // of namespace nest

#endif // of #ifndef CONT_DELAY_CONNECTION_H
