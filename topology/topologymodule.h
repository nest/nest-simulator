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
#include "generic_factory.h"
#include "exceptions.h"

// Includes from sli:
#include "slimodule.h"
#include "sharedptrdatum.h"

// Includes from topology:
#include "ntree.h"
#include "position.h"

namespace nest
{
class AbstractLayer;
class AbstractMask;

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

  static SLIType MaskType; ///< SLI type for masks

  /*
   * SLI functions: See source file for documentation
   */

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
  static sharedPtrDatum< AbstractMask, &TopologyModule::MaskType > /*MaskDatum*/ create_mask( const Token& t );

  /**
   * Create a new Mask object using the mask factory.
   * @param name Mask type to create.
   * @param d    Dictionary with parameters specific for this mask type.
   * @returns dynamically allocated new Mask object.
   */
  static AbstractMask* create_mask( const Name& name, const DictionaryDatum& d );

private:
  /**
   * Return a reference to the mask factory class.
   */
  static MaskFactory& mask_factory_();
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

/**
 * Exception to be thrown if the wrong node is given
 * @ingroup KernelExceptions
 */
class LayerNodeExpected : public KernelException
{
public:
  LayerNodeExpected()
    : KernelException( "LayerNodeExpected" )
  {
  }
  ~LayerNodeExpected() throw()
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

} // namespace nest

#endif
