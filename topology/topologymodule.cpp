/*
 *  topologymodule.cpp
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

#include "topologymodule.h"

// Generated includes:
#include "config.h"

// Includes from nestkernel:
#include "genericmodel.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dictdatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "iostreamdatum.h"

// Includes from topology:
#include "connection_creator_impl.h"
#include "free_layer.h"
#include "grid_layer.h"
#include "grid_mask.h"
#include "layer.h"
#include "layer_impl.h"
#include "mask.h"
#include "mask_impl.h"
#include "topology.h"


namespace nest
{
SLIType TopologyModule::MaskType;

TopologyModule::TopologyModule()
{
  MaskType.settypename( "masktype" );
  MaskType.setdefaultaction( SLIInterpreter::datatypefunction );
}

TopologyModule::~TopologyModule()
{
}

const std::string
TopologyModule::name( void ) const
{
  return std::string( "TopologyModule" ); // Return name of the module
}

GenericFactory< AbstractMask >&
TopologyModule::mask_factory_( void )
{
  static GenericFactory< AbstractMask > factory;
  return factory;
}

MaskDatum
TopologyModule::create_mask( const Token& t )
{
  // t can be either an existing MaskDatum, or a Dictionary containing
  // mask parameters
  MaskDatum* maskd = dynamic_cast< MaskDatum* >( t.datum() );
  if ( maskd )
  {
    return *maskd;
  }
  else
  {

    DictionaryDatum* dd = dynamic_cast< DictionaryDatum* >( t.datum() );
    if ( dd == 0 )
    {
      throw BadProperty( "Mask must be masktype or dictionary." );
    }

    // The dictionary should contain one key which is the name of the
    // mask type, and optionally the key 'anchor'. To find the unknown
    // mask type key, we must loop through all keys. The value for the
    // anchor key will be stored in the anchor_token variable.
    Token anchor_token;
    bool has_anchor = false;
    AbstractMask* mask = 0;

    for ( Dictionary::iterator dit = ( *dd )->begin(); dit != ( *dd )->end(); ++dit )
    {

      if ( dit->first == names::anchor )
      {

        anchor_token = dit->second;
        has_anchor = true;
      }
      else
      {

        if ( mask != 0 )
        { // mask has already been defined
          throw BadProperty( "Mask definition dictionary contains extraneous items." );
        }
        mask = create_mask( dit->first, getValue< DictionaryDatum >( dit->second ) );
      }
    }

    if ( has_anchor )
    {

      // The anchor may be an array of doubles (a spatial position).
      // For grid layers only, it is also possible to provide an array of longs.
      try
      {

        std::vector< double > anchor = getValue< std::vector< double > >( anchor_token );
        AbstractMask* amask;

        switch ( anchor.size() )
        {
        case 2:
          amask = new AnchoredMask< 2 >( dynamic_cast< Mask< 2 >& >( *mask ), anchor );
          break;
        case 3:
          amask = new AnchoredMask< 3 >( dynamic_cast< Mask< 3 >& >( *mask ), anchor );
          break;
        default:
          throw BadProperty( "Anchor must be 2- or 3-dimensional." );
        }

        delete mask;
        mask = amask;
      }
      catch ( TypeMismatch& e )
      {
        std::vector< long > anchor = getValue< std::vector< long > >( anchor_token );

        switch ( anchor.size() )
        {
        case 2:
          try
          {
            GridMask< 2 >& grid_mask_2d = dynamic_cast< GridMask< 2 >& >( *mask );
            grid_mask_2d.set_anchor( Position< 2, int >( anchor[ 0 ], anchor[ 1 ] ) );
          }
          catch ( std::bad_cast& e )
          {
            throw BadProperty( "Mask must be 2-dimensional grid mask." );
          }
          break;
        case 3:
          try
          {
            GridMask< 3 >& grid_mask_3d = dynamic_cast< GridMask< 3 >& >( *mask );
            grid_mask_3d.set_anchor( Position< 3, int >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
          }
          catch ( std::bad_cast& e )
          {
            throw BadProperty( "Mask must be 3-dimensional grid mask." );
          }
          break;
        }
      }
    }

    return mask;
  }
}

static AbstractMask*
create_doughnut( const DictionaryDatum& d )
{
  // The doughnut (actually an annulus) is created using a DifferenceMask
  Position< 2 > center( 0, 0 );
  if ( d->known( names::anchor ) )
  {
    center = getValue< std::vector< double > >( d, names::anchor );
  }

  const double outer = getValue< double >( d, names::outer_radius );
  const double inner = getValue< double >( d, names::inner_radius );
  if ( inner >= outer )
  {
    throw BadProperty(
      "topology::create_doughnut: "
      "inner_radius < outer_radius required." );
  }

  BallMask< 2 > outer_circle( center, outer );
  BallMask< 2 > inner_circle( center, inner );

  return new DifferenceMask< 2 >( outer_circle, inner_circle );
}

void
TopologyModule::init( SLIInterpreter* i )
{
  // Register the topology functions as SLI commands.

  i->createcommand( "CreateLayer_D_D", &createlayer_D_Dfunction );

  i->createcommand( "GetPosition_g", &getposition_gfunction );

  i->createcommand( "Displacement_g_g", &displacement_g_gfunction );

  i->createcommand( "Displacement_a_g", &displacement_a_gfunction );

  i->createcommand( "Distance_g_g", &distance_g_gfunction );

  i->createcommand( "Distance_a_g", &distance_a_gfunction );

  i->createcommand( "CreateMask_D", &createmask_Dfunction );

  i->createcommand( "Inside_a_M", &inside_a_Mfunction );

  i->createcommand( "and_M_M", &and_M_Mfunction );

  i->createcommand( "or_M_M", &or_M_Mfunction );

  i->createcommand( "sub_M_M", &sub_M_Mfunction );

  i->createcommand( "ConnectLayers_g_g_D", &connectlayers_g_g_Dfunction );

  i->createcommand( "GetLayerStatus_g", &getlayerstatus_gfunction );

  i->createcommand( "DumpLayerNodes_os_g", &dumplayernodes_os_gfunction );

  i->createcommand( "DumpLayerConnections_os_g_g_l", &dumplayerconnections_os_g_g_lfunction );

  i->createcommand( "cvdict_M", &cvdict_Mfunction );

  i->createcommand( "SelectNodesByMask_g_a_M", &selectnodesbymask_g_a_Mfunction );

  // Register mask types
  register_mask< BallMask< 2 > >();
  register_mask< BallMask< 3 > >();
  register_mask< EllipseMask< 2 > >();
  register_mask< EllipseMask< 3 > >();
  register_mask< BoxMask< 2 > >();
  register_mask< BoxMask< 3 > >();
  register_mask< BoxMask< 3 > >( "volume" ); // For compatibility with topo 2.0
  register_mask( "doughnut", create_doughnut );
  register_mask< GridMask< 2 > >();
}

/** @BeginDocumentation
  Name: topology::CreateLayer - create a spatial layer of nodes

  Synopsis:
  dict CreateLayer -> layer

  Parameters:
  dict - dictionary with layer specification

  Description: The Topology module organizes neuronal networks in
  layers. A layer is a special type of subnet which contains information
  about the spatial position of its nodes. There are three classes of
  layers: grid-based layers, in which each element is placed at a
  location in a regular grid; free layers, in which elements can be
  placed arbitrarily in space; and random layers, where the elements are
  distributed randomly throughout a region in space.  Which kind of layer
  this command creates depends on the elements in the supplied
  specification dictionary.

  Author: Håkon Enger, Kittel Austvoll
*/
void
TopologyModule::CreateLayer_D_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  DictionaryDatum layer_dict = getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  DictionaryDatum params = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  NodeCollectionDatum layer = create_layer( layer_dict );

  for ( auto&& node_id_triple : *layer )
  {
    set_node_status( node_id_triple.node_id, params );
  }

  i->OStack.pop( 2 );
  i->OStack.push( layer );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::GetPosition - retrieve position of input node

  Synopsis: NodeCollection GetPosition -> [array]

  Parameters:
  layer      - NodeCollection for layer with layer nodes

  Returns:
  [array]    - spatial position of node [x y]

  Description: Retrieves spatial 2D position of layer node(s).

  Examples:

  %%Create layer
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> /dictionary Set

  dictionary CreateLayer /src Set

  src [4] Take GetPosition

  Author: Kittel Austvoll
