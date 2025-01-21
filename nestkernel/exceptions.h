/*
 *  exceptions.h
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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

// C++ includes:
#include <sstream>

// Includes from nestkernel:
#include "nest_time.h"

// Includes from thirdparty:
#include "compose.hpp"

// Include MPI for MPI error string
#ifdef HAVE_MPI
#include <mpi.h>
#endif

namespace nest
{

class Event;

/**
 * @addtogroup Exceptions Exception classes
 *
 * Exception classes that are thrown to indicate a user error.
 *
 * Programmatic errors or deviations from the expected behavior of
 * internal API conventions should never be handles by using
 * exceptions, but C++ `assert`s should be used for such cases.
 */

/**
 * @defgroup KernelExceptions NEST kernel exception classes
 *
 * Exception classes that are thrown by the NEST kernel to indicate
 * an error.
 */

/**
 * Base class for all Kernel exceptions.
 * @ingroup KernelExceptions
 */
class KernelException : public std::runtime_error
{
public:
  explicit KernelException()
    : std::runtime_error( "" )
  {
  }

  explicit KernelException( const std::string& msg )
    : std::runtime_error( msg )
  {
  }

  virtual std::string
  exception_name()
  {
    return "KernelException";
  }
};

/**
 * Exception to be thrown if a feature is unavailable.
 * @ingroup KernelExceptions
 */
class NotImplemented : public KernelException
{
public:
  explicit NotImplemented( const std::string& msg )
    : KernelException( msg )
  {
  }

  std::string
  exception_name() override
  {
    return "NotImplemented";
  }
};

/**
 * Exception to be thrown if a given type does not match the expected type.
 * @ingroup KernelExceptions
 */
class TypeMismatch : public KernelException
{

public:
  explicit TypeMismatch()
    : KernelException( "The expected datatype is unknown in the current context." )
  {
  }

  explicit TypeMismatch( const std::string& expected )
    : KernelException( "Expected datatype: " + expected )
  {
  }

  explicit TypeMismatch( const std::string& expected, const std::string& provided )
    : KernelException( "Expected datatype: " + expected + ", provided datatype: " + provided )
  {
  }

  std::string
  exception_name() override
  {
    return "TypeMismatch";
  }
};


/**
 * @brief Not all elements in a dictionary have been accessed.
 *
 * @param what Which parameter triggers the error
 * @param where Which function the error occurs in
 * @param missed Dictionary keys that have not been accessed
 *
 */
class UnaccessedDictionaryEntry : public KernelException
{
public:
  UnaccessedDictionaryEntry( const std::string& what, const std::string& where, const std::string& missed )
    : KernelException( "Unaccessed elements in " + what + ", in function " + where + ": " + missed )
  {
  }

  std::string
  exception_name() override
  {
    return "UnaccessedDictionaryEntry";
  }
};

/**
 * Exception to be thrown if a model with the the specified name
 * does not exist.
 * @see UnknownComponent
 * @ingroup KernelExceptions
 */
class UnknownModelName : public KernelException
{
  std::string compose_msg_( const std::string& model_name ) const;

public:
  UnknownModelName( const std::string& model_name )
    : KernelException( compose_msg_( model_name ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownModelName";
  }
};

/**
 * Exception to be thrown if a component with the the specified name
 * does not exist.
 * @see UnknownModelName
 * @ingroup KernelExceptions
 */
class UnknownComponent : public KernelException
{
  std::string compose_msg_( const std::string& model_name ) const;

public:
  explicit UnknownComponent( const std::string& component_name )
    : KernelException( compose_msg_( component_name ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownComponent";
  }
};

/**
 * Exception to be thrown if a name requested for a user-defined
 * model exist already.
 * @ingroup KernelExceptions
 */
class NewModelNameExists : public KernelException
{
  std::string compose_msg_( const std::string& model_name ) const;

public:
  NewModelNameExists( const std::string& model_name )
    : KernelException( compose_msg_( model_name ) )
  {
  }

  std::string
  exception_name() override
  {
    return "NewModelNameExists";
  }
};

/**
 * Exception to be thrown if a (neuron/synapse) model with the the specified ID
 * is used within the network and the providing module hence cannot be
 * uninstalled. This exception can occur if the user tries to uninstall a
 * module.
 * @ingroup KernelExceptions
 */
class ModelInUse : public KernelException
{
  std::string compose_msg_( const std::string& model_name ) const;

public:
  ModelInUse( const std::string& model_name )
    : KernelException( compose_msg_( model_name ) )
  {
  }

