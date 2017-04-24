/*
 *  subnet.cpp
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

#include "subnet.h"

// C++ includes:
#include <string>

// Includes from nestkernel:
#include "event.h"
#include "kernel_manager.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"
#include "dictutils.h"

#ifdef N_DEBUG
#undef N_DEBUG
#endif

nest::Subnet::Subnet()
  : Node()
  , nodes_()
  , gids_()
  , label_()
  , customdict_( new Dictionary )
  , homogeneous_( true )
  , last_mid_( 0 )
{
  set_frozen_( true ); // freeze subnet by default
}

nest::Subnet::Subnet( const Subnet& c )
  : Node( c )
  , nodes_( c.nodes_ )
  , gids_( c.gids_ )
  , label_( c.label_ )
  , customdict_( new Dictionary( *( c.customdict_ ) ) )
  , homogeneous_( c.homogeneous_ )
  , last_mid_( c.last_mid_ )
{
}

void
nest::Subnet::set_status( const DictionaryDatum& dict )
{
  updateValue< std::string >( dict, "label", label_ );
  updateValue< DictionaryDatum >( dict, "customdict", customdict_ );
}

void
nest::Subnet::get_status( DictionaryDatum& dict ) const
{
  ( *dict )[ "number_of_children" ] = global_size();
  ( *dict )[ "label" ] = label_;
  ( *dict )[ "customdict" ] = customdict_;
  ( *dict )[ names::element_type ] = LiteralDatum( names::structure );
}

void
nest::Subnet::get_dimensions_( std::vector< int >& dim ) const
{
  dim.push_back( gids_.size() );
  if ( nodes_.empty() )
  {
    return;
  }
  if ( homogeneous_ && ( dynamic_cast< Subnet* >( nodes_.at( 0 ) ) != NULL ) )
  {
    bool homog = true;
    for ( size_t i = 0; i < nodes_.size() - 1; ++i )
    {
      Subnet* c1 = dynamic_cast< Subnet* >( nodes_.at( i ) );
      Subnet* c2 = dynamic_cast< Subnet* >( nodes_.at( i + 1 ) );

      if ( c1->global_size() != c2->global_size() )
      {
        homog = false;
        continue;
      }
    }

    // If homog is true, all child-subnets have the same size
    // and we go one level deeper.
    if ( homog )
    {
      Subnet* c = dynamic_cast< Subnet* >( nodes_.at( 0 ) );
      c->get_dimensions_( dim );
    }
  }
}


std::string
nest::Subnet::print_network( int max_depth, int level, std::string prefix )
{
  // When the function is first called, we have to have a single
  // space as prefix, otherwise everything will by slightly out of
  // format.
  if ( prefix == "" )
  {
    prefix = " ";
  }

  std::ostringstream out;
  if ( get_parent() )
  {
    out << "+-[" << get_lid() + 1 << "] ";

    if ( get_label() != "" )
    {
      out << get_label();
    }
    else
    {
      out << get_name();
    }
  }
  else
  {
    out << "+-"
        << "[0] ";
    if ( get_label() != "" )
    {
      out << get_label();
    }
    else
    {
      out << "root";
    }
  }

  std::vector< int > dim;
  get_dimensions_( dim );

  out << " dim=[";
  for ( size_t k = 0; k < dim.size() - 1; ++k )
  {
    out << dim[ k ] << " ";
  }
  out << dim[ dim.size() - 1 ] << "]" << std::endl;
  if ( max_depth <= level )
  {
    return out.str();
  }

  if ( nodes_.empty() )
  {
    return out.str();
  }
  prefix += "  ";
  out << prefix << "|" << std::endl;

  size_t first = 0;
  for ( size_t i = 0; i < nodes_.size(); ++i )
  {

    size_t next = i + 1;
    if ( nodes_[ i ] == NULL )
    {
      out << prefix << "+-NULL" << std::endl;
      // Print extra line, if we are at the end of a subnet.
      if ( next == nodes_.size() )
      {
        out << prefix << std::endl;
      }
      first = i + 1;
      continue;
    }

    Subnet* c = dynamic_cast< Subnet* >( nodes_[ i ] );
    if ( c != NULL )
    {
      // this node is a subnet,
      // the sequence is printed, so
      // we print the children and move on
      // print subnet
      //
      // If the subnet is the last node of the parent subnet,
      // we must not print the continuation line '|', so we distinguish
      // this case.
      if ( next == nodes_.size() )
      {
        out << prefix
            << nodes_[ i ]->print_network( max_depth, level + 1, prefix + " " );
      }
      else
      {
        out << prefix
            << nodes_[ i ]->print_network( max_depth, level + 1, prefix + "|" );
      }

      first = next;
      continue;
    }

    // now we look one into the future
    // to determine whether this is a sequence
    // or not.

    if ( next < nodes_.size() )
    {
      // we have a successor
      if ( nodes_[ next ] != NULL )
      {
        // it is not NULL

        c = dynamic_cast< Subnet* >( nodes_[ next ] );
        if ( c == NULL )
        {
          // and not a subnet, so we skipp
          // the printout, until the end
          // of the sequence is found.
          if ( ( nodes_[ first ]->get_name() == nodes_[ next ]->get_name() ) )
          {
            continue;
          }
        } // if the next node is a compount we flush the sequence
      }   // if the next node is NULL, we flush the sequence
    }     // if there is no next node, we flush the sequence

    if ( first < i )
    {
      // Here we print the sequence of consecutive nodes.
      // We can be sure that neither first, nor i point to NULL.
      out << prefix << "+-[" << first + 1 << "]...[" << i + 1 << "] "
          << nodes_[ first ]->get_name() << std::endl;
      // Print extra line, if we are at the end of a subnet.
      if ( next == nodes_.size() )
      {
        out << prefix << std::endl;
      }
      first = next;
      continue;
    }

    // Here, we deal the case of an individual Node with no identical
    // neighbours.

    out << prefix << "+-[" << i + 1 << "] " << nodes_[ first ]->get_name()
        << std::endl;

    // Print extra line, if we are at the end of a subnet.
    if ( next == nodes_.size() )
    {
      out << prefix << std::endl;
    }
    first = next;
  }
  return out.str();
}

void
nest::Subnet::set_label( std::string const label )
{
  // set the new label on all sibling threads
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    Node* n = kernel().node_manager.get_node( get_gid(), t );
    Subnet* c = dynamic_cast< Subnet* >( n );
    assert( c );
    c->label_ = label;
  }
}

bool
nest::Subnet::is_subnet() const
{
  return true;
}