*/

void
TopologyModule::GetPosition_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  ArrayDatum result = get_position( layer );

  i->OStack.pop( 1 );
  if ( layer->size() == 1 )
  {
    i->OStack.push( result[ 0 ] );
  }
  else
  {
    i->OStack.push( result );
  }
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::Displacement - compute displacement vector

  Synopsis: layer from_node_id to_node_id Displacement -> [double vector]
            layer from_pos to_node_id Displacement -> [double vector]

  Parameters:
  layer       - NodeCollection for layer
  from_node_id    - int, node_id of node in a topology layer
  from_pos    - double vector, position in layer
  to_node_id      - int, node_id of node in a topology layer

  Returns:
  [double vector] - vector pointing from position "from" to position "to"

  Description:
  This function returns a vector connecting the position of the "from_node_id"
  node or the explicitly given "from_pos" position and the position of the
  "to_node_id" node. Nodes must be parts of topology layers.

  The "from" position is projected into the layer of the "to_node_id" node. If
  this layer has periodic boundary conditions (EdgeWrap is true), then the
  shortest displacement vector is returned, taking into account the
  periodicity. Fixed grid layers are in this case extended so that the
  nodes at the edges of the layer have a distance of one grid unit when
  wrapped.

  Example:

  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> CreateLayer
  /layer Set

  layer [4] Take layer [5] Take Displacement
  [[0.2 0.3]] layer [5] Take Displacement

  Author: Håkon Enger, Hans E Plesser, Kittel Austvoll

  See also: Distance, GetPosition
