/*
 *  ginzburg_neuron.cpp
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

#include "ginzburg_neuron.h"

namespace nest
{

void
gainfunction_ginzburg::get( DictionaryDatum& d ) const
{
  def< double >( d, names::theta, theta_ );
  def< double >( d, names::c_1, c1_ );
  def< double >( d, names::c_2, c2_ );
  def< double >( d, names::c_3, c3_ );
}

void
gainfunction_ginzburg::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::theta, theta_, node );
  updateValueParam< double >( d, names::c_1, c1_, node );
  updateValueParam< double >( d, names::c_2, c2_, node );
  updateValueParam< double >( d, names::c_3, c3_, node );
}

/*
 * Override the create() method with one call to RecordablesMap::insert_()
 * for each quantity to be recorded.
 */
template <>
void
RecordablesMap< nest::ginzburg_neuron >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::S, &nest::ginzburg_neuron::get_output_state__ );
  insert_( names::h, &nest::ginzburg_neuron::get_input__ );
}

} // namespace nest
