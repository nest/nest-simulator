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


const char*
nest::UnknownModelName::what() const noexcept
{
  std::ostringstream msg;
  msg << "/" << model_name_ + " is not a known model name. "
    "Please check the modeldict for a list of available models.";
#ifndef HAVE_GSL
  msg << " A frequent cause for this error is that NEST was compiled "
         "without the GNU Scientific Library, which is required for "
         "the conductance-based neuron models.";
#endif
  return msg.str().c_str();
}

const char*
nest::NewModelNameExists::what() const noexcept
{
  std::ostringstream msg;
  msg << "/" << model_name_ + " is the name of an existing model and cannot be re-used.";
  return msg.str().c_str();
}

const char*
nest::UnknownModelID::what() const noexcept
{
  std::ostringstream msg;

  msg << id_ << " is an invalid model ID. Probably modeldict is corrupted.";
  return msg.str().c_str();
}

const char*
nest::UnknownSynapseType::what() const noexcept
{
  std::ostringstream out;
  if ( synapsename_.empty() )
  {
    out << "Synapse with id " << synapseid_ << " does not exist.";
  }
  else
  {
    out << "Synapse with name " << synapsename_ << " does not exist.";
  }
  return out.str().c_str();
}

const char*
nest::UnknownNode::what() const noexcept
{
  std::ostringstream out;

  if ( id_ >= 0 )
  {
    out << "Node with id " << id_ << " doesn't exist.";
  }
  else
  {
    // Empty message
  }

  return out.str().c_str();
}

const char*
nest::NoThreadSiblingsAvailable::what() const noexcept
{
  std::ostringstream out;

  if ( id_ >= 0 )
  {
    out << "Node with id " << id_ << " does not have thread siblings.";
  }
  else
  {
    // Empty message
  }

  return out.str().c_str();
}

const char*
nest::UnknownReceptorType::what() const noexcept
{
  std::ostringstream msg;

  msg << "Receptor type " << receptor_type_ << " is not available in " << name_ << ".";
  return msg.str().c_str();
}

const char*
nest::IncompatibleReceptorType::what() const noexcept
{
  std::ostringstream msg;

  msg << "Receptor type " << receptor_type_ << " in " << name_ << " does not accept " << event_type_ << ".";
  return msg.str().c_str();
}

const char*
nest::UnknownPort::what() const noexcept
{
  std::ostringstream out;
  out << "Port with id " << id_ << " does not exist.";
  return out.str().c_str();
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
nest::BackendPrepared::what() const noexcept
{
  std::ostringstream msg;
  msg << "Backend " << backend_ << " may not be prepare()'d multiple times.";
  return msg.str().c_str();
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


const char*
nest::UndefinedName::what() const noexcept
{
  std::ostringstream msg;
  msg << "The name " << name_ << " is not defined.";
  return msg.str().c_str();
}
