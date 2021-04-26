/*
 *  conn_parameter.h
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

#ifndef CONN_PARAMETER_H
#define CONN_PARAMETER_H

// C++ includes:
#include <limits>
#include <vector>

// Includes from nestkernel:
#include "exceptions.h"
#include "parameter.h"
#include "nest_datums.h"

// Includes from sli:
#include "token.h"

/**
 * Base class for parameters provided to connection routines.
 *
 * Principles for these parameters are
 * - Each parameter is a single scalar value.
 * - The parameter will be returned as type double.
 * - The parameter values can be given either as
 *   - a single scalar: the same value is returned for each call
 *   - a random deviate generator: a new random values is returned for each call
 *   - an array of scalars: values are returned in order
 */

namespace nest
{

class ConnParameter
{

public:
  ConnParameter()
  {
  }

  virtual ~ConnParameter()
  {
  }

  /**
   * Return parameter value.
   *
   * The parameter value may depend on target threads
   * and random numbers. Both must be supplied, even if
   * a concrete parameter type does not use them.
   *
   * @param target_thread  will be ignored except for array parameters.
   * @param rng   random number generator pointer
   * will be ignored except for random parameters.
   */
  virtual double value_double( thread, RngPtr, index, Node* ) const = 0;
  virtual long value_int( thread, RngPtr, index, Node* ) const = 0;
  virtual void skip( thread, size_t ) const
  {
  }
  virtual bool is_array() const = 0;

  virtual bool
  is_scalar() const
  {
    return false;
  }

  virtual bool
  provides_long() const
  {
    return false;
  }

  virtual void
  reset() const
  {
    throw NotImplemented( "Symmetric connections require parameters that can be reset." );
  }

  /**
   * Returns number of values available.
   *
   * 0 indicates scalar/unlimited supply.
   */
  virtual size_t
  number_of_values() const
  {
    return 0;
  }

  /**
  * @param t parameter
  * type is established by casts to all acceptedpossibilities
  * @param nthread number of threads
  * required to fix number pointers to the iterator (one for each thread)
  */
  static ConnParameter* create( const Token&, const size_t );
};


/**
 * Single double value.
 *
 * On each request, it returns the same value.
 */
class ScalarDoubleParameter : public ConnParameter
{
public:
  ScalarDoubleParameter( double value, const size_t )
    : value_( value )
  {
  }

  double
  value_double( thread, RngPtr, index, Node* ) const
  {
    return value_;
  }

  long
  value_int( thread, RngPtr, index, Node* ) const
  {
    throw KernelException( "ConnParameter calls value function with false return type." );
  }

  inline bool
  is_array() const
  {
    return false;
  }

  void
  reset() const
  {
  }

  bool
  is_scalar() const
  {
    return true;
  }

private:
  double value_;
};

/**
 * Single integer value.
 *
 * On each request, it returns the same value.
 */
class ScalarIntegerParameter : public ConnParameter
{
public:
  ScalarIntegerParameter( long value, const size_t )
    : value_( value )
  {
  }

  double
  value_double( thread, RngPtr, index, Node* ) const
  {
    return static_cast< double >( value_ );
  }

  long
  value_int( thread, RngPtr, index, Node* ) const
  {
    return value_;
  }

  inline bool
  is_array() const
  {
    return false;
  }

  void
  reset() const
  {
  }

  bool
  is_scalar() const
  {
    return true;
  }

  bool
  provides_long() const
  {
    return true;
  }

private:
  long value_;
};


/**
 * Array parameter classes, returning double values in order.
 *
 * - The array of values must not be empty
 *   (so return 0 for number_of_values can signal non-array parameter)
 * - Throws exception if more values requested than available.
 * - The class contains nthread number of pointers (one for each thread)
 *   to an iterator, which runs over the parameters initialised in an array.
 *   Each pointer is moved along the parameter array by the function
 *   value_double(), which returns the current parameter value and moves the
 *   pointer to the subsequent position.
 * - All parameters are  doubles, thus calling the function value_int()
 *   throws an error.
 */

class ArrayDoubleParameter : public ConnParameter
{
public:
  ArrayDoubleParameter( const std::vector< double >& values, const size_t nthreads )
    : values_( &values )
    , next_( nthreads, values_->begin() )
  {
  }

