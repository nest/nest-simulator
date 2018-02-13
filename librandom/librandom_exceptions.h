/*
 *  librandom_exceptions.h
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

#ifndef LIBRANDOM_EXCEPTIONS_H
#define LIBRANDOM_EXCEPTIONS_H

// Includes from sli:
#include "sliexceptions.h"

namespace librandom
{

/**
 * Exception to be thrown if no (suitable)
 * RNG is available.
 */
class MissingRNG : public SLIException
{
  std::string msg_;

public:
  //! @param detailed error message
  MissingRNG()
    : SLIException( "MissingRNG" )
    , msg_()
  {
  }

  MissingRNG( std::string msg )
    : SLIException( "MissingRNG" )
    , msg_( msg )
  {
  }

  ~MissingRNG() throw()
  {
  }

  std::string
  message() const
  {
    return msg_;
  }
};

/**
 * Exception to be thrown if no (suitable)
 * RNG is available.
 */
class UnsuitableRNG : public SLIException
{
  std::string msg_;

public:
  //! @param detailed error message
  UnsuitableRNG()
    : SLIException( "UnsuitableRNG" )
    , msg_()
  {
  }

  UnsuitableRNG( std::string msg )
    : SLIException( "UnsuitableRNG" )
    , msg_( msg )
  {
  }

  ~UnsuitableRNG() throw()
  {
  }

  std::string
  message() const
  {
    return msg_;
  }
};
}


#endif
