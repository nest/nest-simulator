/*
 *  static_connection.h
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


/* BeginDocumentation
  Name: static_synapse - Synapse type for static connections.

  Description:
   static_synapse does not support any kind of plasticity. It simply stores
   the parameters target, weight, delay and receiver port for each connection.

  FirstVersion: October 2005
  Author: Jochen Martin Eppler, Moritz Helias

  Transmits: SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent, DoubleDataEvent, DataLoggingRequest
  
  Remarks: Refactored for new connection system design, March 2007

  SeeAlso: synapsedict, tsodyks_synapse, stdp_synapse
*/

#ifndef STATICCONNECTION_H
#define STATICCONNECTION_H

#include "connection_het_wd.h"

namespace nest
{

/**
 * Class representing a static connection. A static connection has the properties weight, delay and receiver port.
 * This class also serves as the base class for dynamic synapses (like TsodyksConnection, STDPConnection).
 * A suitale Connector containing these connections can be obtained from the template GenericConnector.
 */
class StaticConnection : public ConnectionHetWD
{

 public:

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  StaticConnection() : ConnectionHetWD() {}

  /**
   * Default Destructor.
   */
  ~StaticConnection() {}

  // overloaded for all supported event types
  using Connection::check_event;
  void check_event(SpikeEvent&) {}
  void check_event(RateEvent&) {}
  void check_event(DataLoggingRequest&) {}
  void check_event(CurrentEvent&) {}
  void check_event(ConductanceEvent&) {}
  void check_event(DoubleDataEvent&) {}
  void check_event(DSSpikeEvent&) {}
  void check_event(DSCurrentEvent&) {}

};

} // namespace

#endif /* #ifndef STATICCONNECTION_H */
