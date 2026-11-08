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
#include "../nestkernel/random_generators.h"
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
/**
 * Random generator whose drand() returns a value set by the test, so that the roulette wheel can be
 * probed at chosen points. The roulette wheel only uses drand(); every other draw fails loudly.
 */
class FixedDrandRng : public BaseRandomGenerator
{
public:
  double value = 0.0;

  double
  drand() override
  {
    return value;
  }

  // clang-format off
  unsigned long operator()( std::uniform_int_distribution< unsigned long >& ) override { return unused_(); }
  double operator()( std::uniform_real_distribution<>& ) override { return unused_(); }
  double operator()( std::normal_distribution<>& ) override { return unused_(); }
  double operator()( std::lognormal_distribution<>& ) override { return unused_(); }
  unsigned long operator()( std::binomial_distribution< unsigned long >& ) override { return unused_(); }
  double operator()( std::gamma_distribution<>& ) override { return unused_(); }
  double operator()( std::exponential_distribution<>& ) override { return unused_(); }
  unsigned long operator()( std::discrete_distribution< unsigned long >& ) override { return unused_(); }
  unsigned long operator()( std::poisson_distribution< unsigned long >& ) override { return unused_(); }
  unsigned long operator()( std::uniform_int_distribution< unsigned long >&,
    std::uniform_int_distribution< unsigned long >::param_type& ) override { return unused_(); }
  double operator()( std::uniform_real_distribution<>&, std::uniform_real_distribution<>::param_type& ) override { return unused_(); }
  double operator()( std::normal_distribution<>&, std::normal_distribution<>::param_type& ) override { return unused_(); }
  double operator()( std::lognormal_distribution<>&, std::lognormal_distribution<>::param_type& ) override { return unused_(); }
  unsigned long operator()( std::binomial_distribution< unsigned long >&,
    std::binomial_distribution< unsigned long >::param_type& ) override { return unused_(); }
  double operator()( std::gamma_distribution<>&, std::gamma_distribution<>::param_type& ) override { return unused_(); }
  double operator()( std::exponential_distribution<>&, std::exponential_distribution<>::param_type& ) override { return unused_(); }
  unsigned long operator()( std::discrete_distribution< unsigned long >&,
    std::discrete_distribution< unsigned long >::param_type& ) override { return unused_(); }
  unsigned long operator()( std::poisson_distribution< unsigned long >&,
    std::poisson_distribution< unsigned long >::param_type& ) override { return unused_(); }
  unsigned long ulrand( unsigned long ) override { return unused_(); }
  void stable_sample( NodeCollection::const_iterator, NodeCollection::const_iterator,
    std::back_insert_iterator< std::vector< NodeIDTriple > >, size_t ) override { unused_(); }
  void partial_shuffle( std::vector< size_t >&, const size_t ) override { unused_(); }
  // clang-format on

private:
  static unsigned long
  unused_()
  {
    throw std::logic_error( "FixedDrandRng only provides drand()." );
  }
};

BOOST_AUTO_TEST_SUITE( test_distance_dependent )

BOOST_AUTO_TEST_CASE( test_roulette_wheel_selection_boundaries )
{
  SPManager sp_manager;
  FixedDrandRng rng;

  // Returns the ID selected when drand() yields rnd.
  const auto select = [ & ]( const std::vector< size_t >& ids, const std::vector< double >& weights, double rnd )
  {
    rng.value = rnd;
    return sp_manager.roulette_wheel_selection( ids, weights, &rng );
  };

  // With equal weights, the unit interval is split into equal parts.
  const std::vector< size_t > four_ids { 11, 12, 13, 14 };
  const std::vector< double > uniform { 1.0, 1.0, 1.0, 1.0 };
  BOOST_REQUIRE_EQUAL( select( four_ids, uniform, 0.0 ), 11u );
  BOOST_REQUIRE_EQUAL( select( four_ids, uniform, 0.3 ), 12u );
  BOOST_REQUIRE_EQUAL( select( four_ids, uniform, 0.6 ), 13u );
  BOOST_REQUIRE_EQUAL( select( four_ids, uniform, 0.9 ), 14u );

  // Weights need not be normalised, and are interpreted relative to their sum.
  const std::vector< size_t > two_ids { 21, 22 };
  const std::vector< double > unnormalised { 30.0, 10.0 };
  BOOST_REQUIRE_EQUAL( select( two_ids, unnormalised, 0.7 ), 21u );
  BOOST_REQUIRE_EQUAL( select( two_ids, unnormalised, 0.8 ), 22u );

  // A zero weight is never selected, whatever the random number and wherever it sits.
  BOOST_REQUIRE_EQUAL( select( two_ids, { 0.0, 1.0 }, 0.0 ), 22u );
  BOOST_REQUIRE_EQUAL( select( two_ids, { 0.0, 1.0 }, 0.999999 ), 22u );
  const std::vector< size_t > three_ids { 31, 32, 33 };
  const std::vector< double > zero_in_middle { 1.0, 0.0, 1.0 };
  for ( const double rnd : { 0.0, 0.25, std::nextafter( 0.5, 0.0 ), 0.5, 0.75, std::nextafter( 1.0, 0.0 ) } )
  {
    BOOST_REQUIRE_NE( select( three_ids, zero_in_middle, rnd ), 32u );
  }

  // A random number arbitrarily close to one still yields a valid ID.
  BOOST_REQUIRE_EQUAL( select( four_ids, uniform, std::nextafter( 1.0, 0.0 ) ), 14u );
}

BOOST_AUTO_TEST_CASE( test_roulette_wheel_selection_rejects_degenerate_weights )
{
  SPManager sp_manager;
  FixedDrandRng rng;
  rng.value = 0.5;

  const std::vector< size_t > two_ids { 1, 2 };
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( {}, {}, &rng ), std::runtime_error );
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( two_ids, { 0.0, 0.0 }, &rng ), std::runtime_error );
  BOOST_REQUIRE_THROW(
    sp_manager.roulette_wheel_selection( two_ids, { std::numeric_limits< double >::infinity(), 1.0 }, &rng ),
    std::runtime_error );
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( two_ids, { std::nan( "" ), 1.0 }, &rng ), std::runtime_error );

  // Every candidate ID needs exactly one weight.
  BOOST_REQUIRE_THROW( sp_manager.roulette_wheel_selection( { 1, 2, 3 }, { 1.0, 1.0 }, &rng ), std::runtime_error );
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace nest

#endif /* TEST_DISTANCE_DEPENDENT_H */