  std::string
  exception_name() override
  {
    return "ModelInUse";
  }
};

/**
 * Exception to be thrown if the specified
 * Synapse type does not exist.
 * @ingroup KernelExceptions
 */
class UnknownSynapseType : public KernelException
{
  std::string compose_msg_( const int id ) const;
  std::string compose_msg_( const std::string& name ) const;

public:
  UnknownSynapseType( int id )
    : KernelException( compose_msg_( id ) )
  {
  }

  UnknownSynapseType( std::string name )
    : KernelException( compose_msg_( name ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownSynapseType";
  }
};

/**
 * Exception to be thrown if the specified
 * Node does not exist.
 * This exception is thrown, if
 * -# an address did not point to an existing node.
 * -# a node id did not point to an existing node.
 * @ingroup KernelExceptions
 */
class UnknownNode : public KernelException
{
  std::string compose_msg_( const int id ) const;

public:
  UnknownNode()
    : KernelException( "UnknownNode" )
  {
  }

  UnknownNode( int id )
    : KernelException( compose_msg_( id ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownNode";
  }
};

/**
 * Exception to be thrown if the specified
 * Node does not exist.
 * This exception is thrown, if
 * -# an address did not point to an existing node.
 * -# a node id did not point to an existing node.
 * @ingroup KernelExceptions
 */
class NoThreadSiblingsAvailable : public KernelException
{
  std::string compose_msg_( const int id ) const;

public:
  NoThreadSiblingsAvailable()
    : KernelException( "UnknownNode" )
  {
  }

  NoThreadSiblingsAvailable( int id )
    : KernelException( compose_msg_( id ) )
  {
  }

  std::string
  exception_name() override
  {
    return "NoThreadSiblingsAvailable";
  }
};

class LocalNodeExpected : public KernelException
{
  std::string compose_msg_( const int id ) const;

public:
  LocalNodeExpected( int id )
    : KernelException( compose_msg_( id ) )
  {
  }

  std::string
  exception_name() override
  {
    return "LocalNodeExpected";
  }
};

class NodeWithProxiesExpected : public KernelException
{
  std::string compose_msg_( const int id ) const;

public:
  NodeWithProxiesExpected( int id )
    : KernelException( compose_msg_( id ) )
  {
  }

  std::string
  exception_name() override
  {
    return "NodeWithProxiesExpected";
  }
};

/*
 * Exception to be thrown if the parent
 * compartment does not exist
 */
class UnknownCompartment : public KernelException
{
  std::string compose_msg_( const long compartment_idx, const std::string info ) const;

public:
  UnknownCompartment( long compartment_idx, std::string info )
    : KernelException( compose_msg_( compartment_idx, info ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownCompartment";
  }
};

/**
 * Exception to be thrown if the specified
 * receptor type does not exist in the node.
 */
class UnknownReceptorType : public KernelException
{
  std::string compose_msg_( const long receptor_type, const std::string name ) const;

public:
  UnknownReceptorType( long receptor_type, std::string name )
    : KernelException( compose_msg_( receptor_type, name ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownReceptorType";
  }
};

/**
 * Exception to be thrown if the specified
 * receptor type does not accept the event type.
 */
class IncompatibleReceptorType : public KernelException
{
  std::string compose_msg( const long receptor_type, const std::string name, const std::string event );

public:
  IncompatibleReceptorType( long receptor_type, std::string name, std::string event_type )
    : KernelException( compose_msg( receptor_type, name, event_type ) )
  {
  }

  std::string
  exception_name() override
  {
    return "IncompatibleReceptorType";
  }
};

/**
 * To be thrown if a port does not exists.
 * This exception is thrown if a specified port (or r-port) number
 * was unknown at the specified node.
 * @ingroup KernelExceptions
 */
class UnknownPort : public KernelException
{
  std::string compose_msg_( const int id ) const;
  std::string compose_msg_( const int id, const std::string msg ) const;

public:
  UnknownPort( int id )
    : KernelException( compose_msg_( id ) )
  {
  }