*/
void
TopologyModule::Displacement_g_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer_to = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  const NodeCollectionDatum layer_from = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );

  if ( layer_to->size() != 1 and layer_from->size() != 1 and not( layer_to->size() == layer_from->size() ) )
  {
    throw BadProperty( "NodeCollections must have equal length or one must have size 1." );
  }

  ArrayDatum result = displacement( layer_to, layer_from );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
TopologyModule::Displacement_a_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  const ArrayDatum point = getValue< ArrayDatum >( i->OStack.pick( 1 ) );

  ArrayDatum result = displacement( layer, point );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::Distance - compute distance between nodes

  Synopsis: layer from_node_id to_node_id Distance -> double
            layer from_pos to_node_id Distance -> double

  Parameters:
  layer       - NodeCollection for layer
  from_node_id    - int, node_id of node in a topology layer
  from_pos    - double vector, position in layer
  to_node_id      - int, node_id of node in a topology layer

  Returns:
  double - distance between nodes or given position and node

  Description:
  This function returns the distance between the position of the "from_node_id"
  node or the explicitly given "from_pos" position and the position of the
  "to_node_id" node. Nodes must be parts of topology layers.

  The "from" position is projected into the layer of the "to_node_id" node. If
  this layer has periodic boundary conditions (EdgeWrap is true), then the
  shortest distance is returned, taking into account the
  periodicity. Fixed grid layers are in this case extended so that the
  nodes at the edges of the layer have a distance of one grid unit when
  wrapped.

  Example:

  /layer
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> CreateLayer def

  layer [4] Take layer [5] Take Distance
  [[ 0.2 0.3 ]] layer [5] Take Distance

  Author: Hans E Plesser, Kittel Austvoll

  See also: Displacement, GetPosition
*/
void
TopologyModule::Distance_g_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer_to = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  const NodeCollectionDatum layer_from = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );

  if ( layer_to->size() != 1 and layer_from->size() != 1 and not( layer_to->size() == layer_from->size() ) )
  {
    throw BadProperty( "NodeCollections must have equal length or one must have size 1." );
  }

  Token result = distance( layer_to, layer_from );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

