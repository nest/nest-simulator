/*
 *  mpiutil_impl.h
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

#pragma once

#include "mpiutil.h"

#include <algorithm>
#include <ostream>
#include <type_traits>

namespace arb
{
namespace shadow
{

struct cell_member_type
{
  cell_gid_type gid;
  cell_lid_type index;
};

template < typename I >
struct basic_spike
{
  using id_type = I;

  id_type source = id_type{};
  time_type time = -1;

  basic_spike() = default;

  basic_spike( id_type s, time_type t )
    : source( s )
    , time( t )
  {
  }

  friend bool operator==( const basic_spike& l, const basic_spike& r )
  {
    return l.time == r.time && l.source == r.source;
  }
};

using spike = basic_spike< cell_member_type >;

std::vector< spike >
gather_spikes( const std::vector< spike >& values, MPI_Comm comm )
{
  int size;
  MPI_Comm_size( comm, &size );

  std::vector< int > counts( size );
  int n_local = values.size() * sizeof( spike );
  MPI_Allgather( &n_local, 1, MPI_INT, counts.data(), 1, MPI_INT, comm );
  std::vector< int > displ( size + 1 );
  for ( int i = 0; i < size; ++i )
  {
    displ[ i + 1 ] = displ[ i ] + counts[ i ];
  }

  std::vector< spike > buffer( displ.back() / sizeof( spike ) );
  MPI_Allgatherv( const_cast< spike* >( values.data() ),
    n_local,
    MPI_CHAR, // send buffer
    buffer.data(),
    counts.data(),
    displ.data(),
    MPI_CHAR, // receive buffer
    comm );

  return buffer;
}

int
mpi_rank( MPI_Comm c )
{
  int result;
  MPI_Comm_rank( c, &result );
  return result;
}

int
mpi_size( MPI_Comm c )
{
  int result;
  MPI_Comm_size( c, &result );
  return result;
}

int
broadcast( int local, MPI_Comm comm, int root )
{
  int result = local;
  MPI_Bcast( &result, 1, MPI_INT, root, comm );
  return result;
}

unsigned
broadcast( unsigned local, MPI_Comm comm, int root )
{
  int result = local;
  MPI_Bcast( &result, 1, MPI_UNSIGNED, root, comm );
  return result;
}

float
broadcast( float local, MPI_Comm comm, int root )
{
  float result = local;
  MPI_Bcast( &result, 1, MPI_FLOAT, root, comm );
  return result;
}

struct comm_info
{
  int global_size; //
  int global_rank; //
  int local_rank;  //
  bool is_arbor;   //
  bool is_nest;    //
  int arbor_size;  //
  int nest_size;   //
  int arbor_root;  //
  int nest_root;   //
  MPI_Comm comm;   //
};

comm_info
get_comm_info( bool is_arbor, MPI_Comm comm )
{
  static_assert( ( sizeof( spike ) % alignof( spike ) ) == 0, "Alignment requirements of spike data type not met!" );

  comm_info info;
  info.is_arbor = is_arbor;
  info.is_nest = !is_arbor;

  info.global_rank = mpi_rank( MPI_COMM_WORLD );
  info.global_size = mpi_size( MPI_COMM_WORLD );

  // split MPI_COMM_WORLD: all arbor go into split 1
  // int color = is_arbor? 1: 0;
  // MPI_Comm_split(MPI_COMM_WORLD, color, info.global_rank, &info.comm);
  // std::cerr << "Splitted network" << std::endl;

  info.comm = comm;
  int local_size = mpi_size( info.comm );
  info.local_rank = mpi_rank( info.comm );

  info.arbor_size = is_arbor ? local_size : info.global_size - local_size;
  info.nest_size = info.global_size - info.arbor_size;

  std::vector< int > local_ranks( local_size );
  MPI_Allgather( &info.global_rank, 1, MPI_INT, local_ranks.data(), 1, MPI_INT, info.comm );
  std::sort( local_ranks.begin(), local_ranks.end() );

  auto first_missing = []( const std::vector< int >& x )
  {
    auto it = std::adjacent_find( x.begin(),
      x.end(),
      []( int l, int r )
      {
        return ( r - l ) != 1;
      } );
    return it == x.end() ? x.back() + 1 : ( *it ) + 1;
  };

  if ( info.is_arbor )
  {
    info.arbor_root = local_ranks.front();
    info.nest_root = info.arbor_root == 0 ? first_missing( local_ranks ) : 0;
  }
  else
  {
    info.nest_root = local_ranks.front();
    info.arbor_root = info.nest_root == 0 ? first_missing( local_ranks ) : 0;
  }

  return info;
}


} // namespace shadow
} // namespace arb
