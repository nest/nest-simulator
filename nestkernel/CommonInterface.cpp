/*
 *  CommonInterface.cpp
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


#include "CommonInterface.h"

namespace nest
{

CommonInterface::CommonInterface()
{
}

/**
 * Default implementation of check_connection just throws IllegalConnection
 */
port
CommonInterface::send_test_event( Node&, rport, synindex, bool )
{
  throw IllegalConnection(
    "Source node does not send output.\n"
    "  Note that recorders must be connected as Connect(neuron, recorder)." );
}

port
CommonInterface::handles_test_event( SpikeEvent&, rport )
{
  throw IllegalConnection(
    "The target node or synapse model does not support spike input.\n"
    "  Note that volt/multimeters must be connected as Connect(meter, neuron)." );
}

port
CommonInterface::handles_test_event( WeightRecorderEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support weight recorder events." );
}


port
CommonInterface::handles_test_event( RateEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support rate input." );
}

port
CommonInterface::handles_test_event( CurrentEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support current input." );
}

port
CommonInterface::handles_test_event( DataLoggingRequest&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support data logging requests." );
}

port
CommonInterface::handles_test_event( ConductanceEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support conductance input." );
}

port
CommonInterface::handles_test_event( DoubleDataEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support double data event." );
}


port
CommonInterface::handles_test_event( DSSpikeEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support spike input." );
}

port
CommonInterface::handles_test_event( DSCurrentEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support DS current input." );
}

port
CommonInterface::handles_test_event( GapJunctionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support gap junction input." );
}

void
CommonInterface::sends_secondary_event( GapJunctionEvent& )
{
  throw IllegalConnection( "The source node does not support gap junction output." );
}

port
CommonInterface::handles_test_event( InstantaneousRateConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support instantaneous rate input." );
}


void
CommonInterface::sends_secondary_event( InstantaneousRateConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support instantaneous rate output." );
}

port
CommonInterface::handles_test_event( DiffusionConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support diffusion input." );
}

void
CommonInterface::sends_secondary_event( DiffusionConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support diffusion output." );
}

port
CommonInterface::handles_test_event( DelayedRateConnectionEvent&, rport )
{
  throw IllegalConnection( "The target node or synapse model does not support delayed rate input." );
}

void
CommonInterface::sends_secondary_event( DelayedRateConnectionEvent& )
{
  throw IllegalConnection( "The source node does not support delayed rate output." );
}

DictionaryDatum
CommonInterface::get_status_dict_() const
{
  return DictionaryDatum( new Dictionary );
}


}