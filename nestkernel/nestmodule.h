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
#include "generic_factory_impl.h"
#include "ntree_impl.h"
#include "parameter.h"

// Includes from sli:
#include "sharedptrdatum.h"
#include "slifunction.h"
#include "slimodule.h"
#include "slitype.h"


namespace nest
{
class AbstractLayer;
class AbstractMask;
template < int D >
class Layer;

class Node;
class Parameter;

/**
 * SLI interface of the NEST kernel.
 *
 * This class implements the SLI functions which connect the NEST
 * kernel with the interpreter.
 */

class NestModule : public SLIModule
{
public:
#ifdef HAVE_LIBNEUROSIM
  static SLIType ConnectionGeneratorType;
#endif
  static SLIType ConnectionType;
  static SLIType MaskType;
  static SLIType NodeCollectionType;
  static SLIType NodeCollectionIteratorType;
  static SLIType ParameterType;

  NestModule();
  ~NestModule() override;

  void init( SLIInterpreter* ) override;

  const std::string commandstring() const override;
  const std::string name() const override;

  static sharedPtrDatum< Parameter, &ParameterType > create_parameter( const Token& );
  static Parameter* create_parameter( const Name& name, const DictionaryDatum& d );

  using ParameterFactory = GenericFactory< Parameter >;
  using ParameterCreatorFunction = GenericFactory< Parameter >::CreatorFunction;

  template < class T >
  static bool register_parameter( const Name& name );

  using MaskFactory = GenericFactory< AbstractMask >;
  using MaskCreatorFunction = GenericFactory< AbstractMask >::CreatorFunction;

  /**
   * Register an AbstractMask subclass as a new mask type.
   *
   * The name will be found using the function T::get_name()
   * @returns true if the new type was successfully registered, or false
   *          if a mask type with the same name already exists.
   */
  template < class T >
  static bool register_mask();

  /**
   * Register a new mask type with the given name, with a supplied
   * function to create mask objects of this type.
   *
   * @param name    name of the new mask type.
   * @param creator function creating objects of this type. The function
   *                will be called with the parameter dictionary as
   *                argument and should return a pointer to a new Mask
   *                object.
   * @returns true if the new type was successfully registered, or false
   *          if a mask type with the same name already exists.
   */
  static bool register_mask( const Name& name, MaskCreatorFunction creator );

  /**
   * Return a Mask object.
   *
   * @param t Either an existing MaskDatum, or a Dictionary containing
   *          mask parameters. The dictionary should contain a key with
   *          the name of the mask type, with a dictionary of parameters
   *          as value, and optionally an anchor.
   * @returns Either the MaskDatum given as argument, or a new mask.
   */
  static sharedPtrDatum< AbstractMask, &NestModule::MaskType > /*MaskDatum*/ create_mask( const Token& t );

  /**
   * Create a new Mask object using the mask factory.
   *
   * @param name Mask type to create.
   * @param d    Dictionary with parameters specific for this mask type.
   * @returns dynamically allocated new Mask object.
   */
  static AbstractMask* create_mask( const Name& name, const DictionaryDatum& d );

  /**
   * SLI Interface functions of the NEST kernel.
   *
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
   * - @c is : input stream
   * - @c os : output stream
   * - @c t  : any token
   * - @c C  : connectiontype
   * - @c cg : connectiongeneratortype
   * - @c g  : node collection
   * - @c q  : node collection iterator
   *
   * @subsection compoundtypes Codes for compund data types
   * - @c A  : array
   * - @c D  : dictionary
   * - @c V  : vector
   *
   * @section conventions Conventions
   * -# All interface functions expect and return nodes as vectors
   *    of node IDs (Vi).
   * -# Functions must document how they loop over node ID vectors and
   *    how the function is applied to NodeCollections provided as
   *    arguments.
   * -# Functions that do not require overloading on the SLI level,
   *    need not carry their argument list in the SLI function
   *    name and need not be wrapped by SLI tries.
   * -# Functions which expect a model or synapse type as argument,
   *    must be given this argument as a literal, e.g.,
        @verbatim
        /iaf_psc_alpha 6 Create
        @endverbatim
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

  /** @BeginDocumentation
   *  Name: GetStatus - return the property dictionary of a node, connection, or object
   *
   *  Synopsis:
   *  node_id   GetStatus -> dict
   *  conn  GetStatus -> dict
   *  obj   GetStatus -> dict
   *
   *  Description:
   *  GetStatus returns a dictionary with the status information
   *  for a node (specified by its node_id), a connection (specified by a connection
   *  object), or an object as used in object-oriented programming in SLI (see cvo for more).
   *
   *  The interpreter exchanges data with the network element using
   *  its status dictionary. To abbreviate the access pattern
   *       node_id GetStatus /lit get
   *  a variant of get implicitly calls GetStatus
   *       node_id /lit get .
   *  In this way network elements and dictionaries can be accessed
   *  with the same syntax. Sometimes access to nested data structures in
   *  the status dictionary is required. In this case the advanced addressing
   *  scheme of get is useful in which the second argument is an array of
   *  literals. See the documentation of get for details.
   *
   *  The information contained in the property dictionary depends on the
   *  concrete node model.
   *
   *  Please refer to the model documentation for details.
   *
   *  Standard entries for nodes:
   *
   *  global_id   - local ID of the node
   *  model       - literal, defining the current node
   *  frozen      - frozen nodes are not updated
   *  thread      - the thread the node is allocated on
   *  vp          - the virtual process a node belongs to
   *
   *  Note that the standard entries cannot be modified directly.
   *
   *  Author: Marc-Oliver Gewaltig
   *  Availability: NEST
   *  SeeAlso: ShowStatus, info, SetStatus, get, GetStatus_dict,
   *  GetKernelStatus
   */

