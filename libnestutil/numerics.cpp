/*
 *  numerics.cpp
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

#include "numerics.h"

#ifndef HAVE_M_E

#ifdef HAVE_CMATH_MAKROS_IGNORED
#define M_E_OK
#undef __PURE_CNAME
#include <cmath>
#define __PURE_CNAME
#else
#include <cmath>
#endif

#else
#define M_E_OK
#include <cmath>
#endif


#ifndef HAVE_M_PI

#ifdef HAVE_CMATH_MAKROS_IGNORED
#define M_PI_OK
#endif

#else
#define M_PI_OK
#endif

#if defined( HAVE_STD_NAN )
#include <cmath>
#elif defined( HAVE_NAN )
#include <math.h>
#endif

//
//   e
//
#ifdef HAVE_GSL

#include <gsl/gsl_math.h>
const double numerics::e = M_E;
const double numerics::pi = M_PI;

#else

#ifdef M_E_OK
const double numerics::e = M_E;
#else
const double numerics::e = 2.71828182845904523536028747135;
#endif

#ifdef M_PI_OK
const double numerics::pi = M_PI;
#else
const double numerics::pi = 3.14159265358979323846264338328;
#endif

#endif

#if defined( HAVE_STD_NAN )
const double numerics::nan = ::nan( "" );
#elif defined( HAVE_NAN )
const double numerics::nan = NAN;
#else
const double numerics::nan = 0.0 / 0.0;
#endif

// later also in namespace
long
ld_round( double x )
{
  return ( long ) std::floor( x + 0.5 );
}

double
dround( double x )
{
  return std::floor( x + 0.5 );
}

double
dtruncate( double x )
{
  double ip;

  std::modf( x, &ip );
  return ip;
}
