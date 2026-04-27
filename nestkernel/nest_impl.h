/*
 *  nest_impl.h
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

#ifndef NEST_IMPL_H
#define NEST_IMPL_H

#include "connection_label_impl.h"
#include "generic_factory_impl.h"
#include "kernel_manager.h"
#include "mask_impl.h"
#include "model_manager_impl.h"
#include "nest.h"

namespace nest
{

template < template < typename > class ConnectorModelT >
void
register_connection_model( const std::string& name )
{
  kernel::manager< ModelManager >.template register_connection_model< ConnectorModelT >( name );
}

template < typename NodeModelT >
void
register_node_model( const std::string& name, std::string deprecation_info )
{
  kernel::manager< ModelManager >.template register_node_model< NodeModelT >( name, deprecation_info );
}

template < class T >
inline bool
register_parameter( const std::string& name )
{
  return parameter_factory_().register_subtype< T >( name );
}

template < class T >
inline bool
register_mask()
{
  return mask_factory_().register_subtype< T >( T::get_name() );
}

inline bool
register_mask( const std::string& name, MaskCreatorFunction creator )
{
  return mask_factory_().register_subtype( name, creator );
}

inline MaskPTR
create_mask( const std::string& name, const Dictionary& d )
{
  d.init_access_flags();
  auto mask = MaskPTR( mask_factory_().create( name, d ) );
  d.all_entries_accessed( "CreateMask", "specs" );
  return mask;
}

}

#endif /* NEST_IMPL_H */
