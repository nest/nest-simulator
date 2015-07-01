/*
 *  communicator.h
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

#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <vector>
#include <cassert>
#include <numeric>

#include "config.h"
#include "nest.h"
#include <iostream>
#include <unistd.h>
#include <limits>

#include "dictdatum.h"
#include "nodelist.h"

#ifdef HAVE_MPI
// Do NOT include mpi.h in this header file, otherwise we get into
// trouble on the Blue Gene/L. mpi.h is included in communicator_impl.h

#ifdef HAVE_MUSIC
#include <music.hh>
#include "music_event_handler.h"
#endif


namespace nest
{
class Network;

class Communicator
{
  friend class Network;

public:
  Communicator()
  {
  }
  ~Communicator()
  {
  }

#ifdef HAVE_MUSIC
  static MUSIC::Setup* music_setup;     //!< pointer to a MUSIC setup object
  static MUSIC::Runtime* music_runtime; //!< pointer to a MUSIC runtime object
#endif

  /**
   * Combined storage of GID and offset information for off-grid spikes.
   *
   * @note This class actually stores the GID as @c double_t internally.
   *       This is done so that the user-defined MPI type MPI_OFFGRID_SPIKE,
   *       which we use to communicate off-grid spikes, is homogeneous.
   *       Otherwise, OpenMPI spends extreme amounts of time on packing
   *       and unpacking the data, see #458.
   */
  class OffGridSpike
  {
  public:
    //! We defined this type explicitly, so that the assert function below always tests the correct
    //! type.
    typedef uint_t gid_external_type;

    OffGridSpike()
      : gid_( 0 )
      , offset_( 0.0 )
    {
    }
    OffGridSpike( gid_external_type gidv, double_t offsetv )
      : gid_( gidv )
      , offset_( offsetv )
    {
    }

    uint_t
    get_gid() const
    {
      return static_cast< gid_external_type >( gid_ );
    }
    void
    set_gid( gid_external_type gid )
    {
      gid_ = static_cast< double_t >( gid );
    }
    double_t
    get_offset() const
    {
      return offset_;
    }

  private:
    friend class Communicator; // void Communicator::init(int*, char**);

    double_t gid_;    //!< GID of neuron that spiked
    double_t offset_; //!< offset of spike from grid

    //! This function asserts that doubles can hold GIDs without loss
    static void
    assert_datatype_compatibility()
    {
      assert( std::numeric_limits< double_t >::digits
        > std::numeric_limits< gid_external_type >::digits );

      // the next one is doubling up, better be safe than sorry
      const gid_external_type maxgid = std::numeric_limits< gid_external_type >::max();
      OffGridSpike ogs( maxgid, 0.0 );
      assert( maxgid == ogs.get_gid() );
    }
  };

  class NodeAddressingData
  {
  public:
    NodeAddressingData()
      : gid_( 0 )
      , parent_gid_( 0 )
      , vp_( 0 )
    {
    }
    NodeAddressingData( uint_t gid, uint_t parent_gid, uint_t vp )
      : gid_( gid )
      , parent_gid_( parent_gid )
      , vp_( vp )
    {
    }

    uint_t
    get_gid() const
    {
      return gid_;
    }
    uint_t
    get_parent_gid() const
    {
      return parent_gid_;
    }
    uint_t
    get_vp() const
    {
      return vp_;
    }
    bool operator<( const NodeAddressingData& other ) const
    {
      return this->gid_ < other.gid_;
    }
    bool operator==( const NodeAddressingData& other ) const
    {
      return this->gid_ == other.gid_;
    }

  private:
    friend class Communicator;
    uint_t gid_;        //!< GID of neuron
    uint_t parent_gid_; //!< GID of neuron's parent
    uint_t vp_;         //!< virtual process of neuron
  };

#ifdef HAVE_MUSIC
  /**
   * Enter the runtime mode. This must be done before simulating. After having entered runtime mode
   * ports cannot be published anymore.
   * \param h_min_delay is the length of a time slice, after which commmunication should take place.
   */
  static void enter_runtime( double_t h_min_delay );

  static MUSIC::Setup* get_music_setup();
  static MUSIC::Runtime* get_music_runtime();

  /**
   * Advance the time of music by num_steps simulation steps.
   * \param num_steps number of simulation steps, the time to propagate.
   * /TODO: put this into scheduler.
   */
  static void advance_music_time( long_t num_steps );

  /**
   * Register a music_event_in_proxy for a given port and a given channel.
   * As a consequence, the proxy will be informed, whenever an event over this port and
   * channel comes in.
   */
  static void register_music_event_in_proxy( std::string portname, int channel, nest::Node* mp );
