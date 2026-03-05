/*
 *  conn_parameter.cpp
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

#include "conn_parameter.h"

#include <boost/any.hpp>
#include <boost/type_index/type_index_facade.hpp>
#include <memory>
#include <string>

#include "dictionary.h"
#include "exceptions.h"

namespace nest
{
class Node;
} // namespace nest


nest::ConnParameter*
nest::ConnParameter::create( const boost::any& value, const size_t nthreads )
{
  // single double
  if ( is_type< double >( value ) )
  {
    return new ScalarDoubleParameter( boost::any_cast< double >( value ), nthreads );
  }

  // single integer
  if ( is_type< long >( value ) )
  {
    return new ScalarIntegerParameter( boost::any_cast< long >( value ), nthreads );
  }

  // array of doubles
  if ( is_type< std::vector< double > >( value ) )
  {
    return new ArrayDoubleParameter( boost::any_cast< std::vector< double > >( value ), nthreads );
  }

  // Parameter
  if ( is_type< std::shared_ptr< nest::Parameter > >( value ) )
  {
    return new ParameterConnParameterWrapper( boost::any_cast< ParameterPTR >( value ), nthreads );
  }

  // array of longs
  if ( is_type< std::vector< long > >( value ) )
  {
    return new ArrayLongParameter( boost::any_cast< std::vector< long > >( value ), nthreads );
  }

  throw BadProperty( std::string( "Cannot handle parameter type. Received " ) + value.type().name() );
}


nest::ParameterConnParameterWrapper::ParameterConnParameterWrapper( ParameterPTR p, const size_t )
  : parameter_( p )
{
}

double
nest::ParameterConnParameterWrapper::value_double( size_t, RngPtr rng, size_t, Node* target ) const
{
  return parameter_->value( rng, target );
}

void
nest::ArrayDoubleParameter::skip( size_t tid, size_t n_skip ) const
{
  if ( next_[ tid ] < values_.end() )
  {
    next_[ tid ] += n_skip;
  }
  else
  {
    throw KernelException( "Parameter values exhausted." );
  }
}

double
nest::ArrayDoubleParameter::value_double( size_t tid, RngPtr, size_t, Node* ) const
{
  if ( next_[ tid ] != values_.end() )
  {
    return *next_[ tid ]++;
  }
  else
  {
    throw KernelException( "Parameter values exhausted." );
  }
}

long
nest::ArrayDoubleParameter::value_int( size_t, RngPtr, size_t, Node* ) const
{
  throw KernelException( "ConnParameter calls value function with false return type." );
}

void
nest::ArrayDoubleParameter::reset() const
{
  for ( std::vector< std::vector< double >::const_iterator >::iterator it = next_.begin(); it != next_.end(); ++it )
  {
    *it = values_.begin();
  }
}

void
nest::ArrayLongParameter::skip( size_t tid, size_t n_skip ) const
{
  if ( next_[ tid ] < values_.end() )
  {
    next_[ tid ] += n_skip;
  }
  else
  {
    throw KernelException( "Parameter values exhausted." );
  }
}

long
nest::ArrayLongParameter::value_int( size_t tid, RngPtr, size_t, Node* ) const
{
  if ( next_[ tid ] != values_.end() )
  {
    return *next_[ tid ]++;
  }
  else
  {
    throw KernelException( "Parameter values exhausted." );
  }
}

double
nest::ArrayLongParameter::value_double( size_t tid, RngPtr, size_t, Node* ) const
{
  if ( next_[ tid ] != values_.end() )
  {
    return static_cast< double >( *next_[ tid ]++ );
  }
  else
  {
    throw KernelException( "Parameter values exhausted." );
  }
}

void
nest::ArrayLongParameter::reset() const
{
  for ( std::vector< std::vector< long >::const_iterator >::iterator it = next_.begin(); it != next_.end(); ++it )
  {
    *it = values_.begin();
  }
}
namespace nest
{

// ---- Base class: defaulted specials & defaults ----
ConnParameter::ConnParameter()
{
}
ConnParameter::~ConnParameter()
{
}

void
ConnParameter::skip( size_t, size_t ) const
{
}
bool
ConnParameter::is_scalar() const
{
  return false;
}
bool
ConnParameter::provides_long() const
{
  return false;
}
void
ConnParameter::reset() const
{
  throw NotImplemented( "Symmetric connections require parameters that can be reset." );
}
size_t
ConnParameter::number_of_values() const
{
  return 0;
}

// ---- ScalarDoubleParameter ----
ScalarDoubleParameter::ScalarDoubleParameter( double value, const size_t )
  : value_( value )
{
}
double
ScalarDoubleParameter::value_double( size_t, RngPtr, size_t, Node* ) const
{
  return value_;
}
long
ScalarDoubleParameter::value_int( size_t, RngPtr, size_t, Node* ) const
{
  throw KernelException( "ConnParameter calls value function with false return type." );
}
bool
ScalarDoubleParameter::is_array() const
{
  return false;
}
void
ScalarDoubleParameter::reset() const
{
}
bool
ScalarDoubleParameter::is_scalar() const
{
  return true;
}

// ---- ScalarIntegerParameter ----
ScalarIntegerParameter::ScalarIntegerParameter( long value, const size_t )
  : value_( value )
{
}
double
ScalarIntegerParameter::value_double( size_t, RngPtr, size_t, Node* ) const
{
  return static_cast< double >( value_ );
}
long
ScalarIntegerParameter::value_int( size_t, RngPtr, size_t, Node* ) const
{
  return value_;
}
bool
ScalarIntegerParameter::is_array() const
{
  return false;
}
void
ScalarIntegerParameter::reset() const
{
}
bool
ScalarIntegerParameter::is_scalar() const
{
  return true;
}
bool
ScalarIntegerParameter::provides_long() const
{
  return true;
}

// ---- ArrayDoubleParameter ----
ArrayDoubleParameter::ArrayDoubleParameter( const std::vector< double >& values, const size_t nthreads )
  : values_( values )
  , next_( nthreads, values_.begin() )
{
}

size_t
ArrayDoubleParameter::number_of_values() const
{
  return values_.size();
}

bool
ArrayDoubleParameter::is_array() const
{
  return true;
}

// ---- ArrayLongParameter ----
ArrayLongParameter::ArrayLongParameter( const std::vector< long >& values, const size_t nthreads )
  : values_( values )
  , next_( nthreads, values_.begin() )
{
}

size_t
ArrayLongParameter::number_of_values() const
{
  return values_.size();
}


bool
ArrayLongParameter::is_array() const
{
  return true;
}
bool
ArrayLongParameter::provides_long() const
{
  return true;
}


// ---- ParameterConnParameterWrapper ----
long
ParameterConnParameterWrapper::value_int( size_t tt, RngPtr rng, size_t sn, Node* tgt ) const
{
  return static_cast< long >( value_double( tt, rng, sn, tgt ) );
}
bool
ParameterConnParameterWrapper::is_array() const
{
  return false;
}
bool
ParameterConnParameterWrapper::provides_long() const
{
  return parameter_->returns_int_only();
}

} // namespace nest
