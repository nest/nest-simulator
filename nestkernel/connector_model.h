/*
 *  connector_model.h
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

#ifndef CONNECTOR_MODEL_H
#define CONNECTOR_MODEL_H

// C++ includes:
#include <cmath>
#include <string>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
class ConnectorBase;
class CommonSynapseProperties;
class TimeConverter;
class Node;

/**
 * This function sets the two lowest bits of the pointer depending on the
 * existing connections.
 *
 * - If *p contains primary connections the lowest bit is set to 1
 * - If *p contains secondary connections the second lowest bit is set to 1
 *
 * This implementation relies on the assumption that the two lowest bits of the
 * pointer are 0. This can be assumed with some certainty (see github issue #186
 * for a discussion).
 * The assumption is secured by an assert in the allocate()-function.
 */
inline ConnectorBase*
pack_pointer( ConnectorBase* p, bool has_primary, bool has_secondary )
{
  return reinterpret_cast< ConnectorBase* >(
    reinterpret_cast< unsigned long >( p ) | has_primary
    | ( has_secondary << 1 ) );
}

/**
 * This function removes the setting of the two lowest bits done in the
 * pack_pointer()-function.
 * The returned pointer can again be used as a valid pointer.
 */
inline ConnectorBase*
validate_pointer( ConnectorBase* p )
{
  // erase 2 least significant bits to obtain the correct pointer
  return reinterpret_cast< ConnectorBase* >(
    ( reinterpret_cast< unsigned long >( p ) & ( -1l - 3l ) ) );
}

inline bool
has_primary( ConnectorBase* p )
{
  // the lowest bit is set, if there is at least one primary connection
  return static_cast< bool >( reinterpret_cast< unsigned long >( p ) & 1 );
}

inline bool
has_secondary( ConnectorBase* p )
{
  // the second lowest bit is set, if there is at least one secondary connection
  return static_cast< bool >( reinterpret_cast< unsigned long >( p ) & 2 );
}


class ConnectorModel
{

public:
  ConnectorModel( const std::string,
    bool is_primary,
    bool has_delay,
    bool requires_symmetric );
  ConnectorModel( const ConnectorModel&, const std::string );
  virtual ~ConnectorModel()
  {
  }

  /**
   * numerics::nan is a special value, which describes double values that
   * are not a number. If delay or weight is omitted in an add_connection call,
   * numerics:nan indicates this and weight/delay are set only, if they are
   * valid.
   */
  virtual ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    double delay = numerics::nan,
    double weight = numerics::nan ) = 0;

  virtual ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    DictionaryDatum& d,
    double delay = numerics::nan,
    double weight = numerics::nan ) = 0;

  /**
   * Delete a connection of a given type directed to a defined target Node
   * @param tgt Target node
   * @param target_thread Thread of the target
   * @param conn Connector Base from where the connection will be deleted
   * @param syn_id Synapse type
   * @return A new Connector, equal to the original but with an erased
   * connection to the defined target.
   */
  virtual ConnectorBase* delete_connection( Node& tgt,
    size_t target_thread,
    ConnectorBase* conn,
    synindex syn_id ) = 0;

  virtual ConnectorModel* clone( std::string ) const = 0;

  virtual void calibrate( const TimeConverter& tc ) = 0;

  virtual void get_status( DictionaryDatum& ) const = 0;
  virtual void set_status( const DictionaryDatum& ) = 0;

  virtual const CommonSynapseProperties& get_common_properties() const = 0;

  virtual SecondaryEvent* get_event() const = 0;

  virtual void set_syn_id( synindex syn_id ) = 0;

  virtual std::vector< SecondaryEvent* > create_event( size_t n ) const = 0;

  std::string
  get_name() const
  {
    return name_;
  }

  bool
  is_primary() const
  {
    return is_primary_;
  }

  bool
  has_delay() const
  {
    return has_delay_;
  }

  bool
  requires_symmetric() const
  {
    return requires_symmetric_;
  }

