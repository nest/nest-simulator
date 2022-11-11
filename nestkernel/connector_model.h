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
#include "enum_bitfield.h"
#include "event.h"
#include "nest_time.h"
#include "nest_types.h"
#include "secondary_event.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{
class ConnectorBase;
class CommonSynapseProperties;
class TimeConverter;
class Node;

enum class ConnectionModelProperties : unsigned
{
  SUPPORTS_HPC = 1 << 0,
  SUPPORTS_LBL = 1 << 1,
  IS_PRIMARY = 1 << 2,
  HAS_DELAY = 1 << 3,
  SUPPORTS_WFR = 1 << 4,
  REQUIRES_SYMMETRIC = 1 << 5,
  REQUIRES_CLOPATH_ARCHIVING = 1 << 6,
  REQUIRES_URBANCZIK_ARCHIVING = 1 << 7
};

template <>
struct EnableBitMaskOperators< ConnectionModelProperties >
{
  static const bool enable = true;
};

const ConnectionModelProperties default_connection_model_properties = ConnectionModelProperties::SUPPORTS_HPC
  | ConnectionModelProperties::SUPPORTS_LBL | ConnectionModelProperties::IS_PRIMARY
  | ConnectionModelProperties::HAS_DELAY;

const ConnectionModelProperties default_secondary_connection_model_properties =
  ConnectionModelProperties::SUPPORTS_WFR | ConnectionModelProperties::HAS_DELAY;

class ConnectorModel
{

public:
  ConnectorModel( const std::string, const ConnectionModelProperties& properties );
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

  virtual ConnectorModel* clone( std::string, synindex syn_id ) const = 0;

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

  virtual SecondaryEvent* create_event() const = 0;

  std::string
  get_name() const
  {
    return name_;
  }

  bool
  is_primary() const
  {
    return has_property( properties_, ConnectionModelProperties::IS_PRIMARY );
  }

  bool
  has_delay() const
  {
    return has_property( properties_, ConnectionModelProperties::HAS_DELAY );
  }

  bool
  requires_symmetric() const
  {
    return has_property( properties_, ConnectionModelProperties::REQUIRES_SYMMETRIC );
  }

  bool
  requires_clopath_archiving() const
  {
    return has_property( properties_, ConnectionModelProperties::REQUIRES_CLOPATH_ARCHIVING );
  }

  bool
  requires_urbanczik_archiving() const
  {
    return has_property( properties_, ConnectionModelProperties::REQUIRES_URBANCZIK_ARCHIVING );
  }

  bool
  supports_wfr() const
  {
    return has_property( properties_, ConnectionModelProperties::SUPPORTS_WFR );
  }

  bool
  has_flag_set( const ConnectionModelProperties& flag )
  {
      return has_property( properties_, flag );
  }

  ConnectionModelProperties
  get_properties() const
  {
      return properties_;
  }


protected:
  //! name of the ConnectorModel
  std::string name_;
  //! indicates whether the default delay must be checked
  bool default_delay_needs_check_;
  //! connection properties
  ConnectionModelProperties properties_;
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
  bool has_flag_set( const ConnectionModelProperties& flag ) {
//      return default_connection_.has_flag_set(flag);
      return has_property( properties_, flag );
  }

  GenericConnectorModel( const std::string name )
    : ConnectorModel( name, ConnectionT::properties )
    , receptor_type_( 0 )
  {
  }

  GenericConnectorModel( const GenericConnectorModel& cm, const std::string name )
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
    const double weight ) override;

  ConnectorModel* clone( std::string, synindex ) const override;

  void calibrate( const TimeConverter& tc ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void
  check_synapse_params( const DictionaryDatum& syn_spec ) const override
  {
    default_connection_.check_synapse_params( syn_spec );
  }

  typename ConnectionT::CommonPropertiesType const&
  get_common_properties() const override
  {
    return cp_;
  }

  void set_syn_id( synindex syn_id ) override;

  typename ConnectionT::EventType*
  get_event() const override
  {
    assert( false );
    return 0;
  }

  ConnectionT const&
  get_default_connection() const
  {
    return default_connection_;
  }

  SecondaryEvent*
  create_event() const override
  {
    // Must not be called for a ConnectorModel belonging to a primary
    // connection. Only required for secondary connection types.
    assert( false );
    return nullptr; // make the compiler happy
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
class GenericSecondaryConnectorModel : public GenericConnectorModel< ConnectionT >
{
private:
  //! used to create secondary events that belong to secondary connections
  typename ConnectionT::EventType* pev_;

public:
  GenericSecondaryConnectorModel( const std::string name )
    : GenericConnectorModel< ConnectionT >( name )
    , pev_( 0 )
  {
    pev_ = new typename ConnectionT::EventType();
    this->properties_ = ConnectionT::secondaryProperties;
  }

  GenericSecondaryConnectorModel( const GenericSecondaryConnectorModel& cm, const std::string name )
    : GenericConnectorModel< ConnectionT >( cm, name )
  {
    pev_ = new typename ConnectionT::EventType( *cm.pev_ );
  }


  ConnectorModel*
  clone( std::string name, synindex syn_id ) const
  {
    ConnectorModel* new_cm = new GenericSecondaryConnectorModel( *this, name ); // calls copy construtor
    new_cm->set_syn_id( syn_id );

    if ( not new_cm->is_primary() )
    {
      new_cm->get_event()->add_syn_id( syn_id );
    }

    return new_cm;
  }

  SecondaryEvent*
  create_event() const
  {
    return new typename ConnectionT::EventType();
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