  UnknownPort( int id, std::string msg )
    : KernelException( compose_msg_( id, msg ) )
  {
  }

  std::string
  exception_name() override
  {
    return "UnknownPort";
  }
};

/**
 * To be thrown if a connection is not possible.
 * This exception is e.g. thrown if a connection was attempted with
 * an unsupported Event type.
 * @ingroup KernelExceptions
 */
class IllegalConnection : public KernelException
{
public:
  IllegalConnection()
    : KernelException( "Creation of connection is not possible." )
  {
  }

  IllegalConnection( std::string msg )
    : KernelException( "Creation of connection is not possible because:\n" + msg )
  {
  }

  std::string
  exception_name() override
  {
    return "IllegalConnection";
  }
};

/**
 * To be thrown if a connection does not exists but something is to be done with
 * it.
 * This exception is e.g. thrown if a deletion was attempted with
 * an inexistent connection.
 * @ingroup KernelExceptions
 */
class InexistentConnection : public KernelException
{
private:
  std::string msg_;

public:
  InexistentConnection()
    : KernelException( "InexistentConnection" )
  {
    msg_ = "Deletion of connection is not possible because it does not exist.";
  }

  InexistentConnection( const std::string& msg )
    : KernelException( "InexistentConnection" )
  {
    msg_ = "Deletion of connection is not possible because:\n";
    msg_ += msg;
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "InexistentConnection";
  }
};

/**
 * Exception to be thrown if a thread id outside the range encountered.
 * @ingroup KernelExceptions
 */
class UnknownThread : public KernelException
{
private:
  std::string msg_;

public:
  UnknownThread( int id )
    : KernelException( "UnknownThread" )
  {
    msg_ = String::compose( "Thread with id %1 is outside of range.", id );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "UnknownThread";
  }
};

/**
 * Exception to be thrown if an invalid delay is used in a
 * connection.
 * @ingroup KernelExceptions
 */
class BadDelay : public KernelException
{
private:
  std::string msg_;

public:
  BadDelay( double delay, const std::string& msg )
    : KernelException( "BadDelay" )
  {
    msg_ = String::compose( "Delay value %1 is invalid: %2", delay, msg );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "BadDelay";
  }
};

/**
 * Exception to be thrown by the event handler
 * of a node if it receives an event it cannot handle.
 * This case should be prevented by connect_sender().
 * @ingroup KernelExceptions
 */
class UnexpectedEvent : public KernelException
{
private:
  std::string msg_;

public:
  UnexpectedEvent()
    : KernelException( "UnexpectedEvent" )
  {
    msg_ = "Target node cannot handle input event.\n";
    msg_ += "    A common cause for this is an attempt to connect recording devices incorrectly.\n";
    msg_ += "    Note that recorders such as spike recorders must be connected as\n\n";
    msg_ += "        nest.Connect(neurons, spike_det)\n\n";
    msg_ += "    while meters such as voltmeters must be connected as\n\n";
    msg_ += "        nest.Connect(meter, neurons) ";
  }

  UnexpectedEvent( const std::string& msg )
    : KernelException( "UnexpectedEvent" )
  {
    msg_ = msg;
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "UnexpectedEvent";
  }
};


/**
 * Exception to be thrown by a Connection object if
 * a connection with an unsupported event type is
 * attempted
 * @ingroup KernelExceptions
 */
class UnsupportedEvent : public KernelException
{
  std::string compose_msg_() const;

public:
  UnsupportedEvent()
    : KernelException( compose_msg_() )
  {
  }

  std::string
  exception_name() override
  {
    return "UnsupportedEvent";
  }
};

/**
 * Exception to be thrown if a status parameter
 * is incomplete or inconsistent.
 * Thrown by Node::set_/get_property methods.
 * @ingroup KernelExceptions
 */
class BadProperty : public KernelException
{
private:
  std::string msg_;

public:
  //! @param detailed error message
  BadProperty()
    : KernelException( "BadProperty" )
    , msg_()
  {
  }
  BadProperty( std::string msg )
    : KernelException( "BadProperty" )
    , msg_( msg )
  {
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "BadProperty";
  }
};

/**
 * Exception to be thrown if a parameter
 * cannot be set.
 * Thrown by Node::set_/get_property methods.
 * @ingroup KernelExceptions
 */
class BadParameter : public KernelException
{
private:
  std::string msg_;

public:
  //! @param detailed error message
  BadParameter()
    : KernelException( "BadParameter" )
    , msg_()
  {
  }
  BadParameter( std::string msg )
    : KernelException( "BadParameter" )
    , msg_( msg )
  {
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "BadParameter";
  }
};

/**
 * Exception to be thrown if a parameter value
 * is not acceptable.
 */
class BadParameterValue : public KernelException
{
  std::string msg_;

public:
  //! @param detailed error message
  BadParameterValue()
    : KernelException( "BadParameterValue" )
    , msg_()
  {
  }

