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

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_names.h"


nest::ConnParameter*
nest::ConnParameter::create( const boost::any& value, const size_t nthreads )
{
  // single double
  if ( is_double( value ) )
  {
    return new ScalarDoubleParameter( boost::any_cast< double >( value ), nthreads );
  }

  // single integer
  if ( is_int( value ) )
  {
    return new ScalarIntegerParameter( boost::any_cast< int >( value ), nthreads );
  }

  // array of doubles
  if ( is_double_vector( value ) )
  {
    return new ArrayDoubleParameter( boost::any_cast< std::vector< double > >( value ), nthreads );
  }

  // Parameter
  if ( is_parameter( value ) )
  {
    return new ParameterConnParameterWrapper( boost::any_cast< std::shared_ptr< Parameter > >( value ), nthreads );
  }

  // array of integer
  if ( is_int_vector( value ) )
  {
    return new ArrayIntegerParameter( boost::any_cast< std::vector< int > >( value ), nthreads );
  }

  throw BadProperty( std::string( "Cannot handle parameter type. Received " ) + value.type().name() );
}


nest::ParameterConnParameterWrapper::ParameterConnParameterWrapper( std::shared_ptr< Parameter > p, const size_t )
  : parameter_( p )
{
}

double
nest::ParameterConnParameterWrapper::value_double( thread, RngPtr rng, index, Node* target ) const
{
  return parameter_->value( rng, target );
}
