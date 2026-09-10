/*
 *  test_distance_dependent_structural_plasticity.h
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

#ifndef TEST_DISTANCE_DEPENDENT_H
#define TEST_DISTANCE_DEPENDENT_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

// Includes from nestkernel:
#include "../nestkernel/random_manager.h"
#include "../nestkernel/sp_manager.h"

namespace nest
{

/**
 * Test cases: Distance-dependent connection methods in SPManager
 *
 * The spatial kernel itself is a regular NEST Parameter and is covered by the PyNEST tests in
 * testsuite/pytests/structural_plasticity. What remains specific to the SPManager is the weighted
 * draw over the candidate targets, which is tested here.
 */
BOOST_AUTO_TEST_SUITE( test_distance_dependent )

BOOST_AUTO_TEST_CASE( test_roulette_wheel_selection_boundaries )
{
  SPManager sp_manager;

  // With equal weights, the unit interval is split into equal parts.
  const std::vector< double > uniform { 1.0, 1.0, 1.0, 1.0 };
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( uniform, 0.0 ), 0u );
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( uniform, 0.3 ), 1u );
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( uniform, 0.6 ), 2u );
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( uniform, 0.9 ), 3u );

  // Weights need not be normalised, and are interpreted relative to their sum.
  const std::vector< double > unnormalised { 30.0, 10.0 };
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( unnormalised, 0.7 ), 0u );
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( unnormalised, 0.8 ), 1u );

  // A zero weight is never selected, whatever the random number.
  const std::vector< double > with_zero { 0.0, 1.0 };
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( with_zero, 0.0 ), 1u );
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( with_zero, 0.999999 ), 1u );

  // A random number arbitrarily close to one still yields a valid index.
  BOOST_REQUIRE_EQUAL( sp_manager.roulette_wheel_selection( uniform, std::nextafter( 1.0, 0.0 ) ), uniform.size() - 1 );
}

BOOST_AUTO_TEST_CASE( test_roulette_wheel_selection_rejects_degenerate_weights )
{
  SPManager sp_manager;

  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( {}, 0.5 ), std::runtime_error );
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( { 0.0, 0.0 }, 0.5 ), std::runtime_error );
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( { std::numeric_limits< double >::infinity(), 1.0 }, 0.5 ),
    std::runtime_error );
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( { std::nan( "" ), 1.0 }, 0.5 ), std::runtime_error );
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace nest

#endif /* TEST_DISTANCE_DEPENDENT_H */
