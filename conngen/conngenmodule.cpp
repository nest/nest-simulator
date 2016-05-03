/*
 *  conngenmodule.cpp
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

#include "conngenmodule.h"

// Generated includes:
#include "config.h"

// Includes from conngen:
#include "cg_connect.h"
#include "conngen.h"

// Includes from sli:
#include "lockptrdatum_impl.h"
#include "stringdatum.h"
#include "tokenutils.h"

template class lockPTRDatum< ConnectionGenerator,
  &nest::ConnectionGeneratorType >;

namespace nest
{

ConnectionGeneratorModule::ConnectionGeneratorModule()
{
}

ConnectionGeneratorModule::~ConnectionGeneratorModule()
{
  ConnectionGeneratorType.deletetypename();
}

const std::string
ConnectionGeneratorModule::name() const
{
  return std::string( "ConnectionGeneratorModule" );
}

const std::string
ConnectionGeneratorModule::commandstring() const
{
  return std::string( "(conngen-interface) run" );
}

void
ConnectionGeneratorModule::init( SLIInterpreter* i )
{
  ConnectionGeneratorType.settypename( "connectiongeneratortype" );
  ConnectionGeneratorType.setdefaultaction( SLIInterpreter::datatypefunction );

  // Register the user functions of the connection generator interface
  i->createcommand( "CGConnect_cg_i_i_D_l", &cgconnect_cg_i_i_D_lfunction );
  i->createcommand( "CGConnect_cg_iV_iV_D_l", &cgconnect_cg_iV_iV_D_lfunction );
  i->createcommand( "CGParse", &cgparse_sfunction );
  i->createcommand( "CGParseFile", &cgparsefile_sfunction );
  i->createcommand(
    "CGSelectImplementation", &cgselectimplementation_s_sfunction );

  // Register the low level functions of the connection generator interface
  i->createcommand( "cgsetmask_cg_iV_iV", &cgsetmask_cg_iV_iVfunction );
  i->createcommand( "cgstart", &cgstart_cgfunction );
  i->createcommand( "cgnext", &cgnext_cgfunction );
}

/* BeginDocumentation
   Name: CGConnect - Establish connections contained in a ConnectionGenerator

   Synopsis:
   cg sources targets                  ->  -
   cg sources targets params           ->  -
   cg sources targets        syn_model ->  -
   cg sources targets params syn_model ->  -

   Parameters:
   cg        - ConnectionGenerator
   sources   - The sources. Either a subnet or a list of nodes
   targets   - The targets. Either a subnet or a list of nodes
   params    - A dict specifying the index of /weight and /delay
               in the value set of the connection generator
   syn_model - A literal specifying te synapse model to be used

   Description:
   CGConnect connects a source and a target population according to
   the rules defined in the given connection generator. params is an
   optional dictionary, that maps the names /weight and/or /delay to
   their integer index in the value set in the connection generator.
   If not specified, the synapse model is taken from the Options of
   the Connect command.

   Author: Jochen Martin Eppler
   FirstVersion: August 2012
   SeeAlso: Connect, synapsedict, GetOptions, CGParse, CGParseFile,
   CGSelectImplementation, cgstart,
   cgsetmask, cgnext
*/

// Connect for conn_generator subnet subnet dict synapsetype
void
ConnectionGeneratorModule::CGConnect_cg_i_i_D_lFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 5 );

  ConnectionGeneratorDatum cg =
    getValue< ConnectionGeneratorDatum >( i->OStack.pick( 4 ) );
  index source_id = getValue< long >( i->OStack.pick( 3 ) );
  index target_id = getValue< long >( i->OStack.pick( 2 ) );
  DictionaryDatum params_map =
    getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  const Name synmodel_name = getValue< std::string >( i->OStack.pick( 0 ) );

  cg_connect( cg, source_id, target_id, params_map, synmodel_name );

  i->OStack.pop( 5 );
  i->EStack.pop();
}

// Connect for conn_generator array array dict synapsetype
void
ConnectionGeneratorModule::CGConnect_cg_iV_iV_D_lFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 5 );

  ConnectionGeneratorDatum cg =
    getValue< ConnectionGeneratorDatum >( i->OStack.pick( 4 ) );
  IntVectorDatum sources = getValue< IntVectorDatum >( i->OStack.pick( 3 ) );
  IntVectorDatum targets = getValue< IntVectorDatum >( i->OStack.pick( 2 ) );
  DictionaryDatum params_map =
    getValue< DictionaryDatum >( i->OStack.pick( 1 ) );
  const Name synmodel_name = getValue< std::string >( i->OStack.pick( 0 ) );

  cg_connect( cg, sources, targets, params_map, synmodel_name );

  i->OStack.pop( 5 );
  i->EStack.pop();
}


/* BeginDocumentation
   Name: CGParse - Call ConnectionGenerator::fromXML() and return a
   ConnectionGenerator

   Synopsis:
   xml_string CGParse -> cg

   Parameters:
   xml_string - The XML string to parse.

   Description:
   Return a ConnectionGenerator created by deserializing the given
   XML string. The library to parse the XML string can be selected using
   CGSelectImplementation

   Author: Jochen Martin Eppler
   FirstVersion: September 2013
   SeeAlso: CGParseFile, CGConnect, CGSelectImplementation, cgstart, cgsetmask,
   cgnext
*/
void
ConnectionGeneratorModule::CGParse_sFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum xml = getValue< StringDatum >( i->OStack.pick( 0 ) );
  ConnectionGeneratorDatum cgd = cg_parse( xml );

  i->OStack.pop( 1 );
  i->OStack.push( cgd );
  i->EStack.pop();
}


