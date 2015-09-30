/*
 *  connection_builder_manager.cpp
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

#include "connection_builder_manager.h"
#include "dictutils.h"
#include "network.h"

nest::ConnectionBuilderManager::ConnectionBuilderManager()
  : connruledict_(new Dictionary())
  , connbuilder_factories_()
{
  Network::get_network().interpreter_.def( "connruledict", new DictionaryDatum( connruledict_ ) );

}

void
nest::ConnectionBuilderManager::init()
{
}

void
nest::ConnectionBuilderManager::reset()
{
}

void
nest::ConnectionBuilderManager::set_status( const Dictionary& d )
{
}

void
nest::ConnectionBuilderManager::get_status( Dictionary& d )
{
}

void
ConnectionBuilderManager::connect( const GIDCollection& sources,
  const GIDCollection& targets,
  const DictionaryDatum& conn_spec,
  const DictionaryDatum& syn_spec )
{
  conn_spec->clear_access_flags();
  syn_spec->clear_access_flags();

  if ( !conn_spec->known( names::rule ) )
    throw BadProperty( "Connectivity spec must contain connectivity rule." );
  const Name rule_name = ( *conn_spec )[ names::rule ];

  if ( !connruledict_->known( rule_name ) )
    throw BadProperty( "Unknown connectivty rule: " + rule_name );
  const long rule_id = ( *connruledict_ )[ rule_name ];

  ConnBuilder* cb =
    connbuilder_factories_.at( rule_id )->create( sources, targets, conn_spec, syn_spec );
  assert( cb != 0 );

  // at this point, all entries in conn_spec and syn_spec have been checked
  Name missed;
  if ( !( conn_spec->all_accessed( missed ) && syn_spec->all_accessed( missed ) ) )
  {
    if ( dict_miss_is_error() )
      throw UnaccessedDictionaryEntry( missed );
    else
      LOG( M_WARNING, "Connect", "Unread dictionary entries: " + missed );
  }

  cb->connect();
  delete cb;
}
