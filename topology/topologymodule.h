#ifndef TOPOLOGYMODULE_H
#define TOPOLOGYMODULE_H

/*
 *  topologymodule.h
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

/* 
   topologymodule.h -- Header for the topology module

   See topology/Makefile.am for full list of the files 
   included in the module.
 
   topologymodule.h is the C++ version of topology/sli/topology.sli

   Some additional functions and functionality is also added.

   Note: Nodes in the topologymodule are refered to by their GID
   (not their address array). 

   Author: Hans Ekkehard Plesser (hans.ekkehard.plesser@umb.no)
           Kittel Austvoll      
*/

#include "slimodule.h"
#include "dict.h"
#include "genericmodel.h"
#include "exceptions.h"

#include "position.h"
#include "layer_regular.h"

// place all code for topology module in its own namespace
namespace nest
{
   class TopologyModule: public SLIModule
   {
    public:

     TopologyModule(Network&);
     ~TopologyModule();

     /**
      * Initialize module by registering models with the network.
      * @param SLIInterpreter* SLI interpreter, must know modeldict
      */
     void init(SLIInterpreter*);

     const std::string name(void) const;
     const std::string commandstring(void) const;

     /**
      * See topologymodule.cpp for a description of these class functions.
      * The classes are declared according to the setup described in 
      * nestkernel/nestmodule.cpp
      *
      * The topology functions accessible from the SLI interface are 
      * declared here.
      */

     class ConnectLayers_i_i_dictFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } connectlayers_i_i_dictfunction;

     class GetElement_i_iaFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } getelement_i_iafunction;

     class GetPosition_iFunction: public SLIFunction
     {
     public:
       void execute(SLIInterpreter *) const;
     } getposition_ifunction;
     
     class GetLayer_iFunction: public SLIFunction
     {
     public:
       void execute(SLIInterpreter *) const;
     } getlayer_ifunction;

     class Displacement_i_iFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } displacement_i_ifunction;

     class Displacement_a_iFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } displacement_a_ifunction;

     class Distance_i_iFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } distance_i_ifunction;

     class Distance_a_iFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } distance_a_ifunction;

     static Position<double_t> 
       compute_displacement(const Node&, const Node&);
     static Position<double_t>
       compute_displacement(const Position<double_t>&, const Node&);

     class DumpLayerNodes_os_iFunction: public SLIFunction
       {
       public:
	 void execute(SLIInterpreter *) const;
       } dumplayernodes_os_ifunction;

     class DumpLayerConnections_os_i_lFunction: public SLIFunction
     {
     public:
       void execute(SLIInterpreter *) const;
     } dumplayerconnections_os_i_lfunction;
	
     class CreateLayer_dictFunction: public SLIFunction
     {
     public:
       void execute(SLIInterpreter *) const;
     } createlayer_dictfunction;

   private:
     /**
      * network where models are to be registered
      * @todo This should really be a reference, non-static
      */
     static Network* net_;

   };

} // namespace

#endif
