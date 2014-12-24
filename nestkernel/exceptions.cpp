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
#include "interpret.h"
#include "config.h"

#include <sstream>

std::string nest::UnknownModelName::message()
{
  std::ostringstream msg;
  msg << "/" << n_.toString() + " is not a known model name. "
    "Please check the modeldict for a list of available models."
#ifndef HAVE_GSL
    " A frequent cause for this error is that NEST was compiled "
    "without the GNU Scientific Library, which is required for "
    "the conductance-based neuron models."
#endif
    ;
  return msg.str();
}

std::string nest::NewModelNameExists::message()
{
  std::ostringstream msg;
  msg << "/" << n_.toString() 
    + " is the name of an existing model and cannot be re-used.";
  return msg.str();
}

std::string nest::UnknownModelID::message()
{
  std::ostringstream msg;

  msg << id_ << " is an invalid model ID. Probably modeldict is corrupted.";
  return msg.str();
}

std::string nest::ModelInUse::message()
{
  std::string str = "Model " + modelname_ + " is in use and cannot be unloaded/uninstalled.";
  return str.c_str();
}

std::string nest::UnknownSynapseType::message()
{
  std::ostringstream out;
  if(synapsename_.empty())
    {
      out << "Synapse with id " << synapseid_ << " does not exist.";
    }
  else
    {
      out << "Synapse with name " << synapsename_ << " does not exist.";
    }
  return out.str();
}

std::string nest::UnknownNode::message()
{
  std::ostringstream out;

  if(id_ >= 0)
    {
      out << "Node with id " << id_ << " doesn't exist.";
    }
  else
    {
      //Empty message
    }

  return out.str();
}

std::string nest::NoThreadSiblingsAvailable::message()
{
  std::ostringstream out;

  if(id_ >= 0)
  {
    out << "Node with id " << id_ << " does not have thread siblings.";
  }
  else
  {
    //Empty message
  }

  return out.str();
}


std::string nest::LocalNodeExpected::message()
{
  std::ostringstream out;
  out << "Node with id " << id_ << " is not a local node.";
  return out.str();
}

std::string nest::NodeWithProxiesExpected::message()
{
  std::ostringstream out;
  out << "Nest expected a node with proxies (eg normal model neuron),"
         "but the node with id " << id_ << " is not a node without proxies, "
         "e.g., a subnet or device.";
  return out.str();
}

std::string nest::UnknownReceptorType::message()
{
  std::ostringstream msg;

  msg << "Receptor type " << receptor_type_ << " is not available in "
      << name_ << ".";
  return msg.str();
}

std::string nest::IncompatibleReceptorType::message()
{
  std::ostringstream msg;

  msg << "Receptor type " << receptor_type_ << " in "
      << name_ << " does not accept " << event_type_ << ".";
  return msg.str();
}

std::string nest::UnknownPort::message()
{
  std::ostringstream out;
  out << "Port with id " << id_ << " does not exist.";
  return out.str();
}

std::string nest::IllegalConnection::message()
{
  if ( msg_.empty() )
    return "Creation of connection is not possible.";
  else
    return "Creation of connection is not possible because:\n"
      + msg_;
}

std::string nest::UnknownThread::message()
{
  std::ostringstream out;
  out << "Thread with id " << id_ << " is outside of range.";
  return out.str();
}

std::string nest::BadDelay::message()
{
  std::ostringstream out;
  out << "Delay value " << delay_ << " is invalid: "
	  << message_;
  return out.str();
}

std::string nest::UnexpectedEvent::message()
{
  return "Node cannot handle received event.";
}

std::string nest::UnsupportedEvent::message()
{
  return std::string("The current synapse type does not support the event type of the sender.\n")
    + std::string("       A common reason for this is a dynamic synapse between a device and a neuron.");
}

std::string nest::BadProperty::message()
{
  return msg_;
}

std::string nest::BadParameter::message()
{
  return msg_;
}

