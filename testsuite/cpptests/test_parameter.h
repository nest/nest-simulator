/*
 *  test_parameter.h
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

#ifndef TEST_PARAMETER_H
#define TEST_PARAMETER_H

// C++ includes
#include <type_traits> // std::is_floating_point

#include <boost/version.hpp>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// Includes from nestkernel
#include "nest_datums.h"
#include "random_generators.h"

BOOST_AUTO_TEST_SUITE( test_parameter )

#if BOOST_VERSION >= 105900 // test_redraw_value_impossible uses timeout, which is only available in Boost>=1.59.0

/**
 * This test checks that an exception is thrown if the RedrawParameter exceeds the max number of redraws. In
 * case an exception isn't thrown, the test times out after a given time (in seconds). Because it has to be
 * able to timeout while redrawing, the test has to be on the C++ level.
 */
BOOST_AUTO_TEST_CASE( test_redraw_value_impossible, *boost::unit_test::timeout( 2 ) )
{
  DictionaryDatum d = new Dictionary();
  ( *d )[ nest::names::min ] = 0.0;
  ( *d )[ nest::names::max ] = 1.0;
  ParameterDatum uniform_pd = new nest::UniformParameter( d );
  // Requested region is outside of the parameter limits, so it cannot get an acceptable value.
  ParameterDatum redraw_pd = uniform_pd->redraw( -1.0, -0.5 );

  // We need to go via a factory to avoid compiler confusion. Two somewhat arbitrary sequences are used for seeding.
  nest::RngPtr rng = nest::RandomGeneratorFactory< std::mt19937_64 >().create( { 1234567890, 23423423 } );

  BOOST_CHECK_THROW( redraw_pd->value( rng, nullptr ), nest::KernelException );
}

#endif /* BOOST_VERSION */

/**
 * Tests that uniform int parameter only returns integers.
 */
BOOST_AUTO_TEST_CASE( test_uniform_int_returns_integer )
{
  const int max = 100;
  const int num_iterations = 1000;

  DictionaryDatum d = new Dictionary();
  ( *d )[ nest::names::max ] = max;
  ParameterDatum uniform_int_pd = new nest::UniformIntParameter( d );

  // We need to go via a factory to avoid compiler confusion
  nest::RandomGeneratorFactory< std::mt19937_64 > rf;
  nest::RngPtr rng = rf.create( { 1234567890, 23423423 } );

  for ( int i = 0; i < num_iterations; ++i )
  {
    auto value = uniform_int_pd->value( rng, nullptr );
    // Test makes no sense if the return value of the Parameter is not floating point.
    static_assert( std::is_floating_point< decltype( value ) >::value, "Return type not floating point" );
    BOOST_REQUIRE_EQUAL( value, static_cast< long >( value ) );
    BOOST_REQUIRE_LT( value, max ); // Require value < max
    BOOST_REQUIRE_GE( value, 0 );   // Require value >= 0
  }
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* TEST_PARAMETER_H */
