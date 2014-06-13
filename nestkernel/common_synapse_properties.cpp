/*
 *  common_synapse_properties.cpp
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

#include "nest.h"
#include "dictdatum.h"
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "nest_timeconverter.h"
#include "node.h"

namespace nest {

/**
 * Default implementation of an empty CommonSynapseProperties object.
 */

  CommonSynapseProperties::CommonSynapseProperties() {}

  CommonSynapseProperties::~CommonSynapseProperties() {}

  void CommonSynapseProperties::get_status(DictionaryDatum &) const {}
  
  void CommonSynapseProperties::set_status(const DictionaryDatum &, ConnectorModel&) {}
  
  Node* CommonSynapseProperties::get_node() {return 0;}
 
  void CommonSynapseProperties::calibrate(const TimeConverter &) {}
 
} // namespace nest
