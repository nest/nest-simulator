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

class ConnectorModel
{

public:
  ConnectorModel( const std::string,
    const bool is_primary,
    const bool has_delay,
    const bool requires_symmetric,
    const bool supports_wfr );
  ConnectorModel( const ConnectorModel&, const std::string );
  virtual ~ConnectorModel()
  {
  }

  /**
   * Adds a connection.
   *
   * @param src Source node
   * @param tgt Target node
   * @param hetconn Connector vector
   * @param syn_id Synapse id
   * @param d Parameter dictionary to configure the synapse
   * @param delay Delay of the connection
   * @param weight Weight of the connection
   *
   * Delay and weight have the default value NAN, a special value, which
   * describes double values that are not a number. If delay or weight is
   * omitted, NAN indicates this and weight/delay are set only if they are
   * valid.
   */
  virtual void add_connection( Node& src,
    Node& tgt,
    std::vector< ConnectorBase* >& hetconn,
    const synindex syn_id,
    const DictionaryDatum& d,
    const double delay = NAN,
    const double weight = NAN ) = 0;

  virtual ConnectorModel* clone( std::string ) const = 0;

  virtual void calibrate( const TimeConverter& tc ) = 0;

  virtual void get_status( DictionaryDatum& ) const = 0;
  virtual void set_status( const DictionaryDatum& ) = 0;

  virtual const CommonSynapseProperties& get_common_properties() const = 0;

  /**
   * Checks to see if illegal parameters are given in syn_spec.
   */
  virtual void check_synapse_params( const DictionaryDatum& ) const = 0;

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

  bool
  supports_wfr() const
  {
    return supports_wfr_;
  }

protected:
  //! name of the ConnectorModel
  std::string name_;
  //! indicates whether the default delay must be checked
  bool default_delay_needs_check_;
  //! indicates whether this ConnectorModel belongs to a primary connection
  bool is_primary_;
  //! indicates whether ConnectorModel has a delay
  bool has_delay_;
  //! indicates that ConnectorModel requires symmetric connections
  bool requires_symmetric_;
  //! indicates whether connection can be used during wfr update
  bool supports_wfr_;

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
    bool requires_symmetric,
    bool supports_wfr )
    : ConnectorModel( name,
        is_primary,
        has_delay,
        requires_symmetric,
        supports_wfr )
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

  void add_connection( Node& src,
    Node& tgt,
    std::vector< ConnectorBase* >& hetconn,
    const synindex syn_id,
    const DictionaryDatum& d,
    const double delay,
    const double weight );

  ConnectorModel* clone( std::string ) const;

  void calibrate( const TimeConverter& tc );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  void
  check_synapse_params( const DictionaryDatum& syn_spec ) const
  {
    default_connection_.check_synapse_params( syn_spec );
  }

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

  void add_connection_( Node& src,
    Node& tgt,
    std::vector< ConnectorBase* >& hetconn,
    const synindex syn_id,
    ConnectionT& c,
    const rport receptor_type );

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
    const bool has_delay,
    const bool requires_symmetric,
    const bool supports_wfr )
    : GenericConnectorModel< ConnectionT >( name,
        /*is _primary=*/false,
        has_delay,
        requires_symmetric,
        supports_wfr )
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
