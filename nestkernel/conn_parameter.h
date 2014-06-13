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

#include <limits>
#include <vector>

#include "randomgen.h"
#include "randomdev.h"
#include "token.h"
#include "exceptions.h"

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
    ConnParameter() {}
    virtual ~ConnParameter() {}

    /**
     * Return parameter value.
     *
     * The parameter value may depend on source and target GIDs
     * (when using callback functions, not yet implemented)
     * and random numbers. All three must be supplied, even if
     * a concrete parameter type does not use them.
     *
     * @param sgid  source gid
     * @param tgid  target gid
     * @param rng   random number generator pointer
     * will be ignored except for random parameters.
     */
    virtual double value_double(index, index, librandom::RngPtr&) const =0;       
    virtual long_t value_int(index, index, librandom::RngPtr&) const =0;

    /**
     * Returns number of values available.
     *
     * 0 indicates scalar/unlimited supply.
     */
    virtual size_t number_of_values() const { return 0; }

    static ConnParameter* create(const Token&);
  };


  /**
   * Single double value.
   *
   * On each request, it returns the same value.
   */
  class ScalarDoubleParameter: public ConnParameter
  {
  public:
    ScalarDoubleParameter(double value) : value_(value) {}

    double value_double(index, index, librandom::RngPtr&) const { return value_; }
    long_t value_int(index, index, librandom::RngPtr&) const { throw KernelException("ConnParameter calls value function with false return type."); }

  private:
    double value_;
  };
  
  /**
   * Single integer value.
   *
   * On each request, it returns the same value.
   */
  class ScalarIntegerParameter: public ConnParameter
  {
  public:
    ScalarIntegerParameter(long_t value) : value_(value) {}

    double value_double(index, index, librandom::RngPtr&) const { throw KernelException("ConnParameter calls value function with false return type."); }
    long_t value_int(index, index, librandom::RngPtr&) const { return value_; }

  private:
    long_t value_;
  };


  /**
   * Array parameter, returning values in order.
   *
   * - The array of values must not be empty 
   *   (so return 0 for number_of_values can signal non-array parameter)
   * - Throws exception if more values requested than available.
   *
   */
  class ArrayParameter: public ConnParameter
  {
  public:
    ArrayParameter(const std::vector<double>& values)
      : values_(values), next_(values_.begin()) 
      {}

    size_t number_of_values() const { return values_.size(); }

    //double value(index sgid, index tgid, librandom::RngPtr&) const 
    double value_double(index, index, librandom::RngPtr&) const 
   
    { 
      //return values_[sgid];
      if ( next_ != values_.end() ) 
        return *next_++;
      else 
	throw KernelException("Parameter values exhausted.");
    }
    long_t value_int(index, index, librandom::RngPtr&) const { throw KernelException("ConnParameter calls value function with false return type."); }

  private:
    std::vector<double> values_;
    mutable std::vector<double>::iterator next_;
  };


  /**
   * Random scalar value.
   *
   * On each request, it returns a new value drawn from the given deviate.
   */
  class RandomParameter: public ConnParameter
  {
  public:
    RandomParameter(const DictionaryDatum&);

    double value_double(index, index, librandom::RngPtr& rng) const { return (*rdv_)(rng); }
    long_t value_int(index, index, librandom::RngPtr& rng) const { return (*rdv_)(rng); }

  private:
    librandom::RdvPtr rdv_;
  };

}  // namespace nest

#endif
