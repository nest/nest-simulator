/*
 *  secondary_event.cpp
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


#include "secondary_event_impl.h"

namespace nest
{

// ----- GapJunctionEvent ------------------------------------------------------

GapJunctionEvent*
GapJunctionEvent::clone() const
{
  return new GapJunctionEvent( *this );
}

// ----- InstantaneousRateConnectionEvent --------------------------------------

InstantaneousRateConnectionEvent*
InstantaneousRateConnectionEvent::clone() const
{
  return new InstantaneousRateConnectionEvent( *this );
}

// ----- DelayedRateConnectionEvent --------------------------------------------

DelayedRateConnectionEvent*
DelayedRateConnectionEvent::clone() const
{
  return new DelayedRateConnectionEvent( *this );
}

// ----- DiffusionConnectionEvent ----------------------------------------------

DiffusionConnectionEvent*
DiffusionConnectionEvent::clone() const
{
  return new DiffusionConnectionEvent( *this );
}

double
DiffusionConnectionEvent::get_drift_factor() const
{
  return drift_factor_;
}

double
DiffusionConnectionEvent::get_diffusion_factor() const
{
  return diffusion_factor_;
}

// ----- LearningSignalConnectionEvent -----------------------------------------

LearningSignalConnectionEvent*
LearningSignalConnectionEvent::clone() const
{
  return new LearningSignalConnectionEvent( *this );
}

// ----- SICEvent ---------------------------------------------------------------

SICEvent*
SICEvent::clone() const
{
  return new SICEvent( *this );
}

} // namespace nest
