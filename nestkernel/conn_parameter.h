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
#include "nest_datums.h"
#include "parameter.h"

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
  ConnParameter();

  virtual ~ConnParameter();

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
  virtual double value_double( size_t, RngPtr, size_t, Node* ) const = 0;
  virtual long value_int( size_t, RngPtr, size_t, Node* ) const = 0;

  virtual void skip( size_t, size_t ) const;
  virtual bool is_array() const = 0;
  virtual bool is_scalar() const;
  virtual bool provides_long() const;
  virtual void reset() const;

  /**
   * Returns number of values available.
   *
   * 0 indicates scalar/unlimited supply.
   */
  virtual size_t number_of_values() const;
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
  ScalarDoubleParameter( double value, const size_t );

  double value_double( size_t, RngPtr, size_t, Node* ) const override;
  long value_int( size_t, RngPtr, size_t, Node* ) const override;
  bool is_array() const override;
  void reset() const override;
  bool is_scalar() const override;

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
  ScalarIntegerParameter( long value, const size_t );

  double value_double( size_t, RngPtr, size_t, Node* ) const override;
  long value_int( size_t, RngPtr, size_t, Node* ) const override;
  bool is_array() const override;
  void reset() const override;
  bool is_scalar() const override;
  bool provides_long() const override;

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
  ArrayDoubleParameter( const std::vector< double >& values, const size_t nthreads );

  void skip( size_t tid, size_t n_skip ) const override;
  size_t number_of_values() const override;

  double value_double( size_t tid, RngPtr, size_t, Node* ) const override;
  long value_int( size_t, RngPtr, size_t, Node* ) const override;

  bool is_array() const override;
  void reset() const override;

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
  ArrayIntegerParameter( const std::vector< long >& values, const size_t nthreads );

  void skip( size_t tid, size_t n_skip ) const override;
  size_t number_of_values() const override;

  long value_int( size_t tid, RngPtr, size_t, Node* ) const override;
  double value_double( size_t tid, RngPtr, size_t, Node* ) const override;

  bool is_array() const override;
  bool provides_long() const override;
  void reset() const override;

private:
  const std::vector< long >* values_;
  mutable std::vector< std::vector< long >::const_iterator > next_;
};

class ParameterConnParameterWrapper : public ConnParameter
{
public:
  ParameterConnParameterWrapper( const ParameterDatum&, const size_t );

  double value_double( size_t target_thread, RngPtr rng, size_t snode_id, Node* target ) const override;
  long value_int( size_t target_thread, RngPtr rng, size_t snode_id, Node* target ) const override;

  bool is_array() const override;
  bool provides_long() const override;

private:
  Parameter* parameter_;
};

} // namespace nest

#endif
