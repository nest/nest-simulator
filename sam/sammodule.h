/*
 *  sammodule.h
 *
 *  This file is part of SAM, an extension of NEST.
 *
 *  Copyright (C) 2017 D'Amato
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

#ifndef SAMMODULE_H
#define SAMMODULE_H

// Includes from sli:
#include "slifunction.h"
#include "slimodule.h"

// Put your stuff into your own namespace.
namespace sam
{
  class SamModule : public SLIModule
  {
  public:
    SamModule();
    ~SamModule();

    void init(SLIInterpreter*);

    const std::string name() const;
    const std::string commandstring( void ) const;
  };
} // namespace sam

#endif