  BadParameterValue( std::string msg )
    : KernelException( "BadParameterValue" )
    , msg_( msg )
  {
  }

  ~BadParameterValue() throw()
  {
  }

  std::string
  exception_name() override
  {
    return "BadParameterValue";
  }
};

/**
 * Exception to be thrown if the dimensions
 * of two or more objects do not agree.
 * Thrown by Node::set_/get_property methods.
 * @ingroup KernelExceptions
 */
class DimensionMismatch : public KernelException
{
private:
  std::string msg_;

public:
  DimensionMismatch()
    : KernelException( "DimensionMismatch" )
  {
    msg_ = "Dimensions of two or more variables do not match.";
  }

  DimensionMismatch( int expected, int provided )
    : KernelException( "DimensionMismatch" )
  {
    msg_ = String::compose( "Expected dimension size: %1 Provided dimension size: %2.", expected, provided );
  }

  DimensionMismatch( const std::string& msg )
    : KernelException( "DimensionMismatch" )
  {
    msg_ = msg;
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "DimensionMismatch";
  }
};

/**
 * Exception to be thrown if a problem with the
 * distribution of elements is encountered
 * @ingroup KernelExceptions
 */
class DistributionError : public KernelException
{
public:
  DistributionError()
    : KernelException( "DistributionError" )
  {
  }

  std::string
  exception_name() override
  {
    return "DistributionError";
  }
};

/**
 * Exception to be thrown on prototype construction if Time objects
 * incompatible. This exception is to be thrown by the default constructor of
 * nodes which require that Time objects have properties wrt resolution.
 * @ingroup KernelExceptions
 * @see InvalidTimeInModel
 */
class InvalidDefaultResolution : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param property  name of property conflicting
   * @param value     value of property conflicting
   */
  InvalidDefaultResolution( const std::string& model, const std::string& property, const Time& value )
    : KernelException( "InvalidDefaultResolution" )
  {
    std::ostringstream oss;
    oss << "The default resolution of " << Time::get_resolution() << " is not consistent with the value " << value
        << " of property '" << property << "' in model " << model << ".\n"
        << "This is an internal NEST error, please report it at https://github.com/nest/nest-simulator/issues";

    msg_ = oss.str();
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "InvalidDefaultResolution";
  }
};

/**
 * Exception to be thrown on instance construction if Time objects incompatible.
 * This exception is to be thrown by the copy constructor of nodes which
 * require that Time objects have properties wrt resolution.
 * @ingroup KernelExceptions
 * @see InvalidDefaultResolution
 */
class InvalidTimeInModel : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param property  name of property conflicting
   * @param value     value of property conflicting
   */
  InvalidTimeInModel( const std::string& model, const std::string& property, const Time& value )
    : KernelException( "InvalidTimeInModel" )
  {
    std::ostringstream oss;
    oss << "The time property " << property << " = " << value << " of model " << model
        << " is not compatible with the resolution " << Time::get_resolution() << ".\n"
        << "Please set a compatible value with SetDefaults!";

    msg_ = oss.str();
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "InvalidTimeInModel";
  }
};

/**
 * Exception to be thrown if a Time object should be multiple of the resolution.
 * @see TimeMultipleRequired
 * @ingroup KernelExceptions
 */
class StepMultipleRequired : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param property  name of property conflicting
   * @param value     value of property conflicting
   */
  StepMultipleRequired( const std::string& model, const std::string& property, const Time& value )
    : KernelException( "StepMultipleRequired" )
  {
    std::ostringstream oss;
    oss << "The time property " << property << " = " << value << " of model " << model
        << " must be a multiple of the resolution " << Time::get_resolution() << ".";

    msg_ = oss.str();
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "StepMultipleRequired";
  }
};

/**
 * Exception to be thrown if a Time object should be a multiple of another.
 * @see StepMultipleRequired
 * @ingroup KernelExceptions
 */
class TimeMultipleRequired : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model    name of model causing problem
   * @param name_a   name of dividend
   * @param value_a  value of dividend
   * @param name_b   name of divisor
   * @param value_b  value of divisor
   */
  TimeMultipleRequired( const std::string& model,
    const std::string& name_a,
    const Time& value_a,
    const std::string& name_b,
    const Time& value_b )
    : KernelException( "StepMultipleRequired" )
  {
    std::ostringstream oss;
    oss << "In model " << model << ", the time property " << name_a << " = " << value_a
        << " must be multiple of time property " << name_b << " = " << value_b << '.';

    msg_ = oss.str();
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "TimeMultipleRequired";
  }
};

/**
 * Exception to be thrown if a GSL solver does not return GSL_SUCCESS
 * @ingroup KernelExceptions
 */
class GSLSolverFailure : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model name of model causing problem
   * @param status exit status of the GSL solver
   */
  GSLSolverFailure( const std::string& model, const int status )
    : KernelException( "GSLSolverFailure" )
  {
    msg_ = String::compose(
      "In model %1 the GSL solver returned with exit status %2.\n"
      "Please make sure you have installed a recent GSL version (> gsl-1.10).",
      model,
      status );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "GSLSolverFailure";
  }
};

/**
 * Exception to be thrown if numerical instabilities are detected.
 * @ingroup KernelExceptions
 */
class NumericalInstability : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model name of model causing problem
   */
  NumericalInstability( const std::string& model )
    : KernelException( "NumericalInstability" )
  {
    msg_ = String::compose( "NEST detected a numerical instability while updating %1.", model );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "NumericalInstability";
  }
};