protected:
  std::string name_;
  //! Flag indicating, that the default delay must be checked
  bool default_delay_needs_check_;
  //! indicates, whether this ConnectorModel belongs to a primary connection
  bool is_primary_;
  bool has_delay_; //!< indicates, that ConnectorModel has a delay
  bool requires_symmetric_;
  //!< indicates, that ConnectorModel requires symmetric connections

}; // ConnectorModel


template < typename ConnectionT >
class GenericConnectorModel : public ConnectorModel
{
private:
  typename ConnectionT::CommonPropertiesType cp_;
  //! used to create secondary events that belong to secondary connections
  typename ConnectionT::EventType* pev_;

  ConnectionT default_connection_;
  rport receptor_type_;

public:
  GenericConnectorModel( const std::string name,
    bool is_primary,
    bool has_delay,
    bool requires_symmetric )
    : ConnectorModel( name, is_primary, has_delay, requires_symmetric )
    , receptor_type_( 0 )
  {
  }

  GenericConnectorModel( const GenericConnectorModel& cm,
    const std::string name )
    : ConnectorModel( cm, name )
    , cp_( cm.cp_ )
    , pev_( cm.pev_ )
    , default_connection_( cm.default_connection_ )
    , receptor_type_( cm.receptor_type_ )
  {
  }

  ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    double weight,
    double delay );
  ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    DictionaryDatum& d,
    double weight,
    double delay );

  ConnectorBase* delete_connection( Node& tgt,
    size_t target_thread,
    ConnectorBase* conn,
    synindex syn_id );

  ConnectorModel* clone( std::string ) const;

  void calibrate( const TimeConverter& tc );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  typename ConnectionT::CommonPropertiesType const&
  get_common_properties() const
  {
    return cp_;
  }

  void set_syn_id( synindex syn_id );

  virtual typename ConnectionT::EventType*
  get_event() const
  {
    assert( false );
    return 0;
  }

  ConnectionT const&
  get_default_connection() const
  {
    return default_connection_;
  }

  virtual std::vector< SecondaryEvent* > create_event( size_t ) const
  {
    // Should not be called for a ConnectorModel belonging to a primary
    // connection. Only required for secondary connection types.
    assert( false );
    std::vector< SecondaryEvent* > prototype_events;
    return prototype_events;
  }

private:
  void used_default_delay();

  ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    ConnectionT& c,
    rport receptor_type );

}; // GenericConnectorModel

template < typename ConnectionT >
class GenericSecondaryConnectorModel
  : public GenericConnectorModel< ConnectionT >
{
private:
  //! used to create secondary events that belong to secondary connections
  typename ConnectionT::EventType* pev_;

public:
  GenericSecondaryConnectorModel( const std::string name,
    bool has_delay,
    bool requires_symmetric )
    : GenericConnectorModel< ConnectionT >( name,
        /*is _primary=*/false,
        has_delay,
        requires_symmetric )
    , pev_( 0 )
  {
    pev_ = new typename ConnectionT::EventType();
  }

  GenericSecondaryConnectorModel( const GenericSecondaryConnectorModel& cm,
    const std::string name )
    : GenericConnectorModel< ConnectionT >( cm, name )
  {
    pev_ = new typename ConnectionT::EventType( *cm.pev_ );
  }


  ConnectorModel*
  clone( std::string name ) const
  {
    return new GenericSecondaryConnectorModel(
      *this, name ); // calls copy construtor
  }

  std::vector< SecondaryEvent* >
  create_event( size_t n ) const
  {
    std::vector< SecondaryEvent* > prototype_events( n, NULL );
    for ( size_t i = 0; i < n; i++ )
    {
      prototype_events[ i ] = new typename ConnectionT::EventType();
    }

    return prototype_events;
  }


  ~GenericSecondaryConnectorModel()
  {
    if ( pev_ != 0 )
    {
      delete pev_;
    }
  }

  typename ConnectionT::EventType*
  get_event() const
  {
    return pev_;
  }
};

} // namespace nest

#endif /* #ifndef CONNECTOR_MODEL_H */
