/*
 *  topology.h
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

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dictdatum.h"
#include "iostreamdatum.h"
#include "token.h"

// Includes from topology:
#include "mask.h"
#include "topology_parameter.h"


namespace nest
{
index create_layer( const DictionaryDatum& layer_dict );
std::vector< double > get_position( const index node_gid );
std::vector< double > displacement( const std::vector< double >& point, const index node_gid );
double distance( const std::vector< double >& point, const index node_gid );
MaskDatum create_mask( const DictionaryDatum& mask_dict );
BoolDatum inside( const std::vector< double >& point, const MaskDatum& mask );
MaskDatum intersect_mask( const MaskDatum& mask1, const MaskDatum& mask2 );
MaskDatum union_mask( const MaskDatum& mask1, const MaskDatum& mask2 );
MaskDatum minus_mask( const MaskDatum& mask1, const MaskDatum& mask2 );
ParameterDatum multiply_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum divide_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum add_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ParameterDatum subtract_parameter( const ParameterDatum& param1, const ParameterDatum& param2 );
ArrayDatum get_global_children( const index gid, const MaskDatum& maskd, const std::vector< double >& anchor );
void connect_layers( const index source_gid, const index target_gid, const DictionaryDatum& dict );
ParameterDatum create_parameter( const DictionaryDatum& param_dict );
double get_value( const std::vector< double >& point, const ParameterDatum& param );
void dump_layer_nodes( const index layer_gid, OstreamDatum& out );
void dump_layer_connections( const Token& syn_model, const index layer_gid, OstreamDatum& out_file );
std::vector< index > get_element( const index layer_gid, const TokenArray array );
}

#endif /* TOPOLOGY_H */