void
TopologyModule::Distance_a_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  const ArrayDatum point = getValue< ArrayDatum >( i->OStack.pick( 1 ) );

  Token result = distance( layer, point );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::CreateMask - create a spatial mask

  Synopsis:
  << /type dict >> CreateMask -> mask

  Parameters:
  /type - mask type
  dict  - dictionary with mask specifications

  Description: Masks are used when creating connections in the Topology
  module. A mask describes which area of the pool layer shall be searched
  for nodes to connect for any given node in the driver layer. This
  command creates a mask object which may be combined with other mask
  objects using Boolean operators. The mask is specified in a dictionary.

  Author: Håkon Enger
*/
void
TopologyModule::CreateMask_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const DictionaryDatum mask_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  MaskDatum datum = nest::create_mask( mask_dict );

  i->OStack.pop( 1 );
  i->OStack.push( datum );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::Inside - test if a point is inside a mask

  Synopsis:
  point mask Inside -> bool

  Parameters:
  point - array of coordinates
  mask - mask object

  Returns:
  bool - true if the point is inside the mask
*/
void
TopologyModule::Inside_a_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::vector< double > point = getValue< std::vector< double > >( i->OStack.pick( 1 ) );
  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  bool ret = inside( point, mask );

  i->OStack.pop( 2 );
  i->OStack.push( Token( BoolDatum( ret ) ) );
  i->EStack.pop();
}

void
TopologyModule::And_M_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  MaskDatum mask1 = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  MaskDatum mask2 = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  MaskDatum newmask = intersect_mask( mask1, mask2 );

  i->OStack.pop( 2 );
  i->OStack.push( newmask );
  i->EStack.pop();
}

void
TopologyModule::Or_M_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  MaskDatum mask1 = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  MaskDatum mask2 = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  MaskDatum newmask = union_mask( mask1, mask2 );

  i->OStack.pop( 2 );
  i->OStack.push( newmask );
  i->EStack.pop();
}

void
TopologyModule::Sub_M_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  MaskDatum mask1 = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  MaskDatum mask2 = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  MaskDatum newmask = minus_mask( mask1, mask2 );

  i->OStack.pop( 2 );
  i->OStack.push( newmask );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::ConnectLayers - connect two layers

  Synopsis: sourcelayer targetlayer connection_dict
  ConnectLayers -> -

  Description: Connects nodes in two topological layers.

  The parameters set in the input dictionary decides the nature
  of the connection pattern being created. Please see parameter
  list below for a detailed description of these variables.

  The connections are created by iterating through either the
  source or the target layer, consecutively connecting each node
  to a region in the opposing layer.

  Parameters:
  sourcelayer  - NodeCollection for source layer
  targetlayer  - NodeCollection for target layer

  connection_dict - dictionary containing any of the following
                    elements:

  ------------------------------------------------------------------
  Connection dictionary parameters:
  ------------------------------------------------------------------
  Parameter name: connection-type

  Type: string

  Parameter description:

  Decides the type of connection pattern being created (i.e.
  convergent or divergent topological connection). A convergent
  topological connection is a connection between a source region
  and a target node. A divergent topological connection is a
  connection between a source node and a target region. A convergent
  topological connection can also be called a receptive field connection.
  A divergent topological connection can also be called a projective
  field connection. A one-to-one connection can be created by setting
  the size of the source or target region equal to one. The connection
  type has particular effect on the connection pattern when used together
  with the number_of_connections variable.


  Parameter name: mask

  Type: dictionary

  Parameter description:

  The mask defines the region used in the connection type described
  above. There exists a selection of many different region sizes and
  shapes. Examples are the grid region, the rectangular, circular or
  doughnut region.

  The grid region takes an optional anchor parameter. The anchor
  parameter indicates which node of the grid region is aligned with
  the source node.


  Parameter name: weights, delays and kernel

  Type: dictionary

  Parameter description:

  These parameters can be initialised in many ways. Either as a constant
  value, with the help of a dictionary, or in an array (only for fixed
  grid layers). The dictionary can be of type gaussian, 2D gaussian,
  linear, exponential and other.


  Parameter name: source

  Type: dictionary

  Parameter description:

  The source dictionary enables us to give further detail on
  how the nodes in the source layer used in the connection function
  should be processed.

  Parameters:
  model*             literal
  lid^               integer

  *modeltype (i.e. /iaf_psc_alpha) of nodes that should be connected to
  in the layer. All nodes are used if this variable isn't set.
  ^Nesting depth of nodes that should be connected to. All layers are used
  if this variable isn't set.


  Parameter name: target

  Type: dictionary

  Parameter description:

  See description for source dictionary.


  Parameter name: number_of_connections

  Type: integer

  Parameter description:

  Maximum number of connections that each iterating node is allowed.
  The actual connections being created are picked at random from all
  the candidate connections.


      Parameter name: synapse_model

      Type: literal

      Parameter description:

      The synapse model to be used for creating the connection.
