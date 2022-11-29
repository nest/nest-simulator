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

// Includes from nestkernel:
#include "nest_time.h"

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
 * a user error.
 *
 * @ingroup Exceptions
 */

/**
 * Base class for all Kernel exceptions.
 * @ingroup Exceptions
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
};

/**
 * Class for packaging exceptions thrown in threads.
 *
 * This class is used to wrap exceptions thrown in threads.
 * It essentially packages the message of the wrapped exception,
 * avoiding the need of a clone() operation for each exception type.
 * @ingroup KernelExceptions
 */
class WrappedThreadException : public KernelException
{
public:
  explicit WrappedThreadException( const std::exception& e )
    : KernelException( e.what() )
  {
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
    : KernelException( "unaccessed elements in " + what + ", in function " + where + ": " + missed )
  {
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
};

class LocalNodeExpected : public KernelException
{
  std::string compose_msg_( const int id ) const;

public:
  LocalNodeExpected( int id )
    : KernelException( compose_msg_( id ) )
  {
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
};

/**
 * Exception to be thrown if the specified
 * receptor type does not accept the event type.
 */

class IncompatibleReceptorType : public KernelException
{
  std::string compose_msg( const long receptor_type, const std::string name, const std::string event);

public:
  IncompatibleReceptorType( long receptor_type, std::string name, std::string event_type )
    : KernelException( compose_msg( receptor_type, name, event_type ) )
  {
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
    : KernelException( "IllegalConnection" )
    , msg_()
  {
  }

  IllegalConnection( std::string msg )
    : KernelException( "IllegalConnection" )
    , msg_( msg )
  {
  }

  const char* what() const noexcept override;

private:
  std::string msg_;
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
public:
  InexistentConnection()
    : KernelException( "The connection does not exist" )
    , msg_()
  {
  }

  InexistentConnection( std::string msg )
    : KernelException( "The connection does not exist" )
    , msg_( msg )
  {
  }

  const char* what() const noexcept override;

private:
  std::string msg_;
};

/**
 * Exception to be thrown if a thread id outside the range encountered.
 * @ingroup KernelExceptions
 */
class UnknownThread : public KernelException
{
  int id_;

public:
  UnknownThread( int id )
    : KernelException( "UnknownThread" )
    , id_( id )
  {
  }

  const char* what() const noexcept override;
};

/**
 * Exception to be thrown if an invalid delay is used in a
 * connection.
 * @ingroup KernelExceptions
 */
class BadDelay : public KernelException
{
  double delay_;
  std::string message_;

public:
  BadDelay( double delay, std::string message )
    : KernelException( "BadDelay" )
    , delay_( delay )
    , message_( message )
  {
  }

  const char* what() const noexcept override;
};

/**
 * Exception to be thrown by the event handler
 * of a node if it receives an event it cannot handle.
 * This case should be prevented by connect_sender().
 * @ingroup KernelExceptions
 */
class UnexpectedEvent : public KernelException
{
public:
  UnexpectedEvent()
    : KernelException( "UnexpectedEvent" )
  {
  }

  UnexpectedEvent( std::string msg )
    : KernelException( "UnexpectedEvent" )
    , msg_( msg )
  {
  }

  const char* what() const noexcept override;

private:
  std::string msg_;
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
};

/**
 * Exception to be thrown if a status parameter
 * is incomplete or inconsistent.
 * Thrown by Node::set_/get_property methods.
 * @ingroup KernelExceptions
 */
class BadProperty : public KernelException
{
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

  const char* what() const noexcept override;
};

/**
 * Exception to be thrown if a parameter
 * cannot be set.
 * Thrown by Node::set_/get_property methods.
 * @ingroup KernelExceptions
 */
class BadParameter : public KernelException
{
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

  const char* what() const noexcept override;
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

  std::string message() const;
};

/**
 * Exception to be thrown if the dimensions
 * of two or more objects do not agree.
 * Thrown by Node::set_/get_property methods.
 * @ingroup KernelExceptions
 */
class DimensionMismatch : public KernelException
{
  int expected_;
  int provided_;
  std::string msg_;

public:
  DimensionMismatch()
    : KernelException( "DimensionMismatch" )
    , expected_( -1 )
    , provided_( -1 )
    , msg_( "" )
  {
  }

  DimensionMismatch( int expected, int provided )
    : KernelException( "DimensionMismatch" )
    , expected_( expected )
    , provided_( provided )
    , msg_( "" )
  {
  }

  DimensionMismatch( const std::string& msg )
    : KernelException( "DimensionMismatch" )
    , expected_( -1 )
    , provided_( -1 )
    , msg_( msg )
  {
  }

 const char* what() const noexcept override;
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
    , model_( model )
    , prop_( property )
    , val_( value )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string prop_;
  const Time val_;
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
    , model_( model )
    , prop_( property )
    , val_( value )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string prop_;
  const Time val_;
};

/**
 * Exception to be thrown if a Time object should be multiple of the resolution.
 * @see TimeMultipleRequired
 * @ingroup KernelExceptions
 */
class StepMultipleRequired : public KernelException
{
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
    , model_( model )
    , prop_( property )
    , val_( value )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string prop_;
  const Time val_;
};

/**
 * Exception to be thrown if a Time object should be a multiple of another.
 * @see StepMultipleRequired
 * @ingroup KernelExceptions
 */
class TimeMultipleRequired : public KernelException
{
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
    , model_( model )
    , prop_a_( name_a )
    , val_a_( value_a )
    , prop_b_( name_b )
    , val_b_( value_b )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string prop_a_;
  const Time val_a_;
  const std::string prop_b_;
  const Time val_b_;
};

/**
 * Exception to be thrown if a GSL solver does not return GSL_SUCCESS
 * @ingroup KernelExceptions
 */
class GSLSolverFailure : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model name of model causing problem
   * @param status exit status of the GSL solver
   */
  GSLSolverFailure( const std::string& model, const int status )
    : KernelException( "GSLSolverFailure" )
    , model_( model )
    , status_( status )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const int status_;
};

/**
 * Exception to be thrown if numerical instabilities are detected.
 * @ingroup KernelExceptions
 */
class NumericalInstability : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model name of model causing problem
   */
  NumericalInstability( const std::string& model )
    : KernelException( "NumericalInstability" )
    , model_( model )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
};

/**
 * Exception to be thrown if when trying to delete an entry from
 * DynamicRecordablesMap that does not exist.
 * @ingroup KernelExceptions
 */
class KeyError : public KernelException
{
  const std::string key_;
  const std::string map_type_;
  const std::string map_op_;

public:
  KeyError( const std::string& key, const std::string& map_type, const std::string& map_op )
    : KernelException( "KeyError" )
    , key_( key )
    , map_type_( map_type )
    , map_op_( map_op )
  {
  }

  const char* what() const noexcept override;
};

/**
 * Exception to be thrown if an internal error occurs.
 * @ingroup KernelExceptions
 */
class InternalError : public KernelException
{
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

  const char* what() const noexcept override;
};


#ifdef HAVE_MUSIC
/**
 * Exception to be thrown if a music_event_out_proxy is generated, but the music
 * port is unmapped.
 * @ingroup KernelExceptions
 */
class MUSICPortUnconnected : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of MUSIC port
   */
  MUSICPortUnconnected( const std::string& model, const std::string& portname )
    : KernelException( "MUSICPortUnconnected" )
    , model_( model )
    , portname_( portname )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string portname_;
};

/**
 * Exception to be thrown if a music_event_out_proxy is generated, but the
 * music port has no width.
 * @ingroup KernelExceptions
 */
class MUSICPortHasNoWidth : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   * @param portname  name of music port
   */
  MUSICPortHasNoWidth( const std::string& model, const std::string& portname )
    : KernelException( "MUSICPortHasNoWidth" )
    , model_( model )
    , portname_( portname )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string portname_;
};


