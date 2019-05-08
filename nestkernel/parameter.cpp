/*
 *  parameter.cpp
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

#include "gid_collection.h"
#include "node.h"
#include "topology.h"

// includes from sli
#include "lockptrdatum_impl.h"

#include "parameter.h"

template class lockPTRDatum< nest::Parameter,
  &nest::NestModule::ParameterType >;

namespace nest
{
double
NodePosParameter::value( librandom::RngPtr& rng, Node* node ) const
{
  if ( not node )
  {
    throw KernelException( "NodePosParameter: not node" );
  }
  GIDCollectionPTR gc = node->get_gc();
  if ( not gc.valid() )
  {
    throw KernelException( "NodePosParameter: not gc" );
  }
  GIDCollectionMetadataPTR meta = gc->get_metadata();
  if ( not meta.valid() )
  {
    throw KernelException( "NodePosParameter: not meta" );
  }
  LayerMetadata const* const layer_meta =
    dynamic_cast< LayerMetadata const* >( meta.get() );
  meta.unlock();
  if ( not layer_meta )
  {
    throw KernelException( "NodePosParameter: not layer_meta" );
  }
  AbstractLayerPTR layer = layer_meta->get_layer();
  if ( not layer.valid() )
  {
    throw KernelException( "NodePosParameter: not valid layer" );
  }
  index lid = node->get_gid() - meta->get_first_gid();
  std::vector< double > pos = layer->get_position_vector( lid );
  return pos[ dimension_ ];
}
} /* namespace nest */
