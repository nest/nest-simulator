/*
 *  clipped_randomdev.h
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

#ifndef CLIPPED_RANDOMDEV_H
#define CLIPPED_RANDOMDEV_H

// C++ includes:
#include <cmath>
#include <limits>

// Generated includes:
#include "config.h"

// Includes from librandom:
#include "randomdev.h"
#include "randomgen.h"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

namespace librandom
{

/** @BeginDocumentation
Name: rdevdict::*_clipped - clipped random deviate generators.

Description: Generate random numbers from an underlying distribution,
but restricted to a certain interval.

For continuous distributions,

     low < random < high

will hold true, i.e., numbers are restricted to the open interval (low, high).
If the distribution itself is restricted to, e.g., positiv numbers, setting
low < 0 will still only return positive numbers. Clipping only excludes numbers
outside (low, high).

For discrete distribtions, numbers are limited to {low, low+1, ... high}, i.e.,
in this case min and max are included.

Numbers are clipped by re-drawing any number outside (low, high), until
a number in (log, high) is drawn. The actual distribution of random
numbers drawn will be a distorted version of the underlying
distribution.

Parameters:
/low  lower bound (default: -inf)
/high upper bound (default: +inf)

Remarks:
- Clipped generators can be very inefficient if there is little probability
mass in (low, high).
- For continuous distributions, the probability of actually drawing
min or max is approximately 2^-62 ~ 2e-19, i.e., negligible. Therefore,
one can only clip to open intervals.
- There are also *_clipped_to_boundary versions of the generators. These
return the respective boundary value if a value outside the interval is
drawn. These versions are mainly provided to allow reproduction of
publications that used this strategy.

SeeAlso: CreateRDV, rdevdict

Author: Hans Ekkehard Plesser
*/

/**
 * Wrapper template turning any continuous RDV into a clipped RDV.
 *
 * @ingroup RandomDeviateGenerators
 * @see ClippedRedrawDiscreteRandomDev
 */
template < typename BaseRDV >
class ClippedRedrawContinuousRandomDev : public BaseRDV
{

public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  ClippedRedrawContinuousRandomDev( RngPtr );
  ClippedRedrawContinuousRandomDev(); // threaded

#if not defined( HAVE_XLC_ICE_ON_USING ) and not defined( IS_K )
  using RandomDev::operator();
#endif

  double operator()( void );
  double operator()( RngPtr ) const; // threaded

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  double min_; //!< lower bound
  double max_; //!< upper bound
};

template < typename BaseRDV >
ClippedRedrawContinuousRandomDev< BaseRDV >::ClippedRedrawContinuousRandomDev(
  RngPtr r )
  : BaseRDV( r )
  , min_( -std::numeric_limits< double >::infinity() )
  , max_( std::numeric_limits< double >::infinity() )
{
  assert(
    not BaseRDV::has_ldev() ); // ensure underlying distribution is continuous
}

template < typename BaseRDV >
ClippedRedrawContinuousRandomDev< BaseRDV >::ClippedRedrawContinuousRandomDev()
  : BaseRDV()
  , min_( -std::numeric_limits< double >::infinity() )
  , max_( std::numeric_limits< double >::infinity() )
{
  assert(
    not BaseRDV::has_ldev() ); // ensure underlying distribution is continuous
}

template < typename BaseRDV >
void
ClippedRedrawContinuousRandomDev< BaseRDV >::set_status(
  const DictionaryDatum& d )
{
  BaseRDV::set_status( d );

  double new_min = min_;
  double new_max = max_;

  updateValue< double >( d, names::low, new_min );
  updateValue< double >( d, names::high, new_max );
  if ( new_min >= new_max )
  {
    throw BadParameterValue( "Clipped RDVs require low < high." );
  }

  min_ = new_min;
  max_ = new_max;
}

template < typename BaseRDV >
void
ClippedRedrawContinuousRandomDev< BaseRDV >::get_status(
  DictionaryDatum& d ) const
{
  BaseRDV::get_status( d );

  def< double >( d, names::low, min_ );
  def< double >( d, names::high, max_ );
}

template < typename BaseRDV >
inline double ClippedRedrawContinuousRandomDev< BaseRDV >::operator()( void )
{
  return ( *this )( this->rng_ );
}

template < typename BaseRDV >
inline double ClippedRedrawContinuousRandomDev< BaseRDV >::operator()(
  RngPtr r ) const
{
  double value;

  do
  {
    value = BaseRDV::operator()( r );
  } while ( value <= min_ || max_ <= value );

  return value;
}

// ----------------------------------------------------------

/**
 * Wrapper template turning any discrete RDV into a clipped RDV.
 *
 * @ingroup RandomDeviateGenerators
 * @see ClippedRedrawContinuousRandomDev
 */
template < typename BaseRDV >
class ClippedRedrawDiscreteRandomDev : public BaseRDV
{

public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  ClippedRedrawDiscreteRandomDev( RngPtr );
  ClippedRedrawDiscreteRandomDev(); // threaded

// Forwarding operators are explicitly defined here,
// to ensure that they forward to the clipped generator.
// Null-pointer checking is done in the underlying generator.

#if not defined( HAVE_XLC_ICE_ON_USING ) and not defined( IS_K )
  using RandomDev::operator();
  using RandomDev::ldev;
#endif

