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
#include <stdexcept>
#include <vector>

// Includes from nestkernel:
#include "../nestkernel/random_manager.h"
#include "../nestkernel/sp_manager.h"

namespace nest
{

/**
 * Test cases: Distance-dependent connection methods in SPManager
 */
BOOST_AUTO_TEST_SUITE( test_distance_dependent )

BOOST_AUTO_TEST_CASE( test_gaussianKernel )
{
  SPManager sp_manager;

  // Test for zero distance
  std::vector< double > pos1 = { 0.0, 0.0 };
  std::vector< double > pos2 = { 0.0, 0.0 };
  double sigma = 1.0;

  double expected = 1.0;
  BOOST_REQUIRE_CLOSE( sp_manager.gaussian_kernel( pos1, pos2, sigma ), expected, 1e-6 );

  // Test for unit distance
  pos2 = { 1.0, 0.0 };
  expected = std::exp( -1.0 );
  BOOST_REQUIRE_CLOSE( sp_manager.gaussian_kernel( pos1, pos2, sigma ), expected, 1e-6 );

  // Test for negative sigma (will compute as if sigma were positive)
  sigma = -1.0;
  double result = sp_manager.gaussian_kernel( pos1, pos2, sigma );
  expected = std::exp( -1.0 ); // Same as sigma=1 since squared value is used
  BOOST_REQUIRE_CLOSE( result, expected, 1e-6 );
}

BOOST_AUTO_TEST_CASE( test_get_neuron_pair_index )
{
  SPManager sp_manager;

  // Test with valid IDs
  BOOST_REQUIRE_EQUAL( sp_manager.get_neuron_pair_index( 1, 3 ), 3 );
  BOOST_REQUIRE_EQUAL( sp_manager.get_neuron_pair_index( 3, 1 ), 3 );

  // Test with same IDs
  BOOST_REQUIRE_EQUAL( sp_manager.get_neuron_pair_index( 5, 5 ), 14 );
}


BOOST_AUTO_TEST_CASE( test_global_shuffle_spatial )
{
  SPManager sp_manager;

  // Initialize test data
  std::vector< size_t > pre_ids = { 1, 2 };
  std::vector< size_t > post_ids = { 3, 4 };

  sp_manager.global_ids = { 1, 2, 3, 4 };
  sp_manager.global_positions = {
    0.0,
    0.0, // Neuron 1
    1.0,
    0.0, // Neuron 2
    0.0,
    1.0, // Neuron 3
    1.0,
    1.0 // Neuron 4
  };

  sp_manager.structural_plasticity_gaussian_kernel_sigma_ = 1.0;

  std::vector< size_t > pre_ids_results;
  std::vector< size_t > post_ids_results;

  sp_manager.global_shuffle_spatial( pre_ids, post_ids, pre_ids_results, post_ids_results );

  // Check result sizes
  BOOST_REQUIRE_EQUAL( pre_ids_results.size(), 2 );
  BOOST_REQUIRE_EQUAL( post_ids_results.size(), 2 );

  // Ensure no self-connections
  for ( size_t i = 0; i < pre_ids_results.size(); ++i )
  {
    BOOST_REQUIRE_NE( pre_ids_results[ i ], post_ids_results[ i ] );
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nest

#endif /* TEST_DISTANCE_DEPENDENT_H */
