/*
 *  randomdev.h
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

#ifndef RANDOMDEV_H
#define RANDOMDEV_H

// C++ includes:
#include <cassert>
#include <memory>

// Includes from librandom:
#include "librandom_exceptions.h"
#include "randomgen.h"

// Includes from sli:
#include "dictdatum.h"

/**
 * @defgroup RandomDeviateGenerators Random Deviate Generators.
 * @ingroup RandomNumbers
 *
 * Random deviate generators (RDGs) produce random numbers with
 * various distributions on the basis of [0,1) uniformly distributed
 * numbers.  Discrete and continuous distributions are available.
 * For a complete list of available deviates, please use the following
 * sli command
 *
 * @verbatim
 * SLI ] rdevdict info
 * --------------------------------------------------
 * Name                     Type                Value
 * --------------------------------------------------
 * binomial                 rdvfacttype         <rdvfacttype>
 * poisson                  rdvfacttype         <rdvfacttype>
 * normal                   rdvfacttype         <rdvfacttype>
 * exponential              rdvfacttype         <rdvfacttype>
 * gamma                    rdvfacttype         <rdvfacttype>
 * uniformint               rdvfacttype         <rdvfacttype>
 * --------------------------------------------------
 * @endverbatim
 *
 * @note
 * RDGs that are to be used in multithreaded code, must be called
 * with the thread-RNG as argument whenever a number is drawn, to
 * ensure thread-consistent number generation.
 *
 * @note
 * All RDGs provide double numbers, generators for discrete
 * distributions may provide unsigned longs as well (eg, Poisson).
 * This can be checked with the has_ldev().
 *
 * @note
 * Here is a code example for the use of the Poisson generator:
 *
 * @code
 * #include "poisson_randomdev.h"
 * class poisson_generator :
 *   public Node,
 *   protected Device
 * {
 * ...
 * private:
 *   librandom::PoissonRandomDev poisson_dev_;  //!< random deviate generator
 * ...
 *  };
 * @endcode
 * @code
 * nest::poisson_generator::poisson_generator()
 * :...
 *   poisson_dev_(0.0),
 *  ...
 * {
 *   calibrate(Time::get_resolution());
 * }
 * void nest::poisson_generator::calibrate(Time const & dt)
 * {
 *  poisson_dev_.set_lambda(dt.get_ms() * rate_*1e-3);
 * }
 *
 * void nest::poisson_generator::update(thread thrd, Time const & T)
 * {
 *   ...
 *   librandom::RngPtr rng=kernel().rng_manager.get_rng(thrd);
 *   ...
 *     long n_spikes = poisson_dev_.ldev(rng);
 *   ...
 * }
 * @endcode
 *
 * Note that the RNG used by the deviate generator must be obtained
 * via get_rng(thrd) on each call to update, to get the proper generator
 * for the present thread.
 *
 */

namespace librandom
{

class RandomDev;

/**
 * Common lock-pointer type for Random deviate generators
 *
 * A safe pointer that should be used instead of RandomDev*
 * in user code to manage random number generators.
 */
typedef std::shared_ptr< RandomDev > RdvPtr;


/**
 * Abstract base class for access to non-uniform random deviate
 * generators.
 * @see RandomGen
 * @ingroup RandomDeviate
 * HEP 2002-07-09, 2004-06-28
 */

class RandomDev
{

public:
  /**
   * Construct with (single-threaded) or without (multithreaded)
   * RNG.
   */
  RandomDev( RngPtr rng = RngPtr( 0 ) )
    : rng_( rng )
  {
  }

  //! ensure proper clean-up
  virtual ~RandomDev(){};

  /**
   * Operator delivering doubles
   *
   * All random deviates must deliver doubles
   *
   * @note
   * Operator varieties with and without RngPtr as argument
   * are implemented explicitly instead of via default value
   * of the argument to force all derived classes to implement
   * both varieties.
   */
  virtual double operator()( void );             //!< single-threaded
  virtual double operator()( RngPtr ) const = 0; //!< multi-threaded

  /**
   * integer valued functions for discrete distributions
   */
  virtual long ldev( void );
  virtual long ldev( RngPtr ) const;

  /**
   * true if RDG implements ldev function
   */
  virtual bool
  has_ldev() const
  {
    return false;
  }

  //! set RNG
  void
  set_rng( RngPtr rng )
  {
    rng_ = rng;
  }

  /**
   * set distribution parameters from SLI interface
   *
   * @note
   * RDGs with parameters will usually implement functions
   * to manipulate the parameters directly, without use
   * of a SLI dictionary.  This function is meant only to
   * provide access from the SLI interface.
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * get distribution parameters from SLI interface
   *
   * @note
   * RDGs with parameters will usually implement functions
   * to manipulate the parameters directly, without use
   * of a SLI dictionary.  This function is meant only to
   * provide access from the SLI interface.
   */
  virtual void get_status( DictionaryDatum& ) const;

protected:
  RngPtr rng_; //!< store underlying RNG
};

inline double RandomDev::operator()( void )
{
  return ( *this )( rng_ );
}

inline long
RandomDev::ldev( void )
{
  return this->ldev( rng_ );
}


/**
 * Generic factory class for RandomDev.
 */
class GenericRandomDevFactory
{
public:
  virtual ~GenericRandomDevFactory()
  {
  }
  virtual RdvPtr create() const = 0;
  virtual RdvPtr create( RngPtr rng ) const = 0;
};

/**
 * Factory class for generating objects of type RandomDev
 */

template < typename DevType >
class RandomDevFactory : public GenericRandomDevFactory
{

public:
  //! create unbound deviate generator
  RdvPtr
  create() const
  {
    return RdvPtr( new DevType() );
  }

  //! create deviate generator given uniform number generator
  RdvPtr
  create( RngPtr rng ) const
  {
    return RdvPtr( new DevType( rng ) );
  }
};
}

#endif
