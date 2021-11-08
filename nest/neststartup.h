/*
 *  neststartup.h
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

#ifndef NEST_STARTUP_H
#define NEST_STARTUP_H

// Generated includes:
#include "config.h"

#if defined( HAVE_LIBNEUROSIM ) && defined( _IS_PYNEST )
#include <neurosim/pyneurosim.h>
#include "datum.h"
#define CYTHON_isConnectionGenerator( x ) PNS::isConnectionGenerator( x )
Datum* CYTHON_unpackConnectionGeneratorDatum( PyObject* );
#else
#define CYTHON_isConnectionGenerator( x ) 0
#define CYTHON_unpackConnectionGeneratorDatum( x ) NULL
#endif

class SLIInterpreter;

#ifdef _IS_PYNEST
#define CYTHON_DEREF( x ) ( *x )
#define CYTHON_ADDR( x ) ( &x )
#include <string>
int neststartup( int* argc, char*** argv, SLIInterpreter& engine, std::string modulepath = "" );
#else  // #ifdef _IS_PYNEST
int neststartup( int* argc, char*** argv, SLIInterpreter& engine );
#endif // #ifdef _IS_PYNEST

void nestshutdown( int exitcode );

SLIInterpreter& get_engine();

#ifdef _IS_PYNEST
#include <Python.h>
// Call only with GIL
void set_communicator( PyObject* );

inline bool
nest_has_mpi4py()
{
#ifdef HAVE_MPI4PY
  return true;
#else
  return false;
#endif // HAVE_MPI4PY
}

#endif // _IS_PYNEST

#endif // #ifndef NEST_STARTUP_H
