/*
 *  gauss_rate.cpp
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

#include "gauss_rate.h"

namespace nest
{

void
nonlinearities_gauss_rate::get( DictionaryDatum& d ) const
{
  def< double >( d, names::g, g_ );
  def< double >( d, names::mu, mu_ );
  def< double >( d, names::sigma, sigma_ );
}

void
nonlinearities_gauss_rate::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::g, g_ );
  updateValue< double >( d, names::mu, mu_ );
  updateValue< double >( d, names::sigma, sigma_ );
}

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< nest::gauss_rate_ipn >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::rate, &nest::gauss_rate_ipn::get_rate_ );
  insert_( names::noise, &nest::gauss_rate_ipn::get_noise_ );
}

template <>
void
RecordablesMap< nest::rate_transformer_gauss >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::rate, &nest::rate_transformer_gauss::get_rate_ );
}

} // namespace nest