.
  Parameter name: allow_autapses

  Type: bool

  Parameter description: Used together with the number_of_connections option to
  indicate if autapses are allowed.


  Parameter name: allow_multapses

  Type: bool

  Parameter description: Used together with the number_of_connections option to
  indicate if multapses are allowed.

  ------------------------------------------------------------------

  Example:

  %Create source layer with CreateLayer
  << /rows 15
     /columns 43
     /extent [1.0 2.0]
     /elements /iaf_psc_alpha
  >> /src_dictionary Set

  src_dictionary CreateLayer /src Set

  %Create target layer with CreateLayer
  %%Create layer
  << /rows 34
     /columns 71
     /extent [3.0 1.0]
     /elements /iaf_psc_alpha
  >> /tgt_dictionary Set

  tgt_dictionary CreateLayer /tgt Set

  <<  /connection_type (convergent)
      /mask << /grid << /rows 2 /columns 3 >>
               /anchor << /row 4 /column 2 >> >>
      /weight 2.3
      /delay [2.3 1.2 3.2 1.3 2.3 1.2]
      /kernel << /gaussian << /sigma 1.2 /p_center 1.41 >> >>
      /synapse_model /stdp_synapse

  >> /parameters Set

  src tgt parameters ConnectLayers

  Author: Håkon Enger, Kittel Austvoll

  SeeAlso: topology::CreateLayer
*/
void
TopologyModule::ConnectLayers_g_g_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  const NodeCollectionDatum source = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  const NodeCollectionDatum target = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const DictionaryDatum connection_dict = getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  connect_layers( source, target, connection_dict );

  i->OStack.pop( 3 );
  i->EStack.pop();
}

/** @BeginDocumentation

  Name: topology::GetLayerStatus - return information about layer

  Synopsis:
  layer GetLayerStatus -> dict

  Parameters:
  layer - NodeCollection representing layer

  Returns:
  Status dictionary with information about layer
 */
