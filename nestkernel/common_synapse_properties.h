/*
 *  common_synapse_properties.h
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

#ifndef COMMON_SYNAPSE_PROPERTIES_H
#define COMMON_SYNAPSE_PROPERTIES_H

// Includes from sli:
#include "dictdatum.h"


namespace nest
{

// forward declarations
class weight_recorder;
class TimeConverter;
class ConnectorModel;

/**
 * Class containing the common properties for all connections of a certain type.
 *
 * Everything that needs to be stored commonly for all synapses goes into a
 * CommonProperty class derived from this base class.
 * Base class for all CommonProperty classes.
 * If the synapse type does not have any common properties, this class may be
 * used as a placeholder.
 */
class CommonSynapseProperties
{
public:
  /**
   * Standard constructor. Sets all common properties to default values.
   * Default implementation of an empty CommonSynapseProperties object.
   */
  CommonSynapseProperties();

  ~CommonSynapseProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );


  /**
   * Calibrate all time objects, which might be contained in this object.
   */
  void calibrate( const TimeConverter& );

  /**
   * Get node ID of volume transmitter
   */
  long get_vt_node_id() const;

  /**
   * Get weight_recorder
   */
  weight_recorder* get_weight_recorder() const;


private:
  weight_recorder* weight_recorder_;
};


} // of namespace nest

#endif
