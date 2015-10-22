/*
 *  connection_manager.cpp
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

#include "connection_manager.h"
#include "connector_base.h"
#include "network.h"
#include "spikecounter.h"
#include "nest_time.h"
#include "nest_datums.h"
#include "kernel_manager.h"

#include <algorithm>

#include <typeinfo>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace nest
{

ConnectionManager::ConnectionManager()
{
}

ConnectionManager::~ConnectionManager()
{

}

void
ConnectionManager::init()
{
  init_();
}

void
ConnectionManager::init_()
{

}


void
ConnectionManager::reset()
{
}



void
ConnectionManager::get_status( DictionaryDatum& ) const
{
}

} // namespace
