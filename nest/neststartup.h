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

#if defined( HAVE_LIBNEUROSIM ) && defined( _IS_PYNEST )

#include <neurosim/pyneurosim.h>

#include "datum.h"
#include "conngenmodule.h"

#define CYTHON_isConnectionGenerator( x ) PNS::isConnectionGenerator( x )
Datum* CYTHON_unpackConnectionGeneratorDatum( PyObject* );

#else
#define CYTHON_isConnectionGenerator( x ) 0
#define CYTHON_unpackConnectionGeneratorDatum( x ) NULL
#endif

namespace nest
{
class Network;
}

class SLIInterpreter;

#ifdef _IS_PYNEST

#define CYTHON_DEREF( x ) ( *x )
#define CYTHON_ADDR( x ) ( &x )

#include <string>
int neststartup( int* argc,
  char*** argv,
  SLIInterpreter& engine,
  nest::Network*& pNet,
  std::string modulepath = "" );

#else
int neststartup( int* argc, char*** argv, SLIInterpreter& engine, nest::Network*& pNet );
#endif

void nestshutdown( void );

#endif
