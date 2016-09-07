/*
 *  spikecounter.h
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

/**
 * \file spikecounter.h
 * Part of definition of volume_transmitter to record and manage
 * spike times and multiplicity of neurons releasing a
 * neuromodulator, which is needed for neuromodulated synaptic plasticity
 * (volume transmitter and neuromodulated synapses
 * are not included in the release version of NEST at the moment).
 * \author Wiebke Potjans
 * \note moved to separate file to avoid circular inclusion in node.h
 * \date december 2008
 */

#ifndef SPIKECOUNTER_H
#define SPIKECOUNTER_H

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

// entry in the spiking history
class spikecounter
{
public:
  spikecounter( double spike_time, double multiplicity );

  double spike_time_; // point in time when spike occurred (in ms)
  double multiplicity_;
};
}

#endif
