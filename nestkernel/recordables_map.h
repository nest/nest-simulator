/*
 *  recordables_map.h
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

#ifndef RECORDABLES_MAP_H
#define RECORDABLES_MAP_H

// C++ includes:
#include <cassert>
#include <map>
#include <string>
#include <utility>

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "name.h"

namespace nest
{
/**
 * Map names of recordables to data access functions.
 *
 * This map identifies the data access functions for recordable
 * state variables in model neurons.
 * Each neuron model shall have exactly one static instance
 * of RecordablesMap.
 *
 * @note The map is initialized by the create() member function
 *       and not by the constructor because of the following
 *       conflict: The recordablesMap_ shall be a private static
 *       member of its host class, since the same map applies
 *       to all objects. Creation by a constructor leads to static
 *       initialization conflicts with the Name class. Thus,
 *       creation is deferred to the plain constructor of the host
 *       Node class, which is called only once to create the
 *       model prototype instance.
 *
 * @see multimeter, UniversalDataLogger
 * @ingroup Devices
 */
template < typename HostNode >
class RecordablesMap : public std::map< Name, double ( HostNode::* )() const >
{
  typedef std::map< Name, double ( HostNode::* )() const > Base_;

public:
  virtual ~RecordablesMap()
  {
  }

  //! Datatype for access functions
  typedef double ( HostNode::*DataAccessFct )() const;

  /**
   * Create the map.
   * This function must be specialized for each class owning a
   * Recordables map and must fill the map. This should happen
   * as part of the original constructor for the Node.
   */
  void create();

  /**
   * Obtain SLI list of all recordables, for use by get_status().
   * @todo This fct should return the recordables_ entry, but since
   *       filling recordables_ leads to seg fault on exit, we just
   *       build the list every time, even though that beats the
   *       goal of being more efficient ...
   */
  ArrayDatum
  get_list() const
  {
    ArrayDatum recordables;
    for ( typename Base_::const_iterator it = this->begin(); it != this->end(); ++it )
    {
      recordables.push_back( new LiteralDatum( it->first ) );
    }
    return recordables;

    // the entire function should just be
    // return recordables_;
  }

private:
  //! Insertion functions to be used in create(), adds entry to map and list
  void
  insert_( const Name& n, const DataAccessFct f )
  {
    Base_::insert( std::make_pair( n, f ) );

    // Line below leads to seg-fault if nest is quit right after start,
    // see comment on get_list()
    // recordables_.push_back(LiteralDatum(n));
  }

  /**
   * SLI list of names of recordables
   * @todo Once the segfault-on-exit issue mentioned in the comment on
   * get_list() is resolved, the next code line should be activated again.
   *
   */
  // ArrayDatum recordables_;
};

template < typename HostNode >
void
RecordablesMap< HostNode >::create()
{
  assert( false );
}


//! Class that reads out state vector elements, used by UniversalDataLogger
template < typename HostNode >
class DataAccessFunctor
{
  // Pointer instead of reference required to avoid problems with
  // copying element in std::map when using libc++ under C++11.
  HostNode* parent_;
  size_t elem_;

public:
  DataAccessFunctor( HostNode& n, size_t elem )
    : parent_( &n )
    , elem_( elem ){};

  double operator()() const
  {
    return parent_->get_state_element( elem_ );
  };
};


/**
 * Map names of recordables to DataAccessFunctors.
 *
 * This map identifies the DataAccessFunctors for recordable state
 * variables in multisynapse model neurons.
 * As the number of synapse receptors can be modified at runtime,
 * each neuron shall have its own instance of DynamicRecordablesMap.
 * Furthermore, the neurons are able to insert and erase elements from
 * the map at runtime.
 *
 * @see multimeter, UniversalDataLogger
 * @ingroup Devices
 */
template < typename HostNode >
class DynamicRecordablesMap : public std::map< Name, const DataAccessFunctor< HostNode > >
{
  typedef std::map< Name, const DataAccessFunctor< HostNode > > Base_;

public:
  virtual ~DynamicRecordablesMap()
  {
  }

  //! Datatype for access callable
  typedef DataAccessFunctor< HostNode > DataAccessFct;

  /**
   * Create the map.
   * This function must be specialized for each class instance owning a
   * Recordables map and must fill the map. This should happen
   * as part of the original constructor for the Node.
   */
  void create( HostNode& n );

  /**
   * Obtain SLI list of all recordables, for use by get_status().
   * @todo This fct should return the recordables_ entry, but since
   *       filling recordables_ leads to seg fault on exit, we just
   *       build the list every time, even though that beats the
   *       goal of being more efficient ...
   */
  ArrayDatum
  get_list() const
  {
    ArrayDatum recordables;
    for ( typename Base_::const_iterator it = this->begin(); it != this->end(); ++it )
    {
      recordables.push_back( new LiteralDatum( it->first ) );
    }
    return recordables;
  }

  //! Insertion functions to be used in create(), adds entry to map and list
  void
  insert( const Name& n, const DataAccessFct& f )
  {
    Base_::insert( std::make_pair( n, f ) );
  }

  //! Erase functions to be used when setting state, removes entry from map and
  //! list
  void
  erase( const Name& n )
  {
    // .toString() required as work-around for #339, remove when #348 is solved.
    typename DynamicRecordablesMap< HostNode >::iterator it = this->find( n.toString() );
    // If the Name is not in the map, throw an error
    if ( it == this->end() )
    {
      throw KeyError( n, "DynamicRecordablesMap", "erase" );
    }

    Base_::erase( it );
  }
};

template < typename HostNode >
void
DynamicRecordablesMap< HostNode >::create( HostNode& n )
{
  assert( false );
}
}

#endif
