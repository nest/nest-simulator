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

// The timeout feature of BOOST_AUTO_TEST_CASE is only available starting with Boost version 1.70
#include <boost/version.hpp>
#if BOOST_VERSION >= 107000

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// Includes from nestkernel
#include "nest_datums.h"

// Includes from librandom
#include "randomgen.h"

BOOST_AUTO_TEST_SUITE( test_parameter )

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
  auto rng = librandom::RngPtr( librandom::RandomGen::create_knuthlfg_rng( librandom::RandomGen::DefaultSeed ) );

  BOOST_CHECK_THROW( redraw_pd->value( rng, nullptr ), nest::KernelException );
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* BOOST_VERSION */

#endif /* TEST_PARAMETER_H */
