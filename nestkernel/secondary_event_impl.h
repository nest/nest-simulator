/*
 *  secondary_event_impl.h
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

#ifndef SECONDARY_EVENT_IMPL_H
#define SECONDARY_EVENT_IMPL_H

#include "event.h"
#include "secondary_event.h"
#include <set>

namespace nest
{

/**
 * This template function reads data of type T from a given position of a
 * std::vector< unsigned int >. The function is used to read SecondaryEvents
 * data from the NEST communication buffer. The pos iterator is advanced
 * during execution. For a discussion on the functionality of this function see
 * github issue #181 and pull request #184.
 */
template < typename T >
void
read_from_comm_buffer( T& d, std::vector< unsigned int >::iterator& pos )
{
  // there is no aliasing problem here, since cast to char* invalidate strict
  // aliasing assumptions
  char* const c = reinterpret_cast< char* >( &d );

  const size_t num_uints = number_of_uints_covered< T >();
  size_t left_to_copy = sizeof( T );

  for ( size_t i = 0; i < num_uints; i++ )
  {
    memcpy( c + i * sizeof( unsigned int ), &( *( pos + i ) ), std::min( left_to_copy, sizeof( unsigned int ) ) );
    left_to_copy -= sizeof( unsigned int );
  }

  pos += num_uints;
}

template < typename DataType, typename Subclass >
inline DataType
DataSecondaryEvent< DataType, Subclass >::get_coeffvalue( std::vector< unsigned int >::iterator& pos )
{
  DataType elem;
  read_from_comm_buffer( elem, pos );
  return elem;
}

template < typename DataType, typename Subclass >
std::set< synindex > DataSecondaryEvent< DataType, Subclass >::supported_syn_ids_;

template < typename DataType, typename Subclass >
size_t DataSecondaryEvent< DataType, Subclass >::coeff_length_ = 0;

/**
 * Event for slow inward current (SIC) connections between astrocytes and neurons.
 *
 * The event transmits the slow inward current to the connected neurons.
 */
class SICEvent : public DataSecondaryEvent< double, SICEvent >
{

public:
  SICEvent()
  {
  }

  void operator()();
  SICEvent* clone() const;
};

template < typename DataType, typename Subclass >
void
DataSecondaryEvent< DataType, Subclass >::add_syn_id( const synindex synid )
{
  kernel::manager< VPManager >.assert_thread_parallel();

  // This is done during connection model cloning, which happens thread-parallel.
  // To not risk trashing the set data structure, we let only master register the
  // new synid. This is not performance critical and avoiding collisions elsewhere
  // would be more difficult, so we do it here in a master section.
#pragma omp master
  {
    supported_syn_ids_.insert( synid );
  }
#pragma omp barrier
}

template < typename DataType, typename Subclass >
void
DataSecondaryEvent< DataType, Subclass >::set_coeff_length( const size_t coeff_length )
{
  kernel::manager< VPManager >.assert_single_threaded();
  coeff_length_ = coeff_length;
}
}

#endif
