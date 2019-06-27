/*
 *  sliexceptions.cc
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

#include "sliexceptions.h"

// C++ includes:
#include <sstream>

// Generated includes:
#include "config.h"

// Includes from sli:
#include "interpret.h"

WrappedThreadException::WrappedThreadException( const std::exception& exc )
  : SLIException( exc.what() )
{
  SLIException const* se = dynamic_cast< SLIException const* >( &exc );
  if ( se )
  {
    message_ = se->message();
  }
  else
  {
    message_ = std::string( "C++ exception: " ) + exc.what();
  }
}

std::string
DivisionByZero::message() const
{
  return "You cannot divide by zero.";
}

std::string
TypeMismatch::message() const
{
  if ( not provided_.empty() && not expected_.empty() )
  {
    return "Expected datatype: " + expected_ + "\nProvided datatype: " + provided_;
  }
  else if ( not expected_.empty() )
  {
    return "Expected datatype: " + expected_;
  }
  else
  {
    return "The expected datatype is unknown in the current context.";
  }
}

std::string
RangeCheck::message() const
{
  if ( size_ > 0 )
  {
    std::ostringstream out;
    out << "Array with length " << size_ << " expected.";
    return out.str();
  }
  else
  {
    // Empty message.
    // Added due to incorrect use of RangeCheck
    // in nestmodule.cpp
    return std::string();
  }
}

std::string
ArgumentType::message() const
{
  std::ostringstream out;

  out << "The type of";
  if ( where )
  {
    out << " the ";
    if ( where == 1 )
    {
      out << "first";
    }
    else if ( where == 2 )
    {
      out << "second";
    }
    else if ( where == 3 )
    {
      out << "third";
    }
    else
    {
      out << where << "th";
    }
    out << " parameter";
  }
  else
  {
    out << " one or more parameters";
  }
  out << " did not match the argument(s) of this function.";

  return out.str();
}

std::string
BadParameterValue::message() const
{
  return msg_;
}


std::string
UndefinedName::message() const
{
  return "Key '/" + name_ + "' does not exist in dictionary.";
}

std::string
EntryTypeMismatch::message() const
{
  return "Expected datatype: " + expected_ + "\nProvided datatype: " + provided_;
}

std::string
StackUnderflow::message() const
{
  std::ostringstream out;
  if ( needed )
  {
    out << "Command needs (at least) " << needed << " argument(s)";
    if ( given )
    {
      out << ", but the stack has only " << given;
    }
    out << ".";
  }
  else
  {
    out << "Command needs more arguments";
    if ( given )
    {
      out << "than " << given;
    }
    out << ".";
  }

  return out.str();
}

std::string
IOError::message() const
{
  return std::string();
}

std::string
SystemSignal::message() const
{
  std::ostringstream out;
  out << "The operation was interrupted by the system signal " << signal_ << ".";
  return out.str();
}

std::string
UnaccessedDictionaryEntry::message() const
{
  return "Unused dictionary items: " + msg_;
}

std::string
DynamicModuleManagementError::message() const
{
  if ( msg_.empty() )
  {
    return "Unloading of dynamic modules is not implemented yet.";
  }
  else
  {
    return msg_;
  }
}

std::string
NamingConflict::message() const
{
  return msg_;
}

std::string
NotImplemented::message() const
{
  return msg_;
}
