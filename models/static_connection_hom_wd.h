/*
 *  static_connection_hom_wd.h
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
   Name: static_synapse_hom_wd - Synapse type for static connections with homogeneous weight and delay.
   
   Description:
     static_synapse_hom_wd does not support any kind of plasticity. It simply stores
     the parameters target, and receiver port for each connection and uses a common 
     weight and delay for all connections.

   Transmits: SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent, DataLoggingRequest, DoubleDataEvent

   Parameters:
     No Parameters

   References:
     No References
   FirstVersion: April 2008
   Author: Susanne Kunkel, Moritz Helias
   SeeAlso: synapsedict, static_synapse
*/

#ifndef STATICCONNECTION_HOM_WD_H
#define STATICCONNECTION_HOM_WD_H

#include "connection_hom_wd.h"

namespace nest
{

/**
 * Class representing a static connection. A static connection has the properties weight, delay and receiver port.
 * This class also serves as the base class for dynamic synapses (like TsodyksConnection, STDPConnection).
 * A suitale Connector containing these connections can be obtained from the template GenericConnector.
 */
class StaticConnectionHomWD : public ConnectionHomWD
{

 public:

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  StaticConnectionHomWD() : ConnectionHomWD() {}

  /**
   * Default Destructor.
   */
  ~StaticConnectionHomWD() {}

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

#endif /* #ifndef STATICCONNECTION_HOM_WD_H */