/**
 * Exception to be thrown if the user tries to change the name of an already
 * published port.
 * @ingroup KernelExceptions
 */
class MUSICPortAlreadyPublished : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   */
  MUSICPortAlreadyPublished( const std::string& model, const std::string& portname )
    : KernelException( "MUSICPortAlreadyPublished" )
    , model_( model )
    , portname_( portname )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
  const std::string portname_;
};

/**
 * Exception to be thrown if the user tries to change the name of an already
 * published port.
 * @ingroup KernelExceptions
 */
class MUSICSimulationHasRun : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   */
  MUSICSimulationHasRun( const std::string& model )
    : KernelException( "MUSICSimulationHasRun" )
    , model_( model )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string model_;
};


/**
 * Exception to be thrown if the user tries to map a channel that exceeds the
 * width of the MUSIC port.
 * @ingroup KernelExceptions
 */
class MUSICChannelUnknown : public KernelException
{
public:
  /**
   * @note model should be passed from get_name() to ensure that
   *             names of copied models are reported correctly.
   * @param model     name of model causing problem
   */
  MUSICChannelUnknown( const std::string& model, const std::string& portname, int channel )
    : KernelException( "MUSICChannelUnknown" )
    , portname_( portname )
    , channel_( channel )
    , model_( model )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string portname_;
  const int channel_;
  const std::string model_;
};