  class GetStatus_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getstatus_gfunction;

  class GetStatus_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getstatus_ifunction;

  class GetStatus_CFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getstatus_Cfunction;

  class GetStatus_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getstatus_afunction;

  class GetMetadata_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getmetadata_gfunction;

  class GetKernelStatus_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getkernelstatus_function;

  /** @BeginDocumentation
   *  Name: SetStatus - sets the value of properties of a node, connection, or object
   *
   *  Synopsis:
   *  node_id   dict SetStatus -> -
   *  conn  dict SetStatus -> -
   *  obj   dict SetStatus -> -
   *
   *  Description:
   *  SetStatus changes properties of a node (specified by its node_id), a connection
   *  (specified by a connection object), or an object as used in object-oriented
   *  programming in SLI (see cvo for more). Properties can be inspected with GetStatus.
   *
   *  Note that many properties are read-only and cannot be changed.
   *
   *  Examples:
   *  /dc_generator Create /dc_gen Set  %Creates a dc_generator, which is a node
   *  dc_gen GetStatus info %view properties (amplitude is 0)
   *  dc_gen << /amplitude 1500. >> SetStatus
   *  dc_gen GetStatus info % amplitude is now 1500
   *
   *  Author: docu by Sirko Straube
   *
   *  SeeAlso: ShowStatus, GetStatus, GetKernelStatus, info, modeldict, Set, SetStatus_dict
   */

  class SetStatus_idFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } setstatus_idfunction;

