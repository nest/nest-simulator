/*
 *  sliexceptions.h
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

#ifndef SLIEXCEPTIONS_H
#define SLIEXCEPTIONS_H

// C++ includes:
#include <iostream>
#include <string>
#include <vector>

// Includes from sli:
#include "name.h"

class SLIInterpreter;


#define UNKNOWN "unknown"
#define UNKNOWN_NUM -1

/**
 * @addtogroup Exceptions Exception classes
 * Exception classes that are thrown to indicate
 * an error.
 */

/**
 * @defgroup SLIExceptions SLI exception classes
 * Exception classes that are thrown by SLI to
 * indicate an error.
 * @ingroup Exceptions
 */


/**
 * Base class for all SLI exceptions.
 * @ingroup Exceptions
 * @ingroup SLIExceptions
 */
class SLIException : public std::exception
{
  std::string what_;

public:
  SLIException( char const* const what )
    : what_( what )
  {
  }

  SLIException( const std::string& what )
    : what_( what )
  {
  }

  ~SLIException() throw() override {};

  /**
   * Returns the SLI error name, used by raiseerror.
   *
   * Return name of the exception as C-style string.
   * Use this name to translate the exception to a SLI error.
   * For example:
   * @code
   * catch(IllegalOperation &e)
   * {
   *   i->error("SetStatus","Nodes must be a NodeCollection or SynapseCollection.");
   *   i->raiseerror(e.what());
   *   return;
   * }
   *@endcode
   *@note The catch clause must be terminated with a return
   *statement, if raiseerror was called.
   */
  const char*
  what() const throw() override
  {
    return what_.c_str();
  }

  /**
   * Returns a diagnostic message or empty string.
   */
  virtual std::string message() const = 0;
};

/**
 * Base class for all SLI interpreter exceptions.
 * @ingroup SLIExceptions
 */

class InterpreterError : public SLIException
{
public:
  ~InterpreterError() throw() override
  {
  }

  InterpreterError( char const* const what )
    : SLIException( what )
  {
  }
};

/**
 * Class for packaging exceptions thrown in threads.
 *
 * This class is used to wrap exceptions thrown in threads.
 * It essentially packages the message of the wrapped exception,
 * avoiding the need of a clone() operation for each exception type.
 */
class WrappedThreadException : public SLIException
{
public:
  WrappedThreadException( const std::exception& );
  ~WrappedThreadException() throw() override
  {
  }
  std::string
  message() const override
  {
    return message_;
  }

private:
  std::string message_;
};

class DivisionByZero : public SLIException
{
public:
  ~DivisionByZero() throw() override
  {
  }

  DivisionByZero()
    : SLIException( "DivisionByZero" )
  {
  }
  std::string message() const override;
};

// -------------------- Type Mismatch -------------------------
/**
 * Exception to be thrown if a given SLI type does not match the
 * expected type.
 * @ingroup SLIExceptions
 */

class TypeMismatch : public InterpreterError // SLIException
{
  std::string expected_;
  std::string provided_;

public:
  ~TypeMismatch() throw() override
  {
  }

  TypeMismatch()
    : InterpreterError( "TypeMismatch" )
  {
  }

  TypeMismatch( const std::string& expectedType )
    : InterpreterError( "TypeMismatch" )
    , expected_( expectedType )
  {
  }

  TypeMismatch( const std::string& expectedType, const std::string& providedType )
    : InterpreterError( "TypeMismatch" )
    , expected_( expectedType )
    , provided_( providedType )
  {
  }

  std::string message() const override;
};

class SystemSignal : public InterpreterError
{
  int signal_;

public:
  ~SystemSignal() throw() override
  {
  }
  SystemSignal( int s )
    : InterpreterError( "SystemSignal" )
    , signal_( s )
  {
  }

  std::string message() const override;
};

// -------------------- Array Size Mismatch -------------------------
/**
 * Exception to be thrown if a given SLI array has the wrong size.
 * @ingroup SLIExceptions
 */
class RangeCheck : public InterpreterError
{
  int size_;

public:
  ~RangeCheck() throw() override
  {
  }

  RangeCheck( int s = 0 )
    : InterpreterError( "RangeCheck" )
    , size_( s )
  {
  }

  std::string message() const override;
};

class ArgumentType : public InterpreterError
{
  int where; // Number of the parameter that was wrong.
public:
  ArgumentType( int w )
    : InterpreterError( "ArgumentType" )
    , where( w )
  {
  }

  std::string message() const override;
};

/**
 * Exception to be thrown if a parameter value
 * is not acceptable.
 */