/**
 * Throw if an existing name is attempted to be redefined.
 * This is relevant mainly when a newly loaded module attempts to
 * redefine a model, synapse or function name.
 * @ingroup KernelExceptions
 */
class NamingConflict : public KernelException
{
private:
  std::string msg_;

public:
  NamingConflict( const std::string& msg )
    : KernelException( "NamingConflict" )
    , msg_( msg )
  {
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "NamingConflict";
  }
};

/**
 * Exception to be thrown if a given array has the wrong size.
 * @ingroup KernelExceptions
 */
class RangeCheck : public KernelException
{
private:
  std::string msg_;

public:
  RangeCheck( int size = 0 )
    : KernelException( "RangeCheck" )
  {
    if ( size > 0 )
    {
      msg_ = String::compose( "Array with length %1 expected.", size );
    }
    else
    {
      // TODO-PYNEST-NG: Fix usage, the comment below has been there already
      // Empty message. Added due to incorrect use of RangeCheck in nest.cpp
      msg_ = "";
    }
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "RangeCheck";
  }
};

/**
 * Exception to be thrown if an error occured in an I/O operation.
 * @ingroup KernelExceptions
 */
class IOError : public KernelException
{
private:
  std::string msg_;

public:
  IOError()
    : KernelException( "IOError" )
  {
    msg_ = "";
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "IOError";
  }
};

/**
 * Exception to be thrown if when trying to delete an entry from
 * DynamicRecordablesMap that does not exist.
 * @ingroup KernelExceptions
 */
class KeyError : public KernelException
{
private:
  std::string msg_;

public:
  KeyError( const std::string& key, const std::string& map_type, const std::string& map_op )
    : KernelException( "KeyError" )
  {
    msg_ = String::compose(
      "Key '%1' not found in map. Error encountered with map type: '%2' when applying operation: '%3'.",
      key,
      map_type,
      map_op );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "KeyError";
  }
};

/**
 * Exception to be thrown if an internal error occurs.
 * @ingroup KernelExceptions
 */
class InternalError : public KernelException
{
private:
  std::string msg_;

public:
  //! @param detailed error message
  InternalError()
    : KernelException()
    , msg_( "InternalError" )
  {
  }

  InternalError( std::string msg )
    : KernelException( msg )
  {
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "InternalError";
  }
};