  class SetStatus_CDFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } setstatus_CDfunction;

  class SetKernelStatus_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } setkernelstatus_Dfunction;

  class Cva_CFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } cva_cfunction;

  class SetStatus_aaFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } setstatus_aafunction;

  /** @BeginDocumentation
   *  Name: SetDefaults - Set the default values for a node or synapse model.
   *  Synopsis: /modelname dict SetDefaults -> -
   *  SeeAlso: GetDefaults
   *  Author: Jochen Martin Eppler
   *  FirstVersion: September 2008
   */
  class SetDefaults_l_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } setdefaults_l_Dfunction;

  /** @BeginDocumentation
   *  Name: GetDefaults - Return the default values for a node or synapse model.
   *  Synopsis: /modelname GetDefaults -> dict
   *  SeeAlso: SetDefaults
   *  Author: Jochen Martin Eppler
   *  FirstVersion: September 2008
   */
  class GetDefaults_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getdefaults_lfunction;

  /** @BeginDocumentation
   * Name: CopyModel - copy a model to a new name, set parameters for copy, if
   * given
   * Synopsis:
   * /model /new_model param_dict -> -
   * /model /new_model            -> -
   * Parameters:
   * /model      - literal naming an existing model
   * /new_model  - literal giving the name of the copy to create, must not
   *               exist in modeldict or synapsedict before
   * /param_dict - parameters to set in the new_model
   * Description:
   * A copy of model is created and registered in modeldict or synapsedict
   * under the name new_model. If a parameter dictionary is given, the parameters
   * are set in new_model.
   * Warning: It is impossible to unload modules after use of CopyModel.
   */
  class CopyModel_l_l_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } copymodel_l_l_Dfunction;

  /** @BeginDocumentation
   *  Name: Install - install dynamically loaded module
   */
  class Install_sFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } install_sfunction;

  class GetConnections_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getconnections_Dfunction;

  /** @BeginDocumentation
   *   Name: Simulate - simulate n milliseconds
   *
   *   Synopsis:
   *   n(int) Simulate -> -
   *
   *   Description: Simulate the network for n milliseconds.
   *
   *   SeeAlso: Run, Prepare, Cleanup, unit_conversion
   */
  class SimulateFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } simulatefunction;

  /** @BeginDocumentation
   *  Name: Prepare - prepare the network for a simulation
   *
   *  Synopsis:
   *  Prepare -> -
   *
   *  Description: sets up network calibration before run is called
   *  any number of times
   *
   *  Note: Run must only be used after Prepare is called, and
   *  before Cleanup to finalize state (close files, etc).
   *  Any changes made between Prepare and Cleanup may cause
   *  undefined behavior and incorrect results.
   *
   *  SeeAlso: Run, Cleanup, Simulate
   */

  class PrepareFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } preparefunction;

  /** @BeginDocumentation
   *  Name: Run - simulate n milliseconds
   *
   *  Synopsis:
   *  n(int) Run -> -
   *
   *  Description: Simulate the network for n milliseconds.
   *  Call prepare before, and cleanup after.
   *  t m mul Simulate = Prepare m { t Run } repeat Cleanup
   *
   *  Note: Run must only be used after Prepare is called, and
   *  before Cleanup to finalize state (close files, etc).
   *  Any changes made between Prepare and Cleanup may cause
   *  undefined behavior and incorrect results.
   *
   *  SeeAlso: Simulate, unit_conversion, Prepare, Cleanup
   */
  class RunFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } runfunction;

  /** @BeginDocumentation
   *  Name: Cleanup - cleanup the network after a simulation
   *
   *  Synopsis:
   *  Cleanup -> -
   *
   *  Description: tears down a network after run is called
   *  any number of times
   *
   *  Note: Run must only be used after Prepare is called, and
   *  before Cleanup to finalize state (close files, etc).
   *  Any changes made between Prepare and Cleanup may cause
   *  undefined behavior and incorrect results.
   *
   *  SeeAlso: Run, Prepare, Simulate
   */
  class CleanupFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } cleanupfunction;

  /** @BeginDocumentation
   *  Name: Create - create nodes
   *
   *  Synopsis:
   *  /model          Create -> NodeCollection
   *  /model n        Create -> NodeCollection
   *  /model   params Create -> NodeCollection
   *  /model n params Create -> NodeCollection
   *
   *  Parameters:
   *  /model - literal naming the modeltype (entry in modeldict)
   *  n      - the desired number of nodes
   *  params - parameters for the newly created node(s)
   *
   *  Returns:
   *  node_ids   - NodeCollection representing nodes created
   *
   *  Description:
   *  Create generates n new network objects of the supplied model
   *  type. If n is not given, a single node is created. params is a
   *  dictionary with parameters for the new nodes.
   *
   *  SeeAlso: modeldict
   */
  class Create_l_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } create_l_ifunction;

  class GetNodes_D_b : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getnodes_D_bfunction;

  class Disconnect_g_g_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } disconnect_g_g_D_Dfunction;

  class Disconnect_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } disconnect_afunction;

  class Connect_g_g_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } connect_g_g_D_Dfunction;

  class Connect_g_g_D_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } connect_g_g_D_afunction;

  class ConnectSonata_D_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } ConnectSonata_D_Function;

  class ConnectTripartite_g_g_g_D_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } connect_tripartite_g_g_g_D_D_Dfunction;

  class ResetKernelFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } resetkernelfunction;

  /** @BeginDocumentation
   *  Name: MemoryInfo - Report current memory usage.
   *  Description:
   *  MemoryInfo reports the current utilization of the memory manager for all
   *  models, which are used at least once. The output is sorted ascending
   *  according according to the name of the model is written to stdout. The unit
   *  of the data is byte. Note that MemoryInfo only gives you information about
   *  the memory requirements of the static model data inside of NEST. It does not
   *  tell anything about the memory situation on your computer.
   *  Synopsis:
   *  MemoryInfo -> -
   *  Availability: NEST
   *  Author: Jochen Martin Eppler
   */
  class MemoryInfoFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } memoryinfofunction;

  /** @BeginDocumentation
   *  Name: PrintNodes - Print nodes in the network.
   *  Synopsis:
   *  -  PrintNodes -> -
   *  Description:
   *  Print node ID ranges and model names of the nodes in the network. Print the
   *  information directly to screen.
   */
  class PrintNodesFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } printnodesfunction;

  /** BeginDocumentation
   *  Name: PrintNodesToStream - Redirect printing of nodes in the network.
   *  Synopsis:
   *  -  PrintNodesToStream -> -
   *  Description:
   *  Returns string output that can be used to print information about the nodes
   *  in the network.
   *  The string is the information directly printed by PrintNodes.
   */
  class PrintNodesToStreamFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } printnodestostreamfunction;

  /** @BeginDocumentation
   *  Name: Rank - Return the MPI rank of the process.
   *  Synopsis: Rank -> int
   *  Description:
   *  Returns the rank of the MPI process (MPI_Comm_rank) executing the
   *  command. This function is mainly meant for logging and debugging
   *  purposes. It is highly discouraged to use this function to write
   *  rank-dependent code in a simulation script as this can break NEST
   *  in funny ways, of which dead-locks are the nicest.
   *  Availability: NEST 2.0
   *  Author: Jochen Martin Eppler
   *  FirstVersion: January 2006
   *  SeeAlso: NumProcesses, SyncProcesses, ProcessorName
   */
  class RankFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } rankfunction;

  /** @BeginDocumentation
   *  Name: NumProcesses - Return the number of MPI processes.
   *  Synopsis: NumProcesses -> int
   *  Description:
   *  Returns the number of MPI processes (MPI_Comm_size). This
   *  function is mainly meant for logging and debugging purposes.
   *  Availability: NEST 2.0
   *  Author: Jochen Martin Eppler
   *  FirstVersion: January 2006
   *  SeeAlso: Rank, SyncProcesses, ProcessorName
   */
  class NumProcessesFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } numprocessesfunction;

  /** @BeginDocumentation
   *  Name: SyncProcesses - Synchronize all MPI processes.
   *  Synopsis: SyncProcesses -> -
   *  Availability: NEST 2.0
   *  Author: Alexander Hanuschkin
   *  FirstVersion: April 2009
   *  Description:
   *  This function allows to synchronize all MPI processes at any
   *  point in a simulation script. Internally, the function uses
   *  MPI_Barrier(). Note that during simulation the processes are
   *  automatically synchronized without the need for user interaction.
   *  SeeAlso: Rank, NumProcesses, ProcessorName
   */
  class SyncProcessesFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } syncprocessesfunction;

  /** @BeginDocumentation
   *  Name: TimeCommunication - returns average time taken for MPI_Allgather over n
   *  calls with m bytes
   *  Synopsis:
   *  n m TimeCommunication -> time
   *  Availability: NEST 2.0
   *  Author: Abigail Morrison
   *  FirstVersion: August 2009
   *  Description:
   *  The function allows a user to test how much time a call the Allgather costs
   */
  class TimeCommunication_i_i_bFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } timecommunication_i_i_bfunction;

  /** @BeginDocumentation
   *  Name: TimeCommunicationv - returns average time taken for MPI_Allgatherv over
   *  n calls with m
   *  bytes
   *  Synopsis:
   *  n m TimeCommunication -> time
   *  Availability: NEST 2.0
   *  Author:
   *  FirstVersion: August 2012
   *  Description:
   *  The function allows a user to test how much time a call the Allgatherv costs
   *  Does not work for offgrid!!!
   */
  class TimeCommunicationv_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } timecommunicationv_i_ifunction;

  /** @BeginDocumentation
   *  Name: TimeCommunicationAlltoall - returns average time taken for MPI_Alltoall
   *  over n calls with m
   *  bytes
   *  Synopsis:
   *  n m TimeCommunicationAlltoall -> time
   *  Availability: 10kproject (>r11254)
   *  Author: Jakob Jordan
   *  FirstVersion: June 2014
   *  Description:
   *  The function allows a user to test how much time a call to MPI_Alltoall costs
   *  SeeAlso: TimeCommunication
   */
  class TimeCommunicationAlltoall_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } timecommunicationalltoall_i_ifunction;

  /** @BeginDocumentation
   *  Name: TimeCommunicationAlltoallv - returns average time taken for
   *  MPI_Alltoallv over n calls with
   *  m bytes
   *  Synopsis:
   *  n m TimeCommunicationAlltoallv -> time
   *  Availability: 10kproject (>r11300)
   *  Author: Jakob Jordan
   *  FirstVersion: July 2014
   *  Description:
   *  The function allows a user to test how much time a call to MPI_Alltoallv
   *  costs
   *  SeeAlso: TimeCommunication
   */
  class TimeCommunicationAlltoallv_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } timecommunicationalltoallv_i_ifunction;

  /** @BeginDocumentation
   *  Name: ProcessorName - Returns a unique specifier for the actual node.
   *  Synopsis: ProcessorName -> string
   *  Availability: NEST 2.0
   *  Author: Alexander Hanuschkin
   *  FirstVersion: April 2009
   *  Description:
   *  This function returns the name of the processor it was called
   *  on (MPI_Get_processor_name). See MPI documentation for more details. If NEST
   *  is not compiled with MPI support, this function returns the hostname of
   *  the machine as returned by the POSIX function gethostname().
   *  Examples:
   *  (I'm process ) =only Rank 1 add =only ( of ) =only NumProcesses =only ( on
   *  machine ) =only
   *  ProcessorName =
   *  SeeAlso: Rank, NumProcesses, SyncProcesses
   */
  class ProcessorNameFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } processornamefunction;