class BadParameterValue : public SLIException
{
  std::string msg_;

public:
  //! @param detailed error message
  BadParameterValue()
    : SLIException( "BadParameterValue" )
    , msg_()
  {
  }

  BadParameterValue( std::string msg )
    : SLIException( "BadParameterValue" )
    , msg_( msg )
  {
  }

  ~BadParameterValue() throw() override
  {
  }

  std::string message() const override;
};

// -------------------- Dict Error -------------------------
/**
 * Base Class for all SLI errors related to dictionary processing.
 * @ingroup SLIExceptions
 */
class DictError : public InterpreterError
{
public:
  ~DictError() throw() override
  {
  }

  DictError( char const* const )
    : InterpreterError( "DictError" )
  {
  }
};

// -------------------- Unknown Name -------------------------
/**
 * Exception to be thrown if an entry referenced inside a
 * dictionary does not exist.
 * @ingroup SLIExceptions
 */
class UndefinedName : public DictError // was UnknownName
{
  std::string name_;

public:
  ~UndefinedName() throw() override
  {
  }
  UndefinedName( const std::string& name )
    : DictError( "UndefinedName" )
    , name_( name )
  {
  }

  std::string message() const override;
};

// -------------------- Entry Type Mismatch -------------------------
/**
 * Exception to be thrown if an entry referenced inside a
 * dictionary has the wrong type.
 * @ingroup SLIExceptions
 */
class EntryTypeMismatch : public DictError
{
  std::string expected_;
  std::string provided_;

public:
  ~EntryTypeMismatch() throw() override
  {
  }
  EntryTypeMismatch( const std::string& expectedType, const std::string& providedType )
    : DictError( "EntryTypeMismatch" )
    , expected_( expectedType )
    , provided_( providedType )
  {
  }

  std::string message() const override;
};

// -------------------- Stack Error -------------------------
/**
 * Exception to be thrown if an error occured while accessing the stack.
 * @ingroup SLIExceptions
 */
class StackUnderflow : public InterpreterError
{
  int needed;
  int given;

public:
  StackUnderflow( int n, int g )
    : InterpreterError( "StackUnderflow" )
    , needed( n )
    , given( g ) {};

  std::string message() const override;
};


// -------------------- I/O Error -------------------------
/**
 * Exception to be thrown if an error occured in an I/O operation.
 * @ingroup SLIExceptions
 */
class IOError : public SLIException
{
public:
  ~IOError() throw() override
  {
  }
  IOError()
    : SLIException( "IOError" )
  {
  }

  std::string message() const override;
};

/**
 * Exception to be thrown if unaccessed dictionary items are found.
 */
class UnaccessedDictionaryEntry : public DictError
{
  std::string msg_;

public:
  ~UnaccessedDictionaryEntry() throw() override
  {
  }
  // input: string with names of not accessed
  UnaccessedDictionaryEntry( const std::string& m )
    : DictError( "UnaccessedDictionaryEntry" )
    , msg_( m )
  {
  }

  std::string message() const override;
};


// ------------ Module related error --------------------------

/**
 * Exception to be thrown if an error occurs while
 * loading/unloading dynamic modules.
 * @ingroup SLIExceptions
 * @todo Shouldn't this be a KernelException?
 */
class DynamicModuleManagementError : public SLIException
{
  std::string msg_;

public:
  ~DynamicModuleManagementError() throw() override
  {
  }

  DynamicModuleManagementError()
    : SLIException( "DynamicModuleManagementError" )
    , msg_()
  {
  }

  DynamicModuleManagementError( std::string msg )
    : SLIException( "DynamicModuleManagementError" )
    , msg_( msg )
  {
  }

  std::string message() const override;
};

/**
 * Throw if an existing name is attempted to be redefined.
 * This is relevant mainly when a newly loaded module attempts to
 * redefine a model, synapse or function name.
 * @ingroup SLIExceptions
 */
class NamingConflict : public SLIException
{
  std::string msg_;

public:
  ~NamingConflict() throw() override
  {
  }
  NamingConflict( const std::string& m )
    : SLIException( "NamingConflict" )
    , msg_( m )
  {
  }

  std::string message() const override;
};

/**
 * Throw if an feature is unavailable.
 * @ingroup SLIExceptions
 */
class NotImplemented : public SLIException
{
  std::string msg_;

public:
  ~NotImplemented() throw() override
  {
  }
  NotImplemented( const std::string& m )
    : SLIException( "NotImplemented" )
    , msg_( m )
  {
  }

  std::string message() const override;
};

#endif
