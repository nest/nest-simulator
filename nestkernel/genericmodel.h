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

#ifndef GENERICMODEL_H
#define GENERICMODEL_H

// C++ includes:
#include <new>

// Includes from nestkernel:
#include "model.h"

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
template < typename ElementT >
class GenericModel : public Model
{
public:
  GenericModel( const std::string&, const std::string& deprecation_info );

  /**
   * Create copy of model with new name.
   */
  GenericModel( const GenericModel&, const std::string& );

  /**
   * Return pointer to cloned model with same name.
   */
  Model* clone( const std::string& ) const;

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

     Model::send_test_event() is a forwarding function that calls
     send_test_event() from the prototype. Since proxies know the
     model they represent, they can now answer a call to check
     connection by referring back to the model.
   */
  port send_test_event( Node&, rport, synindex, bool );

  void sends_secondary_event( GapJunctionEvent& ge );

  SignalType sends_signal() const;

  void sends_secondary_event( InstantaneousRateConnectionEvent& re );

  void sends_secondary_event( DiffusionConnectionEvent& de );

  void sends_secondary_event( DelayedRateConnectionEvent& re );

  Node const& get_prototype() const;

  void set_model_id( int );

  int get_model_id();

  void deprecation_warning( const std::string& );

private:
  void set_status_( DictionaryDatum );
  DictionaryDatum get_status_();

  size_t get_element_size() const;

  /**
   * Call placement new on the supplied memory position.
   */
  Node* allocate_( void* );

  /**
   * Initialize the pool allocator with the node specific properties.
   */
  void init_memory_( sli::pool& );

  /**
   * Prototype node from which all instances are constructed.
   */
  ElementT proto_;

  /**
   * String containing deprecation information; empty if model not deprecated.
   */
  std::string deprecation_info_;

  /**
   * False until deprecation warning has been issued once
   */
  bool deprecation_warning_issued_;
};

template < typename ElementT >
GenericModel< ElementT >::GenericModel( const std::string& name, const std::string& deprecation_info )
  : Model( name )
  , proto_()
  , deprecation_info_( deprecation_info )
  , deprecation_warning_issued_( false )
{
  set_threads();
}

template < typename ElementT >
GenericModel< ElementT >::GenericModel( const GenericModel& oldmod, const std::string& newname )
  : Model( newname )
  , proto_( oldmod.proto_ )
  , deprecation_info_( oldmod.deprecation_info_ )
  , deprecation_warning_issued_( false )
{
  set_type_id( oldmod.get_type_id() );
  set_threads();
}

template < typename ElementT >
Model*
GenericModel< ElementT >::clone( const std::string& newname ) const
{
  return new GenericModel( *this, newname );
}

template < typename ElementT >
Node*
GenericModel< ElementT >::allocate_( void* adr )
{
  Node* n = new ( adr ) ElementT( proto_ );
  return n;
}

template < typename ElementT >
void
GenericModel< ElementT >::init_memory_( sli::pool& mem )
{
  mem.init( sizeof( ElementT ), 1000, 1 );
}

template < typename ElementT >
inline bool
GenericModel< ElementT >::has_proxies()
{
  return proto_.has_proxies();
}

template < typename ElementT >
inline bool
GenericModel< ElementT >::one_node_per_process()
{
  return proto_.one_node_per_process();
}

template < typename ElementT >
inline bool
GenericModel< ElementT >::is_off_grid()
{
  return proto_.is_off_grid();
}

template < typename ElementT >
inline port
GenericModel< ElementT >::send_test_event( Node& target, rport receptor, synindex syn_id, bool dummy_target )
{
  return proto_.send_test_event( target, receptor, syn_id, dummy_target );
}

template < typename ElementT >
inline void
GenericModel< ElementT >::sends_secondary_event( GapJunctionEvent& ge )
{
  return proto_.sends_secondary_event( ge );
}

template < typename ElementT >
inline void
GenericModel< ElementT >::sends_secondary_event( InstantaneousRateConnectionEvent& re )
{
  return proto_.sends_secondary_event( re );
}

template < typename ElementT >
inline void
GenericModel< ElementT >::sends_secondary_event( DiffusionConnectionEvent& de )
{
  return proto_.sends_secondary_event( de );
}

template < typename ElementT >
inline void
GenericModel< ElementT >::sends_secondary_event( DelayedRateConnectionEvent& re )
{
  return proto_.sends_secondary_event( re );
}

template < typename ElementT >
inline nest::SignalType
GenericModel< ElementT >::sends_signal() const
{
  return proto_.sends_signal();
}

template < typename ElementT >
void
GenericModel< ElementT >::set_status_( DictionaryDatum d )
{
  proto_.set_status( d );
}

template < typename ElementT >
DictionaryDatum
GenericModel< ElementT >::get_status_()
{
  DictionaryDatum d = proto_.get_status_base();
  ( *d )[ names::elementsize ] = sizeof( ElementT );
  return d;
}

template < typename ElementT >
size_t
GenericModel< ElementT >::get_element_size() const
{
  return sizeof( ElementT );
}

template < typename ElementT >
Node const&
GenericModel< ElementT >::get_prototype() const
{
  return proto_;
}

template < typename ElementT >
void
GenericModel< ElementT >::set_model_id( int i )
{
  proto_.set_model_id( i );
}

template < typename ElementT >
int
GenericModel< ElementT >::get_model_id()
{
  return proto_.get_model_id();
}
}

#endif