#ifdef HAVE_MPI
  /** @BeginDocumentation
   *  Name: abort - Abort all NEST processes gracefully.
   *  Parameters:
   *  exitcode - The exitcode to quit with
   *  Description:
   *  This function can be run by the user to end all NEST processes as
   *  gracefully as possible. If NEST is compiled without MPI support,
   *  this will just call quit_i. If compiled with MPI support, it will
   *  call MPI_Abort, which will kill all processes of the application
   *  and thus prevents deadlocks. The exitcode is userabort in both
   *  cases (see statusdict/exitcodes).
   *  Availability: NEST 2.0
   *  Author: Jochen Martin Eppler
   *  FirstVersion: October 2012
   *  SeeAlso: quit, Rank, SyncProcesses, ProcessorName
   */
  class MPIAbort_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } mpiabort_ifunction;
#endif

  class Cvdict_CFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } cvdict_Cfunction;

  class Cvnodecollection_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } cvnodecollection_i_ifunction;

  class Cvnodecollection_iaFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } cvnodecollection_iafunction;

  class Cvnodecollection_ivFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } cvnodecollection_ivfunction;

  class Cva_g_lFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } cva_g_lfunction;

  class Size_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } size_gfunction;

  class ValidQ_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } validq_gfunction;

  class Join_g_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } join_g_gfunction;

  class MemberQ_g_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } memberq_g_ifunction;

  class Find_g_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } find_g_ifunction;

  class eq_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } eq_gfunction;

  class BeginIterator_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } beginiterator_gfunction;

  class EndIterator_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } enditerator_gfunction;

  class GetNodeID_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } getnodeid_qfunction;

  class GetNodeIDModelID_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } getnodeidmodelid_qfunction;

  class Next_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } next_qfunction;

  class Eq_q_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } eq_q_qfunction;

  class Lt_q_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } lt_q_qfunction;

  class Get_g_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } get_g_ifunction;

  /** @BeginDocumentation
   *  Name: nest::Take_g_a - slice a NodeCollection

   *  Synopsis:
   *  nc array Take_g_a -> NodeCollection

   *  Parameters:
   *  nc - NodeCollection to be sliced
   *  array - array of the form [start stop step]

   *  Description:
   *  Slice a `NodeCollection` using pythonic slicing conventions:
   *  - Include elements from and including `start` to but excluding `stop`.
   *  - `step` is the step length in the slice and must be positive.
   *  - Negative values for `start` and `stop` count from the end of the `NodeCollection`,  i.e., -1 is the last
   element.
   */
  class Take_g_aFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const override;
  } take_g_afunction;

#ifdef HAVE_MUSIC

  /** @BeginDocumentation
   *  Name: SetAcceptableLatency - set the acceptable latency of a MUSIC input port
   *
   *  Synopsis:
   *  (spikes_in) 0.5 SetAcceptableLatency -> -
   *
   *  Parameters:
   *  port_name - the name of the MUSIC input port
   *  latency   - the acceptable latency (ms) to set for the port
   *
   *  Author: Jochen Martin Eppler
   *  FirstVersion: April 2009
   *  Availability: Only when compiled with MUSIC
   *  SeeAlso: music_event_in_proxy
   */
  class SetAcceptableLatencyFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } setacceptablelatency_l_dfunction;

  class SetMaxBufferedFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } setmaxbuffered_l_ifunction;
