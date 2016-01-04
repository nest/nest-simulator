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
  ConnectorModel( const std::string );
  ConnectorModel( const ConnectorModel&, const std::string );
  virtual ~ConnectorModel()
  {
  }

  /**
   * NAN is a special value in cmath, which describes double values that
   * are not a number. If delay or weight is omitted in an add_connection call,
   * NAN indicates this and weight/delay are set only, if they are valid.
   */
  virtual ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    double_t delay = numerics::nan,
    double_t weight = numerics::nan ) = 0;
  virtual ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    DictionaryDatum& d,
    double_t delay = numerics::nan,
    double_t weight = numerics::nan ) = 0;

  virtual ConnectorModel* clone( std::string ) const = 0;

  virtual void calibrate( const TimeConverter& tc ) = 0;

  virtual void get_status( DictionaryDatum& ) const = 0;
  virtual void set_status( const DictionaryDatum& ) = 0;

  virtual const CommonSynapseProperties& get_common_properties() const = 0;

  virtual void set_syn_id( synindex syn_id ) = 0;

  std::string
  get_name() const
  {
    return name_;
  }

protected:
  std::string name_;
  bool default_delay_needs_check_; //!< Flag indicating, that the default delay must be checked

}; // ConnectorModel


template < typename ConnectionT >
class GenericConnectorModel : public ConnectorModel
{
  typename ConnectionT::CommonPropertiesType cp_;
  ConnectionT default_connection_;
  rport receptor_type_;

public:
  GenericConnectorModel( const std::string name )
    : ConnectorModel( name )
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

  ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    double_t weight,
    double_t delay );
  ConnectorBase* add_connection( Node& src,
    Node& tgt,
    ConnectorBase* conn,
    synindex syn_id,
    DictionaryDatum& d,
    double_t weight,
    double_t delay );

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

  ConnectionT const&
  get_default_connection() const
  {
    return default_connection_;
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


} // namespace nest

#endif /* #ifndef CONNECTOR_MODEL_H */
