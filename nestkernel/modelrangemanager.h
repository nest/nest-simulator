/*
 *  modelrangemanager.h
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

#ifndef MODELRANGEMANAGER_H
#define MODELRANGEMANAGER_H

#include <vector>
#include "nest.h"
#include "modelrange.h"

namespace nest {

  class Modelrangemanager
  {
  public:
    Modelrangemanager();
    void add_range(index model, index first_gid, index last_gid);
    bool is_in_range(index gid) const {return ((gid <= last_gid_) && (gid >= first_gid_));}
    long_t get_model_id(index gid);
    bool model_in_use(index i) const;
    void clear();
    void print() const;
    const modelrange& get_range(index gid) const;

  private:
    std::vector<modelrange> modelranges_;
    index first_gid_;
    index last_gid_;
  };
}
#endif
