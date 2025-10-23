/*
 *  nest_extension_interface.h
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

#ifndef NEST_EXTENSION_INTERFACE_H
#define NEST_EXTENSION_INTERFACE_H

// Includes from nestkernel; placed here so module developer does not need to
// include them manually
#include "config.h"
#include "exceptions.h"
#include "genericmodel.h"
#include "kernel_manager.h"
#include "model.h"
#include "nest.h"
#include "nestmodule.h"
#include "target_identifier.h"

// C++ includes
#include <string>

/**
 * Interface for NEST Extenstion Modules.
 *
 * All NEST Extension Modules must be derived from this interface class.
 *
 * Method init() will be called after the module is loaded.
 *
 * The constructor should be empty.
 */

namespace nest
{

class NESTExtensionInterface
{
public:
  virtual ~NESTExtensionInterface()
  {
  }

  /**
   * Initialize module, register all components with kernel
   */
  virtual void initialize() = 0;
};

}

#endif // #ifndef NEST_EXTENSION_INTERFACE_H