void
TopologyModule::GetLayerStatus_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );

  DictionaryDatum result = get_layer_status( layer );

  i->OStack.pop( 1 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::DumpLayerNodes - write information about layer nodes to file

  Synopsis: ostream layer DumpLayerNodes -> ostream

  Parameters:
  ostream - open output stream
  layer   - NodeCollection for layer

  Description:
  Write information about each element in the given layer to the
  output stream. The file format is one line per element with the
  following contents:

  node ID x-position y-position [z-position]

  X and y position are given as physical coordinates in the extent,
  not as grid positions. The number of decimals can be controlled by
  calling setprecision on the output stream before calling DumpLayerNodes.

  Remarks:
  In distributed simulations, this function should only be called for
  MPI rank 0. If you call it on several MPI ranks, you must use a
  different file name on each.

  Examples:

  topology using
  /my_layer << /rows 5 /columns 4 /elements /iaf_psc_alpha >> CreateLayer def

  (my_layer_dump.lyr) (w) file
  my_layer DumpLayerNodes
  close

  Author: Kittel Austvoll, Hans Ekkehard Plesser

  SeeAlso: topology::DumpLayerConnections, setprecision, modeldict
*/
void
TopologyModule::DumpLayerNodes_os_gFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const NodeCollectionDatum layer = getValue< NodeCollectionDatum >( i->OStack.pick( 0 ) );
  OstreamDatum out = getValue< OstreamDatum >( i->OStack.pick( 1 ) );

  dump_layer_nodes( layer, out );

  i->OStack.pop( 1 ); // leave ostream on stack
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::DumpLayerConnections - prints a list of the connections of the
                                         nodes in the layer to file

  Synopsis: ostream source_layer synapse_model DumpLayerConnections ->
                                                                         ostream

  Parameters:
  ostream          - open outputstream
  source_layer     - NodeCollection for layer
  synapse_model    - synapse model (literal)

  Description:
  Dumps information about all connections of the given type having their source
  in the given layer to the given output stream. The data format is one line per
  connection as follows:

  source_node_id target_node_id weight delay displacement[x,y,z]

  where displacement are up to three coordinates of the vector from the source
  to the target node. If targets do not have positions (eg. spike detectors
  outside any layer), NaN is written for each displacement coordinate.

  Remarks:
  For distributed simulations
  - this function will dump the connections with local targets only.
  - the user is responsible for writing to a different output stream (file)
    on each MPI process.

  Examples:

  topology using
  ...
  (out.cnn) (w) file layer_node_id /static_synapse PrintLayerConnections close

  Author: Kittel Austvoll, Hans Ekkehard Plesser

  SeeAlso: topology::DumpLayerNodes
*/

void
TopologyModule::DumpLayerConnections_os_g_g_lFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 4 );

  OstreamDatum out_file = getValue< OstreamDatum >( i->OStack.pick( 3 ) );
  const NodeCollectionDatum source_layer = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  const NodeCollectionDatum target_layer = getValue< NodeCollectionDatum >( i->OStack.pick( 1 ) );
  const Token syn_model = i->OStack.pick( 0 );

  dump_layer_connections( syn_model, source_layer, target_layer, out_file );

  i->OStack.pop( 3 ); // leave ostream on stack
  i->EStack.pop();
}

void
TopologyModule::Cvdict_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );
  DictionaryDatum dict = mask->get_dict();

  i->OStack.pop();
  i->OStack.push( dict );
  i->EStack.pop();
}


void
TopologyModule::SelectNodesByMask_g_a_MFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  const NodeCollectionDatum layer_nc = getValue< NodeCollectionDatum >( i->OStack.pick( 2 ) );
  std::vector< double > anchor = getValue< std::vector< double > >( i->OStack.pick( 1 ) );
  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  std::vector< index > mask_node_ids;

  const int dim = anchor.size();

  if ( dim != 2 and dim != 3 )
  {
    throw BadProperty( "Center must be 2- or 3-dimensional." );
  }

  AbstractLayerPTR abstract_layer = get_layer( layer_nc );

  if ( dim == 2 )
  {
    Layer< 2 >* layer = dynamic_cast< Layer< 2 >* >( abstract_layer.get() );
    if ( not layer )
    {
      throw TypeMismatch( "2D layer", "other type" );
    }

    MaskedLayer< 2 > ml = MaskedLayer< 2 >( *layer, mask, false );

    for ( Ntree< 2, index >::masked_iterator it = ml.begin( Position< 2 >( anchor[ 0 ], anchor[ 1 ] ) ); it != ml.end();
          ++it )
    {
      mask_node_ids.push_back( it->second );
    }
  }
  else
  {
    Layer< 3 >* layer = dynamic_cast< Layer< 3 >* >( abstract_layer.get() );
    if ( not layer )
    {
      throw TypeMismatch( "3D layer", "other type" );
    }

    MaskedLayer< 3 > ml = MaskedLayer< 3 >( *layer, mask, false );

    for ( Ntree< 3, index >::masked_iterator it = ml.begin( Position< 3 >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
          it != ml.end();
          ++it )
    {
      mask_node_ids.push_back( it->second );
    }
  }

  i->OStack.pop( 3 );
  i->OStack.push( mask_node_ids );
  i->EStack.pop();
}


std::string
LayerExpected::message() const
{
  return std::string();
}

std::string
LayerNodeExpected::message() const
{
  return std::string();
}


} // namespace nest
