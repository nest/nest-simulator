/*
 *  test_node_collection_step.h
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

#ifndef TEST_NODE_COLLECTION_STEP_H
#define TEST_NODE_COLLECTION_STEP_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes
#include <vector>

// Includes from nestkerne
#include "kernel_manager.h"
#include "node_collection.h"

namespace nest
{
/**
 * Test cases: Getter for NodeCollection step
 */
BOOST_AUTO_TEST_SUITE( test_node_collection_step )


/**
 * This test checks that a NodeCollection created from a vector of homogeneous and
 * contiguous node IDs (primitive NodeCollection) has a step size of 1.
 */
BOOST_AUTO_TEST_CASE( test_primitive_node_collection )
{
  KernelManager::create_kernel_manager();
  const std::vector< index > node_ids { 1, 2, 3 };
  auto nc = NodeCollection::create_test_collection( node_ids );
  size_t expected_nc_step = 1;
  auto actual_nc_step = nc->step();
  BOOST_REQUIRE_EQUAL( actual_nc_step, expected_nc_step );
}


/**
 * This test checks if an empty NodeCollection has a step size of 1, since an empty
 * NodeCollection should be primitive.
 */
BOOST_AUTO_TEST_CASE( test_empty_node_collection )
{
  KernelManager::create_kernel_manager();
  const std::vector< index > node_ids {};
  auto nc = NodeCollection::create_test_collection( node_ids );
  size_t expected_nc_step = 1;
  auto actual_nc_step = nc->step();
  BOOST_REQUIRE_EQUAL( actual_nc_step, expected_nc_step );
}

/**
 * This test checks that the step getter retrieves the expected step from a composite
 * NodeCollection created from a NodeCollection slice with a step of 2 between node IDs.
 */
BOOST_AUTO_TEST_CASE( test_composite_node_collection )
{
  KernelManager::create_kernel_manager();
  std::vector< index > node_ids { 1, 2, 3, 4, 5 };
  auto nc = NodeCollection::create_test_collection( node_ids );
  auto nc_slice = nc->slice( 0, 4, 2 );
  size_t expected_nc_step = 2;
  auto actual_nc_step = nc_slice->step();
  BOOST_REQUIRE_EQUAL( actual_nc_step, expected_nc_step );
}


BOOST_AUTO_TEST_SUITE_END()
}
#endif /* TEST_NODE_COLLECTION_STEP_H */