#endif

  /**
   * Enable Structural Plasticity within the simulation.
   *
   * This allows dynamic rewiring of the network based on mean electrical activity.
   * Please note that, in the current implementation of structural plasticity,
   * spikes could occasionally be delivered via connections that were not present
   * at the time of the spike.
   * @param i
   */
  class EnableStructuralPlasticity_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } enablestructuralplasticity_function;

  /**
   * Disable Structural Plasticity in the network.
   * @param i
   */
  class DisableStructuralPlasticity_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } disablestructuralplasticity_function;

  /**
   * Set epsilon that is used for comparing spike times in STDP.
   *
   * Spike times in STDP synapses are currently represented as double
   * values. The epsilon defines the maximum distance between spike
   * times that is still considered 0.
   *
   * Note: See issue #894
   */
  class SetStdpEps_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } setstdpeps_dfunction;

  class Mul_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } mul_P_Pfunction;

  class Div_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } div_P_Pfunction;

  class Add_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } add_P_Pfunction;

  class Sub_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } sub_P_Pfunction;

  class Compare_P_P_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } compare_P_P_Dfunction;

  class Conditional_P_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } conditional_P_P_Pfunction;

  class Min_P_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } min_P_dfunction;

  class Max_P_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } max_P_dfunction;

  class Redraw_P_d_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } redraw_P_d_dfunction;

  class Exp_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } exp_Pfunction;

  class Sin_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } sin_Pfunction;

  class Cos_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } cos_Pfunction;

  class Pow_P_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } pow_P_dfunction;

  class Dimension2d_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } dimension2d_P_Pfunction;

  class Dimension3d_P_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } dimension3d_P_P_Pfunction;

  /** @BeginDocumentation
    Name: CreateParameter
  */
  class CreateParameter_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } createparameter_Dfunction;

  /** @BeginDocumentation
    Name: GetValue
  */
  class GetValue_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getvalue_Pfunction;

  class IsSpatial_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } isspatial_Pfunction;

  /** @BeginDocumentation
    Name: Apply
  */
  class Apply_P_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } apply_P_Dfunction;

  class Apply_P_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } apply_P_gfunction;

#ifdef HAVE_LIBNEUROSIM

  /** @BeginDocumentation
   * Name: CGParse - Call ConnectionGenerator::fromXML() and return a
   * ConnectionGenerator
   *
   * Synopsis:
   * xml_string CGParse -> cg
   *
   * Parameters:
   * xml_string - The XML string to parse.
   *
   * Description:
   * Return a ConnectionGenerator created by deserializing the given
   * XML string. The library to parse the XML string can be selected using
   * CGSelectImplementation
   *
   * Availability: Only if compiled with libneurosim support
   * Author: Jochen Martin Eppler
   * FirstVersion: September 2013
   * SeeAlso: CGParseFile, CGSelectImplementation
   */
  class CGParse_sFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgparse_sfunction;

  /** @BeginDocumentation
   * Name: CGParseFile - Call ConnectionGenerator::fromXMLFile() and return a
   * ConnectionGenerator
   *
   * Synopsis:
   * xml_filename CGParseFile -> cg
   *
   * Parameters:
   * xml_filename - The XML file to read.
   *
   * Description:
   * Return a ConnectionGenerator created by deserializing the given
   * XML file. The library to parse the XML file can be selected using
   * CGSelectImplementation
   *
   * Availability: Only if compiled with libneurosim support
   * Author: Jochen Martin Eppler
   * FirstVersion: February 2014
   * SeeAlso: CGParse, CGSelectImplementation
   */
  class CGParseFile_sFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgparsefile_sfunction;

  /** @BeginDocumentation
   * Name: CGSelectImplementation - Call
   * ConnectionGenerator::selectCGImplementation()
   *
   * Synopsis:
   * tag library CGParse -> -
   *
   * Parameters:
   * tag     - The XML tag to associate with a library.
   * library - The library to provide the parsing for CGParse
   *
   * Description:
   * Select a library to provide a parser for XML files and associate
   * an XML tag with the library.
   *
   * Availability: Only if compiled with libneurosim support
   * Author: Jochen Martin Eppler
   * FirstVersion: September 2013
   * SeeAlso: CGParse, CGParseFile
   */
  class CGSelectImplementation_s_sFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cgselectimplementation_s_sfunction;
