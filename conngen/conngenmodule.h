/*
 *  conngenmodule.h
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

#ifndef CONNGENMODULE_H
#define CONNGENMODULE_H

// C++ includes:
#include <string>

// Includes from conngen:
#include "conngendatum.h"

// Includes from nestkernel:
#include "modelrange.h"

// Includes from sli:
#include "dictdatum.h"
#include "slimodule.h"

namespace nest
{
class ConnectionGeneratorModule : public SLIModule
{
public:
  ConnectionGeneratorModule();
  ~ConnectionGeneratorModule();

  /**
   * Initialize module by registering models with the network.
   * @param SLIInterpreter* SLI interpreter, must know modeldict
   */
  void init( SLIInterpreter* );

  const std::string name() const;
  const std::string commandstring() const;

  class CGConnect_cg_g_g_D_lFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgconnect_cg_g_g_D_lfunction;

  class CGParse_sFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgparse_sfunction;

  class CGParseFile_sFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgparsefile_sfunction;

  class CGSelectImplementation_s_sFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgselectimplementation_s_sfunction;

  class CGSetMask_cg_g_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgsetmask_cg_g_gfunction;

  class CGStart_cgFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgstart_cgfunction;

  class CGNext_cgFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgnext_cgfunction;
};

} // namespace nest

#endif /* #ifndef CONNGENMODULE_H */
