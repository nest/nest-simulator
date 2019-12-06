/*
 *  connector_model.cpp
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

#include "connector_model.h"

namespace nest
{

ConnectorModel::ConnectorModel( const std::string name,
  const bool is_primary,
  const bool has_delay,
  const bool requires_symmetric,
  const bool supports_wfr,
  const bool requires_clopath_archiving )
  : name_( name )
  , default_delay_needs_check_( true )
  , is_primary_( is_primary )
  , has_delay_( has_delay )
  , requires_symmetric_( requires_symmetric )
  , supports_wfr_( supports_wfr )
  , requires_clopath_archiving_( requires_clopath_archiving )
{
}

ConnectorModel::ConnectorModel( const ConnectorModel& cm, const std::string name )
  : name_( name )
  , default_delay_needs_check_( true )
  , is_primary_( cm.is_primary_ )
  , has_delay_( cm.has_delay_ )
  , requires_symmetric_( cm.requires_symmetric_ )
  , supports_wfr_( cm.supports_wfr_ )
  , requires_clopath_archiving_( cm.requires_clopath_archiving_ )
{
}

} // namespace nest
