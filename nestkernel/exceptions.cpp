/*
 *  exceptions.cpp
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

#include "exceptions.h"

// Include MPI for MPI error string
#ifdef HAVE_MPI
#include <mpi.h>
#endif

// C++ includes:
#include <sstream>

// Generated includes:
#include "config.h"

// Includes from thirdparty:
#include "compose.hpp"


namespace nest
{
std::string
UnknownModelName::compose_msg_( const std::string& model_name ) const
{
  std::string msg = String::compose( "%1 is not a known model name.", model_name );
#ifndef HAVE_GSL
  msg += "\nA frequent cause for this error is that NEST was compiled ";
  msg += "without the GNU Scientific Library, which is required for ";
  msg += "the conductance-based neuron models.";
#endif
  return msg;
}

std::string
UnknownComponent::compose_msg_( const std::string& component_name ) const
{
  std::string msg = String::compose( "%1 is not a known component.", component_name );
#ifndef HAVE_GSL
  msg += "\nA frequent cause for this error is that NEST was compiled ";
  msg += "without the GNU Scientific Library, which is required for ";
  msg += "the conductance-based neuron models.";
#endif
  return msg;
}

std::string
NewModelNameExists::compose_msg_( const std::string& model_name ) const
{
  std::string msg = String::compose( "Model %1 is the name of an existing model and cannot be re-used.", model_name );
  return msg;
}

std::string
ModelInUse::compose_msg_( const std::string& model_name ) const
{
  std::string msg = String::compose( "Model %1 is in use and cannot be unloaded/uninstalled.", model_name );
  return msg;
}

std::string
UnknownSynapseType::compose_msg_( const int id ) const
{
  std::string msg = String::compose( "Synapse with id %1 does not exist.", id );
  return msg;
}

std::string
UnknownSynapseType::compose_msg_( const std::string& name ) const
{
  std::string msg = String::compose( "Synapse with name %1 does not exist.", name );
  return msg;
}

std::string
UnknownNode::compose_msg_( const int id ) const
{
  std::string msg = String::compose( "Node with id %1 does not exist.", id );
  return msg;
}

std::string
NoThreadSiblingsAvailable::compose_msg_( const int id ) const
{
  std::string msg = String::compose( "Node with id %1 does not have thread siblings.", id );
  return msg;
}

std::string
LocalNodeExpected::compose_msg_( const int id ) const
{
  std::string msg = String::compose( "Node with id %1 is not a local node.", id );
  return msg;
}

std::string
NodeWithProxiesExpected::compose_msg_( const int id ) const
{
  std::string msg = String::compose(
    "A node with proxies (usually a neuron) is expected, "
    "but the node with id %1 is a node without proxies (usually a device).",
    id );
  return msg;
}

std::string
UnknownCompartment::compose_msg_( const long compartment_idx, const std::string info ) const
{
  std::string msg = String::compose( "Compartment %1 %2.", compartment_idx, info );
  return msg;
}

std::string
UnknownReceptorType::compose_msg_( const long receptor_type, const std::string name ) const
{
  std::string msg = String::compose( "Receptor type %1 is not available in %2.", receptor_type, name );
  return msg;
}

std::string
IncompatibleReceptorType::compose_msg( const long receptor_type, const std::string name, const std::string event_type )
{
  std::string msg = String::compose( "Receptor type %1 in %2 does not accept %3.", receptor_type, name, event_type );
  return msg;
}

std::string
UnknownPort::compose_msg_( const int id ) const
{
  std::string msg = String::compose( "Port with id %1 does not exist.", id );
  return msg;
}

std::string
UnknownPort::compose_msg_( const int id, const std::string msg ) const
{
  std::string msg_out;
  msg_out = String::compose( "Port with id %1 does not exist. ", id );
  msg_out += msg;
  return msg_out;
}

std::string
UnsupportedEvent::compose_msg_() const
{
  std::string msg;
  msg = "The current synapse type does not support the event type of the sender.\n";
  msg += "    A common cause for this is a plastic synapse between a device and a neuron.";
  return msg;
}

#ifdef HAVE_MPI
MPIErrorCode::MPIErrorCode( const int error_code )
{
  char errmsg_[ 2 * MPI_MAX_ERROR_STRING ];  // Multiply by two for extra safety
  int len_;
  MPI_Error_string( error_code, errmsg_, &len_ );
  std::string error_;
  error_.assign( errmsg_, len_ );
  msg_ = String::compose( "MPI Error: %1", error_ );
}
#endif

}  // namespace nest