#endif

  //
  // SLI functions for spatial networks
  //

  /** @BeginDocumentation
   * Name: nest::CreateLayer - create nodes with spatial properties
   *
   * Synopsis:
   * dict CreateLayer -> layer
   *
   * Parameters:
   * dict - dictionary with layer specification
   *
   * Description: Creates a NodeCollection which contains information
   * about the spatial position of its nodes. Positions can be organized
   * in one of two layer classes: grid-based layers, in which each element
   * is placed at a location in a regular grid, and free layers, in which
   * elements can be placed arbitrarily in space.  Which kind of layer
   * this command creates depends on the elements in the supplied
   * specification dictionary.
   *
   * Author: Håkon Enger, Kittel Austvoll
   */
  class CreateLayer_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } createlayer_D_Dfunction;

  /** @BeginDocumentation
   *  Name: nest::GetPosition - retrieve position of input node
   *
   *  Synopsis: NodeCollection GetPosition -> [array]
   *
   *  Parameters:
   *  layer      - NodeCollection for layer with layer nodes
   *
   *  Returns:
   *  [array]    - spatial position of node [x y]
   *
   *  Description: Retrieves spatial 2D position of layer node(s).
   *
   *  Examples:
   *
   *  %%Create layer
   *  << /rows 5
   *     /columns 4
   *     /elements /iaf_psc_alpha
   *  >> /dictionary Set
   *
   *  dictionary CreateLayer /src Set
   *
   *  src [4] Take GetPosition
   *
   *  Author: Kittel Austvoll
   */
  class GetPosition_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getposition_gfunction;

  /** @BeginDocumentation
   * Name: nest::Displacement - compute displacement vector
   *
   * Synopsis: layer from_node_id to_node_id Displacement -> [double vector]
   *           layer from_pos to_node_id Displacement -> [double vector]
   *
   * Parameters:
   * layer           - NodeCollection for layer
   * from_node_id    - int, node_id of node in a spatial NodeCollection
   * from_pos        - double vector, position in layer
   * to_node_id      - int, node_id of node in a spatial NodeCollection
   *
   * Returns:
   * [double vector] - vector pointing from position "from" to position "to"
   *
   * Description:
   * This function returns a vector connecting the position of the "from_node_id"
   * node or the explicitly given "from_pos" position and the position of the
   * "to_node_id" node. Nodes must be parts of a spatial NodeCollection.
   *
   * The "from" position is projected into the layer of the "to_node_id" node. If
   * this layer has periodic boundary conditions (EdgeWrap is true), then the
   * shortest displacement vector is returned, taking into account the
   * periodicity. Fixed grid layers are in this case extended so that the
   * nodes at the edges of the layer have a distance of one grid unit when
   * wrapped.
   *
   * Example:
   *
   * << /rows 5
   *    /columns 4
   *    /elements /iaf_psc_alpha
   * >> CreateLayer
   * /layer Set
   *
   * layer [4] Take layer [5] Take Displacement
   * [[0.2 0.3]] layer [5] Take Displacement
   *
   * Author: Håkon Enger, Hans E Plesser, Kittel Austvoll
   *
   * See also: Distance, GetPosition
   */
  class Displacement_g_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } displacement_g_gfunction;

  class Displacement_a_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } displacement_a_gfunction;

  /** @BeginDocumentation
   *  Name: nest::Distance - compute distance between nodes
   *
   *  Synopsis: layer from_node_id to_node_id Distance -> double
   *            layer from_pos to_node_id Distance -> double
   *
   *  Parameters:
   *  layer       - NodeCollection for layer
   *  from_node_id    - int, node_id of node in a spatial NodeCollection
   *  from_pos    - double vector, position in layer
   *  to_node_id      - int, node_id of node in a spatial NodeCollection
   *
   *  Returns:
   *  double - distance between nodes or given position and node
   *
   *  Description:
   *  This function returns the distance between the position of the "from_node_id"
   *  node or the explicitly given "from_pos" position and the position of the
   *  "to_node_id" node. Nodes must be parts of a spatial NodeCollection.
   *
   *  The "from" position is projected into the layer of the "to_node_id" node. If
   *  this layer has periodic boundary conditions (EdgeWrap is true), then the
   *  shortest distance is returned, taking into account the
   *  periodicity. Fixed grid layers are in this case extended so that the
   *  nodes at the edges of the layer have a distance of one grid unit when
   *  wrapped.
   *
   *  Example:
   *
   *  /layer
   *  << /rows 5
   *     /columns 4
   *     /elements /iaf_psc_alpha
   *  >> CreateLayer def
   *
   *  layer [4] Take layer [5] Take Distance
   *  [[ 0.2 0.3 ]] layer [5] Take Distance
   *
   *  Author: Hans E Plesser, Kittel Austvoll
   *
   *  See also: Displacement, GetPosition
   */
  class Distance_g_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } distance_g_gfunction;

  class Distance_a_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } distance_a_gfunction;

  class Distance_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } distance_afunction;

  /** @BeginDocumentation
   * Name: nest::ConnectLayers - connect two layers
   *
   * Synopsis: sourcelayer targetlayer connection_dict
   * ConnectLayers -> -
   *
   * Description: Connects nodes in two topological layers.
   *
   * The parameters set in the input dictionary decides the nature
   * of the connection pattern being created. Please see parameter
   * list below for a detailed description of these variables.
   *
   * The connections are created by iterating through either the
   * source or the target layer, consecutively connecting each node
   * to a region in the opposing layer.
   *
   * Parameters:
   * sourcelayer  - NodeCollection for source layer
   * targetlayer  - NodeCollection for target layer
   *
   * connection_dict - dictionary containing any of the following
   *                   elements:
   *
   * ------------------------------------------------------------------
   * Connection dictionary parameters:
   * ------------------------------------------------------------------
   * Parameter name: connection-type
   *
   * Type: string
   *
   * Parameter description:
   *
   * Decides the type of connection pattern being created (i.e.
   * convergent or divergent topological connection). A convergent
   * topological connection is a connection between a source region
   * and a target node. A divergent topological connection is a
   * connection between a source node and a target region. A convergent
   * topological connection can also be called a receptive field connection.
   * A divergent topological connection can also be called a projective
   * field connection. A one-to-one connection can be created by setting
   * the size of the source or target region equal to one. The connection
   * type has particular effect on the connection pattern when used together
   * with the number_of_connections variable.
   *
   *
   * Parameter name: mask
   *
   * Type: dictionary
   *
   * Parameter description:
   *
   * The mask defines the region used in the connection type described
   * above. There exists a selection of many different region sizes and
   * shapes. Examples are the grid region, the rectangular, circular or
   * doughnut region.
   *
   * The grid region takes an optional anchor parameter. The anchor
   * parameter indicates which node of the grid region is aligned with
   * the source node.
   *
   *
   * Parameter name: weights, delays and kernel
   *
   * Type: dictionary
   *
   * Parameter description:
   *
   * These parameters can be initialised in many ways. Either as a constant
   * value, with the help of a dictionary, or in an array (only for fixed
   * grid layers). The dictionary can be of type gaussian, 2D gaussian,
   * linear, exponential and other.
   *
   *
   * Parameter name: number_of_connections
   *
   * Type: integer
   *
   * Parameter description:
   *
   * Maximum number of connections that each iterating node is allowed.
   * The actual connections being created are picked at random from all
   * the candidate connections.
   *
   *
   *     Parameter name: synapse_model
   *
   *     Type: literal
   *
   *     Parameter description:
   *
   *     The synapse model to be used for creating the connection.
  .*
   * Parameter name: allow_autapses
   *
   * Type: bool
   *
   * Parameter description: Used together with the number_of_connections option to
   * indicate if autapses are allowed.
   *
   *
   * Parameter name: allow_multapses
   *
   * Type: bool
   *
   * Parameter description: Used together with the number_of_connections option to
   * indicate if multapses are allowed.
   *
   * ------------------------------------------------------------------
   *
   * Example:
   *
   * %Create source layer with CreateLayer
   * << /rows 15
   *    /columns 43
   *    /extent [1.0 2.0]
   *    /elements /iaf_psc_alpha
   * >> /src_dictionary Set
   *
   * src_dictionary CreateLayer /src Set
   *
   * %Create target layer with CreateLayer
   * %%Create layer
   * << /rows 34
   *    /columns 71
   *    /extent [3.0 1.0]
   *    /elements /iaf_psc_alpha
   * >> /tgt_dictionary Set
   *
   * tgt_dictionary CreateLayer /tgt Set
   *
   * <<  /connection_type (convergent)
   *     /mask << /grid << /rows 2 /columns 3 >>
   *              /anchor << /row 4 /column 2 >> >>
   *     /weight 2.3
   *     /delay [2.3 1.2 3.2 1.3 2.3 1.2]
   *     /kernel << /gaussian << /sigma 1.2 /p_center 1.41 >> >>
   *     /synapse_model /stdp_synapse
   *
   * >> /parameters Set
   *
   * src tgt parameters ConnectLayers
   *
   * Author: Håkon Enger, Kittel Austvoll
   *
   * SeeAlso: nest::CreateLayer
   *//** @BeginDocumentation
   * Name: nest::ConnectLayers - connect two layers
   *
   * Synopsis: sourcelayer targetlayer connection_dict
   * ConnectLayers -> -
   *
   * Description: Connects nodes in two topological layers.
   *
   * The parameters set in the input dictionary decides the nature
   * of the connection pattern being created. Please see parameter
   * list below for a detailed description of these variables.
   *
   * The connections are created by iterating through either the
   * source or the target layer, consecutively connecting each node
   * to a region in the opposing layer.
   *
   * Parameters:
   * sourcelayer  - NodeCollection for source layer
   * targetlayer  - NodeCollection for target layer
   *
   * connection_dict - dictionary containing any of the following
   *                   elements:
   *
   * ------------------------------------------------------------------
   * Connection dictionary parameters:
   * ------------------------------------------------------------------
   * Parameter name: connection-type
   *
   * Type: string
   *
   * Parameter description:
   *
   * Decides the type of connection pattern being created (i.e.
   * convergent or divergent topological connection). A convergent
   * topological connection is a connection between a source region
   * and a target node. A divergent topological connection is a
   * connection between a source node and a target region. A convergent
   * topological connection can also be called a receptive field connection.
   * A divergent topological connection can also be called a projective
   * field connection. A one-to-one connection can be created by setting
   * the size of the source or target region equal to one. The connection
   * type has particular effect on the connection pattern when used together
   * with the number_of_connections variable.
   *
   *
   * Parameter name: mask
   *
   * Type: dictionary
   *
   * Parameter description:
   *
   * The mask defines the region used in the connection type described
   * above. There exists a selection of many different region sizes and
   * shapes. Examples are the grid region, the rectangular, circular or
   * doughnut region.
   *
   * The grid region takes an optional anchor parameter. The anchor
   * parameter indicates which node of the grid region is aligned with
   * the source node.
   *
   *
   * Parameter name: weights, delays and kernel
   *
   * Type: dictionary
   *
   * Parameter description:
   *
   * These parameters can be initialised in many ways. Either as a constant
   * value, with the help of a dictionary, or in an array (only for fixed
   * grid layers). The dictionary can be of type gaussian, 2D gaussian,
   * linear, exponential and other.
   *
   *
   * Parameter name: source
   *
   * Type: dictionary
   *
   * Parameter description:
   *
   * The source dictionary enables us to give further detail on
   * how the nodes in the source layer used in the connection function
   * should be processed.
   *
   * Parameters:
   * model*             literal
   * lid^               integer
   *
   * *modeltype (i.e. /iaf_psc_alpha) of nodes that should be connected to
   * in the layer. All nodes are used if this variable isn't set.
   * ^Nesting depth of nodes that should be connected to. All layers are used
   * if this variable isn't set.
   *
   *
   * Parameter name: target
   *
   * Type: dictionary
   *
   * Parameter description:
   *
   * See description for source dictionary.
   *
   *
   * Parameter name: number_of_connections
   *
   * Type: integer
   *
   * Parameter description:
   *
   * Maximum number of connections that each iterating node is allowed.
   * The actual connections being created are picked at random from all
   * the candidate connections.
   *
   *
   *     Parameter name: synapse_model
   *
   *     Type: literal
   *
   *     Parameter description:
   *
   *     The synapse model to be used for creating the connection.
  .*
   * Parameter name: allow_autapses
   *
   * Type: bool
   *
   * Parameter description: Used together with the number_of_connections option to
   * indicate if autapses are allowed.
   *
   *
   * Parameter name: allow_multapses
   *
   * Type: bool
   *
   * Parameter description: Used together with the number_of_connections option to
   * indicate if multapses are allowed.
   *
   * ------------------------------------------------------------------
   *
   * Example:
   *
   * %Create source layer with CreateLayer
   * << /rows 15
   *    /columns 43
   *    /extent [1.0 2.0]
   *    /elements /iaf_psc_alpha
   * >> /src_dictionary Set
   *
   * src_dictionary CreateLayer /src Set
   *
   * %Create target layer with CreateLayer
   * %%Create layer
   * << /rows 34
   *    /columns 71
   *    /extent [3.0 1.0]
   *    /elements /iaf_psc_alpha
   * >> /tgt_dictionary Set
   *
   * tgt_dictionary CreateLayer /tgt Set
   *
   * <<  /connection_type (convergent)
   *     /mask << /grid << /rows 2 /columns 3 >>
   *              /anchor << /row 4 /column 2 >> >>
   *     /weight 2.3
   *     /delay [2.3 1.2 3.2 1.3 2.3 1.2]
   *     /kernel << /gaussian << /sigma 1.2 /p_center 1.41 >> >>
   *     /synapse_model /stdp_synapse
   *
   * >> /parameters Set
   *
   * src tgt parameters ConnectLayers
   *
   * Author: Håkon Enger, Kittel Austvoll
   *
   * SeeAlso: nest::CreateLayer
   */
  class ConnectLayers_g_g_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } connectlayers_g_g_Dfunction;

  /** @BeginDocumentation
   *  Name: nest::CreateMask - create a spatial mask
   *
   *  Synopsis:
   *  << /type dict >> CreateMask -> mask
   *
   *  Parameters:
   *  /type - mask type
   *  dict  - dictionary with mask specifications
   *
   *  Description: Masks can be used when creating connections between nodes
   *  with spatial parameters. A mask describes which area of the pool layer
   *  shall be searched for nodes to connect for any given node in the driver
   *  layer. This command creates a mask object which may be combined with other
   *  mask objects using Boolean operators. The mask is specified in a dictionary.
   *
   *  Author: Håkon Enger
   */
  class CreateMask_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } createmask_Dfunction;

  /** @BeginDocumentation
   *
   *  Name: nest::GetLayerStatus - return information about layer
   *
   *  Synopsis:
   *  layer GetLayerStatus -> dict
   *
   *  Parameters:
   *  layer - NodeCollection representing layer
   *
   *  Returns:
   *  Status dictionary with information about layer
   */
  class GetLayerStatus_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } getlayerstatus_gfunction;

  /** @BeginDocumentation
   * Name: nest::Inside - test if a point is inside a mask
   *
   * Synopsis:
   * point mask Inside -> bool
   *
   * Parameters:
   * point - array of coordinates
   * mask - mask object
   *
   * Returns:
   * bool - true if the point is inside the mask
   */
  class Inside_a_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } inside_a_Mfunction;

  class And_M_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } and_M_Mfunction;

  class Or_M_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } or_M_Mfunction;

  class Sub_M_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } sub_M_Mfunction;

  /** @BeginDocumentation
   *  Name: nest::DumpLayerNodes - write information about layer nodes to file
   *
   *  Synopsis: ostream layer DumpLayerNodes -> ostream
   *
   *  Parameters:
   *  ostream - open output stream
   *  layer   - NodeCollection for layer
   *
   *  Description:
   *  Write information about each element in the given layer to the
   *  output stream. The file format is one line per element with the
   *  following contents:
   *
   *  node ID x-position y-position [z-position]
   *
   *  X and y position are given as physical coordinates in the extent,
   *  not as grid positions. The number of decimals can be controlled by
   *  calling setprecision on the output stream before calling DumpLayerNodes.
   *
   *  Remarks:
   *  In distributed simulations, this function should only be called for
   *  MPI rank 0. If you call it on several MPI ranks, you must use a
   *  different file name on each.
   *
   *  Examples:
   *
   *  /my_layer << /rows 5 /columns 4 /elements /iaf_psc_alpha >> CreateLayer def
   *
   *  (my_layer_dump.lyr) (w) file
   *  my_layer DumpLayerNodes
   *  close
   *
   *  Author: Kittel Austvoll, Hans Ekkehard Plesser
   *
   *  SeeAlso: nest::DumpLayerConnections, setprecision, modeldict
   */
  class DumpLayerNodes_os_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } dumplayernodes_os_gfunction;

  /** @BeginDocumentation
   * Name: nest::DumpLayerConnections - prints a list of the connections of the
   *                                        nodes in the layer to file
   *
   * Synopsis: ostream source_layer synapse_model DumpLayerConnections ->
   *                                                                        ostream
   *
   * Parameters:
   * ostream          - open outputstream
   * source_layer     - NodeCollection for layer
   * synapse_model    - synapse model (literal)
   *
   * Description:
   * Dumps information about all connections of the given type having their source
   * in the given layer to the given output stream. The data format is one line per
   * connection as follows:
   *
   * source_node_id target_node_id weight delay displacement[x,y,z]
   *
   * where displacement are up to three coordinates of the vector from the source
   * to the target node. If targets do not have positions (eg. spike recorders
   * outside any layer), NaN is written for each displacement coordinate.
   *
   * Remarks:
   * For distributed simulations
   * - this function will dump the connections with local targets only.
   * - the user is responsible for writing to a different output stream (file)
   *   on each MPI process.
   *
   * Examples:
   *
   * (out.cnn) (w) file layer_node_id /static_synapse PrintLayerConnections close
   *
   * Author: Kittel Austvoll, Hans Ekkehard Plesser
   *
   * SeeAlso: nest::DumpLayerNodes
   */
  class DumpLayerConnections_os_g_g_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } dumplayerconnections_os_g_g_lfunction;

  class Cvdict_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } cvdict_Mfunction;

  class SelectNodesByMask_g_a_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const override;
  } selectnodesbymask_g_a_Mfunction;

private:
  static ParameterFactory& parameter_factory_();
  static MaskFactory& mask_factory_();

  //@}
};

template < class T >
inline bool
NestModule::register_parameter( const Name& name )
{
  return parameter_factory_().register_subtype< T >( name );
}

template < class T >
inline bool
NestModule::register_mask()
{
  return mask_factory_().register_subtype< T >( T::get_name() );
}

} // namespace

#endif
