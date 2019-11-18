/*
 *  nest_types.h
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

#ifndef NEST_TYPES_H
#define NEST_TYPES_H

// C++ includes:
#include <cfloat>
#include <climits>
#include <cstddef>
#include <limits>
#include <stdint.h>

// Generated includes:
#include "config.h"

#ifdef HAVE_32BIT_ARCH
#ifdef HAVE_UINT64_T // 32-bit platforms usually provide the ...
#include <stdint.h>  // ... 64-bit unsigned integer data type 'uint64_t' in stdint.h
#else
#error "32-bit platform does not provide a 64-bit unsigned integer data type"
#endif
#else
#include <cstdint> // `uint64_t` on 64-bit platforms
#endif

/**
 * Namespace for the NEST simulation kernel.
 */

namespace nest
{

/**
 * \file nest_types.h
 * Default types used by the NEST kernel.
 * These typedefs should be used
 * in place of the primitive C/C++ types.
 * Thus, it will be easy to change
 * the precision of the kernel or to adapt the kernel to
 * different architectures (e.g. 32 or 64 bit).
 */

// constexpr-functions for convenient compile-time generation of the bit-masks
// and bit-constants. An ill-defined length or size will cause a compile-time
// error, e.g., num_bits to be shifted exceeds the sizeof(<datatype>) * 8.
constexpr uint64_t
generate_bit_mask( const uint8_t num_bits, const uint8_t bit_position )
{
  return ( ( ( static_cast< uint64_t >( 1 ) << num_bits ) - 1 ) << bit_position );
}

constexpr uint64_t
generate_max_value( const uint8_t num_bits )
{
  return ( ( static_cast< uint64_t >( 1 ) << num_bits ) - 1 );
}

/*
 * Sizes of bitfields used in various classes in the kernel.
 */
#if TARGET_BITS_SPLIT == TARGET_BITS_SPLIT_STANDARD
constexpr uint8_t NUM_BITS_RANK = 18U;
constexpr uint8_t NUM_BITS_TID = 9U;
constexpr uint8_t NUM_BITS_SYN_ID = 9U;
#elif TARGET_BITS_SPLIT == TARGET_BITS_SPLIT_HPC
constexpr uint8_t NUM_BITS_RANK = 20U;
constexpr uint8_t NUM_BITS_TID = 10U;
constexpr uint8_t NUM_BITS_SYN_ID = 6U;
#endif
constexpr uint8_t NUM_BITS_LCID = 27U;
constexpr uint8_t NUM_BITS_PROCESSED_FLAG = 1U;
constexpr uint8_t NUM_BITS_MARKER_SPIKE_DATA = 2U;
constexpr uint8_t NUM_BITS_LAG = 14U;
constexpr uint8_t NUM_BITS_DELAY = 21U;
constexpr uint8_t NUM_BITS_GID = 62U;

/*
 * Maximally allowed values for bitfields
 */
constexpr uint64_t MAX_LCID = generate_max_value( NUM_BITS_LCID );
constexpr int64_t MAX_RANK = generate_max_value( NUM_BITS_RANK );
constexpr int64_t MAX_TID = generate_max_value( NUM_BITS_TID );
constexpr uint64_t MAX_SYN_ID = generate_max_value( NUM_BITS_SYN_ID );
constexpr uint64_t DISABLED_GID = generate_max_value( NUM_BITS_GID );
constexpr uint64_t MAX_GID = DISABLED_GID - 1;

/**
 * Type for Time tics.
 */
#ifdef HAVE_LONG_LONG
typedef long long tic_t;
#ifdef LLONG_MAX
const tic_t tic_t_max = LLONG_MAX;
const tic_t tic_t_min = LLONG_MIN;
#else
const tic_t tic_t_max = LONG_LONG_MAX;
const tic_t tic_t_min = LONG_LONG_MIN;
#endif
#else
typedef long tic_t;
const tic_t tic_t_max = LONG_MAX;
const tic_t tic_t_min = LONG_MIN;
#endif

/**
 *  Unsigned long type for enumerations.
 */
typedef size_t index;
#ifndef SIZE_MAX
#define SIZE_MAX ( static_cast< std::size_t >( -1 ) )
#endif
__attribute__( ( __unused__ ) ) const index invalid_index = SIZE_MAX;

/**
 *  For enumerations of synapse types.
 */
typedef unsigned int synindex;
const synindex invalid_synindex = MAX_SYN_ID;

/**
 * Unsigned short type for compact target representation.
 *
 * See Kunkel et al, Front Neuroinform 8:78 (2014).
 */
//! target index into thread local node vector
typedef unsigned short targetindex;
const targetindex invalid_targetindex = USHRT_MAX;
__attribute__( ( __unused__ ) ) const index max_targetindex = invalid_targetindex - 1;

/**
 * Thread index type.
 * NEST threads are assigned non-negative numbers for
 * identification.
 * For invalid or undefined threads, the value -1 is used.
 */
typedef int thread;

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
typedef long rport;

/**
 * Connection port number to distinguis outgoing connections.
 * Connections between Nodes are assigned port numbers.
 * Valid port numbers start at zero (0).
 * The value -1 is used for invalid or unassigned ports.
 */
typedef long port;

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
typedef double weight;

/**
 * Delay of a connection.
 * The delay defines the number of simulation steps which elapse
 * before an Event arrives at the receiving Node.
 * Delays must be equal or larger than one.
 */
typedef long delay;
const long delay_min = LONG_MIN;
const long delay_max = LONG_MAX;

/**
 * enum type of signal conveyed by spike events of a node.
 * These types are used upon connect to check if spikes sent by one
 * neuron are interpreted the same way by receiving neuron.
 *
 * Each possible signal that may be represented (currently SPIKE and BINARY)
 * is interpreted as a separate bit flag. This way, upon connection, we
 * determine by a bitwise AND operation if sender and receiver are compatible.
 * The check takes place in connection::check_connection().
 *
 * A device, such as the spike-generator or spike_detector,
 * that can in a meaningful way be connected to either neuron model
 * can use the wildcard ALL, that will match any connection partner.
 */
enum SignalType
{
  NONE = 0,
  SPIKE = 1,
  BINARY = 2,
  ALL = SPIKE | BINARY
};
}

#endif // NEST_TYPES_H
