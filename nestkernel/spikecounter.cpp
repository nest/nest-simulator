/*
 *  spikecounter.cpp
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
 * \file spikecounter.cpp
 * Implementation of volume_transmitter to record and manage spike times and
 * multiplicity of neurons releasing a neuromodulator
 * (volume_transmitter is not included in the current release version of NEST)
 * \author Wiebke Potjans
 * \date december 2008
 * \note moved to separate file to avoid circular inclusion in node.h
 */

#include "spikecounter.h"

nest::spikecounter::spikecounter( double spike_time, double multiplicity )
  : spike_time_( spike_time )
  , multiplicity_( multiplicity )
{
}
