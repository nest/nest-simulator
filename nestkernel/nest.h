#ifndef NEST_H
#define NEST_H
/*
 *  nest.h
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

#include <cstddef>
#include <climits>
#include "config.h"

/**
 * @mainpage NEST: Neural Simulation Tool
 *
 * The main resource on information about NEST is the homepage of the
 * NEST Initiative at http://www.nest-initiative.org
 * <p>
 *
 * @see Diesmann, Markus and Gewaltig, Marc-Oliver (2002) NEST: An
 * Environment for Neural Systems Simulations Goettingen : Ges. fuer
 * Wiss. Datenverarbeitung, Forschung und wisschenschaftliches
 * Rechnen, Beitraege zum Heinz-Billing-Preis 2001. 58 : 43--70
 *
 * @see Morrison, Abigail and Mehring, Carsten and Geisel, Theo and
 * Aertsen, Ad and Diesmann, Markus (2005) Advancing the boundaries of
 * high connectivity network simulation with distributed computing
 * Neural Computation. 17 (8) : 1776--1801
 *
 * @see Eppler, Jochen, Diploma Thesis, University of Freiburg (2006),
 * http://mindzoo.de/files/DiplomaThesis-Eppler.pdf
 *
 * @see Eppler, Jochen and Helias, Moritz and Muller, Eilif and
 * Diesmann, Markus and Gewaltig, Marc-Oliver (2008) PyNEST: A
 * convenient interface to the NEST simulator.  Front. Neuroinform. 2
 * : 12. doi:10.3389/neuro.11.012.2008
 *
 * (C) Copyright 1995-2006 The NEST Initiative.
 */


/**
 * Namespace for the NEST simulation kernel.
 */

namespace nest {

  /**
   * \file nest.h 
   * Default types used by the NEST kernel. 
   * These typedefs should be used
   * in place of the primitive C/C++ types. 
   * Thus, it will be easy to change
   * the precision of the kernel or to adapt the kernel to
   * different architectures (e.g. 32 or 64 bit).
   */

  /**
   * Type for Time tics.
   */
#ifdef HAVE_LONG_LONG
  typedef long long tic_t;
#else
  typedef long      tic_t;
#endif

  using std::size_t;

  typedef double   double_t; ///< Double precision floating point numbers.
  typedef float    float_t;  ///< Single precision floating point numbers. 
  typedef int      int_t;    ///< Integer number with at least 16 bit.
  typedef long     long_t;   ///< Integer number with at least 32 bit.
  typedef unsigned int  uint_t; ///< Unsigned int_t.
  typedef unsigned long ulong_t; ///< Unsigned long_t.

  /**
   *  Unsigned long type for enumerations.
   */

  typedef size_t index;
  const size_t INDEX_MAX = UINT_MAX;

  /**
   * Thread index type.
   * NEST threads are assigned non-negative numbers for
   * identification.
   * For invalid or undefined threads, the value -1 is used.
   */
  typedef int_t thread;

  /**
   * Value for invalid connection port number.
   */
  const thread invalid_thread_ = -1;

  /**
   * Connection port number to distinguish incoming connections,
   * also called receiver port.
   * Connections between Nodes are assigned port numbers.
   * Valid port numbers start at zero (0).
   * The value -1 is used for invalid or unassigned ports.
   */
  typedef long_t rport;

  /**
   * Connection port number to distinguis outgoing connections.
   * Connections between Nodes are assigned port numbers.
   * Valid port numbers start at zero (0).
   * The value -1 is used for invalid or unassigned ports.
   */
  typedef long_t port;

  /**
   * Value for invalid connection port number.
   */
  const rport invalid_port_ = -1;
  
  /**
   * Weight of a connection.
   * Connections have a weight which is used to scale the influence
   * of an event.
   * A weight of 0 should have the same influence on the receiving node 
   * as a non-existing connection. Otherwise, there is no default range for 
   * connection weights.
   */
  typedef double_t weight;

  /**
   * Delay of a connection.
   * The delay defines the number of simulation steps which elapse 
   * before an Event arrives at the receiving Node.
   * Delays must be equal or larger than one.
   */
  typedef uint_t delay;

}
#endif
