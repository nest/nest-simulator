/*
 *  sigmoid_rate.cpp
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

#include "sigmoid_rate.h"

namespace nest
{

void
nonlinearities_sigmoid_rate::get( DictionaryDatum& d ) const
{
  def< double >( d, names::g, g_ );
  def< double >( d, names::beta, beta_ );
  def< double >( d, names::theta, theta_ );
}

void
nonlinearities_sigmoid_rate::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::g, g_, node );
  updateValueParam< double >( d, names::beta, beta_, node );
  updateValueParam< double >( d, names::theta, theta_, node );
}

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< nest::sigmoid_rate_ipn >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::rate, &nest::sigmoid_rate_ipn::get_rate_ );
  insert_( names::noise, &nest::sigmoid_rate_ipn::get_noise_ );
}

template <>
void
RecordablesMap< nest::rate_transformer_sigmoid >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::rate, &nest::rate_transformer_sigmoid::get_rate_ );
}

} // namespace nest
