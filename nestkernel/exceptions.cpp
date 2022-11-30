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

// C++ includes:
#include <sstream>

// Generated includes:
#include "config.h"


std::string
nest::UnknownModelName::compose_msg_( const std::string& model_name ) const
{
  std::ostringstream msg;
  msg << model_name << " is not a known model name.";
#ifndef HAVE_GSL
  msg << " A frequent cause for this error is that NEST was compiled"
         " without the GNU Scientific Library, which is required for"
         " the conductance-based neuron models.";
#endif
  return msg.str();
}

std::string
nest::UnknownComponent::compose_msg_( const std::string& component_name ) const
{
  std::ostringstream msg;
  msg << component_name << " is not a known component.";
#ifndef HAVE_GSL
  msg << " A frequent cause for this error is that NEST was compiled"
         " without the GNU Scientific Library, which is required for"
         " the conductance-based neuron models.";
#endif
  return msg.str();
}

std::string
nest::NewModelNameExists::compose_msg_( const std::string& model_name ) const
{
  std::string msg  = "Model " + model_name + " is the name of an existing model and cannot be re-used.";
  return msg;
}

std::string
nest::ModelInUse::compose_msg_( const std::string& model_name ) const
{
  std::string msg = "Model " + model_name + " is in use and cannot be unloaded/uninstalled.";
  return msg;
}

std::string
nest::UnknownSynapseType::compose_msg_( const int id ) const
{
  std::ostringstream msg;
  msg << "Synapse with id " << id << " does not exist.";
  return msg.str();
}

std::string
nest::UnknownSynapseType::compose_msg_( const std::string& name ) const
{
  std::ostringstream msg;
  msg << "Synapse with name " << name << " does not exist.";
  return msg.str();
}

std::string
nest::UnknownNode::compose_msg_( const int id ) const
{
  std::ostringstream out;
  out << "Node with id " << id << " doesn't exist.";
  return out.str();
}

std::string
nest::NoThreadSiblingsAvailable::compose_msg_( const int id ) const
{
  std::ostringstream out;
  out << "Node with id " << id << " does not have thread siblings.";
  return out.str();
}

std::string
nest::LocalNodeExpected::compose_msg_( const int id ) const
{
  std::ostringstream out;
  out << "Node with id " << id << " is not a local node.";
  return out.str();
}

std::string
nest::NodeWithProxiesExpected::compose_msg_( const int id ) const
{
  std::ostringstream out;
  out << "A node with proxies (usually a neuron) is expected, but the node with id "
      << id << " is a node without proxies (usually a device).";
  return out.str();
}

std::string
nest::UnknownCompartment::compose_msg_( const long compartment_idx, const std::string info ) const
{
  std::ostringstream msg;
  msg << "Compartment " << compartment_idx << " " << info << ".";
  return msg.str();
}


std::string
nest::UnknownReceptorType::compose_msg_( const long receptor_type, const std::string name ) const
{
  std::ostringstream msg;
  msg << "Receptor type " << receptor_type << " is not available in " << name << ".";
  return msg.str();
}

std::string
nest::IncompatibleReceptorType::compose_msg( const long receptor_type, const std::string name, const std::string event_type)
{
  std::ostringstream msg;
  msg << "Receptor type " << receptor_type << " in " << name << " does not accept " << event_type << ".";
  return msg.str();
}

std::string
nest::UnknownPort::compose_msg_( const int id ) const
{
  std::ostringstream out;
  out << "Port with id " << id << " does not exist.";
  return out.str();
}

std::string
nest::UnknownPort::compose_msg_( const int id, const std::string msg ) const
{
  std::ostringstream out;
  out << "Port with id " << id << " does not exist. " << msg;
  return out.str();
}

const char*
nest::IllegalConnection::what() const noexcept
{
  if ( msg_.empty() )
  {
    return "Creation of connection is not possible.";
  }
  else
  {
    return ( "Creation of connection is not possible because:\n" + msg_ ).c_str();
  }
}

