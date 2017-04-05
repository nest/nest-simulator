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

// Includes from nestkernel:
#include "event.h"
#include "exceptions.h"

// Includes from sli:
#include "dict.h"
#include "slifunction.h"
#include "slimodule.h"
#include "slitype.h"

namespace nest
{
class Node;

/**
 * SLI interface of the NEST kernel.
 * This class implements the SLI functions which connect the NEST
 * kernel with the interpreter.
 */

class NestModule : public SLIModule
{
public:
  static SLIType ConnectionType;
  static SLIType GIDCollectionType;

  NestModule();
  ~NestModule();

  void init( SLIInterpreter* );

  const std::string commandstring( void ) const;
  const std::string name( void ) const;

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
   * - @c cg : connectiongeneratortype
   * - @c g  : gid collection
   *
   * @subsection compoundtypes Codes for compund data types
   * - @c A  : array
   * - @c D  : dictionary
   * - @c V  : vector
   *
   * @section conventions Conventions
   * -# All interface functions expect and return nodes as vectors
   *    of GIDs (Vi).
   * -# Functions must document how they loop over GID vectors and
   *    how the function is applied to subnets provided as
   *    arguments.
   * -# Functions that do not require overloading on the SLI level,
   *    need not carry their argument list in the SLI function
   *    name and need not be wrapped by SLI tries.
   * -# Functions which expect a model or synapse type as argument,
   *    must be given this argument as a literal, e.g.,
        @verbatim
        /iaf_psc_alpha 6 Create
        @endverbatim
   *    Literals will be looked up in the corresponding dictionaries
   *    (modeldict, synapsedict).
   * -# The network is accessed using the get_network() accessor
   *    function.
   * -# Each interface function shall verify that there are enough
   *    elements on the stack using (replace n by correct integer)
   *    @verbatim
   *    i->assert_stack_load(n);
   *    @endverbatim
   * -# Errors should trigger C++ exceptions. They will be caught
   *    in the main interpreter loop.
   *
   * @section slidoc SLI Documentation
   * SLI documentation should be provided in nestmodule.cpp, ahead of each
   * group of related functions.
   */

  //@{

  class ChangeSubnet_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } changesubnet_ifunction;

  class CurrentSubnetFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } currentsubnetfunction;

  class GetNodes_i_D_b_bFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getnodes_i_D_b_bfunction;

  class GetLeaves_i_D_bFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getleaves_i_D_bfunction;

  class GetChildren_i_D_bFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getchildren_i_D_bfunction;

  class GetStatus_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getstatus_ifunction;

  class GetStatus_CFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getstatus_Cfunction;

  class GetStatus_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getstatus_afunction;

  class SetStatus_idFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setstatus_idfunction;

  class SetStatus_CDFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setstatus_CDfunction;

  class Cva_CFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } cva_cfunction;

  class SetStatus_aaFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setstatus_aafunction;

  class SetDefaults_l_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setdefaults_l_Dfunction;

  class GetDefaults_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getdefaults_lfunction;

  class CopyModel_l_l_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } copymodel_l_l_Dfunction;

  class GetConnections_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getconnections_Dfunction;

  class SimulateFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } simulatefunction;

  class PrepareFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } preparefunction;

  class RunFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } runfunction;

  class CleanupFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } cleanupfunction;

  class Create_l_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } create_l_ifunction;

  class RestoreNodes_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } restorenodes_afunction;

  class DataConnect_i_D_sFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dataconnect_i_D_sfunction;

  class DataConnect_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dataconnect_afunction;

  class Disconnect_i_i_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } disconnect_i_i_lfunction;

  class Disconnect_g_g_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } disconnect_g_g_D_Dfunction;

  class Connect_g_g_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } connect_g_g_D_Dfunction;

  class ResetKernelFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } resetkernelfunction;

  class ResetNetworkFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } resetnetworkfunction;

  class MemoryInfoFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } memoryinfofunction;

  class PrintNetworkFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } printnetworkfunction;

  class RankFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } rankfunction;

  class NumProcessesFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } numprocessesfunction;

  class SetFakeNumProcesses_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } setfakenumprocesses_ifunction;

  class SyncProcessesFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } syncprocessesfunction;

  class TimeCommunication_i_i_bFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } timecommunication_i_i_bfunction;

  class TimeCommunicationv_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } timecommunicationv_i_ifunction;

  class TimeCommunicationAlltoall_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } timecommunicationalltoall_i_ifunction;

  class TimeCommunicationAlltoallv_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } timecommunicationalltoallv_i_ifunction;

  class ProcessorNameFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } processornamefunction;

#ifdef HAVE_MPI
  class MPIAbort_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } mpiabort_ifunction;
#endif

  class GetVpRngFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } getvprngfunction;

  class GetGlobalRngFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } getglobalrngfunction;

  class Cvdict_CFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvdict_Cfunction;

  class Cvgidcollection_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvgidcollection_i_ifunction;

  class Cvgidcollection_iaFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvgidcollection_iafunction;

  class Cvgidcollection_ivFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvgidcollection_ivfunction;

  class Size_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } size_gfunction;

#ifdef HAVE_MUSIC
  class SetAcceptableLatencyFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } setacceptablelatency_l_dfunction;

  class SetMaxBufferedFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } setmaxbuffered_l_ifunction;
#endif

  class SetStructuralPlasticityStatus_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setstructuralplasticitystatus_Dfunction;

  class GetStructuralPlasticityStatus_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getstructuralplasticitystatus_function;

  class EnableStructuralPlasticity_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } enablestructuralplasticity_function;

  class DisableStructuralPlasticity_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } disablestructuralplasticity_function;

  //@}
};

} // namespace

#endif
