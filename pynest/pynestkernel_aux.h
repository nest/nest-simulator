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
#include "dictionary.h"
#include <memory>
#include <neurosim/connection_generator.h>
#include <neurosim/pyneurosim.h>
#include <string>

#define CYTHON_isConnectionGenerator( x ) PNS::isConnectionGenerator( x )

static inline void
CYTHON_insertConnectionGenerator( Dictionary& d, const std::string& key, PyObject* obj )
{
  ConnectionGenerator* raw = PNS::unpackConnectionGenerator( obj );
  if ( raw )
  {
    d[ key ] = std::shared_ptr< ConnectionGenerator >( raw );
  }
}

#else  // #if defined( HAVE_LIBNEUROSIM )

#define CYTHON_isConnectionGenerator( x ) 0

#endif  // #if defined( HAVE_LIBNEUROSIM )

#define CYTHON_DEREF( x ) ( *x )
#define CYTHON_ADDR( x ) ( &x )

#endif  // #ifndef PYNESTKERNEL_AUX_H
