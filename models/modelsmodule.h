/*
 *  modelsmodule.h
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

#ifndef MODELSMODULE_H
#define MODELSMODULE_H

// C++ includes:
#include <string>

// Includes from sli:
#include "connector_model.h"
#include "genericmodel.h"
#include "slimodule.h"

namespace nest
{
/**
 * Module supplying all models that are included in the NEST release.
 * @todo Should this be a dynamic module?
 */
class ModelsModule : public SLIModule
{
public:
  ModelsModule();
  ~ModelsModule();

  /**
   * Initialize module by registering models with the network.
   * @param SLIInterpreter* SLI interpreter
   */
  void init( SLIInterpreter* );

  const std::string name( void ) const;
  const std::string commandstring( void ) const;

private:
  /**
   * Adds all the known synapse models to the NEST kernel by calling
   * kernel().model_manager.register_connection_model() with the given
   * connection data type and name. This is an internal helper function
   * that is called during construction of the ModelsModule and is not
   * intended to be used outside of this.
  **/
  template < typename ConnectionT >
  void register_connection_models( std::string name_postfix = "" );
};


} // namespace

#endif
