#ifndef GENERICMODEL_H
#define GENERICMODEL_H
/*
 *  genericmodel.h
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

#include "model.h"
#include "dynmodule.h"
#include <new>

namespace nest
{
  /**
   * Generic Model template.
   * The template GenericModel should be used
   * as base class for custom model classes. It already includes the
   * element factory functionality, as well as a pool based memory
   * manager, so that the user can concentrate on the "real" model
   * aspects.
   * @ingroup user_interface
   */
  template <typename ElementT>
  class GenericModel: public Model
  {
  public:
    GenericModel(const std::string &);
    GenericModel(const char[]);

    /**
     * Create copy of model with new name.
     */
    GenericModel(const GenericModel&, const std::string&);

    /**
     * Return pointer to cloned model with same name.
     */
    Model* clone(const std::string&) const;

    bool has_proxies();
    bool one_node_per_process();
    bool is_off_grid();
    /**
       @note The decision of whether one node can receive a certain
       event was originally in the node. But in the distributed case,
       it may be that you only have a proxy node and not he real
       thing. Thus, you need to be able to make this decision without
       having the node. Since the model now takes responsibility for a
       lot of general node properties, it was a natural place to put
       this function.

       Model::check_connection() is a forwarding function that call
       check_connection() from the prototype. Since proxies know the
       model they represent, they can now answer a call to check
       connection by refering back to the model.
     */
    port check_connection(Connection&, port);

    Node const & get_prototype() const;

    void set_model_id(int);

  private:

    void set_status_(DictionaryDatum);
    DictionaryDatum get_status_();

    size_t get_element_size() const;

    /**
     * Call placement new on the supplied memory position.
     */
    Node* allocate_(void *);

    /**
     * Initialize the pool allocator with the node specific properties.
     */
    void init_memory_(sli::pool &);

    /**
     * Prototype node from which all instances are constructed.
     */
    ElementT proto_;
  };

  template< typename ElementT>
  GenericModel<ElementT>::GenericModel(const std::string &name)
    : Model(name),
      proto_()
  {
    set_threads();
  }

  template< typename ElementT>
  GenericModel<ElementT>::GenericModel(const char name[])
    : Model(std::string(name)),
      proto_()
  {
    set_threads();
  }

  template <typename ElementT>
  GenericModel<ElementT>::GenericModel(const GenericModel& oldmod, 
				       const std::string& newname)
    : Model(newname),
      proto_(oldmod.proto_)
  {
    set_type_id(oldmod.get_type_id());
    set_threads();
  }

  template <typename ElementT>
  Model* GenericModel<ElementT>::clone(const std::string& newname) const
  {
    return new GenericModel(*this, newname);
  }

  template< typename ElementT>
  Node* GenericModel<ElementT>::allocate_(void *adr)
  {
    Node *n = new(adr) ElementT(proto_);
    return n;
  }

  template< typename ElementT>
  void GenericModel<ElementT>::init_memory_(sli::pool &mem)
  {
    mem.init(sizeof(ElementT), 1000, 1);
  }

  template <typename ElementT>
  inline
  bool GenericModel<ElementT>::has_proxies()
  {
    return proto_.has_proxies();
  }

  template <typename ElementT>
  inline
  bool GenericModel<ElementT>::one_node_per_process()
  {
    return proto_.one_node_per_process();
  }

  template <typename ElementT>
  inline
  bool GenericModel<ElementT>::is_off_grid()
  {
    return proto_.is_off_grid();
  }

  template <typename ElementT>
  inline
  port GenericModel<ElementT>::check_connection(Connection& c, port receptor)
  {
    return proto_.check_connection(c, receptor);
  }

  template <typename ElementT>
  void GenericModel<ElementT>::set_status_(DictionaryDatum d)
  {
    proto_.set_status(d);
  }

  template <typename ElementT>
  DictionaryDatum GenericModel<ElementT>::get_status_()
  {
    DictionaryDatum d = proto_.get_status_base();
    (*d)["elementsize"]= sizeof(ElementT);
    return d;
  }

  template <typename ElementT>
  size_t GenericModel<ElementT>::get_element_size() const
  {
    return sizeof(ElementT);
  }
  
  template <typename ElementT>
  Node const & GenericModel<ElementT>::get_prototype() const
  {
    return proto_;
  }

  template <typename ElementT>
  void GenericModel<ElementT>::set_model_id(int i)
  {
    proto_.set_model_id(i);
  }
  
      
  /**
   * Register a model prototype with the network. 
   * This function must be called exactly once for each model class to make
   * it known to the network. The natural place for a call to this function
   * is in a *module.cpp file.
   * @param reference to network
   * @param name under which model is to be known in simulator
   * @return Model ID assigned by network
   *
   * @see register_private_prototype_model, register_prototype_connection
   */
  template <class ModelT>
  index register_model(Network& net, const std::string &name, 
                      bool private_model = false)
  {
    Model* prototype = new GenericModel<ModelT>(name);
    assert(prototype != 0);
    return net.register_model(*prototype, private_model);
  }

  /**
   * Register a pre-configured model prototype with the network. 
   * This function must be called exactly once for each model class to make
   * it known to the network. The natural place for a call to this function
   * is in a *module.cpp file.
   *
   * Pre-configured models are models based on the same class, as 
   * another model, but have different parameter settings; e.g., 
   * voltmeter is a pre-configured multimeter.
   *
   * @param reference to network
   * @param name under which model is to be known in simulator
   * @param dictionary to use to pre-configure model
   * @param register as private model if true
   * @return Model ID assigned by network
   *
   * @see register_private_prototype_model, register_prototype_connection
   */
  template <class ModelT>
  index register_preconf_model(Network& net, const std::string &name, 
			       Dictionary& conf, bool private_model = false)
  {
    Model* prototype = new GenericModel<ModelT>(name);
    assert(prototype != 0);
    conf.clear_access_flags();
    prototype->set_status(conf);
    std::string missed;
    assert(conf.all_accessed(missed));  // we only get here from C++ code, no need for exception
    return net.register_model(*prototype, private_model);
  }
   
  /**
   * Register a private model prototype with the network. 
   * Private model prototypes differ from normal ones in that they are not 
   * visible in the modeldict. This is important for node types that serve
   * only as in-/output elements of layers, but cannot "live" independently.
   * @param reference to network
   * @param name under which model is to be known in simulator
   * @return Model ID assigned by network
   *
   * @see register_prototype_model, register_prototype_connection, 
   *      layerdc_node, layerdc_generator
   */
  template <class ModelT>
  index register_private_model(Network& net, const std::string &name)
  {
    return register_model<ModelT>(net, name, true);
  }
}
#endif