/**
 * Exception to be thrown if the user tries to use a port that is not known to
 * NEST.
 * @ingroup KernelExceptions
 */
class MUSICPortUnknown : public KernelException
{
public:
  MUSICPortUnknown( const std::string& portname )
    : KernelException( "MUSICPortUnknown" )
    , portname_( portname )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string portname_;
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
   */
  MUSICChannelAlreadyMapped( const std::string& model, const std::string& portname, int channel )
    : KernelException( "MUSICChannelAlreadyMapped" )
    , portname_( portname )
    , channel_( channel )
    , model_( model )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string portname_;
  const int channel_;
  const std::string model_;
};

#endif

#ifdef HAVE_MPI
class MPIPortsFileUnknown : public KernelException
{
public:
  explicit MPIPortsFileUnknown( const index node_id )
    : node_id_( node_id )
  {
  }

  const char* what() const noexcept override;

private:
  const index node_id_;
};
#endif

class UnmatchedSteps : public KernelException
{
public:
  UnmatchedSteps( int steps_left, int total_steps )
    : current_step_( total_steps - steps_left )
    , total_steps_( total_steps )
  {
  }

  const char* what() const noexcept override;

private:
  const int current_step_;
  const int total_steps_;
};

class BackendPrepared : public KernelException
{
public:
  BackendPrepared( const std::string& backend )
    : backend_( backend )
  {
  }

  BackendPrepared( std::string&& backend )
    : backend_( std::move( backend ) )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string backend_;
};

class BackendNotPrepared : public KernelException
{
public:
  BackendNotPrepared( const std::string& backend )
    : backend_( backend )
  {
  }

  BackendNotPrepared( std::string&& backend )
    : backend_( std::move( backend ) )
  {
  }

  const char* what() const noexcept override;

private:
  const std::string backend_;
};

class LayerExpected : public KernelException
{
public:
  LayerExpected()
    : KernelException( "LayerExpected" )
  {
  }
};

class LayerNodeExpected : public KernelException
{
public:
  LayerNodeExpected()
    : KernelException( "LayerNodeExpected" )
  {
  }
};

class UndefinedName : public KernelException
{
  const std::string name_;

public:
  UndefinedName( std::string name )
    : KernelException( "UndefinedName" )
    , name_( name )
  {
  }

  const char* what() const noexcept override;
};

} // namespace nest

#endif
