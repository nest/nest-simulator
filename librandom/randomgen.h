/*
 *  randomgen.h
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

#ifndef RANDOMGEN_H
#define RANDOMGEN_H

/**
 * @defgroup RandomNumbers Random Number and Deviate Generation in NEST.
 *
 * This module contains the interface to random numbers and random
 * deviate generators.  We differentiate between
 *
 * random number generators: generate uniformly distributed numbers.
 *
 * random deviate generators: generate non-uniformly distributed
 * numbers, eg, Poisson, binomial or gamma distributed numbers.
 *
 * @note
 * In the NEST kernel, random number generators are managed
 * by the scheduler on a thread-by-thread basis.  Nodes requiring
 * random numbers must access random numbers and deviates only through the
 * interface provided by the scheduler, as in the follwing example taken
 * from nest::poisson_generator.
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
 * If one wanted just uniformly distributed numbers in [0, 1), one could
 * obtain them with
 * @code
 *   librandom::RngPtr rng=Network::get_network().get_rng(thrd);
 *   double r = rng();
 * @endcode
 *
 * @note
 * Random number generators should be managed through safe RngPtr pointers,
 * not through plain RandomGen*.
 *
 * @note
 * All elements are in namespace librandom.
 */

/**
 * @defgroup RandomNumberGenerators Uniform Random Number Generators.
 *
 * @ingroup RandomNumbers
 *
 * All generators are derived from base class RandomGen.
 *
 * All generators return only uniformly distributed random
 * numbers in well-defined intervals.  The following
 * functions are implemented
 *
 * @verbatim
 * Function                    Comment
 * -------------------------------------------------------
 * double drand()              [0, 1)
 *        ()                   [0, 1)
 * double drandpos()           (0, 1)
 * unsigned long  ulrand(N)            [0, N-1]
 *
 * void   seed(N)              seed the RNG, N: unsigned long
 * -------------------------------------------------------
 * @endverbatim
 *
 * @note
 * The drand() method is the core method for RNG production;
 * all other methods draw random numbers by calls to drand.
 *
 * @note
 * For access to random numbers from the SLI interface, see
 * the SLI documentation.
 *
 * @note
 * For a list of available RNGs, see rngdict info in SLI.
 *
 * NEST comes at present with two built-in random number generators:
 * - knuthlfg, the lagged Fibonacci generator from D.E.Knuth,
 *   The Art of Computer Programming, 3rd ed, vol 2, sec 3.6.
 * - MT19937, the Mersenne Twister by Matsumoto and Nishimura.
 * Implementations of both are directly derived from free code published
 * by the original authors.
 *
 * If the GNU Scientific Library (v 1.2 or later) is installed,
 * all uniform random number generators from the GSL are made available,
 * too. These have names prefixed with gsl_. For more information on the
 * RNGs linked in from the GSL, see http://www.gnu.org/software/gsl.
 *
 * The following generators produce idential sequences:
 * - knuthlfg and gsl_knuthran2002
 * - MT19937 and gsl_mt19937 (for seeds different from 0)
 *
 * @note
 * - Random generator constructors should always require a seed.
 * - A default seed value is available globally through
 *   librandom::RandomGen::DefaultSeed.
 * - If you need to create a random generator in a place without
 *   access to rngdict, you can use
 *   librandom::RandomGen::create_knuthlfg_rng(librandom::RandomGen::DefaultSeed)
 * - This is not generally recommended practice. Both nodes and any
 *   module routines shall use the RNGs provided by Network.
 */

/*
 * Problems:
 * Some problems remain to be solved:
 * -- portability has to be confirmed (on compilers other
 *    than g++ and > 32 Bit architectures.
 *
 * History:
 *     (1) first version (<= V1.0) by Gewaltig
 *     (2) rewritten for C++ (V2.0) by Diesmann 6.9.93
 *         Note! The July 8,1993 Version contains an error
 *         in random3 and in random2. In this Version the
 *         correct lines are marked.
 *     (3) 16.9.93 class Normal is no longer a RNG   but
 *         a deviation. This change was necessary to pre-
 *         vent side effects in cases where random numbers
 *         are used in several classes of one program.
 *     (4) Rewritten C++ base class and three more
 *         generators added.     (Gewaltig)
 *     (5) Completely re-designed: GSL based, randomdict
 *         introduced (HEP)
 *     (6) GSL and standalone version now provide the
 *         knuthlfg originally introduced in (4)
 *         Diesmann 21.08.02
 *     (7) virtual function binomial added as interface
 *         for BinomialRandomDev to reach binomial function
 *         of gslrandomgen
 *     (8) Buffered drawing of RNG, all non-drand() methods
 *         now go via drand(); binomial removed, now imple-
 *         mented in BinomialRandomDev. (HEP, 28.06.04)
 *     (9) All code derived from the GSL removed (HEP, 09.01.08).
 *    (10) All code related to buffering removed because
 *         buffering may hurt performance in distributed
 *         simulations (and does not result in any speed-up in
 *         small simulations anyway) (WS, 01.05.15; see #947).
 */

