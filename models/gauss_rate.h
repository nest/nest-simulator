/*
 *  gauss_rate.h
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

#ifndef GAUSS_RATE_H
#define GAUSS_RATE_H

// Includes from c++:
#include <cmath>

// Includes from models:
#include "rate_neuron_ipn.h"
#include "rate_neuron_ipn_impl.h"
#include "rate_neuron_opn.h"
#include "rate_neuron_opn_impl.h"


namespace nest
{

class nonlinearities_gauss_rate
{
private:
  /** gain factor of gain function */
  double g_;
  double mu_;
  double sigma_;

public:
  /** sets default parameters */
  nonlinearities_gauss_rate()
    : g_( 1.0 )
    , mu_( 0.0 )
    , sigma_( 0.0 )
  {
  }

  void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  void set( const DictionaryDatum& ); //!< Set values from dicitonary

  double input( double h );            // non-linearity on input
  double mult_coupling_ex( double h ); // factor of multiplicative coupling
  double mult_coupling_in( double h ); // factor of multiplicative coupling
};

inline double
nonlinearities_gauss_rate::input( double h )
{
  return g_ * ( std::exp( -pow( h - mu_, 2. ) / ( 2. * pow( sigma_, 2. ) ) ) );
}

inline double
nonlinearities_gauss_rate::mult_coupling_ex( double h )
{
  return 1.;
}

inline double
nonlinearities_gauss_rate::mult_coupling_in( double h )
{
  return 1.;
}

typedef rate_neuron_ipn< nest::nonlinearities_gauss_rate > gauss_rate_ipn;
typedef rate_neuron_opn< nest::nonlinearities_gauss_rate > gauss_rate_opn;

template <>
void RecordablesMap< gauss_rate_ipn >::create();
template <>
void RecordablesMap< gauss_rate_opn >::create();

} // namespace nest


#endif /* #ifndef GAUSS_RATE_H */
