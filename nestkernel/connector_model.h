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
#include "enum_bitfield.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event.h"
#include "nest_types.h"
#include "secondary_event.h"
#include "simulation_manager.h"


namespace nest
{
class ConnectorBase;
class CommonSynapseProperties;
class TimeConverter;
class Node;

enum class ConnectionModelProperties : unsigned
{
  NONE = 0,
  SUPPORTS_HPC = 1 << 0,
  SUPPORTS_LBL = 1 << 1,
  IS_PRIMARY = 1 << 2,
  HAS_DELAY = 1 << 3,
  SUPPORTS_WFR = 1 << 4,
  REQUIRES_SYMMETRIC = 1 << 5,
  REQUIRES_CLOPATH_ARCHIVING = 1 << 6,
  REQUIRES_URBANCZIK_ARCHIVING = 1 << 7,
  REQUIRES_EPROP_ARCHIVING = 1 << 8
};

template <>
struct EnableBitMaskOperators< ConnectionModelProperties >
{
  static const bool enable = true;
};

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
   *
   * Checks against setting CommonSynapseProperties upon Connect() are implemented in GenericConnectorModel.
   * Any further checks need to be implemented by the connection model class by overriding
   * Connection::check_synapse_params().
   */
  virtual void check_synapse_params( const DictionaryDatum& ) const = 0;

  virtual std::unique_ptr< SecondaryEvent > get_secondary_event() = 0;

  virtual size_t get_syn_id() const = 0;
  virtual void set_syn_id( synindex syn_id ) = 0;

  std::string
  get_name() const
  {
    return name_;
  }

  bool
  has_property( const ConnectionModelProperties& property ) const
  {
    return flag_is_set( properties_, property );
  }

  ConnectionModelProperties
  get_properties() const
  {
    return properties_;
  }

protected:
  // helper function to avoid circular dependency
  static size_t get_synapse_model_id( const std::string& name );

  std::string name_;                     //!< name of the ConnectorModel
  bool default_delay_needs_check_;       //!< indicates whether the default delay must be checked
  ConnectionModelProperties properties_; //!< connection properties
};


template < typename ConnectionT >
class GenericConnectorModel : public ConnectorModel
{
private:
  typename ConnectionT::CommonPropertiesType cp_;

  ConnectionT default_connection_;
  size_t receptor_type_;

public:
  GenericConnectorModel( const std::string name )
    : ConnectorModel( name, ConnectionT::properties )
    , receptor_type_( 0 )
  {
  }

  GenericConnectorModel( const GenericConnectorModel& cm, const std::string name )
    : ConnectorModel( cm, name )
    , cp_( cm.cp_ )
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

  typename ConnectionT::CommonPropertiesType const&
  get_common_properties() const override
  {
    return cp_;
  }

  size_t get_syn_id() const override;
  void set_syn_id( synindex syn_id ) override;

  void check_synapse_params( const DictionaryDatum& syn_spec ) const override;

  std::unique_ptr< SecondaryEvent > get_secondary_event() override;

  ConnectionT const&
  get_default_connection() const
  {
    return default_connection_;
  }

private:
  void used_default_delay();

  void add_connection_( Node& src,
    Node& tgt,
    std::vector< ConnectorBase* >& hetconn,
    const synindex syn_id,
    ConnectionT& c,
    const size_t receptor_type );

}; // GenericConnectorModel

} // namespace nest

#endif /* #ifndef CONNECTOR_MODEL_H */