#ifdef HAVE_MUSIC
/**
 * Exception to be thrown if a music_event_out_proxy is generated, but the music
 * port is unmapped.
 * @ingroup KernelExceptions
 */
class MUSICPortUnconnected : public KernelException
{
private:
  std::string msg;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of MUSIC port
   */
  MUSICPortUnconnected( const std::string& model, const std::string& portname )
    : KernelException( "MUSICPortUnconnected" )
  {
    msg_ = String::compose(
      "Cannot use instance of model %1 because the MUSIC "
      "port %2 is unconnected.",
      model,
      portname );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICPortUnconnected";
  }
};

/**
 * Exception to be thrown if a music_event_out_proxy is generated, but the
 * music port has no width.
 * @ingroup KernelExceptions
 */
class MUSICPortHasNoWidth : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of MUSIC port
   */
  MUSICPortHasNoWidth( const std::string& model, const std::string& portname )
    : KernelException( "MUSICPortHasNoWidth" )
  {
    msg_ = String::compose(
      "Cannot use instance of model %1 because the MUSIC "
      "port %2 has no width specified in configuration file.",
      model,
      portname );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICPortHasNoWidth";
  }
};

/**
 * Exception to be thrown if the user tries to change the name of an already
 * published port.
 * @ingroup KernelExceptions
 */
class MUSICPortAlreadyPublished : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of MUSIC port
   */
  MUSICPortAlreadyPublished( const std::string& model, const std::string& portname )
    : KernelException( "MUSICPortAlreadyPublished" )
  {
    msg_ = String::compose(
      "The instance of model %1 cannot change the MUSIC "
      "port / establish connections %2 since it is already published.",
      model,
      portname );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICPortAlreadyPublished";
  }
};

/**
 * Exception to be thrown if the user tries to change the name of an already
 * published port.
 * @ingroup KernelExceptions
 */
class MUSICSimulationHasRun : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   */
  MUSICSimulationHasRun( const std::string& model )
    : KernelException( "MUSICSimulationHasRun" )
  {
    msg_ = String::compose(
      "The instance of model %1 won't work, since the simulation "
      "has already been running",
      model );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICSimulationHasRun";
  }
};

/**
 * Exception to be thrown if the user tries to map a channel that exceeds the
 * width of the MUSIC port.
 * @ingroup KernelExceptions
 */
class MUSICChannelUnknown : public KernelException
{
private:
  std::string msg_;

public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of MUSIC port
   * @param channel   channel number
   */
  MUSICChannelUnknown( const std::string& model, const std::string& portname, int channel )
    : KernelException( "MUSICChannelUnknown" )
  {
    msg_ = String::compose(
      "The port %1 cannot be mapped in %2 because the channel %3 "
      "does not exist.",
      portname,
      model,
      channel );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICChannelUnknown";
  }
};

/**
 * Exception to be thrown if the user tries to use a port that is not known to
 * NEST.
 * @ingroup KernelExceptions
 */
class MUSICPortUnknown : public KernelException
{
private:
  std::string msg_;

  /**
   * @param portname  name of MUSIC port
   */
public:
  MUSICPortUnknown( const std::string& portname )
    : KernelException( "MUSICPortUnknown" )
  {
    msg_ = String::compose( "The port %1 does not exist.", portname );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICPortUnknown";
  }
};

/**
 * Exception to be thrown if the user tries to map a channel that exceeds the
 * width of the MUSIC port.
 * @ingroup KernelExceptions
 */
class MUSICChannelAlreadyMapped : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of MUSIC port
   * @param channel   channel number
   */
  MUSICChannelAlreadyMapped( const std::string& model, const std::string& portname, int channel )
    : KernelException( "MUSICChannelAlreadyMapped" )
    , portname_( portname )
    , channel_( channel )
    , model_( model )
  {
    msg_ = String::compose(
      "The channel %1 of port %2 has already be mapped "
      "to another proxy in %3.",
      channel,
      portname,
      model );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MUSICChannelAlreadyMapped";
  }
};
#endif // HAVE_MUSIC

#ifdef HAVE_MPI
class MPIPortsFileUnknown : public KernelException
{
private:
  std::string msg_;

public:
  explicit MPIPortsFileUnknown( const size_t node_id )
  {
    msg_ = String::compose(
      "The node with ID %1 requires a label, which specifies the "
      "folder with files containing the MPI ports.",
      node_id );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MPIPortsUnknown";
  }
};

