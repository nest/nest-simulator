/*
 *  puresli.cc
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

/*
    SLI main
*/

// Generated includes:
#include "config.h"

// Includes from sli:
#include "filesystem.h"
#include "gnureadline.h"
#include "integerdatum.h"
#include "interpret.h"
#include "oosupport.h"
#include "processes.h"
#include "sliarray.h"
#include "sliregexp.h"
#include "slistartup.h"
#include "tokenutils.h"

int
main( int argc, char* argv[] )
{
/**
  We disable synchronization between stdio and istd::ostreams
  this has to be done before any in- or output has been
  done.
*/

#ifdef __GNUC__
#if __GNUC__ < 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ < 1 )
  // Broken with GCC 3.1 and higher.
  // cin.get() never returns, or leaves cin in a broken state.
  //
  std::ios::sync_with_stdio( false );
#endif
#else
  // This is for all other compilers
  std::ios::sync_with_stdio( false );
#endif


  // Create the interpreter object. Due to its dependence
  // on various static objects (e.g. of class Name), the
  // interpreter engine MUST NOT be global.

  SLIInterpreter engine;
#ifdef HAVE_READLINE
  addmodule< GNUReadline >( engine );
#endif
  addmodule< SLIArrayModule >( engine );
  addmodule< OOSupportModule >( engine );
  engine.addmodule( new SLIStartup( argc, argv ) );
  addmodule< Processes >( engine );
  addmodule< RegexpModule >( engine );
  addmodule< FilesystemModule >( engine );
  int exitcode = engine.execute( 1 );

  return exitcode;
}
