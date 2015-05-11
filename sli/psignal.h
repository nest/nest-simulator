/*
 *  psignal.h
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

#ifndef PSIGNAL_H
#define PSIGNAL_H

/* Implement Posix Conforming signal function
    using Posix.1 sigaction
*/
/*
   The following is a POSIX.1 conforming implementation of
   the ISO C signal() funtion.
   Since Solaris 7 still sticks to the unreliable Signal mechanism
   of Unix SVR4, we decide to implement a new version, using
   the POSIX.1 sigaction function.
   The implementation is taken from
   Stevens, Richard W. (1993) "Advanced Programming in the UNIX Environment",
       Addison Wesley Longman, Reading, MA
*/
#ifndef _POSIX_SOURCE
#define _SYNOD__SET_POSIX_SOURCE
#define _POSIX_SOURCE
#endif


#include "config.h"


#ifdef HAVE_SIGUSR_IGNORED
#undef __PURE_CNAME
#include <signal.h>
#define __PURE_CNAME
#else
#include <signal.h>
#endif


typedef void Sigfunc( int );


#ifdef __cplusplus
extern "C" {
#endif


extern int SLIsignalflag;

Sigfunc* posix_signal( int, Sigfunc* );


void SLISignalHandler( int );

#ifdef _SYNOD__SET_POSIX_SOURCE
#undef _SYNOD__SET_POSIX_SOURCE
#undef _POSIX_SOURCE
#endif

#ifdef __cplusplus
}
#endif

#endif