  void
  skip( thread tid, size_t n_skip ) const
  {
    if ( next_[ tid ] < values_->end() )
    {
      next_[ tid ] += n_skip;
    }
    else
    {
      throw KernelException( "Parameter values exhausted." );
    }
  }

  size_t
  number_of_values() const
  {
    return values_->size();
  }

  double
  value_double( thread tid, RngPtr, index, Node* ) const
  {
    if ( next_[ tid ] != values_->end() )
    {
      return *next_[ tid ]++;
    }
    else
    {
      throw KernelException( "Parameter values exhausted." );
    }
  }

  long
  value_int( thread, RngPtr, index, Node* ) const
  {
    throw KernelException( "ConnParameter calls value function with false return type." );
  }

  inline bool
  is_array() const
  {
    return true;
  }

  void
  reset() const
  {
    for ( std::vector< std::vector< double >::const_iterator >::iterator it = next_.begin(); it != next_.end(); ++it )
    {
      *it = values_->begin();
    }
  }

private:
  const std::vector< double >* values_;
  mutable std::vector< std::vector< double >::const_iterator > next_;
};

/**
 * Array parameter classes, returning integer values in order.
 *
 * - The array of values must not be empty
 *   (so return 0 for number_of_values can signal non-array parameter)
 * - Throws exception if more values requested than available.
 * - The class contains nthread number of pointers (one for each thread)
 *   to an iterator, which runs over the parameters initialised in an array.
 *   Each pointer is moved along the parameter array by the function
 *   value_int(), which returns the current parameter value and moves the
 *   pointer to the subsequent position.
 * - All parameters are integer, thus calling the function value_double()
 *   throws an error.
 */

class ArrayIntegerParameter : public ConnParameter
{
public:
  ArrayIntegerParameter( const std::vector< long >& values, const size_t nthreads )
    : values_( &values )
    , next_( nthreads, values_->begin() )
  {
  }

  void
  skip( thread tid, size_t n_skip ) const
  {
    if ( next_[ tid ] < values_->end() )
    {
      next_[ tid ] += n_skip;
    }
    else
    {
      throw KernelException( "Parameter values exhausted." );
    }
  }

  size_t
  number_of_values() const
  {
    return values_->size();
  }

  long
  value_int( thread tid, RngPtr, index, Node* ) const
  {
    if ( next_[ tid ] != values_->end() )
    {
      return *next_[ tid ]++;
    }
    else
    {
      throw KernelException( "Parameter values exhausted." );
    }
  }

  double
  value_double( thread tid, RngPtr, index, Node* ) const
  {
    if ( next_[ tid ] != values_->end() )
    {
      return static_cast< double >( *next_[ tid ]++ );
    }
    else
    {
      throw KernelException( "Parameter values exhausted." );
    }
  }

  inline bool
  is_array() const
  {
    return true;
  }

  bool
  provides_long() const
  {
    return true;
  }

  void
  reset() const
  {
    for ( std::vector< std::vector< long >::const_iterator >::iterator it = next_.begin(); it != next_.end(); ++it )
    {
      *it = values_->begin();
    }
  }

private:
  const std::vector< long >* values_;
  mutable std::vector< std::vector< long >::const_iterator > next_;
};

class ParameterConnParameterWrapper : public ConnParameter
{
public:
  ParameterConnParameterWrapper( const ParameterDatum&, const size_t );

  double value_double( thread target_thread, RngPtr rng, index snode_id, Node* target ) const;

  long
  value_int( thread target_thread, RngPtr rng, index snode_id, Node* target ) const
  {
    return value_double( target_thread, rng, snode_id, target );
  }

  inline bool
  is_array() const
  {
    return false;
  }

  bool
  provides_long() const
  {
    return parameter_->returns_int_only();
  }

private:
  Parameter* parameter_;
};

} // namespace nest

#endif
