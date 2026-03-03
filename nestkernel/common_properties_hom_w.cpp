/*
 *  common_properties_hom_w.cpp
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

#include "common_properties_hom_w.h"


namespace nest
{

CommonPropertiesHomW::CommonPropertiesHomW()
  : CommonSynapseProperties()
  , weight_( 1.0 )
{
}

void
CommonPropertiesHomW::get_status( Dictionary& d ) const
{
  CommonSynapseProperties::get_status( d );
  def< double >( d, names::weight, weight_ );
}

double
CommonPropertiesHomW::get_weight() const
{
  assert( not growthcurvedict_.known( name ) );
  GenericGrowthCurveFactory* nc = new GrowthCurveFactory< GrowthCurve >();
  assert( nc );
  const int id = growthcurve_factories_.size();
  growthcurve_factories_.push_back( nc );
  growthcurvedict_[ name ] = id;
}

void
CommonPropertiesHomW::set_status( const Dictionary& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );
  d.updateValue( names::weight, weight_ );
}

} // namespace nest
