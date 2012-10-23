/*
 *  nestmodule.h
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

#ifndef NESTMODULE_H
#define NESTMODULE_H
/* 
    This file is part of NEST

    nestmodule.h -- Header to the nestmodule
    (see cpp file for details)

    Author: Marc-Oliver Gewaltig (marc-oliver.gewaltig@honda-ri.de)

    $Date: 2012-10-11 15:50:10 +0200 (Thu, 11 Oct 2012) $
    Last change: $Author: enger $
    $Revision: 9902 $
*/
#include "slimodule.h"
#include "slifunction.h"
#include "slitype.h"
#include "dict.h"
#include "network.h"
#include "scheduler.h"
#include "event.h"
#include "exceptions.h"

namespace nest
{
   class Node;
   
  /**
   * SLI interface of the NEST kernel.
   * This class implements the SLI functions which connect the NEST
   * kernel with the interpreter.
   */

   class NestModule: public SLIModule
   {
    public:
  
     static SLIType ConnectionType;

     NestModule();
     ~NestModule();

     /**
      * Set pointer to network.
      * This function sets the pointer to the network which NestModule
      * shall manage. It must be called once, and before NestModule is
      * constructed.
      * @param Network&  Th network to manage
      * @note One should find a cleaner solution, more in keeping with
      *       the initialization of dynamic modules. HEP
      */
     static void register_network(Network&);

     void init(SLIInterpreter *);

     const std::string commandstring(void) const;
     const std::string name(void) const;

     /**
      * @defgroup NestSliHelpers Helper functions for nestmodule.
      */

     //@{

     /**
      * Return a reference to the network managed by nestmodule.
      */
     static Network &get_network();

     /**
      * Get number of threads.
      * @todo This functions is a hack, to make the number of 
      *       threads globally available. It should obviously
      *       be part of Scheduler or Network, but implementing
      *       it there would require a major re-design of the class.
      */
     static index get_num_threads(); 
     
     //@}
     
     /**
      * @defgroup NestSliInterface SLI Interface functions of the NEST kernel.
      * This group contains the functions that form the SLI interface
      * of the NEST kernel. 
      *
      * @section namemangling Name mangling
      * All function names are mangled, so that type checking can occur
      * using type tries in the SLI interpreter. No unmangled names 
      * should be introduced.  
      *
      * Name mangling is based on the following principles:
      * -# For each argument expected, _# is appended to the function name 
      * in the order in which arguments are expected on the stack, 
      * i.e., deepest first.
      * -# # is a (sequence of) lowercase letter for plain data types, 
      * an uppercase letter for compound types. 
      * -# For compound types, a lowercase letter can be appended to indicate 
      * the type of the elements expected (eg. Ai for array of int).
      *
      * @subsection plaintypes Codes for plain data types
      * - @c i  : int (actually long)                 
      * - @c d  : double               
      * - @c u  : numeric (long or double)
      * - @c s  : string
      * - @c l  : literal
      * - @c f  : function
      * - @c r  : rng                 
      * - @c is : input stream
      * - @c os : output stream 
      * - @c t  : any token
      * - @c C  : connectiontype
      *
      * @subsection compoundtypes Codes for compund data types
      * - @c A  : array
      * - @c D  : dictionary
      * - @c V  : vector
      *
      * @section conventions Conventions
      * -# All interface functions expect and return nodes as vectors
      *    of GIDs (Vi), with exception of only those functions that
      *    absolutey need to take or provide addresses.
      * -# Functions must document how they loop over GID vectors and
      *    how the function is applied to subnets provided as
      *    arguments.
      * -# Functions expecting or returning addresses have the name
      *    suffix @c ByAddr .
      * -# Functions that do not require overloading on the SLI level,
      *    need not carry their argument list in the SLI function
      *    name and need not be wrapped by SLI tries.
      * -# Functions which expect a model or synapse type as argument,
      *    must be given this argument as a literal, e.g.,
           @verbatim
           /iaf_neuron 6 Create
           @endverbatim
      *    Literals will be looked up in the corresponding dictionaries
      *    (modeldict, synapsedict).
      * -# The network is accessed using the get_network() accessor
      *    function. 
      * -# Each interface function shall verify that there are enough
      *    elements on the stack and check their data type as early 
      *    as possible. Proper SLI error messages shall be issued in
      *    case of error using
           @verbatim
           i->error(function, message);  // may be repeated for multi-line msg
           i->raiseerror(i->{Kernel,InternalKernel,***}Error);    // set error flag in interpreter
           return;                       // important to return to interpreter
           @endverbatim
      * -# No C++ exceptions should escape the interface function. Unspecified
      *    exceptions should be caught and passed to the fallbackHandler, before
      *    returning control to the interpreter.
           @verbatim
           catch ( std::exception& e )
           {
             KernelException::fallbackHandler(i, __PRETTY_FUNCTION__, e);
             return;
           }
           @endverbatim
      *
      * @section slidoc SLI Documentation
      * SLI documentation should be provided in nestmodule.cpp, ahead of each
      * group of related functions.
      */

