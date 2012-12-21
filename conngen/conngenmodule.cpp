/*
 *  conngenmodule.cpp
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

#include "config.h"
#include "conngenmodule.h"

#include "network.h"
#include "communicator.h"
#include "modelrange.h"

#include "cg_connect.h"

#include "stringdatum.h"
#include "lockptrdatum_impl.h"

template class lockPTRDatum<ConnectionGenerator, &nest::ConnectionGeneratorModule::ConnectionGeneratorType>;

namespace nest
{
  SLIType ConnectionGeneratorModule::ConnectionGeneratorType;

  Network* ConnectionGeneratorModule::net_ = 0;

  ConnectionGeneratorModule::ConnectionGeneratorModule(Network& net) 
  {
    net_ = &net;
  }

  ConnectionGeneratorModule::~ConnectionGeneratorModule()
  {
    ConnectionGeneratorType.deletetypename();
  }

  const std::string ConnectionGeneratorModule::name(void) const
  {
    return std::string("ConnectionGeneratorModule"); // Return name of the module
  }

  const std::string ConnectionGeneratorModule::commandstring(void) const
  {
    return std::string("/conngen /C++ ($Revision: 10104 $) provide-component "
                       "/conngen-interface /SLI (9843) require-component ");
  }

  void ConnectionGeneratorModule::init(SLIInterpreter *i)
  {
    ConnectionGeneratorType.settypename("connectiongeneratortype");
    ConnectionGeneratorType.setdefaultaction(SLIInterpreter::datatypefunction);

    // Register the connection generator functions as SLI commands.
    i->createcommand("CGConnect_cg_i_i_D_l", &cgconnect_cg_i_i_D_lfunction);
    i->createcommand("CGConnect_cg_a_a_D_l", &cgconnect_cg_a_a_D_lfunction);
    i->createcommand("cgsetmask_cg_a_a", &cgsetmask_cg_a_afunction);
    i->createcommand("cgstart", &cgstartfunction);
    i->createcommand("cgnext", &cgnextfunction);
  }

  // Connect for conn_generator subnet subnet dict synapsetype
  // See lib/sli/nest-init.sli for details
  void ConnectionGeneratorModule::CGConnect_cg_i_i_D_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(5);

    ConnectionGeneratorDatum cg = getValue<ConnectionGeneratorDatum>(i->OStack.pick(4));
    index source_id = getValue<long>(i->OStack.pick(3));
    index target_id = getValue<long>(i->OStack.pick(2));
    DictionaryDatum params_map = getValue<DictionaryDatum>(i->OStack.pick(1));
    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));

    Subnet* sources = dynamic_cast<Subnet*>(get_network().get_node(source_id));
    if (sources == NULL)
    {
      i->message(SLIInterpreter::M_ERROR, "CGConnect_cg_i_i_D_l", "sources must be a subnet.");
      throw SubnetExpected();
    }
    if (! sources->is_homogeneous())
    {
      i->message(SLIInterpreter::M_ERROR, "CGConnect_cg_i_i_D_l", "sources must be a homogeneous subnet.");
      throw SubnetExpected();
    }

    Subnet* targets = dynamic_cast<Subnet*>(get_network().get_node(target_id));
    if (targets == NULL)
    {
      i->message(SLIInterpreter::M_ERROR, "CGConnect_cg_i_i_D_l", "targets must be a subnet.");
      throw SubnetExpected();
    }
    if (! targets->is_homogeneous())
    {
      i->message(SLIInterpreter::M_ERROR, "CGConnect_cg_i_i_D_l", "targets must be a homogeneous subnet.");
      throw SubnetExpected();
    }

    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    const modelrange source_range = get_network().get_contiguous_gid_range((*sources->local_begin())->get_gid());
    index source_offset = source_range.get_first_gid();
    RangeSet source_ranges;
    source_ranges.push_back(Range(source_range.get_first_gid(), source_range.get_last_gid()));

    const modelrange target_range = get_network().get_contiguous_gid_range((*targets->local_begin())->get_gid());
    index target_offset = target_range.get_first_gid();
    RangeSet target_ranges;
    target_ranges.push_back(Range(target_range.get_first_gid(), target_range.get_last_gid()));

    cg_connect(cg, source_ranges, source_offset, target_ranges, target_offset, params_map, synmodel_id);
    
    i->OStack.pop(5);
    i->EStack.pop();
  }


  // Connect for conn_generator array array dict synapsetype
  // See lib/sli/nest-init.sli for details
  void ConnectionGeneratorModule::CGConnect_cg_a_a_D_lFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(5);

    ConnectionGeneratorDatum cg = getValue<ConnectionGeneratorDatum>(i->OStack.pick(4));
    IntVectorDatum sources = getValue<IntVectorDatum>(i->OStack.pick(3));
    IntVectorDatum targets = getValue<IntVectorDatum>(i->OStack.pick(2));
    DictionaryDatum params_map = getValue<DictionaryDatum>(i->OStack.pick(1));
    const Name synmodel_name = getValue<std::string>(i->OStack.pick(0));

    const Token synmodel = get_network().get_synapsedict().lookup(synmodel_name);
    if ( synmodel.empty() )
      throw UnknownSynapseType(synmodel_name.toString());
    const index synmodel_id = static_cast<index>(synmodel);

    RangeSet source_ranges;
    cg_get_ranges(source_ranges, (*sources));

    RangeSet target_ranges;
    cg_get_ranges(target_ranges, (*targets));

    cg_connect(cg, source_ranges, (*sources), target_ranges, (*targets), params_map, synmodel_id);

    i->OStack.pop(5);
    i->EStack.pop();
  }


  void ConnectionGeneratorModule::CGSetMask_cg_a_aFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(3);

    ConnectionGeneratorDatum cg = getValue<ConnectionGeneratorDatum>(i->OStack.pick(2));
    IntVectorDatum sources = getValue<IntVectorDatum>(i->OStack.pick(1));
    IntVectorDatum targets = getValue<IntVectorDatum>(i->OStack.pick(0));

    RangeSet source_ranges;
    cg_get_ranges(source_ranges, (*sources));

    RangeSet target_ranges;
    cg_get_ranges(target_ranges, (*targets));

    cg_set_masks(cg, source_ranges, target_ranges);

    i->OStack.pop(3);
    i->EStack.pop();
  }


  /* BeginDocumentation
     Name: CGStartFunction - Call start() on a ConnectionGenerator

     Synopsis:
     cg cgstart -> -

     Parameters: 
     cg - ConnectionGenerator

     Author: Mikael Djurfeldt
     FirstVersion: March 2011
     Availability: 
     SeeAlso: CGNextFunction
  */  
  void ConnectionGeneratorModule::CGStartFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);
     
    ConnectionGeneratorDatum cgd = getValue<ConnectionGeneratorDatum>(i->OStack.pick(0));
    
    cgd->start ();

    i->OStack.pop(1);
    i->EStack.pop();
  }


  /* BeginDocumentation
     Name: CGNextFunction - Call next() on a ConnectionGenerator

     Synopsis:
     cg cgnext -> source target v[0] ... true | false

     Parameters: 
     cg - ConnectionGenerator

     Author: Mikael Djurfeldt
     FirstVersion: December 2012
     Availability: 
     SeeAlso: CGStartFunction
  */  
  void ConnectionGeneratorModule::CGNextFunction::execute(SLIInterpreter *i) const
  {
    i->assert_stack_load(1);
     
    ConnectionGeneratorDatum cgd = getValue<ConnectionGeneratorDatum>(i->OStack.pick(0));
    ConnectionGenerator* generator = cgd.get ();

    int j, k;
    int arity = generator->arity ();
    double* values = new double[arity];
    i->OStack.pop(1);
    i->EStack.pop();
    if (generator->next (j, k, values))
      {
	i->OStack.push (j);
	i->OStack.push (k);
	for (int m = 0; m < arity; ++m)
	  i->OStack.push (values[m]);
	delete[] values;
	i->OStack.push (true);
      }
    else
      i->OStack.push (false);

    cgd.unlock ();
  }


} // namespace nest