const char*
nest::InexistentConnection::what() const noexcept
{
  if ( msg_.empty() )
  {
    return "Deletion of connection is not possible.";
  }
  else
  {
    return ( "Deletion of connection is not possible because:\n" + msg_ ).c_str();
  }
}

const char*
nest::UnknownThread::what() const noexcept
{
  std::ostringstream out;
  out << "Thread with id " << id_ << " is outside of range.";
  return out.str().c_str();
}

const char*
nest::BadDelay::what() const noexcept
{
  std::ostringstream out;
  out << "Delay value " << delay_ << " is invalid: " << message_;
  return out.str().c_str();
}

const char*
nest::UnexpectedEvent::what() const noexcept
{
  if ( msg_.empty() )
  {
    return std::string(
      "Target node cannot handle input event.\n"
      "    A common cause for this is an attempt to connect recording devices incorrectly.\n"
      "    Note that recorders such as spike recorders must be connected as\n\n"
      "        nest.Connect(neurons, spike_det)\n\n"
      "    while meters such as voltmeters must be connected as\n\n"
      "        nest.Connect(meter, neurons) " )
      .c_str();
  }
  else
  {
    return ( "UnexpectedEvent: " + msg_ ).c_str();
  }
}

std::string
nest::UnsupportedEvent::compose_msg_() const
{
  return "The current synapse type does not support the event type of the sender.\n"
    "    A common cause for this is a plastic synapse between a device and a neuron.";
}

const char*
nest::BadProperty::what() const noexcept
{
  return msg_.c_str();
}

const char*
nest::BadParameter::what() const noexcept
{
  return msg_.c_str();
}

const char*
nest::DimensionMismatch::what() const noexcept
{
  std::ostringstream out;

  if ( not msg_.empty() )
  {
    out << msg_;
  }
  else if ( expected_ == -1 )
  {
    out << "Dimensions of two or more variables do not match.";
  }
  else
  {
    out << "Expected dimension size: " << expected_ << "\nProvided dimension size: " << provided_;
  }

  return out.str().c_str();
}

const char*
nest::InvalidDefaultResolution::what() const noexcept
{
  std::ostringstream msg;
  msg << "The default resolution of " << Time::get_resolution() << " is not consistent with the value " << val_
      << " of property '" << prop_ << "' in model " << model_ << ".\n"
      << "This is an internal NEST error, please report it at "
         "https://github.com/nest/nest-simulator/issues";
  return msg.str().c_str();
}

const char*
nest::InvalidTimeInModel::what() const noexcept
{
  std::ostringstream msg;
  msg << "The time property " << prop_ << " = " << val_ << " of model " << model_
      << " is not compatible with the resolution " << Time::get_resolution() << ".\n"
      << "Please set a compatible value with SetDefaults!";
  return msg.str().c_str();
}

const char*
nest::StepMultipleRequired::what() const noexcept
{
  std::ostringstream msg;
  msg << "The time property " << prop_ << " = " << val_ << " of model " << model_
      << " must be a multiple of the resolution " << Time::get_resolution() << ".";
  return msg.str().c_str();
}

const char*
nest::TimeMultipleRequired::what() const noexcept
{
  std::ostringstream msg;
  msg << "In model " << model_ << ", the time property " << prop_a_ << " = " << val_a_
      << " must be multiple of time property " << prop_b_ << " = " << val_b_ << '.';
  return msg.str().c_str();
}

const char*
nest::GSLSolverFailure::what() const noexcept
{
  std::ostringstream msg;
  msg << "In model " << model_ << ", the GSL solver "
      << "returned with exit status " << status_ << ".\n"
      << "Please make sure you have installed a recent "
      << "GSL version (> gsl-1.10).";
  return msg.str().c_str();
}

const char*
nest::NumericalInstability::what() const noexcept
{
  std::ostringstream msg;
  msg << "NEST detected a numerical instability while "
      << "updating " << model_ << ".";
  return msg.str().c_str();
}