     //@{
     
     class ChangeSubnet_aFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } changesubnet_afunction;
  
     class ChangeSubnet_iFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } changesubnet_ifunction;
  
     class CurrentSubnetFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } currentsubnetfunction;
  
     class GetNodes_i_bFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getnodes_i_bfunction;

     class GetLeaves_i_bFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getleaves_i_bfunction;

     class GetChildren_i_bFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getchildren_i_bfunction;

     class GetAddressFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getaddressfunction;

     class GetGIDFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getgidfunction;
     
     class GetLIDFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getlidfunction;
     
     class GetStatus_iFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getstatus_ifunction;

     class GetStatus_CFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getstatus_Cfunction;

     class SetStatus_idFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } setstatus_idfunction;

     class SetStatus_CDFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } setstatus_CDfunction;
     
     class SetDefaults_l_DFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } setdefaults_l_Dfunction;
     
     class GetDefaults_lFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } getdefaults_lfunction;

     class CopyModel_l_l_DFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } copymodel_l_l_Dfunction;

     class FindConnections_DFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } findconnections_Dfunction;

     class SimulateFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } simulatefunction;

     class ResumeSimulationFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } resumesimulationfunction;
  
     class Create_l_iFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } create_l_ifunction;

     class Connect_i_i_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } connect_i_i_lfunction;

     class Connect_i_i_d_d_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } connect_i_i_d_d_lfunction;

     class Connect_i_i_D_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } connect_i_i_D_lfunction;

     class CompoundConnect_i_i_i_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } compoundconnect_i_i_i_lfunction;

     class DivergentConnect_i_ia_a_a_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } divergentconnect_i_ia_a_a_lfunction;

     class RDivergentConnect_i_i_ia_da_da_b_b_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } rdivergentconnect_i_i_ia_da_da_b_b_lfunction;
          
     class ConvergentConnect_ia_i_a_a_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } convergentconnect_ia_i_a_a_lfunction;

     class RConvergentConnect_ia_i_i_da_da_b_b_lFunction: public SLIFunction
     {
      public:
       void execute(SLIInterpreter *) const;
     } rconvergentconnect_ia_i_i_da_da_b_b_lfunction;

     class ResetKernelFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } resetkernelfunction;

     class ResetNetworkFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } resetnetworkfunction;

     class NetworkDimensions_aFunction: public SLIFunction
     { 
      public:
       void execute(SLIInterpreter *) const;
     } networkdimensions_afunction;
  
     class MemoryInfoFunction: public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } memoryinfofunction;

     class PrintNetworkFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } printnetworkfunction;

     class RankFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } rankfunction;

     class NumProcessesFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } numprocessesfunction;

     class SyncProcessesFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } syncprocessesfunction;

     class TimeCommunication_i_i_bFunction : public SLIFunction 
     { 
       void execute(SLIInterpreter *) const; 
     } timecommunication_i_i_bfunction; 

     class MPIProcessorNameFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } mpiprocessornamefunction;

     class GetVpRngFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } getvprngfunction;

     class GetGlobalRngFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } getglobalrngfunction;

     class Cvdict_CFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } cvdict_Cfunction;

#ifdef HAVE_MUSIC     
     class SetAcceptableLatencyFunction : public SLIFunction
     {
       void execute(SLIInterpreter *) const;
     } setacceptablelatency_l_dfunction;
#endif

     //@}

   private:

     /**
      * Pointer to network.
      * @note - @c net must be initialized before NestModule is
      *     constructed.
      * - @c net must be static, so that the execute() members of the
      *   SliFunction classes in the module can access the network.
      * - @c net is, however, dynamic in order to avoid the 
      *   static-initialization problem, i.e., the undefined
      *   initialization order of static variables across compilation
      *   units.
      */
     static Network *net_;
   };

inline
Network &NestModule::get_network()
{
  assert(net_ != 0);
  return *net_;
}

inline
index NestModule::get_num_threads()
{
  if ( net_ == 0 )
    return 1;  // module not initialized, thus certainly single thread
  else  
    return net_->get_num_threads();
}

} // namespace

#endif