  double operator()( void );
  double operator()( RngPtr ) const; // threaded

  long ldev( void );
  long ldev( RngPtr ) const;


  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  long min_; //!< smallest value
  long max_; //!< largest value
};

template < typename BaseRDV >
ClippedRedrawDiscreteRandomDev< BaseRDV >::ClippedRedrawDiscreteRandomDev(
  RngPtr r )
  : BaseRDV( r )
  , min_( std::numeric_limits< long >::min() )
  , max_( std::numeric_limits< long >::max() )
{
  assert( BaseRDV::has_ldev() ); // ensure underlying distribution is discrete
}

template < typename BaseRDV >
ClippedRedrawDiscreteRandomDev< BaseRDV >::ClippedRedrawDiscreteRandomDev()
  : BaseRDV()
  , min_( std::numeric_limits< long >::min() )
  , max_( std::numeric_limits< long >::max() )
{
  assert( BaseRDV::has_ldev() ); // ensure underlying distribution is discrete
}

template < typename BaseRDV >
void
ClippedRedrawDiscreteRandomDev< BaseRDV >::set_status(
  const DictionaryDatum& d )
{
  BaseRDV::set_status( d );

  long new_min = min_;
  long new_max = max_;

  updateValue< long >( d, names::low, new_min );
  updateValue< long >( d, names::high, new_max );
  if ( new_min >= new_max )
  {
    throw BadParameterValue( "Clipped RDVs require low < high." );
  }

  min_ = new_min;
  max_ = new_max;
}

template < typename BaseRDV >
void
ClippedRedrawDiscreteRandomDev< BaseRDV >::get_status(
  DictionaryDatum& d ) const
{
  BaseRDV::get_status( d );

  def< long >( d, names::low, min_ );
  def< long >( d, names::high, max_ );
}

template < typename BaseRDV >
inline double ClippedRedrawDiscreteRandomDev< BaseRDV >::operator()( void )
{
  return ( *this )( this->rng_ );
}

template < typename BaseRDV >
inline double ClippedRedrawDiscreteRandomDev< BaseRDV >::operator()(
  RngPtr r ) const
{
  double value;

  do
  {
    value = BaseRDV::operator()( r );
  } while ( value < min_ || max_ < value );

  return value;
}

template < typename BaseRDV >
inline long
ClippedRedrawDiscreteRandomDev< BaseRDV >::ldev( void )
{
  return ClippedRedrawDiscreteRandomDev< BaseRDV >::ldev( this->rng_ );
}

template < typename BaseRDV >
inline long
ClippedRedrawDiscreteRandomDev< BaseRDV >::ldev( RngPtr r ) const
{
  long value;

  do
  {
    value = BaseRDV::ldev( r );
  } while ( value < min_ || max_ < value );

  return value;
}

// -------------------------------------------------------------------

/**
 * Wrapper template turning any continuous RDV into a clipped-to-boundary RDV.
 *
 * To-boundary RDVs return the boundary value if a number outside the
 * interval is drawn. This is mainly for reproduction of existing models
 * using this approach, it does not make much sense!

 * @ingroup RandomDeviateGenerators
 * @see ClippedToBoundaryDiscreteRandomDev
 */
template < typename BaseRDV >
class ClippedToBoundaryContinuousRandomDev : public BaseRDV
{

public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  ClippedToBoundaryContinuousRandomDev( RngPtr );
  ClippedToBoundaryContinuousRandomDev(); // threaded

#if not defined( HAVE_XLC_ICE_ON_USING ) and not defined( IS_K )
  using RandomDev::operator();
#endif

  double operator()( void );
  double operator()( RngPtr ) const; // threaded

  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  double min_; //!< lower bound
  double max_; //!< upper bound
};

template < typename BaseRDV >
ClippedToBoundaryContinuousRandomDev< BaseRDV >::
  ClippedToBoundaryContinuousRandomDev( RngPtr r )
  : BaseRDV( r )
  , min_( -std::numeric_limits< double >::infinity() )
  , max_( std::numeric_limits< double >::infinity() )
{
  assert(
    not BaseRDV::has_ldev() ); // ensure underlying distribution is continuous
}

template < typename BaseRDV >
ClippedToBoundaryContinuousRandomDev< BaseRDV >::
  ClippedToBoundaryContinuousRandomDev()
  : BaseRDV()
  , min_( -std::numeric_limits< double >::infinity() )
  , max_( std::numeric_limits< double >::infinity() )
{
  assert(
    not BaseRDV::has_ldev() ); // ensure underlying distribution is continuous
}

template < typename BaseRDV >
void
ClippedToBoundaryContinuousRandomDev< BaseRDV >::set_status(
  const DictionaryDatum& d )
{
  BaseRDV::set_status( d );

  double new_min = min_;
  double new_max = max_;

  updateValue< double >( d, names::low, new_min );
  updateValue< double >( d, names::high, new_max );
  if ( new_min >= new_max )
  {
    throw BadParameterValue( "Clipped RDVs require low < high." );
  }

  min_ = new_min;
  max_ = new_max;
}

