/*
 *  music_manager.cpp
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

#include "music_manager.h"

// C includes:
//#include <dirent.h>
//#include <errno.h>
//#include <sys/types.h>

// C++ includes:
//#include <cstdlib>

// Includes from libnestutil:
//#include "compose.hpp"
//#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest {

MUSICManager::MUSICManager()
{
}

void
MUSICManager::initialize()
{
}

void
MUSICManager::finalize()
{
}

/*
     - set the ... properties
*/
void
MUSICManager::set_status( const DictionaryDatum& d )
{
}

void
MUSICManager::get_status( DictionaryDatum& d )
{
}

void
MUSICManager::init_music( int* argc, char** argv[] )
{
#ifdef HAVE_MUSIC
  int provided_thread_level;
  music_setup = new MUSIC::Setup( *argc, *argv, MPI_THREAD_FUNNELED, &provided_thread_level );
#endif
}

void
MUSICManager::set_music_in_port_acceptable_latency( std::string portname, double_t latency )
{
}

void
MUSICManager::set_music_in_port_max_buffered( std::string portname, int_t maxbuffered )
{
}

#ifdef HAVE_MUSIC
MPI_Comm
MUSICManager::communicator ()
{
  return music_setup->communicator ();
}
#endif

}
