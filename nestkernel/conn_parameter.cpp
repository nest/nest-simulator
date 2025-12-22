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

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_names.h"


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
