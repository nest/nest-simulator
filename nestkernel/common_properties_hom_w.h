/*
 *  common_properties_hom_w.h
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

#ifndef COMMON_PROPERTIES_HOM_W_H
#define COMMON_PROPERTIES_HOM_W_H

/** @BeginDocumentation

   Name: static_synapse_hom_w - Static synapse type
         using homogeneous weight, i.e. all synapses
         feature the same w.

   FirstVersion: April 2008

   Author: Moritz Helias, Susanne Kunkel

   SeeAlso: synapsedict, static_synapse
*/

// Includes from nestkernel:
#include "common_synapse_properties.h"

namespace nest
{

/**
 * Class containing the common properties for all synapses with common weight.
 */
class CommonPropertiesHomW : public CommonSynapseProperties
{
public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  CommonPropertiesHomW()
    : CommonSynapseProperties()
    , weight_( 1.0 )
  {
  }

  /**
   * Get all properties and put them into a dictionary.
   */
  void
  get_status( DictionaryDatum& d ) const
  {
    CommonSynapseProperties::get_status( d );
    def< double >( d, names::weight, weight_ );
  }

  double
  get_weight() const
  {
    return weight_;
  }

  /**
   * Set properties from the values given in dictionary.
   */
  void
  set_status( const DictionaryDatum& d, ConnectorModel& cm )
  {
    CommonSynapseProperties::set_status( d, cm );
    updateValue< double >( d, names::weight, weight_ );
  }

private:
  // data members common to all connections
  double weight_;
};

} // namespace nest

#endif // COMMON_PROPERTIES_HOM_W_H
