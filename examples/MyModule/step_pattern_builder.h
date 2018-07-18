/*
 *  step_pattern_builder.h
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

#ifndef STEP_PATTERN_BUILDER_H
#define STEP_PATTERN_BUILDER_H

// Includes from nestkernel:
#include "conn_builder.h"

namespace mynest
{

/* BeginDocumentation
   Name: step_pattern - Rule connecting sources and targets with a step pattern

   Synopsis:
   source targets << /rule /step_pattern
                     /source_step M
                     /target_step N >> << syn spec >> Connect

   Parameters:
   source_step   - Make connection from every source_step'th neuron
   target_step   - Make connection to every target_step'th neuron

   Description:
   This connection rule subsamples the source and target arrays given with
   step sizes source_step and target_step, beginning with the first element
   in each array, and connects the selected nodes. If source_step and
   target_step both are equal 1, step_pattern is equivalent to all_to_all.

   Example:

   /n /iaf_psc_alpha 10 Create 1 exch cvgidcollection def
   n n << /rule /step_pattern /source_step 4 /target_step 3 >> Connect
   << >> GetConnections ==

     [<1,1,0,0,0> <1,4,0,0,1> <1,7,0,0,2> <1,10,0,0,3>
      <5,1,0,0,0> <5,4,0,0,1> <5,7,0,0,2> <5,10,0,0,3>
      <9,1,0,0,0> <9,4,0,0,1> <9,7,0,0,2> <9,10,0,0,3>]

   Remark:
   This rule is only provided as an example for how to write your own
   connection rule function.

   Author:
   Hans Ekkehard Plesser

   SeeAlso: Connect
*/

class StepPatternBuilder : public nest::ConnBuilder
{
public:
  StepPatternBuilder( const nest::GIDCollection& sources,
    const nest::GIDCollection& targets,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_spec );

protected:
  void connect_();

private:
  /**
   * Helper function advancing iterator by step positions.
   *
   * Will stop at end of container.
   *
   * @param Initial position
   * @param End of container
   * @param Number of positions to advance
   * @return Iterator after advance
   */
  static nest::GIDCollection::const_iterator& advance_(
    nest::GIDCollection::const_iterator&,
    const nest::GIDCollection::const_iterator&,
    size_t );

  size_t source_step_;
  size_t target_step_;
};

} // namespace mynest

#endif
