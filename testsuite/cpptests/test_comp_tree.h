/*
 *  test_comp_tree.h
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

#ifndef TEST_COMP_TREE_H
#define TEST_COMP_TREE_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "compartment_tree_neat.h"
#include "synapses_neat.h"

namespace nest
{

BOOST_AUTO_TEST_SUITE( test_comp_tree )

BOOST_AUTO_TEST_CASE( test_matrix_inversion )
{
  const double dt = Time::get_resolution().get_ms();
  const double ca0 = 1.0, gc0 = 0.1, gl0 = 0.10, el0 = -70.0;
  const double ca1 = 0.1, gc1 = 0.1, gl1 = 0.01, el1 = -70.0;

  CompTree m_c_tree;

  m_c_tree.add_node( 0, -1, ca0, gc0, gl0, el0 );
  m_c_tree.add_node( 1, 0, ca1, gc1, gl1, el1 );
  m_c_tree.init();

  // input current
  std::vector< double > i_in{ 0.1, 0.2 };

  // compartment tree solution
  m_c_tree.construct_matrix( i_in, 0 );
  m_c_tree.solve_matrix();
  const std::vector< double > v_sol = m_c_tree.get_voltage();

  // hand-crafted solution
  const double a00 = ca0 / dt + gl0 / 2. + gc1 / 2.;
  const double a01 = -gc1 / 2.;
  const double a10 = -gc1 / 2.;
  const double a11 = ca1 / dt + gl1 / 2. + gc1 / 2.;

  const double b0 = ca0 / dt * el0 - gl0 * ( el0 / 2. - el0 ) + gc1 * ( el0 - el1 ) / 2. + i_in[ 0 ];
  const double b1 = ca1 / dt * el1 - gl1 * ( el1 / 2. - el1 ) + gc1 * ( el1 - el0 ) / 2. + i_in[ 1 ];

  const double det = a00 * a11 - a10 * a01;
  const double v0 = ( b0 * a11 - b1 * a01 ) / det;
  const double v1 = ( b1 * a00 - b0 * a10 ) / det;

  // compare tree solution to handcrafted solution
  BOOST_CHECK_CLOSE( v0, v_sol[ 0 ], 0.00001 );
  BOOST_CHECK_CLOSE( v1, v_sol[ 1 ], 0.00001 );
}

BOOST_AUTO_TEST_CASE( test_matrix_inversion3 )
{
  const double dt = Time::get_resolution().get_ms();
  const double ca0 = 1.0, gc0 = 0.10, gl0 = 0.10, el0 = -70.0;
  const double ca1 = 0.1, gc1 = 0.10, gl1 = 0.01, el1 = -70.0;
  const double ca2 = 0.2, gc2 = 0.15, gl2 = 0.02, el2 = -70.0;

  CompTree m_c_tree;

  m_c_tree.add_node( 0, -1, ca0, gc0, gl0, el0 );
  m_c_tree.add_node( 1, 0,  ca1, gc1, gl1, el1 );
  m_c_tree.add_node( 2, 0,  ca2, gc2, gl2, el2 );
  m_c_tree.init();

  // input current
  std::vector< double > i_in{ 0.1, 0.2, 0.3 };

  // compartment tree solution
  m_c_tree.construct_matrix( i_in, 0 );
  m_c_tree.solve_matrix();
  const std::vector< double > v_sol = m_c_tree.get_voltage();

  // hand-crafted solution
  const double a00 = ca0 / dt + gl0 / 2. + gc1 / 2. + gc2 / 2.;
  const double a01 = -gc1 / 2.;
  const double a10 = -gc1 / 2.;
  const double a11 = ca1 / dt + gl1 / 2. + gc1 / 2.;
  const double a02 = -gc2 / 2.;
  const double a20 = -gc2 / 2.;
  const double a22 = ca2 / dt + gl2 / 2. + gc2 / 2.;

  const double b0 = ca0 / dt * el0 - gl0 * ( el0 / 2. - el0 ) + gc1 * ( el0 - el1 ) / 2 + gc2 * ( el0 - el2 ) / 2. + i_in[ 0 ];
  const double b1 = ca1 / dt * el1 - gl1 * ( el1 / 2. - el1 ) + gc1 * ( el1 - el0 ) / 2. + i_in[ 1 ];
  const double b2 = ca2 / dt * el2 - gl2 * ( el2 / 2. - el2 ) + gc2 * ( el2 - el0 ) / 2. + i_in[ 1 ];

  // compute the minors


  const double det = a00 * a11 - a10 * a01;
  const double v0 = ( b0 * a11 - b1 * a01 ) / det;
  const double v1 = ( b1 * a00 - b0 * a10 ) / det;

  // compare tree solution to handcrafted solution
  BOOST_CHECK_CLOSE( v0, v_sol[ 0 ], 0.00001 );
  BOOST_CHECK_CLOSE( v1, v_sol[ 1 ], 0.00001 );
}

BOOST_AUTO_TEST_CASE( test_attenuation_integration )
{
  const double ca0 = 0.10, gc0 = 0.00, gl0 = 0.010, el0 = -70.0;
  const double ca1 = 0.01, gc1 = 0.01, gl1 = 0.001, el1 = -70.0;

  CompTree m_c_tree;

  m_c_tree.add_node( 0, -1, ca0, gc0, gl0, el0 );
  m_c_tree.add_node( 1, 0, ca1, gc1, gl1, el1 );
  m_c_tree.init();

  std::vector< double > i_in = std::vector< double >( 2 );

  // attenuation 1->0
  m_c_tree.init();
  i_in[0] = 0., i_in[1] = 0.001;
  for(int ii = 0; ii < 10000; ii++){
    m_c_tree.construct_matrix(i_in, 0);
    m_c_tree.solve_matrix();

  }
  std::vector< double > v_sol = m_c_tree.get_voltage();

  BOOST_CHECK_CLOSE( gc1 / (gl0 + gc1), (v_sol[0] - el0) / (v_sol[1] - el1), 0.00000001 );

  // attenuation 0->1
  m_c_tree.init();
  i_in[0] = 0.15, i_in[1] = 0.;
  for(int ii = 0; ii < 10000; ii++){
    m_c_tree.construct_matrix(i_in, 0);
    m_c_tree.solve_matrix();
  }
  v_sol = m_c_tree.get_voltage();

  BOOST_CHECK_CLOSE( gc1 / (gl1 + gc1), (v_sol[1] - el1) / (v_sol[0] - el0), 0.00000001 );
}

BOOST_AUTO_TEST_SUITE_END()
}

#endif /* TEST_COMP_TREE_H */