std::string nest::DimensionMismatch::message()
{
  std::ostringstream out;
  if(expected_ != provided_)
    {
      out << "Expected dimension size: " << expected_ 
	  << "\nProvided dimension size: " << provided_;      
    }
  else
    {
      out << "Dimension of two or more variables do not match.";
    }
  return out.str();
}

std::string nest::DistributionError::message()
{
  return std::string();
}

std::string nest::SubnetExpected::message()
{
  return std::string();
}

std::string nest::SimulationError::message()
{
  return std::string("One or more nodes reported an error. Please check the output preceeding this message.");
}

std::string nest::InvalidDefaultResolution::message()
{
  std::ostringstream msg;
  msg << "The default resolution of " << Time::get_resolution()
      << " is not consistent with the value " 
      << val_ << " of property '"
      << prop_.toString() << "' in model " << model_ << ".\n"
      << "This is an internal NEST error, please report to the "
      "nest_user@nest-initiative.org mailing list!";
  return msg.str();
}

std::string nest::InvalidTimeInModel::message()
{
  std::ostringstream msg;
  msg << "The time property " << prop_.toString() << " = "
      << val_ << " of model " << model_ 
      << " is not compatible with the resolution " 
      << Time::get_resolution() << ".\n"
      << "Please set a compatible value with SetDefaults!";
  return msg.str();
}

std::string nest::StepMultipleRequired::message()
{
  std::ostringstream msg;
  msg << "The time property " << prop_.toString() << " = "
      << val_ << " of model " << model_ 
      << " must be a multiple of the resolution "
      << Time::get_resolution() << ".";
  return msg.str();
}

std::string nest::TimeMultipleRequired::message()
{
  std::ostringstream msg;
  msg << "In model " << model_ << ", the time property " 
      << prop_a_.toString() << " = " << val_a_
      << " must be multiple of time property "
      << prop_b_.toString() << " = " << val_b_
      << '.';
  return msg.str();
}

#ifdef HAVE_MUSIC
std::string nest::MUSICPortUnconnected::message()
{
  std::ostringstream msg;
  msg << "Cannot use instance of model " << model_ << " because the MUSIC port " << portname_ << " is unconnected.";
  return msg.str();
}

std::string nest::MUSICPortHasNoWidth::message()
{
  std::ostringstream msg;
  msg << "Cannot use instance of model " << model_ << " because the MUSIC port " << portname_ << " has no width specified in configuration file.";
  return msg.str();
}

std::string nest::MUSICPortAlreadyPublished::message()
{
  std::ostringstream msg;
  msg << "The instance of model " << model_ << " cannot change the MUSIC port / establish connections " << portname_ << " since it is already published.";
  return msg.str();
}

std::string nest::MUSICSimulationHasRun::message()
{
  std::ostringstream msg;
  msg << "The instance of model " << model_ << " won't work, since the simulation has already been running";
  return msg.str();
}

std::string nest::MUSICChannelUnknown::message()
{
  std::ostringstream msg;
  msg << "The port " << portname_ << " cannot be mapped in " << model_ << " because the channel " << channel_ << " does not exists.";
  return msg.str();
}

std::string nest::MUSICPortUnknown::message()
{
  std::ostringstream msg;
  msg << "The port " << portname_ << " does not exist.";
  return msg.str();
}


std::string nest::MUSICChannelAlreadyMapped::message()
{
  std::ostringstream msg;
  msg << "The channel " << channel_ << " of port " << portname_ << " has already be mapped to another proxy in " << model_;
  return msg.str();
}
#endif

std::string nest::GSLSolverFailure::message()
{
  std::ostringstream msg;
  msg << "In model " << model_ << ", the GSL solver "
      << "returned with exit status " << status_ << ".\n"
      << "Please make sure you have installed a recent "
      << "GSL version (> gsl-1.10), or contact the "
      << "nest_user@nest-initiative.org mailing list"
      << "for further support.";
  return msg.str();
}

std::string nest::NumericalInstability::message()
{
  std::ostringstream msg;
  msg << "NEST detected a numerical instability while "
      << "updating " << model_ << ". Try to reduce "
      << "gsl_error_tol for the model.";
  return msg.str();
}
