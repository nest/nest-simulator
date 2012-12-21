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

#include "slimodule.h"
#include "slitype.h" 

#include "modelrange.h"

#include "connection_generator.h"
typedef std::vector<ConnectionGenerator::ClosedInterval> RangeSet;
typedef ConnectionGenerator::ClosedInterval Range;

#include "dictdatum.h"

namespace nest
{
  class Network;

  class ConnectionGeneratorModule: public SLIModule
  {
  public:

    static SLIType ConnectionGeneratorType;

    ConnectionGeneratorModule(Network&);
    ~ConnectionGeneratorModule();

    /**
     * Initialize module by registering models with the network.
     * @param SLIInterpreter* SLI interpreter, must know modeldict
     */
    void init(SLIInterpreter*);

    const std::string name(void) const;
    const std::string commandstring(void) const;

    /*
     * SLI functions: See source file for documentation
     */

    class CGConnect_cg_i_i_D_lFunction : public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } cgconnect_cg_i_i_D_lfunction;
    
    class CGConnect_cg_a_a_D_lFunction : public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } cgconnect_cg_a_a_D_lfunction;

    class CGSetMask_cg_a_aFunction : public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } cgsetmask_cg_a_afunction;

    class CGStartFunction : public SLIFunction
    {
      void execute(SLIInterpreter *) const;
    } cgstartfunction;

    class CGNextFunction : public SLIFunction
    {
      void execute(SLIInterpreter *) const;
    } cgnextfunction;

    /**
     * Return a reference to the network managed by the topology module.
     */
    static Network &get_network();

  private:

    /**
     * - @c net must be static, so that the execute() members of the
     *   SliFunction classes in the module can access the network.
     */
    static Network* net_;
  };
  
  inline
  Network &ConnectionGeneratorModule::get_network()
  {
    assert(net_ != 0);
    return *net_;
  }

  typedef lockPTRDatum<ConnectionGenerator, &nest::ConnectionGeneratorModule::ConnectionGeneratorType> ConnectionGeneratorDatum;

} // namespace nest

#endif /* #ifndef CONNGENMODULE_H */