/* BeginDocumentation
   Name: CGParseFile - Call ConnectionGenerator::fromXMLFile() and return a
   ConnectionGenerator

   Synopsis:
   xml_filename CGParseFile -> cg

   Parameters:
   xml_filename - The XML file to read.

   Description:
   Return a ConnectionGenerator created by deserializing the given
   XML file. The library to parse the XML file can be selected using
   CGSelectImplementation

   Author: Jochen Martin Eppler
   FirstVersion: February 2014
   SeeAlso: CGParse, CGConnect, CGSelectImplementation, cgstart, cgsetmask,
   cgnext
*/
void
ConnectionGeneratorModule::CGParseFile_sFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  StringDatum xml = getValue< StringDatum >( i->OStack.pick( 0 ) );
  ConnectionGeneratorDatum cgd = cg_parse_file( xml );

  i->OStack.pop( 1 );
  i->OStack.push( cgd );
  i->EStack.pop();
}


/* BeginDocumentation
   Name: CGSelectImplementation - Call
   ConnectionGenerator::selectCGImplementation()

   Synopsis:
   tag library CGParse -> -

   Parameters:
   tag     - The XML tag to associate with a library.
   library - The library to provide the parsing for CGParse

   Description:
   Select a library to provide a parser for XML files and associate
   an XML tag with the library.

   Author: Jochen Martin Eppler
   FirstVersion: September 2013
   SeeAlso: CGParse, CGParseFile, CGConnect, cgstart, cgsetmask, cgnext
*/
void
ConnectionGeneratorModule::CGSelectImplementation_s_sFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  StringDatum library = getValue< StringDatum >( i->OStack.pick( 0 ) );
  StringDatum tag = getValue< StringDatum >( i->OStack.pick( 1 ) );

  cg_select_implementation( library, tag );

  i->OStack.pop( 1 );
  i->EStack.pop();
}


/* BeginDocumentation
   Name: cgsetmask - Call setMasks() on a ConnectionGenerator

   Synopsis:
   cg sources targets cgsetmask -> -

   Parameters:
   cg      - ConnectionGenerator
   sources - A list of nodes used as source masks
   targets - A list of nodes used as target masks

   Description:
   Set masks for sources and targets on a given ConnectionGenerator
   cg. This is calling the setMasks() function on cg internally.

   Remarks:
   This function is part of the low-level access API for the
   ConnectionGenerator module. It is mainly used for debugging
   purposes. Usually, connections are created from a
   ConnectionGenerator using CGConnect.

   Author: Mikael Djurfeldt
   FirstVersion: March 2011
   SeeAlso: CGParse, CGParseFile, CGConnect, CGSelectImplementation, cgstart,
   cgnext
*/
void
ConnectionGeneratorModule::CGSetMask_cg_iV_iVFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 3 );

  ConnectionGeneratorDatum cg =
    getValue< ConnectionGeneratorDatum >( i->OStack.pick( 2 ) );
  IntVectorDatum sources = getValue< IntVectorDatum >( i->OStack.pick( 1 ) );
  IntVectorDatum targets = getValue< IntVectorDatum >( i->OStack.pick( 0 ) );

  cg_set_masks( cg, sources, targets );

  i->OStack.pop( 3 );
  i->EStack.pop();
}


/* BeginDocumentation
   Name: cgstart - Call start() on a ConnectionGenerator

   Synopsis:
   cg cgstart -> -

   Parameters:
   cg - ConnectionGenerator

   Description:
   Call the start() function on a given ConnectionGenerator cg.

   Remarks:
   This function is part of the low-level access API for the
   ConnectionGenerator module. It is mainly used for debugging
   purposes. Usually, connections are created from a
   ConnectionGenerator using CGConnect.

   Author: Mikael Djurfeldt
   FirstVersion: March 2011
   SeeAlso: CGParse, CGParseFile, CGConnect, CGSelectImplementation, cgsetmask,
   cgnext
*/
void
ConnectionGeneratorModule::CGStart_cgFunction::execute(
  SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ConnectionGeneratorDatum cgd =
    getValue< ConnectionGeneratorDatum >( i->OStack.pick( 0 ) );

  cg_start( cgd );

  i->OStack.pop( 1 );
  i->EStack.pop();
}


/* BeginDocumentation
   Name: cgnext - Call next() on a ConnectionGenerator

   Synopsis:
   cg cgnext -> source target v[0] ... true | false

   Parameters:
   cg - ConnectionGenerator

   Description:
   Call the next() function on a given ConnectionGenerator cg
   to iterate cg's connections on the SLI level. This function
   will return the source and the target of the connection, a
   list containing the values for the connection (if there are
   any), and true, or false, if cg cannot be iterated further.

   Remarks:
   This function is part of the low-level access API for the
   ConnectionGenerator module. It is mainly used for debugging
   purposes. Usually, connections are created from a
   ConnectionGenerator using CGConnect.

   Author: Mikael Djurfeldt
   FirstVersion: December 2012
   SeeAlso: CGParse, CGParseFile, CGConnect, CGSelectImplementation, cgstart,
   cgsetmask
*/
void
ConnectionGeneratorModule::CGNext_cgFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 1 );

  ConnectionGeneratorDatum cgd =
    getValue< ConnectionGeneratorDatum >( i->OStack.pick( 0 ) );

  int j, k;
  std::vector< double > values;
  i->OStack.pop( 1 );
  i->EStack.pop();
  if ( cg_next( cgd, j, k, values ) )
  {
    i->OStack.push( j );
    i->OStack.push( k );
    for ( size_t m = 0; m < values.size(); ++m )
    {
      i->OStack.push( values[ m ] );
    }
    i->OStack.push( true );
  }
  else
  {
    i->OStack.push( false );
  }
}


} // namespace nest
