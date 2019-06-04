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
#include "topology_parameter.h"


namespace nest
{
SLIType TopologyModule::MaskType;
SLIType TopologyModule::ParameterType;

TopologyModule::TopologyModule()
{
  MaskType.settypename( "masktype" );
  MaskType.setdefaultaction( SLIInterpreter::datatypefunction );
  ParameterType.settypename( "parametertype" );
  ParameterType.setdefaultaction( SLIInterpreter::datatypefunction );
}

TopologyModule::~TopologyModule()
{
}

const std::string
TopologyModule::name( void ) const
{
  return std::string( "TopologyModule" ); // Return name of the module
}

const std::string
TopologyModule::commandstring( void ) const
{
  return std::string( "(topology-interface) run" );
}

GenericFactory< AbstractMask >&
TopologyModule::mask_factory_( void )
{
  static GenericFactory< AbstractMask > factory;
  return factory;
}

GenericFactory< TopologyParameter >&
TopologyModule::parameter_factory_( void )
{
  static GenericFactory< TopologyParameter > factory;
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

    for ( Dictionary::iterator dit = ( *dd )->begin(); dit != ( *dd )->end();
          ++dit )
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
          throw BadProperty(
            "Mask definition dictionary contains extraneous items." );
        }
        mask =
          create_mask( dit->first, getValue< DictionaryDatum >( dit->second ) );
      }
    }

    if ( has_anchor )
    {

      // The anchor may be an array of doubles (a spatial position), or a
      // dictionary containing the keys 'column' and 'row' (for grid
      // masks only)
      try
      {

        std::vector< double > anchor =
          getValue< std::vector< double > >( anchor_token );
        AbstractMask* amask;

        switch ( anchor.size() )
        {
        case 2:
          amask = new AnchoredMask< 2 >(
            dynamic_cast< Mask< 2 >& >( *mask ), anchor );
          break;
        case 3:
          amask = new AnchoredMask< 3 >(
            dynamic_cast< Mask< 3 >& >( *mask ), anchor );
          break;
        default:
          throw BadProperty( "Anchor must be 2- or 3-dimensional." );
        }

        delete mask;
        mask = amask;
      }
      catch ( TypeMismatch& e )
      {

        DictionaryDatum ad = getValue< DictionaryDatum >( anchor_token );

        int dim = 2;
        int column = getValue< long >( ad, names::column );
        int row = getValue< long >( ad, names::row );
        int layer;
        if ( ad->known( names::layer ) )
        {
          layer = getValue< long >( ad, names::layer );
          dim = 3;
        }
        switch ( dim )
        {
        case 2:
          try
          {
            GridMask< 2 >& grid_mask_2d =
              dynamic_cast< GridMask< 2 >& >( *mask );
            grid_mask_2d.set_anchor( Position< 2, int >( column, row ) );
          }
          catch ( std::bad_cast& e )
          {
            throw BadProperty( "Mask must be 2-dimensional grid mask." );
          }
          break;
        case 3:
          try
          {
            GridMask< 3 >& grid_mask_3d =
              dynamic_cast< GridMask< 3 >& >( *mask );
            grid_mask_3d.set_anchor( Position< 3, int >( column, row, layer ) );
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

ParameterDatum
TopologyModule::create_parameter( const Token& t )
{
  // t can be an existing ParameterDatum, a DoubleDatum containing a
  // constant value for this parameter, or a Dictionary containing
  // parameters
  ParameterDatum* pd = dynamic_cast< ParameterDatum* >( t.datum() );
  if ( pd )
  {
    return *pd;
  }

  // If t is a DoubleDatum, create a ConstantParameter with this value
  DoubleDatum* dd = dynamic_cast< DoubleDatum* >( t.datum() );
  if ( dd )
  {
    return new ConstantParameter( *dd );
  }

  DictionaryDatum* dictd = dynamic_cast< DictionaryDatum* >( t.datum() );
  if ( dictd )
  {

    // The dictionary should only have a single key, which is the name of
    // the parameter type to create.
    if ( ( *dictd )->size() != 1 )
    {
      throw BadProperty(
        "Parameter definition dictionary must contain one single key only." );
    }

    Name n = ( *dictd )->begin()->first;
    DictionaryDatum pdict = getValue< DictionaryDatum >( *dictd, n );
    return create_parameter( n, pdict );
  }
  else
  {
    throw BadProperty(
      "Parameter must be parametertype, constant or dictionary." );
  }
}

TopologyParameter*
TopologyModule::create_parameter( const Name& name, const DictionaryDatum& d )
{
  // The parameter factory will create the parameter without regard for
  // the anchor
  TopologyParameter* param = parameter_factory_().create( name, d );

  // Wrap the parameter object created above in an AnchoredParameter if
  // the dictionary contains an anchor
  if ( d->known( names::anchor ) )
  {
    std::vector< double > anchor =
      getValue< std::vector< double > >( d, names::anchor );
    TopologyParameter* aparam;
    switch ( anchor.size() )
    {
    case 2:
      aparam = new AnchoredParameter< 2 >( *param, anchor );
      break;
    case 3:
      aparam = new AnchoredParameter< 3 >( *param, anchor );
      break;
    default:
      throw BadProperty( "Anchor must be 2- or 3-dimensional." );
    }

    delete param;
    param = aparam;
  }

  return param;
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

  i->createcommand( "CreateLayer_D", &createlayer_Dfunction );

  i->createcommand( "GetPosition_i", &getposition_ifunction );

  i->createcommand( "Displacement_a_i", &displacement_a_ifunction );

  i->createcommand( "Distance_a_i", &distance_a_ifunction );

  i->createcommand( "CreateMask_D", &createmask_Dfunction );

  i->createcommand( "Inside_a_M", &inside_a_Mfunction );

  i->createcommand( "and_M_M", &and_M_Mfunction );

  i->createcommand( "or_M_M", &or_M_Mfunction );

  i->createcommand( "sub_M_M", &sub_M_Mfunction );

  i->createcommand( "mul_P_P", &mul_P_Pfunction );

  i->createcommand( "div_P_P", &div_P_Pfunction );

  i->createcommand( "add_P_P", &add_P_Pfunction );

  i->createcommand( "sub_P_P", &sub_P_Pfunction );

  i->createcommand(
    "GetGlobalChildren_i_M_a", &getglobalchildren_i_M_afunction );

  i->createcommand( "ConnectLayers_i_i_D", &connectlayers_i_i_Dfunction );

  i->createcommand( "CreateParameter_D", &createparameter_Dfunction );

  i->createcommand( "GetValue_a_P", &getvalue_a_Pfunction );

  i->createcommand( "DumpLayerNodes_os_i", &dumplayernodes_os_ifunction );

  i->createcommand(
    "DumpLayerConnections_os_i_l", &dumplayerconnections_os_i_lfunction );

  i->createcommand( "GetElement_i_ia", &getelement_i_iafunction );

  i->createcommand( "cvdict_M", &cvdict_Mfunction );

  i->createcommand(
    "SelectNodesByMask_L_a_M", &selectnodesbymask_L_a_Mfunction );

  kernel().model_manager.register_node_model< FreeLayer< 2 > >(
    "topology_layer_free" );
  kernel().model_manager.register_node_model< FreeLayer< 3 > >(
    "topology_layer_free_3d" );
  kernel().model_manager.register_node_model< GridLayer< 2 > >(
    "topology_layer_grid" );
  kernel().model_manager.register_node_model< GridLayer< 3 > >(
    "topology_layer_grid_3d" );

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

  // Register parameter types
  register_parameter< ConstantParameter >( "constant" );
  register_parameter< LinearParameter >( "linear" );
  register_parameter< ExponentialParameter >( "exponential" );
  register_parameter< GaussianParameter >( "gaussian" );
  register_parameter< Gaussian2DParameter >( "gaussian2D" );
  register_parameter< GammaParameter >( "gamma" );
  register_parameter< UniformParameter >( "uniform" );
  register_parameter< NormalParameter >( "normal" );
  register_parameter< LognormalParameter >( "lognormal" );
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
TopologyModule::CreateLayer_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  DictionaryDatum layer_dict =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  index layernode = create_layer( layer_dict );

  i->OStack.pop( 1 );
  i->OStack.push( layernode );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::GetPosition - retrieve position of input node

  Synopsis: node_gid GetPosition -> [array]

  Parameters:
  node_gid      - gid of layer node
  [array]       - spatial position of node [x y]

  Description: Retrieves spatial 2D position of layer node.

  Examples:

  topology using

  %%Create layer
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> /dictionary Set

  dictionary CreateLayer /src Set

  4 GetPosition

  Author: Kittel Austvoll
*/

void
TopologyModule::GetPosition_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  index node_gid = getValue< long >( i->OStack.pick( 0 ) );

  Token result = get_position( node_gid );

  i->OStack.pop( 1 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::Displacement - compute displacement vector

  Synopsis: from_gid to_gid Displacement -> [double vector]
            from_pos to_gid Displacement -> [double vector]

  Parameters:
  from_gid    - int, gid of node in a topology layer
  from_pos    - double vector, position in layer
  to_gid      - int, gid of node in a topology layer

  Returns:
  [double vector] - vector pointing from position "from" to position "to"

  Description:
  This function returns a vector connecting the position of the "from_gid"
  node or the explicitly given "from_pos" position and the position of the
  "to_gid" node. Nodes must be parts of topology layers.

  The "from" position is projected into the layer of the "to_gid" node. If
  this layer has periodic boundary conditions (EdgeWrap is true), then the
  shortest displacement vector is returned, taking into account the
  periodicity. Fixed grid layers are in this case extended so that the
  nodes at the edges of the layer have a distance of one grid unit when
  wrapped.

  Example:

  topology using
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> CreateLayer ;

  4 5         Displacement
  [0.2 0.3] 5 Displacement

  Author: Håkon Enger, Hans E Plesser, Kittel Austvoll

  See also: Distance, GetPosition
*/
void
TopologyModule::Displacement_a_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::vector< double > point =
    getValue< std::vector< double > >( i->OStack.pick( 1 ) );

  index node_gid = getValue< long >( i->OStack.pick( 0 ) );

  Token result = displacement( point, node_gid );

  i->OStack.pop( 2 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::Distance - compute distance between nodes

  Synopsis: from_gid to_gid Distance -> double
            from_pos to_gid Distance -> double

  Parameters:
  from_gid    - int, gid of node in a topology layer
  from_pos    - double vector, position in layer
  to_gid      - int, gid of node in a topology layer

  Returns:
  double - distance between nodes or given position and node

  Description:
  This function returns the distance between the position of the "from_gid"
  node or the explicitly given "from_pos" position and the position of the
  "to_gid" node. Nodes must be parts of topology layers.

  The "from" position is projected into the layer of the "to_gid" node. If
  this layer has periodic boundary conditions (EdgeWrap is true), then the
  shortest distance is returned, taking into account the
  periodicity. Fixed grid layers are in this case extended so that the
  nodes at the edges of the layer have a distance of one grid unit when
  wrapped.

  Example:

  topology using
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> CreateLayer ;

  4 5         Distance
  [0.2 0.3] 5 Distance

  Author: Hans E Plesser, Kittel Austvoll

  See also: Displacement, GetPosition
*/
void
TopologyModule::Distance_a_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::vector< double > point =
    getValue< std::vector< double > >( i->OStack.pick( 1 ) );

  index node_gid = getValue< long >( i->OStack.pick( 0 ) );

  Token result = distance( point, node_gid );

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

  const DictionaryDatum mask_dict =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

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

  std::vector< double > point =
    getValue< std::vector< double > >( i->OStack.pick( 1 ) );
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

void
TopologyModule::Mul_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = multiply_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
TopologyModule::Div_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = divide_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
TopologyModule::Add_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = add_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}

void
TopologyModule::Sub_P_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  ParameterDatum param1 = getValue< ParameterDatum >( i->OStack.pick( 1 ) );
  ParameterDatum param2 = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  ParameterDatum newparam = subtract_parameter( param1, param2 );

  i->OStack.pop( 2 );
  i->OStack.push( newparam );
  i->EStack.pop();
}


void
TopologyModule::GetGlobalChildren_i_M_aFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  index gid = getValue< long >( i->OStack.pick( 2 ) );
  MaskDatum maskd = getValue< MaskDatum >( i->OStack.pick( 1 ) );
  std::vector< double > anchor =
    getValue< std::vector< double > >( i->OStack.pick( 0 ) );

  ArrayDatum result = get_global_children( gid, maskd, anchor );

  i->OStack.pop( 3 );
  i->OStack.push( result );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::ConnectLayers - connect two layers

  Synopsis: sourcelayergid targetlayergid connection_dict
  ConnectLayers -> -

  Description: Connects nodes in two topological layers.

  The parameters set in the input dictionary decides the nature
  of the connection pattern being created. Please see parameter
  list below for a detailed description of these variables.

  The connections are created by iterating through either the
  source or the target layer, consecutively connecting each node
  to a region in the opposing layer.

  Parameters:
  sourcelayergid  - GID of source layer
  targetlayergid  - GID of target layer

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

  topology using

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
     /elements {/iaf_psc_alpha Create ; /iaf_psc_alpha Create ;}
  >> /tgt_dictionary Set

  tgt_dictionary CreateLayer /tgt Set

  <<  /connection_type (convergent)
      /mask << /grid << /rows 2 /columns 3 >>
               /anchor << /row 4 /column 2 >> >>
      /weights 2.3
      /delays [2.3 1.2 3.2 1.3 2.3 1.2]
      /kernel << /gaussian << /sigma 1.2 /p_center 1.41 >> >>
      /sources << /model /iaf_psc_alpha
                  /lid 1 >>
      /targets << /model /iaf_psc_alpha
                  /lid 2 >>
      /synapse_model /stdp_synapse

  >> /parameters Set

  src tgt parameters ConnectLayers

  Author: Håkon Enger, Kittel Austvoll

  SeeAlso: topology::CreateLayer
*/
void
TopologyModule::ConnectLayers_i_i_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  index source_gid = getValue< long >( i->OStack.pick( 2 ) );
  index target_gid = getValue< long >( i->OStack.pick( 1 ) );
  const DictionaryDatum connection_dict =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  connect_layers( source_gid, target_gid, connection_dict );

  i->OStack.pop( 3 );
  i->EStack.pop();
}


/** @BeginDocumentation
  Name: topology::CreateParameter - create a spatial function

  Synopsis:
  << /type dict >> CreateParameter -> parameter

  Parameters:
  /type - parameter type
  dict  - dictionary with parameter specifications

  Description: Parameters are spatial functions which are used when
  creating connections in the Topology module. A parameter may be used as
  a probability kernel when creating connections or as synaptic
  parameters (such as weight and delay). This command creates a parameter
  object which may be combined with other parameter objects using
  arithmetic operators. The parameter is specified in a dictionary.

  Author: Håkon Enger
*/
void
TopologyModule::CreateParameter_DFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );
  const DictionaryDatum param_dict =
    getValue< DictionaryDatum >( i->OStack.pick( 0 ) );

  ParameterDatum datum = nest::create_parameter( param_dict );

  i->OStack.pop( 1 );
  i->OStack.push( datum );
  i->EStack.pop();
}


/** @BeginDocumentation
  Name: topology::GetValue - compute value of parameter at a point

  Synopsis:
  point param GetValue -> value

  Parameters:
  point - array of coordinates
  param - parameter object

  Returns:
  value - the value of the parameter at the point.

  Author: Håkon Enger
*/
void
TopologyModule::GetValue_a_PFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  std::vector< double > point =
    getValue< std::vector< double > >( i->OStack.pick( 1 ) );
  ParameterDatum param = getValue< ParameterDatum >( i->OStack.pick( 0 ) );

  double value = get_value( point, param );

  i->OStack.pop( 2 );
  i->OStack.push( value );
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::DumpLayerNodes - write information about layer nodes to file

  Synopsis: ostream layer_gid DumpLayerNodes -> ostream

  Parameters:
  ostream   - open output stream
  layer_gid - topology layer

  Description:
  Write information about each element in the given layer to the
  output stream. The file format is one line per element with the
  following contents:

  GID x-position y-position [z-position]

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
TopologyModule::DumpLayerNodes_os_iFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const index layer_gid = getValue< long >( i->OStack.pick( 0 ) );
  OstreamDatum out = getValue< OstreamDatum >( i->OStack.pick( 1 ) );

  dump_layer_nodes( layer_gid, out );

  i->OStack.pop( 1 ); // leave ostream on stack
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::DumpLayerConnections - prints a list of the connections of the
                                         nodes in the layer to file

  Synopsis: ostream source_layer_gid synapse_model DumpLayerConnections ->
                                                                         ostream

  Parameters:
  ostream          - open outputstream
  source_layer_gid - topology layer
  synapse_model    - synapse model (literal)

  Description:
  Dumps information about all connections of the given type having their source
  in the given layer to the given output stream. The data format is one line per
  connection as follows:

  source_gid target_gid weight delay displacement[x,y,z]

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
  (out.cnn) (w) file layer_gid /static_synapse PrintLayerConnections close

  Author: Kittel Austvoll, Hans Ekkehard Plesser

  SeeAlso: topology::DumpLayerNodes
*/

void
TopologyModule::DumpLayerConnections_os_i_lFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  OstreamDatum out_file = getValue< OstreamDatum >( i->OStack.pick( 2 ) );

  const index layer_gid = getValue< long >( i->OStack.pick( 1 ) );

  const Token syn_model = i->OStack.pick( 0 );

  dump_layer_connections( syn_model, layer_gid, out_file );

  i->OStack.pop( 2 ); // leave ostream on stack
  i->EStack.pop();
}

/** @BeginDocumentation
  Name: topology::GetElement - return node GID at specified layer position

  Synopsis: layer_gid [array] GetElement -> node_gid

  Parameters:
  layer_gid     - topological layer
  [array]       - position of node
  node_gid      - node GID

  Description: Retrieves node at the layer grid position
  given in [array]. [array] is on the format [column row].
  The layer must be of grid type. Returns an array of GIDs
  if there are several nodes per grid point.

  Examples:

  topology using

  %%Create layer
  << /rows 5
     /columns 4
     /elements /iaf_psc_alpha
  >> /dictionary Set

  dictionary CreateLayer /src Set

  src [2 3] GetElement

  Author: Kittel Austvoll, Håkon Enger
*/
void
TopologyModule::GetElement_i_iaFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  const index layer_gid = getValue< long >( i->OStack.pick( 1 ) );
  TokenArray array = getValue< TokenArray >( i->OStack.pick( 0 ) );

  std::vector< index > node_gids = get_element( layer_gid, array );

  i->OStack.pop( 2 );

  // For compatibility reasons, return either single node or array
  if ( node_gids.size() == 1 )
  {
    i->OStack.push( node_gids[ 0 ] );
  }
  else
  {
    i->OStack.push( node_gids );
  }

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
TopologyModule::SelectNodesByMask_L_a_MFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  const index& layer_gid = getValue< long >( i->OStack.pick( 2 ) );
  std::vector< double > anchor =
    getValue< std::vector< double > >( i->OStack.pick( 1 ) );
  MaskDatum mask = getValue< MaskDatum >( i->OStack.pick( 0 ) );

  std::vector< index > mask_gids;

  const int dim = anchor.size();

  if ( dim != 2 and dim != 3 )
  {
    throw BadProperty( "Center must be 2- or 3-dimensional." );
  }

  if ( dim == 2 )
  {
    Layer< 2 >* layer = dynamic_cast< Layer< 2 >* >(
      kernel().node_manager.get_node( layer_gid ) );

    MaskedLayer< 2 > ml =
      MaskedLayer< 2 >( *layer, Selector(), mask, true, false );

    for ( Ntree< 2, index >::masked_iterator it =
            ml.begin( Position< 2 >( anchor[ 0 ], anchor[ 1 ] ) );
          it != ml.end();
          ++it )
    {
      mask_gids.push_back( it->second );
    }
  }
  else
  {
    Layer< 3 >* layer = dynamic_cast< Layer< 3 >* >(
      kernel().node_manager.get_node( layer_gid ) );

    MaskedLayer< 3 > ml =
      MaskedLayer< 3 >( *layer, Selector(), mask, true, false );

    for ( Ntree< 3, index >::masked_iterator it =
            ml.begin( Position< 3 >( anchor[ 0 ], anchor[ 1 ], anchor[ 2 ] ) );
          it != ml.end();
          ++it )
    {
      mask_gids.push_back( it->second );
    }
  }

  i->OStack.pop( 3 );
  i->OStack.push( mask_gids );
  i->EStack.pop();
}


std::string
LayerExpected::message() const
{
  return std::string();
}


} // namespace nest
