/*
 *  node.cpp
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


#include "node.h"
#include "exceptions.h"
#include "event.h"
#include "network.h"
#include "namedatum.h"
#include "arraydatum.h"
#include "dictutils.h"

namespace nest {

  Network *Node::net_=NULL;

  Node::Node()
      :gid_(0),
       lid_(0),
       model_id_(-1),
       parent_(0),
       stat_(),
       thread_(0),
       vp_(invalid_thread_)
  {
    /**
     *
     * the scheduler starts with update_reference()==true,
     * thus we must reset the updated flag, so that
     * is_updated() will return false at the beginning of
     * each time-slice.
     */
    stat_.reset(updated); //!< Set to opposite of the scheduler's value
  }

  Node::Node(const Node &n)
      :gid_(0),
       lid_(0),
       model_id_(n.model_id_),
       parent_(n.parent_),
       stat_(n.stat_),  // copied from model prototype, frozen may be set
       thread_(n.thread_),
       vp_(n.vp_)
  {
  }

  Node::~Node()
  {}

  void Node::init_state()
  {
    Model const * const model= net_->get_model(model_id_);
    assert(model);
    init_state_(model->get_prototype());
  }
  
  void Node::init_buffers()
  {
    if ( stat_.test(buffers_initialized) )
      return;
      
    init_buffers_();
    stat_.set(buffers_initialized);
  }
  
  std::string Node::get_name() const
  {
    if(net_==0 || model_id_<0)
      return std::string("UnknownNode");

    return net_->get_model(model_id_)->get_name();
  }

  Model & Node::get_model_() const
  {
    if(net_==0 || model_id_<0)
      throw UnknownModelID(model_id_);
    
    return *net_->get_model(model_id_);
  }      

  bool Node::is_updated() const
  {
    return stat_.test(updated)==net_->update_reference();
  }

  bool Node::is_local() const
  {
    return !is_proxy();
  }

  DictionaryDatum Node::get_status_dict_()
  {
    return DictionaryDatum(new Dictionary);
  }  

  DictionaryDatum Node::get_status_base()
  {
    DictionaryDatum dict = get_status_dict_();

    assert(dict.valid());

    // add information available for all nodes
    (*dict)[names::local] = is_local();
    (*dict)[names::model] = LiteralDatum(get_name());

    // add information available only for local nodes
    if ( is_local() )
    {
      (*dict)[names::global_id] = get_gid();
      (*dict)[names::state] = get_status_flag();
      (*dict)[names::frozen] = is_frozen();
      (*dict)[names::thread] = get_thread();
      (*dict)[names::vp] = get_vp();
      if ( parent_ )
      {
        (*dict)[names::parent] = parent_->get_gid();

        // LIDs are only sensible for nodes with parents.
        // Add 1 as we count lids internally from 0, but from
        // 1 in the user interface.
        (*dict)[names::local_id] = get_lid() + 1;
      }
    }

    // This is overwritten with a corresponding value in the
    // base classes for stimulating and recording devices, and
    // in other special node classes
    (*dict)[names::type] = LiteralDatum(names::neuron);

    // now call the child class' hook
    get_status(dict);

    assert(dict.valid());
    return dict;
  }

  void Node::set_status_base(const DictionaryDatum &dict)
  {
    assert(dict.valid());

    // We call the child's set_status first, so that the Node remains
    // unchanged if the child should throw an exception.
    set_status(dict);

    if(dict->known(names::frozen))
    {
      bool frozen_val=(*dict)[names::frozen];

      if( frozen_val == true )
	set(frozen);
      else
	unset(frozen);
    }
  }

  /**
   * Default implementation of just throws UnexpectedEvent
   */
  port Node::check_connection(Connection&, port)
  {
    throw UnexpectedEvent();
    return invalid_port_;
  }

  /**
   * Default implementation of register_stdp_connection() just 
   * throws IllegalConnection
   */
  void Node::register_stdp_connection(double_t)
  {
    throw IllegalConnection();
  }

  /**
   * Default implementation of unregister_stdp_connection() just 
   * throws IllegalConnection
   */
  void Node::unregister_stdp_connection(double_t)
  {
    throw IllegalConnection();
  }

  /**
   * Default implementation of event handlers just throws
   * an UnexpectedEvent exception.
   * @see class UnexpectedEvent
   * @throws UnexpectedEvent  This is the default event to throw.
   */
  void Node::handle(SpikeEvent&)
  {
    throw UnexpectedEvent();
  }

  port Node::connect_sender(SpikeEvent&, port)
  {
    throw IllegalConnection();
    return invalid_port_;
  }

  void Node::handle(RateEvent&)
  {
    throw UnexpectedEvent();
  }

  port Node::connect_sender(RateEvent&, port)
  {
    throw IllegalConnection();
    return invalid_port_;
  }

  void Node::handle(CurrentEvent&)
  {
    throw UnexpectedEvent();
  }

  port Node::connect_sender(CurrentEvent&, port)
  {
    throw IllegalConnection();
    return invalid_port_;
  }

  void Node::handle(DataLoggingRequest&)
  {
    throw UnexpectedEvent();
  }

  port Node::connect_sender(DataLoggingRequest&, port)
  {
    throw IllegalConnection();
    return invalid_port_;
  }

  void Node::handle(DataLoggingReply&)
  {
    throw UnexpectedEvent();
  }

  void Node::handle(ConductanceEvent&)
  {
    throw UnexpectedEvent();
  }

  port Node::connect_sender(ConductanceEvent&, port)
  {
    throw IllegalConnection();
    return invalid_port_;
  }

  void Node::handle(DoubleDataEvent&)
  {
    throw UnexpectedEvent();
  }

  port Node::connect_sender(DoubleDataEvent&, port)
  {
    throw IllegalConnection();
    return invalid_port_;
  }

  double_t Node::get_K_value(double_t)
  {
    throw UnexpectedEvent();
    return 0;
  }


  void Node::get_K_values(double_t, double_t&, double_t&)
  {
    throw UnexpectedEvent();
  }

  void nest::Node::get_history(double_t, double_t,
			       std::deque<histentry>::iterator*,
			       std::deque<histentry>::iterator*)
  {
    throw UnexpectedEvent();
  }


  void Node::event_hook(DSSpikeEvent& e)
  {
    e.get_receiver().handle(e);
  }

  void Node::event_hook(DSCurrentEvent& e)
  {
    e.get_receiver().handle(e);
  }

  bool Node::allow_entry() const
  {
    return false;
  }

} // namespace
