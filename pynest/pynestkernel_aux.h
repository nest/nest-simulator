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

// Always required: Dictionary and std::string must be visible in both the
// HAVE_LIBNEUROSIM and no-op branches because Cython emits calls to both
// CYTHON_isConnectionGenerator and CYTHON_insertConnectionGenerator
// unconditionally in the generated C++ regardless of the preprocessor state.
#include "dictionary.h"
#include <Python.h>
#include <string>

#if defined( HAVE_LIBNEUROSIM )

// External includes:
#include <memory>
#include <neurosim/connection_generator.h>
#include <neurosim/pyneurosim.h>

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

static inline void
CYTHON_insertConnectionGenerator( Dictionary& /* d */, const std::string& /* key */, PyObject* /* obj */ )
{
  // no-op: libneurosim not available
}

#endif  // #if defined( HAVE_LIBNEUROSIM )

#define CYTHON_DEREF( x ) ( *x )
#define CYTHON_ADDR( x ) ( &x )

#endif  // #ifndef PYNESTKERNEL_AUX_H
