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

#include "slimodule.h"
#include "network.h"
#include "position.h"
#include "ntree.h"
#include "exceptions.h"
#include "generic_factory.h"

namespace nest
{
  class Parameter;
  class AbstractMask;
  class AbstractLayer;

  template<int D>
  class Layer;

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

    static SLIType MaskType;         ///< SLI type for masks
    static SLIType ParameterType;    ///< SLI type for parameters

    /*
     * SLI functions: See source file for documentation
     */

    class CreateLayer_DFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } createlayer_Dfunction;

    class GetPosition_iFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } getposition_ifunction;

    class Displacement_a_iFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } displacement_a_ifunction;

    class Distance_a_iFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } distance_a_ifunction;

    class GetGlobalChildren_i_M_aFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } getglobalchildren_i_M_afunction;

    class ConnectLayers_i_i_DFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } connectlayers_i_i_Dfunction;

    class CreateMask_DFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } createmask_Dfunction;

    class Inside_a_MFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } inside_a_Mfunction;

    class And_M_MFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } and_M_Mfunction;

    class Or_M_MFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } or_M_Mfunction;

    class Sub_M_MFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } sub_M_Mfunction;

    class Mul_P_PFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } mul_P_Pfunction;

    class Div_P_PFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } div_P_Pfunction;

    class Add_P_PFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } add_P_Pfunction;

    class Sub_P_PFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } sub_P_Pfunction;

    class CreateParameter_DFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } createparameter_Dfunction;

    class GetValue_a_PFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } getvalue_a_Pfunction;

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

    class GetElement_i_iaFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } getelement_i_iafunction;

    class Cvdict_MFunction: public SLIFunction
    {
    public:
      void execute(SLIInterpreter *) const;
    } cvdict_Mfunction;

    /**
     * Return a reference to the network managed by the topology module.
     */
    static Network &get_network();

    typedef GenericFactory<AbstractMask> MaskFactory;
    typedef GenericFactory<AbstractMask>::CreatorFunction MaskCreatorFunction;

    template<class T>
    static bool register_mask();
    template<class T>
    static bool register_mask(const Name & name);
    static bool register_mask(const Name& name, MaskCreatorFunction creator);

    static lockPTRDatum<AbstractMask, &TopologyModule::MaskType> /*MaskDatum*/ create_mask(const Token &);
    static AbstractMask *create_mask(const Name& name, const DictionaryDatum &d);

    typedef GenericFactory<Parameter> ParameterFactory;
    typedef GenericFactory<Parameter>::CreatorFunction ParameterCreatorFunction;

    template<class T>
    static bool register_parameter(const Name & name);
    static bool register_parameter(const Name& name, ParameterCreatorFunction creator);

    static lockPTRDatum<Parameter, &TopologyModule::ParameterType> /*ParameterDatum*/ create_parameter(const Token &);
    static Parameter *create_parameter(const Name& name, const DictionaryDatum &d);

  private:


    /**
     * Return a reference to the mask factory class.
     */
    static MaskFactory &mask_factory_();

    /**
     * Return a reference to the parameter factory class.
     */
    static ParameterFactory &parameter_factory_();

    /**
     * - @c net must be static, so that the execute() members of the
     *   SliFunction classes in the module can access the network.
     */
    static Network* net_;
  };

  /**
   * Exception to be thrown if the wrong argument type
   * is given to a function
   * @ingroup KernelExceptions
   */
  class LayerExpected: public KernelException
  {
  public:
  LayerExpected()
    : KernelException("LayerExpected") {}
    ~LayerExpected() throw () {}

    std::string message();
  };

  inline
  Network &TopologyModule::get_network()
  {
    assert(net_ != 0);
    return *net_;
  }

  template<class T>
  inline
  bool TopologyModule::register_mask()
  {
    return mask_factory_().register_subtype<T>(T::get_name());
  }

  template<class T>
  inline
  bool TopologyModule::register_mask(const Name& name)
  {
    return mask_factory_().register_subtype<T>(name);
  }

  inline
  bool TopologyModule::register_mask(const Name& name, MaskCreatorFunction creator)
  {
    return mask_factory_().register_subtype(name, creator);
  }

  inline
  AbstractMask *TopologyModule::create_mask(const Name& name, const DictionaryDatum &d)
  {
    return mask_factory_().create(name,d);
  }

  template<class T>
  inline
  bool TopologyModule::register_parameter(const Name& name)
  {
    return parameter_factory_().register_subtype<T>(name);
  }

  inline
  bool TopologyModule::register_parameter(const Name& name, ParameterCreatorFunction creator)
  {
    return parameter_factory_().register_subtype(name, creator);
  }


} // namespace nest

#endif
