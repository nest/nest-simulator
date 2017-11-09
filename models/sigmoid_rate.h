/*
 *  sigmoid_rate.h
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

#ifndef SIGMOID_RATE_H
#define SIGMOID_RATE_H

// Includes from c++:
#include <cmath>

// Includes from models:
#include "rate_neuron_ipn.h"
#include "rate_neuron_ipn_impl.h"
#include "rate_neuron_opn.h"
#include "rate_neuron_opn_impl.h"


namespace nest
{

class nonlinearities_sigmoid_rate
{
private:
  /** gain factor of gain function */
  double g_;
  double beta_;
  double bias_;

public:
  /** sets default parameters */
  nonlinearities_sigmoid_rate()
    : g_( 1.0 )
    , beta_( 1.0 )
    , bias_( 0.0 )
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  double input( double h );               // non-linearity on input
  double mult_coupling_ex( double rate ); // factor of multiplicative coupling
  double mult_coupling_in( double rate ); // factor of multiplicative coupling
};

inline double
nonlinearities_sigmoid_rate::input( double h )
{
  return g_ / ( 1. + std::exp( -beta_ * ( h - bias_ ) ) );
}

inline double
nonlinearities_sigmoid_rate::mult_coupling_ex( double rate )
{
  return 1.;
}

inline double
nonlinearities_sigmoid_rate::mult_coupling_in( double rate )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_sigmoid_rate > sigmoid_rate_ipn;
typedef rate_neuron_opn< nest::nonlinearities_sigmoid_rate > sigmoid_rate_opn;

template <>
void RecordablesMap< sigmoid_rate_ipn >::create();
template <>
void RecordablesMap< sigmoid_rate_opn >::create();

} // namespace nest


#endif /* #ifndef SIGMOID_RATE_H */
