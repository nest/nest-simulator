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

class StepPatternBuilder : public nest::ConnBuilder
{
public:
  StepPatternBuilder( const nest::NodeCollectionPTR sources,
    const nest::NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_spec );

protected:
  void connect_() override;

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
  static void advance_( nest::NodeCollection::const_iterator&, const nest::NodeCollection::const_iterator&, size_t );

  size_t source_step_;
  size_t target_step_;
};

} // namespace mynest

#endif
