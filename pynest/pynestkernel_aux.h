/*
 *  pynestkernel_aux.h
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

#ifndef PYNESTKERNEL_AUX_H
#define PYNESTKERNEL_AUX_H

// Generated includes:
#include "config.h"

#if defined( HAVE_LIBNEUROSIM )

// External includes:
#include <neurosim/pyneurosim.h>

// Includes from conngen:
#include "conngenmodule.h"

#define CYTHON_isConnectionGenerator( x ) PNS::isConnectionGenerator( x )

Datum*
CYTHON_unpackConnectionGeneratorDatum( PyObject* obj )
{
  Datum* ret = NULL;
  ConnectionGenerator* cg = NULL;

  cg = PNS::unpackConnectionGenerator( obj );
  if ( cg != NULL )
  {
    ret = static_cast< Datum* >( new nest::ConnectionGeneratorDatum( cg ) );
  }

  return ret;
}

#else // #if defined( HAVE_LIBNEUROSIM )

#define CYTHON_isConnectionGenerator( x ) 0
#define CYTHON_unpackConnectionGeneratorDatum( x ) NULL

#endif // #if defined( HAVE_LIBNEUROSIM )

#define CYTHON_DEREF( x ) ( *x )
#define CYTHON_ADDR( x ) ( &x )

#endif // #ifndef PYNESTKERNEL_AUX_H