class MPIPortsFileMissing : public KernelException
{
private:
  std::string msg_;

public:
  explicit MPIPortsFileMissing( const size_t node_id, const std::string path )
  {
    msg_ = String::compose(
      "The node with ID %1 expects a file with the MPI address at location %2. "
      "The file does not seem to exist.",
      node_id,
      path );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MPIPortsFileMissing";
  }
};

class MPIErrorCode : public KernelException
{
private:
  std::string msg_;
  std::string error_;
  char errmsg_[ 256 ];
  int len_;

public:
  explicit MPIErrorCode( const int error_code )
  {
    MPI_Error_string( error_code, errmsg_, &len_ );
    error_.assign( errmsg_, len_ );
    msg_ = String::compose( "MPI Error: %1", error_ );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "MPIErrorCode";
  }
};
#endif // HAVE_MPI

class UnmatchedSteps : public KernelException
{
private:
  std::string msg_;

public:
  UnmatchedSteps( int steps_left, int total_steps )
    : KernelException( "UnmatchedSteps" )
  {
    msg_ = String::compose(
      "Steps for backend device don't match NEST steps: "
      "steps expected: %1 steps executed: %2.",
      total_steps,
      total_steps - steps_left );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "UnmatchedSteps";
  }
};

class BackendPrepared : public KernelException
{
private:
  const std::string backend_;
  std::string msg_;

public:
  BackendPrepared( const std::string& backend )
    : KernelException( "BackendPrepared" )
    , backend_( backend )
  {
    msg_ = String::compose( "Backend %1 may not be prepare()'d multiple times.", backend_ );
  }

  BackendPrepared( std::string&& backend )
    : KernelException( "BackendPrepared" )
    , backend_( std::move( backend ) )
  {
    msg_ = String::compose( "Backend %1 may not be prepare()'d multiple times.", backend_ );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "BackendPrepared";
  }
};

class BackendAlreadyRegistered : public KernelException
{
private:
  const std::string backend_;
  std::string msg_;

public:
  BackendAlreadyRegistered( const std::string& backend )
    : KernelException( "BackendPrepared" )
    , backend_( backend )
  {
    msg_ = String::compose( "Backend %1 has already been registered.", backend_ );
  }

  BackendAlreadyRegistered( std::string&& backend )
    : KernelException( "BackendPrepared" )
    , backend_( std::move( backend ) )
  {
    msg_ = String::compose( "Backend %1 has already been registered.", backend_ );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "BackendAlreadyRegistered";
  }
};


class BackendNotPrepared : public KernelException
{
private:
  const std::string backend_;
  std::string msg_;

public:
  BackendNotPrepared( const std::string& backend )
    : KernelException( "BackendNotPrepared" )
    , backend_( backend )
  {
    msg_ = String::compose(
      "Backend %1 may not be cleanup()'d "
      "without preparation (multiple cleanups?).",
      backend_ );
  }

  BackendNotPrepared( std::string&& backend )
    : KernelException( "BackendNotPrepared" )
    , backend_( std::move( backend ) )
  {
    msg_ = String::compose(
      "Backend %1 may not be cleanup()'d "
      "without preparation (multiple cleanups?).",
      backend_ );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "BackendNotPrepared";
  }
};

class LayerExpected : public KernelException
{
public:
  LayerExpected()
    : KernelException( "LayerExpected" )
  {
  }

  std::string
  exception_name() override
  {
    return "LayerExpected";
  }
};

class LayerNodeExpected : public KernelException
{
public:
  LayerNodeExpected()
    : KernelException( "LayerNodeExpected" )
  {
  }

  std::string
  exception_name() override
  {
    return "LayerNodeExpected";
  }
};

class UndefinedName : public KernelException
{
private:
  std::string msg_;

public:
  UndefinedName( std::string name )
    : KernelException( "UndefinedName" )
  {
    msg_ = String::compose( "The name %1 is not defined.", name );
  }

  const char*
  what() const noexcept override
  {
    return msg_.data();
  };

  std::string
  exception_name() override
  {
    return "UndefinedName";
  }
};

} // namespace nest
#endif // EXCEPTIONS_H