// C++ includes:
#include <cmath>
#include <vector>

#include <memory>

// Includes from librandom:
#include "librandom_names.h"

/**
 * Namespace for random number generators.
 */

namespace librandom
{

class RandomGen;

/**
 * Common shared_ptr type for RNG
 *
 * A smart pointer that should be used instead of RandomGen*
 * in user code to manage random number generators.
 */
typedef std::shared_ptr< RandomGen > RngPtr;

/**
 * Abstract base class for all random generator objects
 *
 * Class RandomGen is the top of the random generator object
 * hierarchy.  It defines the interface to random generators:
 *
 * - creation
 * - seeding
 * - drawing
 *
 * @see class RandomDev
 * @ingroup RandomNumberGenerators
 */

class RandomGen
{
public:
  /**
   * @note All classes derived from RandomGen should
   *       have only a single constructor, taking
   *       an unsigned long as seed value. Use
   *       RandomGen::DefaultSeed if you want to
   *       create a generator with a default seed value.
   */
  RandomGen(){};

  // ensures proper clean up
  virtual ~RandomGen(){};

  /**
     The following functions implement the user interface of the
     RandomGen class. The actual interface to the underlying
     random generator is provided by protected member functions below.
   */
  double drand( void );                        //!< draw from [0, 1)
  double operator()( void );                   //!< draw from [0, 1)
  double drandpos( void );                     //!< draw from (0, 1)
  unsigned long ulrand( const unsigned long ); //!< draw from [0, n-1]

  void seed( const unsigned long ); //!< set random seed to a new value

  /**
   * Create built-in Knuth Lagged Fibonacci random generator.
   * This function is provided so that RNGs can be created in places
   * where the SLI rngdict is not accessible.
   * @see KnuthLFG
   */
  static RngPtr create_knuthlfg_rng( unsigned long );

  //! Default value for seeding generators in places where no seed is supplied.
  static const unsigned long DefaultSeed;

  //! clone a random number generator of same type initialized with given seed
  virtual RngPtr clone( const unsigned long ) = 0;

protected:
  /**
     The following functions provide the interface to the actual
     random generator.  They must be implemented by each derived
     generator class.
   */
  virtual void seed_( unsigned long ) = 0; //!< seeding interface
  virtual double drand_() = 0;             //!< drawing interface

private:
  // prohibit copying of RNG
  RandomGen( const RandomGen& );
};

/**
 * Factory class for random generators.
 * @ingroup RandomNumberGenerators
 */
class GenericRNGFactory
{
public:
  /**
   * Create generator with given seed.
   * @note Generators cannot be created without a seed.
   *       If you want to create a generator with a default
   *       seed value, you should explicitly use
   *       RandomGen::DefaultSeed as seed value.
   */
  virtual RngPtr create( unsigned long ) const = 0;

  virtual ~GenericRNGFactory()
  {
  }
};

/**
 * Concrete template for factories for built-in
 * (non GSL) random generators.
 * @ingroup RandomNumberGenerators
 */
template < typename Generator >
class BuiltinRNGFactory : public GenericRNGFactory
{
  RngPtr
  create( unsigned long s ) const
  {
    return RngPtr( new Generator( s ) );
  }
};

inline double
RandomGen::drand( void )
{
  return drand_();
}

inline double RandomGen::operator()( void )
{
  return drand();
}

inline double
RandomGen::drandpos( void )
{
  double r;

  do
  {
    r = drand();
  } while ( r == 0.0 );

  return r;
}

inline unsigned long
RandomGen::ulrand( const unsigned long n )
{
  // no check for size of n required, since n is unsigned long
  return static_cast< unsigned long >( std::floor( n * drand() ) );
}
}

#endif // RANDOMGEN_H