const char*
nest::NamingConflict::what() const noexcept
{
  return msg_.c_str();
}

const char*
nest::RangeCheck::what() const noexcept
{
  if ( size_ > 0 )
  {
    std::ostringstream out;
    out << "Array with length " << size_ << " expected.";
    return out.str().c_str();
  }
  else
  {
    // TODO-PYNEST-NG: Fix usage, the comment below has been there already
    // Empty message. Added due to incorrect use of RangeCheck in nest.cpp
    return "";
  }
}

const char*
nest::IOError::what() const noexcept
{
  return std::string().c_str();
}

const char*
nest::KeyError::what() const noexcept
{
  std::ostringstream msg;
  msg << "Key '" << key_ << "' not found in map."
      << "Error encountered with map type: '" << map_type_ << "'"
      << " when applying operation: '" << map_op_ << "'";
  return msg.str().c_str();
}

const char*
nest::InternalError::what() const noexcept
{
  return msg_.c_str();
}

#ifdef HAVE_MUSIC
const char*
nest::MUSICPortUnconnected::what() const noexcept
{
  std::ostringstream msg;
  msg << "Cannot use instance of model " << model_ << " because the MUSIC port " << portname_ << " is unconnected.";
  return msg.str().c_str();
}

const char*
nest::MUSICPortHasNoWidth::what() const noexcept
{
  std::ostringstream msg;
  msg << "Cannot use instance of model " << model_ << " because the MUSIC port " << portname_
      << " has no width specified in configuration file.";
  return msg.str().c_str();
}

const char*
nest::MUSICPortAlreadyPublished::what() const noexcept
{
  std::ostringstream msg;
  msg << "The instance of model " << model_ << " cannot change the MUSIC port / establish connections " << portname_
      << " since it is already published.";
  return msg.str().c_str();
}

const char*
nest::MUSICSimulationHasRun::what() const noexcept
{
  std::ostringstream msg;
  msg << "The instance of model " << model_ << " won't work, since the simulation has already been running";
  return msg.str().c_str();
}

const char*
nest::MUSICChannelUnknown::what() const noexcept
{
  std::ostringstream msg;
  msg << "The port " << portname_ << " cannot be mapped in " << model_ << " because the channel " << channel_
      << " does not exists.";
  return msg.str().c_str();
}

const char*
nest::MUSICPortUnknown::what() const noexcept
{
  std::ostringstream msg;
  msg << "The port " << portname_ << " does not exist.";
  return msg.str().c_str();
}


const char*
nest::MUSICChannelAlreadyMapped::what() const noexcept
{
  std::ostringstream msg;
  msg << "The channel " << channel_ << " of port " << portname_ << " has already be mapped to another proxy in "
      << model_;
  return msg.str().c_str();
}
#endif

#ifdef HAVE_MPI
const char*
nest::MPIPortsFileUnknown::what() const noexcept
{
  std::ostringstream msg;
  msg << "The node with ID " << node_id_ << " requires a label,"
      << " which specifies the folder with files containing the MPI ports";
  return msg.str().c_str();
}
#endif

const char*
nest::UnmatchedSteps::what() const noexcept
{
  std::ostringstream msg;
  msg << "Steps for backend device don't match NEST steps: "
      << "steps expected: " << total_steps_ << " "
      << "steps executed: " << current_step_ << ".";
  return msg.str().c_str();
}

const char*
nest::BackendPrepared::what() const noexcept
{
  std::ostringstream msg;
  msg << "Backend " << backend_ << " may not be prepare()'d multiple times.";
  return msg.str().c_str();
}

const char*
nest::BackendNotPrepared::what() const noexcept
{
  std::ostringstream msg;
  msg << "Backend " << backend_ << " may not be cleanup()'d without preparation (multiple cleanups?).";
  return msg.str().c_str();
}

const char*
nest::UndefinedName::what() const noexcept
{
  std::ostringstream msg;
  msg << "The name " << name_ << " is not defined.";
  return msg.str().c_str();
}
