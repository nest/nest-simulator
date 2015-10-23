/*
 *  precisemodule.h
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

#ifndef PRECISEMODULE_H
#define PRECISEMODULE_H

// Includes from sli:
#include "slimodule.h"

namespace nest
{
/**
 * Module supplying models support precise spike timing.
 */
class PreciseModule : public SLIModule
{
public:
  PreciseModule();
  ~PreciseModule();

  /**
   * Initialize module by registering models with the network.
   * @param SLIInterpreter* SLI interpreterm, must know modeldict
   */
  void init( SLIInterpreter* );

  const std::string name( void ) const;
  const std::string commandstring( void ) const;
};

} // namespace

#endif