#endif

  static void init( int* argc, char** argv[] );
  static void finalize();
  static void mpi_abort( int exitcode );

  static void communicate( std::vector< uint_t >& send_buffer,
    std::vector< uint_t >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate( std::vector< OffGridSpike >& send_buffer,
    std::vector< OffGridSpike >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate( std::vector< double_t >& send_buffer,
    std::vector< double_t >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate( double_t, std::vector< double_t >& );
  static void communicate( std::vector< int_t >& );
  static void communicate( std::vector< long_t >& );

  /**
   * Collect GIDs for all nodes in a given node list across processes.
   * The NodeListType should be one of LocalNodeList, LocalLeafList, LocalChildList.
   */
  template < typename NodeListType >
  static void communicate( const NodeListType& local_nodes,
    std::vector< NodeAddressingData >& all_nodes,
    bool remote = false );
  template < typename NodeListType >
  static void communicate( const NodeListType& local_nodes,
    std::vector< NodeAddressingData >& all_nodes,
    Network& net,
    DictionaryDatum params,
    bool remote = false );

  static void communicate_connector_properties( DictionaryDatum& dict );

  static void synchronize();
  static void test_link( int, int );
  static void test_links();

  static bool grng_synchrony( unsigned long );
  static double_t time_communicate( int num_bytes, int samples = 1000 );
  static double_t time_communicatev( int num_bytes, int samples = 1000 );
  static double_t time_communicate_offgrid( int num_bytes, int samples = 1000 );
  static double_t time_communicate_alltoall( int num_bytes, int samples = 1000 );
  static double_t time_communicate_alltoallv( int num_bytes, int samples = 1000 );

  static std::string get_processor_name();

  static int get_rank();
  static int get_num_processes();
  static void set_num_processes( int );
  static int get_num_virtual_processes();
  static int get_send_buffer_size();
  static int get_recv_buffer_size();
  static bool get_initialized();

  static void set_num_threads( thread num_threads );
  static void set_buffer_sizes( int send_buffer_size, int recv_buffer_size );

private:
  static Network* net_; //!< Pointer to the Network class

  static int rank_;             //!< the rank of the machine
  static int num_processes_;    //!< the number of mpi-processes
  static int n_vps_;            //!< the number of virtual processes
  static int send_buffer_size_; //!< expected size of send buffer
  static int recv_buffer_size_; //!< size of receive buffer
  static bool initialized_;     //!< whether MPI is initialized

  static std::vector< int > comm_step_; //!< array containing communication partner for each step.
  static uint_t COMM_OVERFLOW_ERROR;

  static void communicate_Allgather( std::vector< uint_t >& send_buffer,
    std::vector< uint_t >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate_Allgather( std::vector< OffGridSpike >& send_buffer,
    std::vector< OffGridSpike >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate_Allgather( std::vector< int_t >& );
  static void communicate_Allgather( std::vector< long_t >& );

  template < typename T >
  static void communicate_Allgatherv( std::vector< T >& send_buffer,
    std::vector< T >& recv_buffer,
    std::vector< int >& displacements,
    std::vector< int >& recv_counts );

  template < typename T >
  static void communicate_Allgather( std::vector< T >& send_buffer,
    std::vector< T >& recv_buffer,
    std::vector< int >& displacements );
};
}

#else  /* #ifdef HAVE_MPI */

namespace nest
{
class Network;

class Communicator
{
  friend class Network;

public:
  Communicator()
  {
  }
  ~Communicator()
  {
  }

  class OffGridSpike
  {
  public:
    OffGridSpike()
      : gid_( 0 )
      , offset_( 0.0 )
    {
    }
    OffGridSpike( uint_t gidv, double_t offsetv )
      : gid_( gidv )
      , offset_( offsetv )
    {
    }

    uint_t
    get_gid() const
    {
      return static_cast< uint_t >( gid_ );
    }
    void
    set_gid( uint_t gid )
    {
      gid_ = static_cast< double_t >( gid );
    }
    double_t
    get_offset() const
    {
      return offset_;
    }

  private:
    friend class Communicator; // void Communicator::init(int*, char**);

    double_t gid_;    //!< GID of neuron that spiked
    double_t offset_; //!< offset of spike from grid
  };

  class NodeAddressingData
  {
  public:
    NodeAddressingData()
      : gid_( 0 )
      , parent_gid_( 0 )
      , vp_( 0 )
    {
    }
    NodeAddressingData( uint_t gid, uint_t parent_gid, uint_t vp )
      : gid_( gid )
      , parent_gid_( parent_gid )
      , vp_( vp )
    {
    }

    uint_t
    get_gid() const
    {
      return gid_;
    }
    uint_t
    get_parent_gid() const
    {
      return parent_gid_;
    }
    uint_t
    get_vp() const
    {
      return vp_;
    }
    bool operator<( const NodeAddressingData& other ) const
    {
      return this->gid_ < other.gid_;
    }
    bool operator==( const NodeAddressingData& other ) const
    {
      return this->gid_ == other.gid_;
    }

  private:
    friend class Communicator;
    uint_t gid_;        //!< GID of neuron
    uint_t parent_gid_; //!< GID of neuron's parent
    uint_t vp_;         //!< virtual process of neuron
  };

  static void communicate( std::vector< uint_t >& send_buffer,
    std::vector< uint_t >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate( std::vector< OffGridSpike >& send_buffer,
    std::vector< OffGridSpike >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate( std::vector< double_t >& send_buffer,
    std::vector< double_t >& recv_buffer,
    std::vector< int >& displacements );
  static void communicate( double_t, std::vector< double_t >& );
  static void
  communicate( std::vector< int_t >& )
  {
  }
  static void
  communicate( std::vector< long_t >& )
  {
  }

  /**
  * Collect GIDs for all nodes in a given node list across processes.
  * The NodeListType should be one of LocalNodeList, LocalLeafList, LocalChildList.
  */
  /**
   * Collect GIDs for all nodes in a given node list across processes.
   * The NodeListType should be one of LocalNodeList, LocalLeafList, LocalChildList.
   */
  template < typename NodeListType >
  static void communicate( const NodeListType& local_nodes,
    std::vector< NodeAddressingData >& all_nodes,
    bool remote = false );
  template < typename NodeListType >
  static void communicate( const NodeListType& local_nodes,
    std::vector< NodeAddressingData >& all_nodes,
    Network& net,
    DictionaryDatum params,
    bool remote = false );

  static void
  communicate_connector_properties( DictionaryDatum& )
  {
  }

  static void
  synchronize()
  {
  }

  /* replaced u_long with unsigned long since u_long is not known when
         mpi.h is not available. This is a rather ugly fix.
         HEP 2007-03-09
   */
  static bool
  grng_synchrony( unsigned long )
  {
    return true;
  }
  static double_t
  time_communicate( int, int )
  {
    return 0.0;
  }
  static double_t
  time_communicatev( int, int )
  {
    return 0.0;
  }
  static double_t
  time_communicate_offgrid( int, int )
  {
    return 0.0;
  }
  static double_t
  time_communicate_alltoall( int, int )
  {
    return 0.0;
  }
  static double_t
  time_communicate_alltoallv( int, int )
  {
    return 0.0;
  }

  static std::string get_processor_name();

  static int get_rank();
  static int get_num_processes();
  static void set_num_processes( int );
  static int get_num_virtual_processes();
  static int get_send_buffer_size();
  static int get_recv_buffer_size();
  static bool get_use_Allgather();
  static bool get_initialized();

  static void set_num_threads( thread num_threads );
  static void set_buffer_sizes( int send_buffer_size, int recv_buffer_size );

private:
  static Network* net_; //!< Pointer to the Network class

  static int rank_;             //!< the rank of the machine
  static int num_processes_;    //!< the number of mpi-processes
  static int n_vps_;            //!< the number of virtual processes
  static int send_buffer_size_; //!< expected size of send buffer
  static int recv_buffer_size_; //!< size of receive buffer
  static bool initialized_;     //!< whether MPI is initialized
  static bool use_Allgather_;   //!< using Allgather communication
};


inline std::string
Communicator::get_processor_name()
{
  char name[ 1024 ];
  name[ 1023 ] = '\0';
  gethostname( name, 1023 );
  return name;
}
}
#endif /* #ifdef HAVE_MPI */

namespace nest
{

inline int
Communicator::get_rank()
{
  return rank_;
}

inline int
Communicator::get_num_processes()
{
  return num_processes_;
}

inline void
Communicator::set_num_processes( int np )
{
  num_processes_ = np;
}

inline int
Communicator::get_num_virtual_processes()
{
  return n_vps_;
}

inline int
Communicator::get_send_buffer_size()
{
  return send_buffer_size_;
}

inline int
Communicator::get_recv_buffer_size()
{
  return recv_buffer_size_;
}

inline bool
Communicator::get_initialized()
{
  return initialized_;
}

inline void
Communicator::set_num_threads( thread num_threads )
{
  n_vps_ = num_processes_ * num_threads;
}

inline void
Communicator::set_buffer_sizes( int send_buffer_size, int recv_buffer_size )
{
  send_buffer_size_ = send_buffer_size;
  recv_buffer_size_ = recv_buffer_size;
}

} // namespace nest

#endif /* #ifndef COMMUNICATOR_H */
