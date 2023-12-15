/*
 *  test_target_fields.h
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

#ifndef TEST_TARGET_FIELDS_H
#define TEST_TARGET_FIELDS_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <cmath>
#include <cstdlib>

// Includes from nestkernel:
#include "nest_types.h"
#include "target.h"

namespace nest
{

/**
 * Test cases: Target data type object
 */
BOOST_AUTO_TEST_SUITE( test_target_data_type_object )

// Run several trials with random data
constexpr int NUM_TEST_TRIALS = 50U;

BOOST_AUTO_TEST_CASE( test_target_object_type_size )
{
  // Test the size of the Target data type object: must be 64-bit!
  Target target_id_testSize;
  BOOST_REQUIRE( sizeof( target_id_testSize ) == 8U );
}

BOOST_AUTO_TEST_CASE( test_target_object_type_constructor )
{
  std::srand( 1234567 );
  for ( int i = 0; i < NUM_TEST_TRIALS; ++i )
  {
    // tid and rank can take on all values up to MAX_{TID,RANK}
    // syn_id and lcid can only take values up to MAX_{SYN_ID,LCID}-1
    const size_t tid = std::rand() % ( MAX_TID + 1 );
    const size_t rank = std::rand() % ( MAX_RANK + 1 );
    const synindex syn_id = std::rand() % MAX_SYN_ID;
    const size_t lcid = std::rand() % MAX_LCID;

    Target target_id_testInit( tid, rank, syn_id, lcid );

    BOOST_REQUIRE( target_id_testInit.get_tid() == tid );
    BOOST_REQUIRE( target_id_testInit.get_rank() == rank );
    BOOST_REQUIRE( target_id_testInit.get_syn_id() == syn_id );
    BOOST_REQUIRE( target_id_testInit.get_lcid() == lcid );
    BOOST_REQUIRE( target_id_testInit.get_status() == TARGET_ID_UNPROCESSED );
  }
}

BOOST_AUTO_TEST_CASE( test_target_object_type_set_get )
{
  std::srand( 2345678 );
  Target target_id_testSetGet;
  for ( int i = 0; i < NUM_TEST_TRIALS; ++i )
  {
    // tid and rank can take on all values up to MAX_{TID,RANK}
    // syn_id and lcid can only take values up to MAX_{SYN_ID,LCID}-1
    const size_t tid = std::rand() % ( MAX_TID + 1 );
    const size_t rank = std::rand() % ( MAX_RANK + 1 );
    const synindex syn_id = std::rand() % MAX_SYN_ID;
    const size_t lcid = std::rand() % MAX_LCID;

    enum_status_target_id status_target_id = TARGET_ID_UNPROCESSED;
    if ( static_cast< bool >( std::rand() % 2 ) )
    {
      status_target_id = TARGET_ID_PROCESSED;
    }

    target_id_testSetGet.set_tid( tid );
    target_id_testSetGet.set_rank( rank );
    target_id_testSetGet.set_syn_id( syn_id );
    target_id_testSetGet.set_lcid( lcid );
    target_id_testSetGet.set_status( status_target_id );

    BOOST_REQUIRE( target_id_testSetGet.get_tid() == tid );
    BOOST_REQUIRE( target_id_testSetGet.get_rank() == rank );
    BOOST_REQUIRE( target_id_testSetGet.get_syn_id() == syn_id );
    BOOST_REQUIRE( target_id_testSetGet.get_lcid() == lcid );
    BOOST_REQUIRE( target_id_testSetGet.get_status() == status_target_id );
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nest

#endif /* TEST_TARGET_FIELDS_H */
