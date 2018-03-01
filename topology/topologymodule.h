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

#ifndef TOPOLOGYMODULE_H
#define TOPOLOGYMODULE_H

// Includes from nestkernel:
#include "exceptions.h"

// Includes from sli:
#include "lockptr.h"
#include "slimodule.h"

// Includes from topology:
#include "generic_factory.h"
#include "ntree.h"
#include "position.h"

namespace nest
{
class AbstractLayer;
class AbstractMask;
class TopologyParameter;

template < int D >
class Layer;

class TopologyModule : public SLIModule
{
public:
  TopologyModule();
  ~TopologyModule();

  /**
   * @param SLIInterpreter* SLI interpreter, must know modeldict
   */
  void init( SLIInterpreter* );

  const std::string name( void ) const;
  const std::string commandstring( void ) const;

  static SLIType MaskType;      ///< SLI type for masks
  static SLIType ParameterType; ///< SLI type for parameters

  /*
   * SLI functions: See source file for documentation
   */

  class CreateLayer_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } createlayer_Dfunction;

  class GetPosition_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getposition_ifunction;

  class Displacement_a_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } displacement_a_ifunction;

  class Distance_a_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } distance_a_ifunction;

  class GetGlobalChildren_i_M_aFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getglobalchildren_i_M_afunction;

  class ConnectLayers_i_i_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } connectlayers_i_i_Dfunction;

  class CreateMask_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } createmask_Dfunction;

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

  class CreateParameter_DFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } createparameter_Dfunction;

  class GetValue_a_PFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getvalue_a_Pfunction;

  class DumpLayerNodes_os_iFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dumplayernodes_os_ifunction;

  class DumpLayerConnections_os_i_lFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } dumplayerconnections_os_i_lfunction;

  class GetElement_i_iaFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } getelement_i_iafunction;

  class Cvdict_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } cvdict_Mfunction;

  class SelectNodesByMask_L_a_MFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  } selectnodesbymask_L_a_Mfunction;

  typedef GenericFactory< AbstractMask > MaskFactory;
  typedef GenericFactory< AbstractMask >::CreatorFunction MaskCreatorFunction;

  /**
   * Register an AbstractMask subclass as a new mask type. The name will
   * be found using the function T::get_name()
   * @returns true if the new type was successfully registered, or false
   *          if a mask type with the same name already exists.
   */
  template < class T >
  static bool register_mask();

  /**
   * Register an AbstractMask subclass as a new mask type with the given
   * name.
   * @param name name of the new mask type.
   * @returns true if the new type was successfully registered, or false
   *          if a mask type with the same name already exists.
   */
  template < class T >
  static bool register_mask( const Name& name );

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
  static lockPTRDatum< AbstractMask,
    &TopologyModule::MaskType > /*MaskDatum*/ create_mask( const Token& t );

  /**
   * Create a new Mask object using the mask factory.
   * @param name Mask type to create.
   * @param d    Dictionary with parameters specific for this mask type.
   * @returns dynamically allocated new Mask object.
   */
  static AbstractMask* create_mask( const Name& name,
    const DictionaryDatum& d );

  typedef GenericFactory< TopologyParameter > ParameterFactory;
  typedef GenericFactory< TopologyParameter >::CreatorFunction
    ParameterCreatorFunction;

  /**
   * Register an Parameter subclass as a new parameter type with the
   * given name.
   * @param name name of the new parameter type.
   * @returns true if the new type was successfully registered, or false
   *          if a parameter type with the same name already exists.
   */
  template < class T >
  static bool register_parameter( const Name& name );

  /**
   * Register a new parameter type with the given name, with a supplied
   * function to create parameter objects of this type.
   * @param name    name of the new parameter type.
   * @param creator function creating objects of this type. The function
   *                will be called with the parameter dictionary as
   *                argument and should return a pointer to a new
   *                Parameter object.
   * @returns true if the new type was successfully registered, or false
   *          if a parameter type with the same name already exists.
   */
  static bool register_parameter( const Name& name,
    ParameterCreatorFunction creator );

  /**
   * Return a Parameter object.
   * @param t Either an existing ParameterDatum, or a DoubleDatum
   *          containing a constant value for this parameter, or a
   *          Dictionary containing parameters. The dictionary
   *          should contain a single key with the name of the parameter
   *          type, with a dictionary of parameters as value.
   * @returns Either the ParameterDatum given as argument, or a new
   *          parameter.
   */
  static lockPTRDatum< TopologyParameter,
    &TopologyModule::
      ParameterType > /*ParameterDatum*/ create_parameter( const Token& );

  /**
   * Create a new Parameter object using the parameter factory.
   * @param name Parameter type to create.
   * @param d    Dictionary with parameters specific for this parameter
   *             type.
   * @returns dynamically allocated new Parameter object.
   */
  static TopologyParameter* create_parameter( const Name& name,
    const DictionaryDatum& d );

private:
  /**
   * Return a reference to the mask factory class.
   */
  static MaskFactory& mask_factory_();

  /**
   * Return a reference to the parameter factory class.
   */
  static ParameterFactory& parameter_factory_();
};


/**
 * Exception to be thrown if the wrong argument type
 * is given to a function
 * @ingroup KernelExceptions
 */
class LayerExpected : public KernelException
{
public:
  LayerExpected()
    : KernelException( "LayerExpected" )
  {
  }
  ~LayerExpected() throw()
  {
  }

  std::string message() const;
};

template < class T >
inline bool
TopologyModule::register_mask()
{
  return mask_factory_().register_subtype< T >( T::get_name() );
}

template < class T >
inline bool
TopologyModule::register_mask( const Name& name )
{
  return mask_factory_().register_subtype< T >( name );
}

inline bool
TopologyModule::register_mask( const Name& name, MaskCreatorFunction creator )
{
  return mask_factory_().register_subtype( name, creator );
}

inline AbstractMask*
TopologyModule::create_mask( const Name& name, const DictionaryDatum& d )
{
  return mask_factory_().create( name, d );
}

template < class T >
inline bool
TopologyModule::register_parameter( const Name& name )
{
  return parameter_factory_().register_subtype< T >( name );
}

inline bool
TopologyModule::register_parameter( const Name& name,
  ParameterCreatorFunction creator )
{
  return parameter_factory_().register_subtype( name, creator );
}


} // namespace nest

#endif
