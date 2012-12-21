/*
 *  modelrangemanager.cpp
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

#include <assert.h>
#include <iostream>
#include "modelrangemanager.h"
#include "exceptions.h"

namespace nest {

Modelrangemanager::Modelrangemanager() :
  first_gid_(0), last_gid_(0)
  { } 

void Modelrangemanager::add_range(index model, index first_gid, index last_gid)
{
  if (!modelranges_.empty())
  {
    assert(first_gid == last_gid_ + 1);
    if (model == modelranges_.back().get_model_id())
      modelranges_.back().extend_range(last_gid);
    else
      modelranges_.push_back(modelrange(model,first_gid,last_gid));
  }
  else
  {
    modelranges_.push_back(modelrange(model,first_gid,last_gid));
    first_gid_ = first_gid;
  }

  last_gid_ = last_gid;
}

  long_t Modelrangemanager::get_model_id(index gid)
{
  int left = -1;
  int right = modelranges_.size();
  assert(right >= 1);
  assert(is_in_range(gid));

  // to ensure thread-safety, use local range_idx
  size_t range_idx = right / 2;  // start in center
  while (!modelranges_[range_idx].is_in_range(gid))
  {
    if (gid > modelranges_[range_idx].get_last_gid())
    {
      left = range_idx;
      range_idx += (right - range_idx)/2;
    }
    else
    {
      right = range_idx;
      range_idx -= (range_idx - left)/2;
    }
    assert(left+1 < right);
    assert(range_idx < modelranges_.size());
  }
  return modelranges_[range_idx].get_model_id();
}

bool Modelrangemanager::model_in_use(index i) const
{
   bool found = false;

   for (std::vector<modelrange>::const_iterator it = modelranges_.begin(); it != modelranges_.end(); it++)
     if ( it->get_model_id() == i )
     {
       found = true;
       break;
     }

  return found;
}

void Modelrangemanager::clear()
{
  modelranges_.clear();
}

const modelrange& Modelrangemanager::get_range(index gid) const
{
  if (!is_in_range(gid))
      throw UnknownNode(gid);
  
  for (std::vector<modelrange>::const_iterator it = modelranges_.begin(); it != modelranges_.end(); it++)
    if ( it->is_in_range(gid) )
      return (*it);

  throw UnknownNode(gid);
}

} // namespace nest
