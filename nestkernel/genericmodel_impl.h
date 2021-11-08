/*
 *  genericmodel_impl.h
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

#ifndef GENERICMODEL_IMPL_H
#define GENERICMODEL_IMPL_H

#include "genericmodel.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "logging_manager.h"

namespace nest
{

template < typename ElementT >
void
GenericModel< ElementT >::deprecation_warning( const std::string& caller )
{
  if ( deprecation_warning_issued_ or deprecation_info_.empty() )
  {
    return;
  }

  if ( not deprecation_info_.empty() )
  {
    LOG( M_DEPRECATED, caller, "Model " + get_name() + " is deprecated in " + deprecation_info_ + "." );
  }

  deprecation_warning_issued_ = true;
}
}
#endif
