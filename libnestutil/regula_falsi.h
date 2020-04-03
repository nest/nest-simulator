/*
 *  regula_falsi.h
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

#ifndef REGULA_FALSI_H
#define REGULA_FALSI_H


namespace nest
{

/**
 * Localize threshold crossing by using Illinois algorithm of regula falsi method.
 *
 * See https://en.wikipedia.org/wiki/Regula_falsi#The_Illinois_algorithm for
 * details on the algorithm.
 *
 * @param   Node   model object that call function
 * @param   double length of interval since previous event
 * @returns time from previous event to threshold crossing
 */
template < typename CN >
double
regula_falsi( const CN& node, const double dt )
{
  double root;
  double threshold_dist_root;

  int last_threshold_sign = 0;

  double a_k = 0.0;
  double b_k = dt;

  double threshold_dist_a_k = node.threshold_distance( a_k );
  double threshold_dist_b_k = node.threshold_distance( b_k );

  if ( threshold_dist_a_k * threshold_dist_b_k > 0 )
  {
    throw NumericalInstability( "regula_falsi: time step too short to reach threshold." );
  }

  const int MAX_ITER = 500;
  const double TERMINATION_CRITERION = 1e-14;

  for ( int iter = 0; iter < MAX_ITER; ++iter )
  {
    assert( threshold_dist_b_k != threshold_dist_a_k );

    root = ( a_k * threshold_dist_b_k - b_k * threshold_dist_a_k ) / ( threshold_dist_b_k - threshold_dist_a_k );
    threshold_dist_root = node.threshold_distance( root );

    if ( std::abs( threshold_dist_root ) < TERMINATION_CRITERION )
    {
      break;
    }

    if ( threshold_dist_a_k * threshold_dist_root > 0.0 )
    {
      // threshold_dist_a_k and threshold_dist_root have the same sign
      a_k = root;
      threshold_dist_a_k = threshold_dist_root;

      if ( last_threshold_sign == 1 )
      {
        // If threshold_dist_a_k and threshold_dist_root had the same sign in
        // the last time step, we halve the value of threshold_distance_(b_k) to
        // force the root in the next time step to occur on b_k's side. This is
        // done to improve the convergence rate.
        threshold_dist_b_k /= 2;
      }
      last_threshold_sign = 1;
    }
    else if ( threshold_dist_b_k * threshold_dist_root > 0.0 )
    {
      // threshold_dist_b_k and threshold_dist_root have the same sign
      b_k = root;
      threshold_dist_b_k = threshold_dist_root;

      if ( last_threshold_sign == -1 )
      {
        threshold_dist_a_k /= 2;
      }
      last_threshold_sign = -1;
    }
    else
    {
      throw NumericalInstability( "regula_falsi: Regula falsi method did not converge" );
    }

    if ( iter == MAX_ITER - 1 )
    {
      throw NumericalInstability(
        "regula_falsi: Regula falsi method did not converge during set number of iterations" );
    }
  }
  return root;
}

} // namespace nest

#endif // REGULA_FALSI_H
