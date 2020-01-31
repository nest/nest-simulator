/*
 *  voltmeter.h
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

#ifndef VOLTMETER_H
#define VOLTMETER_H

// C++ includes:
#include <vector>

// Includes from models:
#include "multimeter.h"

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "recording_device.h"

// Includes from sli:
#include "dictutils.h"
#include "name.h"


namespace nest
{

/* BeginUserDocs: voltmeter

voltmeter - Device to record membrane potential from neurons
############################################################

Synopsis: voltmeter Create

Description
+++++++++++

A voltmeter records the membrane potential (V_m) of connected nodes
to memory, file or stdout.

By default, voltmeters record values once per ms. Set the parameter
/interval to change this. The recording interval cannot be smaller
than the resolution.

Results are returned in the /events entry of the status dictionary,
which contains membrane potential as vector /V_m and pertaining
times as vector /times and node GIDs as /senders, if /withtime and
/withgid are set, respectively.

Accumulator mode:
voltmeter can operate in accumulator mode. In this case, values for all
recorded variables are added across all recorded nodes (but kept separate in
time). This can be useful to record average membrane potential in a
population.

To activate accumulator mode, either set /to_accumulator to true, or set
/record_to [ /accumulator ].  In accumulator mode, you cannot record to file,
to memory, to screen, with GID or with weight. You must activate accumulator
mode before simulating. Accumulator data is never written to file. You must
extract it from the device using GetStatus.

Remarks:

- The voltmeter model is implemented as a multimeter preconfigured to
    record /V_m.
- The set of variables to record and the recording interval must be set
    BEFORE the voltmeter is connected to any node, and cannot be changed
    afterwards.
- A voltmeter cannot be frozen.
- If you record with voltmeter in accumulator mode and some of the nodes
    you record from are frozen and others are not, data will only be collected
    from the unfrozen nodes. Most likely, this will lead to confusing results,
    so you should not use voltmeter with frozen nodes.

Parameters
++++++++++

    The following parameter can be set in the status dictionary:
    interval     double - Recording interval in ms

Examples:
SLI ] /iaf_cond_alpha Create /n Set
SLI ] /voltmeter Create /vm Set
SLI ] vm << /interval 0.5 >> SetStatus
SLI ] vm n Connect
SLI ] 10 Simulate
SLI ] vm /events get info
--------------------------------------------------
Name                     Type                Value
--------------------------------------------------
senders                  intvectortype       <intvectortype>
times                    doublevectortype    <doublevectortype>
V_m                      doublevectortype    <doublevectortype>
--------------------------------------------------
Total number of entries: 3


Sends
+++++

DataLoggingRequest

See also
++++++++

device, RecordingDevice, multimeter

EndUserDocs */

class voltmeter : public multimeter
{

public:
  voltmeter();
  voltmeter( const voltmeter& );
};

} // Namespace

#endif
