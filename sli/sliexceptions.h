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

  virtual ~SLIException() throw(){};

  /**
   * Returns the SLI error name, used by raiseerror.
   *
   * Return name of the exception as C-style string.
   * Use this name to translate the exception to a SLI error.
   * For example:
   * @code
   * catch(IllegalOperation &e)
   * {
   *   i->error("ChangeSubnet","Target node must be a subnet.");
   *   i->raiseerror(e.what());
   *   return;
   * }
   *@endcode
   *@note The catch clause must be terminated with a return
   *statement, if raiseerror was called.
   */
  virtual const char*
  what() const throw()
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
  virtual ~InterpreterError() throw()
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
  virtual ~WrappedThreadException() throw()
  {
  }
  std::string
  message() const
  {
    return message_;
  }

private:
  std::string message_;
};

class DivisionByZero : public SLIException
{
public:
  virtual ~DivisionByZero() throw()
  {
  }

  DivisionByZero()
    : SLIException( "DivisionByZero" )
  {
  }
  std::string message() const;
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
  ~TypeMismatch() throw()
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

  std::string message() const;
};

class SystemSignal : public InterpreterError
{
  int signal_;

public:
  ~SystemSignal() throw()
  {
  }
  SystemSignal( int s )
    : InterpreterError( "SystemSignal" )
    , signal_( s )
  {
  }

  std::string message() const;
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
  ~RangeCheck() throw()
  {
  }

  RangeCheck( int s = 0 )
    : InterpreterError( "RangeCheck" )
    , size_( s )
  {
  }

  std::string message() const;
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

  std::string message() const;
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

  ~BadParameterValue() throw()
  {
  }

  std::string message() const;
};

// -------------------- Dict Error -------------------------
/**
 * Base Class for all SLI errors related to dictionary processing.
 * @ingroup SLIExceptions
 */
class DictError : public InterpreterError
{
public:
  virtual ~DictError() throw()
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
  ~UndefinedName() throw()
  {
  }
  UndefinedName( const std::string& name )
    : DictError( "UndefinedName" )
    , name_( name )
  {
  }

  std::string message() const;
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
  ~EntryTypeMismatch() throw()
  {
  }
  EntryTypeMismatch( const std::string& expectedType, const std::string& providedType )
    : DictError( "EntryTypeMismatch" )
    , expected_( expectedType )
    , provided_( providedType )
  {
  }

  std::string message() const;
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
    , given( g ){};

  std::string message() const;
};


// -------------------- I/O Error -------------------------
/**
 * Exception to be thrown if an error occured in an I/O operation.
 * @ingroup SLIExceptions
 */
class IOError : public SLIException
{
public:
  ~IOError() throw()
  {
  }
  IOError()
    : SLIException( "IOError" )
  {
  }

  std::string message() const;
};

/**
 * Exception to be thrown if unaccessed dictionary items are found.
 */
class UnaccessedDictionaryEntry : public DictError
{
  std::string msg_;

public:
  ~UnaccessedDictionaryEntry() throw()
  {
  }
  // input: string with names of not accessed
  UnaccessedDictionaryEntry( const std::string& m )
    : DictError( "UnaccessedDictionaryEntry" )
    , msg_( m )
  {
  }

  std::string message() const;
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
  ~DynamicModuleManagementError() throw()
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

  std::string message() const;
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
  ~NamingConflict() throw()
  {
  }
  NamingConflict( const std::string& m )
    : SLIException( "NamingConflict" )
    , msg_( m )
  {
  }

  std::string message() const;
};

/**
 * Throw if an feature is unavailable.
 * @ingroup SLIExceptions
 */
class NotImplemented : public SLIException
{
  std::string msg_;

public:
  ~NotImplemented() throw()
  {
  }
  NotImplemented( const std::string& m )
    : SLIException( "NotImplemented" )
    , msg_( m )
  {
  }

  std::string message() const;
};

#endif
