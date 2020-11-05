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
#include "generic_factory.h"
#include "ntree.h"
#include "parameter.h"
#include "position.h"

// Includes from sli:
#include "dict.h"
#include "slifunction.h"
#include "slimodule.h"
#include "slitype.h"
#include "sharedptrdatum.h"


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
  ~NestModule();

  void init( SLIInterpreter* );

  const std::string commandstring( void ) const;
  const std::string name( void ) const;

  static sharedPtrDatum< Parameter, &ParameterType > create_parameter( const Token& );
  static Parameter* create_parameter( const Name& name, const DictionaryDatum& d );

  using ParameterFactory = GenericFactory< Parameter >;
  using ParameterCreatorFunction = GenericFactory< Parameter >::CreatorFunction;

  template < class T >
  static bool register_parameter( const Name& name );

  using MaskFactory = GenericFactory< AbstractMask >;
  using MaskCreatorFunction = GenericFactory< AbstractMask >::CreatorFunction;

  /**
   * Register an AbstractMask subclass as a new mask type. The name will
   * be found using the function T::get_name()
   * @returns true if the new type was successfully registered, or false
   *          if a mask type with the same name already exists.
   */
  template < class T >
  static bool register_mask();

  /**
   * Register a new mask type with the given name, with a supplied
   * function to create mask objects of this type.
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
   * @param t Either an existing MaskDatum, or a Dictionary containing
   *          mask parameters. The dictionary should contain a key with
   *          the name of the mask type, with a dictionary of parameters
   *          as value, and optionally an anchor.
   * @returns Either the MaskDatum given as argument, or a new mask.
   */
  static sharedPtrDatum< AbstractMask, &NestModule::MaskType > /*MaskDatum*/ create_mask( const Token& t );

  /**
   * Create a new Mask object using the mask factory.
   * @param name Mask type to create.
   * @param d    Dictionary with parameters specific for this mask type.
   * @returns dynamically allocated new Mask object.
   */
  static AbstractMask* create_mask( const Name& name, const DictionaryDatum& d );

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

  class GetStatus_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getstatus_gfunction;

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

  class GetMetadata_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getmetadata_gfunction;

  class GetKernelStatus_Function : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getkernelstatus_function;

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

  class SetKernelStatus_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setkernelstatus_Dfunction;

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

  class GetNodes_D_b : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getnodes_D_bfunction;

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

  class Connect_g_g_D_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } connect_g_g_D_afunction;

  class ResetKernelFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } resetkernelfunction;

  class MemoryInfoFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } memoryinfofunction;

  class PrintNodesFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } printnodesfunction;

  class PrintNodesToStreamFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } printnodestostreamfunction;

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

  class GetGlobalRngFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } getglobalrngfunction;

  class Cvdict_CFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvdict_Cfunction;

  class Cvnodecollection_i_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvnodecollection_i_ifunction;

  class Cvnodecollection_iaFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvnodecollection_iafunction;

  class Cvnodecollection_ivFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cvnodecollection_ivfunction;

  class Cva_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } cva_gfunction;

  class Size_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } size_gfunction;

  class ValidQ_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } validq_gfunction;

  class Join_g_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } join_g_gfunction;

  class MemberQ_g_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } memberq_g_ifunction;

  class Find_g_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } find_g_ifunction;

  class eq_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } eq_gfunction;

  class BeginIterator_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } beginiterator_gfunction;

  class EndIterator_gFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } enditerator_gfunction;

  class GetNodeID_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } getnodeid_qfunction;

  class GetNodeIDModelID_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } getnodeidmodelid_qfunction;

  class Next_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } next_qfunction;

  class Eq_q_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } eq_q_qfunction;

  class Lt_q_qFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } lt_q_qfunction;

  class Get_g_iFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } get_g_ifunction;

  class Take_g_aFunction : public SLIFunction
  {
    void execute( SLIInterpreter* ) const;
  } take_g_afunction;

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

  class SetStdpEps_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } setstdpeps_dfunction;

  class Mul_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } mul_P_Pfunction;

  class Div_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } div_P_Pfunction;

  class Add_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } add_P_Pfunction;

  class Sub_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } sub_P_Pfunction;

  class Compare_P_P_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } compare_P_P_Dfunction;

  class Conditional_P_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } conditional_P_P_Pfunction;

  class Min_P_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } min_P_dfunction;

  class Max_P_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } max_P_dfunction;

  class Redraw_P_d_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } redraw_P_d_dfunction;

  class Exp_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } exp_Pfunction;

  class Sin_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } sin_Pfunction;

  class Cos_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } cos_Pfunction;

  class Pow_P_dFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } pow_P_dfunction;

  class Dimension2d_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dimension2d_P_Pfunction;

  class Dimension3d_P_P_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dimension3d_P_P_Pfunction;

  class CreateParameter_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } createparameter_Dfunction;

  class GetValue_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getvalue_Pfunction;

  class IsSpatial_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } isspatial_Pfunction;

  class Apply_P_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } apply_P_Dfunction;

  class Apply_P_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } apply_P_gfunction;

#ifdef HAVE_LIBNEUROSIM
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
#endif

  //
  // SLI functions for spatial networks
  //

  class CreateLayer_D_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } createlayer_D_Dfunction;

  class GetPosition_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getposition_gfunction;

  class Displacement_g_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } displacement_g_gfunction;

  class Displacement_a_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } displacement_a_gfunction;

  class Distance_g_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } distance_g_gfunction;

  class Distance_a_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } distance_a_gfunction;

  class Distance_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } distance_afunction;

  class ConnectLayers_g_g_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } connectlayers_g_g_Dfunction;

  class CreateMask_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } createmask_Dfunction;

  class GetLayerStatus_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getlayerstatus_gfunction;

  class Inside_a_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } inside_a_Mfunction;

  class And_M_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } and_M_Mfunction;

  class Or_M_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } or_M_Mfunction;

  class Sub_M_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } sub_M_Mfunction;

  class DumpLayerNodes_os_gFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dumplayernodes_os_gfunction;

  class DumpLayerConnections_os_g_g_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dumplayerconnections_os_g_g_lfunction;

  class Cvdict_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } cvdict_Mfunction;

  class SelectNodesByMask_g_a_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
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

inline bool
NestModule::register_mask( const Name& name, MaskCreatorFunction creator )
{
  return mask_factory_().register_subtype( name, creator );
}

inline AbstractMask*
NestModule::create_mask( const Name& name, const DictionaryDatum& d )
{
  return mask_factory_().create( name, d );
}

} // namespace

#endif