template < typename BaseRDV >
void
ClippedToBoundaryContinuousRandomDev< BaseRDV >::get_status(
  DictionaryDatum& d ) const
{
  BaseRDV::get_status( d );

  def< double >( d, names::low, min_ );
  def< double >( d, names::high, max_ );
}

template < typename BaseRDV >
inline double ClippedToBoundaryContinuousRandomDev< BaseRDV >::operator()(
  void )
{
  return ( *this )( this->rng_ );
}

template < typename BaseRDV >
inline double ClippedToBoundaryContinuousRandomDev< BaseRDV >::operator()(
  RngPtr r ) const
{
  const double value = BaseRDV::operator()( r );
  if ( value < min_ )
  {
    return min_;
  }
  if ( value > max_ )
  {
    return max_;
  }
  return value;
}

// ----------------------------------------------------------

/**
 * Wrapper template turning any discrete RDV into a clipped-to-boundary RDV.
 *
 * To-boundary RDVs return the boundary value if a number outside the
 * interval is drawn. This is mainly for reproduction of existing models
 * using this approach, it does not make much sense!
 *
 * @ingroup RandomDeviateGenerators
 * @see ClippedToBoundaryContinuousRandomDev
 */
template < typename BaseRDV >
class ClippedToBoundaryDiscreteRandomDev : public BaseRDV
{

public:
  // accept only lockPTRs for initialization,
  // otherwise creation of a lock ptr would
  // occur as side effect---might be unhealthy
  ClippedToBoundaryDiscreteRandomDev( RngPtr );
  ClippedToBoundaryDiscreteRandomDev(); // threaded

// Forwarding operators are explicitly defined here,
// to ensure that they forward to the clipped generator.
// Null-pointer checking is done in the underlying generator.

#if not defined( HAVE_XLC_ICE_ON_USING ) and not defined( IS_K )
  using RandomDev::operator();
  using RandomDev::ldev;
#endif

  double operator()( void );
  double operator()( RngPtr ) const; // threaded

  long ldev( void );
  long ldev( RngPtr ) const;


  //! set distribution parameters from SLI dict
  void set_status( const DictionaryDatum& );

  //! get distribution parameters from SLI dict
  void get_status( DictionaryDatum& ) const;

private:
  long min_; //!< smallest value
  long max_; //!< largest value
};

template < typename BaseRDV >
ClippedToBoundaryDiscreteRandomDev< BaseRDV >::
  ClippedToBoundaryDiscreteRandomDev( RngPtr r )
  : BaseRDV( r )
  , min_( std::numeric_limits< long >::min() )
  , max_( std::numeric_limits< long >::max() )
{
  assert( BaseRDV::has_ldev() ); // ensure underlying distribution is discrete
}

template < typename BaseRDV >
ClippedToBoundaryDiscreteRandomDev< BaseRDV >::
  ClippedToBoundaryDiscreteRandomDev()
  : BaseRDV()
  , min_( std::numeric_limits< long >::min() )
  , max_( std::numeric_limits< long >::max() )
{
  assert( BaseRDV::has_ldev() ); // ensure underlying distribution is discrete
}

template < typename BaseRDV >
void
ClippedToBoundaryDiscreteRandomDev< BaseRDV >::set_status(
  const DictionaryDatum& d )
{
  BaseRDV::set_status( d );

  long new_min = min_;
  long new_max = max_;

  updateValue< long >( d, names::low, new_min );
  updateValue< long >( d, names::high, new_max );
  if ( new_min >= new_max )
  {
    throw BadParameterValue( "Clipped RDVs require low < high." );
  }

  min_ = new_min;
  max_ = new_max;
}

template < typename BaseRDV >
void
ClippedToBoundaryDiscreteRandomDev< BaseRDV >::get_status(
  DictionaryDatum& d ) const
{
  BaseRDV::get_status( d );

  def< long >( d, names::low, min_ );
  def< long >( d, names::high, max_ );
}

template < typename BaseRDV >
inline double ClippedToBoundaryDiscreteRandomDev< BaseRDV >::operator()( void )
{
  return ( *this )( this->rng_ );
}

template < typename BaseRDV >
inline double ClippedToBoundaryDiscreteRandomDev< BaseRDV >::operator()(
  RngPtr r ) const
{
  const double value = BaseRDV::operator()( r );
  if ( value < min_ )
  {
    return min_;
  }
  if ( value > max_ )
  {
    return max_;
  }
  return value;
}

template < typename BaseRDV >
inline long
ClippedToBoundaryDiscreteRandomDev< BaseRDV >::ldev( void )
{
  return ClippedToBoundaryDiscreteRandomDev< BaseRDV >::ldev( this->rng_ );
}

template < typename BaseRDV >
inline long
ClippedToBoundaryDiscreteRandomDev< BaseRDV >::ldev( RngPtr r ) const
{
  const long value = BaseRDV::ldev( r );
  if ( value < min_ )
  {
    return min_;
  }
  if ( value > max_ )
  {
    return max_;
  }
  return value;
}

} // namespace librandom

#endif
