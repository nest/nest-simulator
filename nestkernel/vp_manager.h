/*
 *  vp_manager.h
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

#ifndef VP_MANAGER_H
#define VP_MANAGER_H

#include <omp.h>

#include "manager_interface.h"
#include "dict.h"

namespace nest
{

class VPManager : ManagerInterface
{
public:
  virtual void init();
  virtual void reset();

  virtual void set_status( const Dictionary& );
  virtual void get_status( Dictionary& );

  /**
   * Gets ID of local thread.
   * Returns thread ID if OPENMP is installed
   * and zero otherwise.
   */
  int get_thread_id() const;
};
}

inline int
nest::VPManager::get_thread_id() const
{
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0;
#endif
}

#endif /* VP_MANAGER_H */
