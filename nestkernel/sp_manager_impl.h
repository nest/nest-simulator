/*
 *  sp_manager_impl.h
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

#ifndef SP_MANAGER_IMPL_H
#define SP_MANAGER_IMPL_H

#include "sp_manager.h"

// C++ includes:
#include <string>

// Includes from nestkernel:
#include "growth_curve.h"
#include "growth_curve_factory.h"

namespace nest
{

template < typename GrowthCurve >
void
SPManager::register_growth_curve( const std::string& name )
{
  assert( not growthcurvedict_->known( name ) );
  GenericGrowthCurveFactory* nc = new GrowthCurveFactory< GrowthCurve >();
  assert( nc != 0 );
  const int id = growthcurve_factories_.size();
  growthcurve_factories_.push_back( nc );
  growthcurvedict_->insert( name, id );
}

} // namespace nest

#endif /* SP_MANAGER_IMPL_H */
