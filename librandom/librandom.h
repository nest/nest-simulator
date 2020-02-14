/*
 *  librandom.h
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

#ifndef LIBRANDOM_H
#define LIBRANDOM_H

// Generated includes:
#include "config.h"

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "exceptions.h"

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

namespace nest {

namespace random {

struct BaseRNG
{
  virtual int operator()() = 0;
  virtual BaseRNG* clone( long seed ) = 0;
  virtual double drand() = 0;
  virtual unsigned long ulrand(unsigned long N) = 0;
  virtual double min() = 0;
  virtual double max() = 0;
};

struct BaseRDist
{
  BaseRDist() = delete;
  BaseRDist( Name name )
    : name_( name )
  {
  }

  virtual void get_status( DictionaryDatum& ) const = 0;
  virtual void set_status( const DictionaryDatum& ) = 0;

  virtual BaseRDist* clone() = 0;

  virtual double drand()
  {
    std::string msg = String::compose( "Function drand() not implemented for distribution '%1'", name_ );
    throw KernelException( msg.c_str() );
  }

//TODO: lrand()
  virtual int irand()
  {
    std::string msg = String::compose( "Function irand() not implemented for distribution '%1'", name_ );
    throw KernelException( msg.c_str() );
  }

  enum class ResultType { continuous, discrete };

  virtual ResultType get_result_type() = 0;

protected:
    Name name_;
};

template < class RNG >
struct ClippedRedrawDist : BaseRDist
{
};

template < class RNG >
struct ClippedToBoundaryDist : BaseRDist
{
};

} // namespace random

} // namespace nest

#endif /* #ifndef LIBRANDOM_H */
