/*
 *  communicator_impl.h
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

#include "communicator.h"
#include "network.h"

#include "config.h"

/* To avoid problems on BlueGene/L, mpi.h MUST be the
   first included file after config.h.
 */
#ifdef HAVE_MPI
#include <mpi.h>
#endif /* #ifdef HAVE_MPI */

#ifdef HAVE_MPI

// Variable to hold the MPI communicator to use.
#ifdef HAVE_MUSIC
extern MPI::Intracomm comm;
#else /* #ifdef HAVE_MUSIC */
extern MPI_Comm comm;
#endif /* #ifdef HAVE_MUSIC */


/* ------------------------------------------------------
   The following datatypes are defined here in communicator_impl.h
   file instead of as static class members, to avoid inclusion
   of mpi.h in the .h file. This is necessary, because on
   BlueGene/L mpi.h MUST be included FIRST. Having mpi.h in
   the .h file would lead to requirements on include-order
   throughout the NEST code base and is not acceptable.
   Reported by Mikael Djurfeldt.
   Hans Ekkehard Plesser, 2010-01-28
 */
template <typename T>
struct MPI_Type { static MPI_Datatype type; };

template <typename T>
void nest::Communicator::communicate_Allgatherv(std::vector<T>& send_buffer,
                                                std::vector<T>& recv_buffer,
                                                std::vector<int>& displacements,
                                                std::vector<int>& recv_counts)
{
  //attempt Allgather
  MPI_Allgatherv(&send_buffer[0], send_buffer.size(), MPI_Type<T>::type,
      &recv_buffer[0], &recv_counts[0], &displacements[0], MPI_Type<T>::type, comm);
}

template <typename NodeListType>
void nest::Communicator::communicate(const NodeListType& local_nodes,
                                     vector<NodeAddressingData>& all_nodes,
                                     bool remote)
{
  size_t np = Communicator::num_processes_;
  if (np > 1 && remote)
  {
    vector<long_t> localnodes;
    for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
    {
      localnodes.push_back((*n)->get_gid());
      localnodes.push_back(((*n)->get_parent())->get_gid());
      localnodes.push_back((*n)->get_vp());
    }
    //get size of buffers
    std::vector<nest::int_t> n_nodes(np);
    n_nodes[Communicator::rank_] = localnodes.size();
    communicate(n_nodes);
    // Set up displacements vector.
    std::vector<int> displacements(np,0);

    for ( size_t i = 1; i < np; ++i )
      displacements.at(i) = displacements.at(i-1)+n_nodes.at(i-1);

    // Calculate total number of node data items to be gathered.
    size_t n_globals =
        std::accumulate(n_nodes.begin(),n_nodes.end(), 0);
    assert(n_globals % 3 == 0);
    vector<long_t> globalnodes;
    if (n_globals != 0)
    {
      globalnodes.resize(n_globals,0L);
      communicate_Allgatherv<nest::long_t>(localnodes, globalnodes, displacements, n_nodes);
    
    //Create unflattened vector
    for ( size_t i = 0; i < n_globals -2; i +=3)
      all_nodes.push_back(NodeAddressingData(globalnodes[i],globalnodes[i+1],globalnodes[i+2]));

    //get rid of any multiple entries
    std::sort(all_nodes.begin(), all_nodes.end());
    vector<NodeAddressingData>::iterator it;
    it = std::unique(all_nodes.begin(), all_nodes.end());
    all_nodes.resize(it - all_nodes.begin());
    }
  }
  else   //on one proc or not including remote nodes
  {
    for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
      all_nodes.push_back(NodeAddressingData((*n)->get_gid(),
                          ((*n)->get_parent())->get_gid(),
                          (*n)->get_vp()));
    std::sort(all_nodes.begin(), all_nodes.end());
  }
}


template <typename NodeListType>
void nest::Communicator::communicate(const NodeListType& local_nodes,
                                     vector<NodeAddressingData>& all_nodes,
                                     Network& net, DictionaryDatum params, 
				     bool remote)
{
  size_t np = Communicator::num_processes_;

  if ( np > 1 && remote)
  {
    vector<long_t> localnodes;
    if (params->empty())
    {
      for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
      {
        localnodes.push_back((*n)->get_gid());
        localnodes.push_back(((*n)->get_parent())->get_gid());
        localnodes.push_back((*n)->get_vp());
      }
    } else {
      for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
      {
        //select those nodes fulfilling the key/value pairs of the dictionary
        bool match = true;
        index gid = (*n)->get_gid();
        DictionaryDatum node_status = net.get_status(gid);
        for (Dictionary::iterator i = params->begin(); i != params->end(); ++i)
        {
          if (node_status->known(i->first))
          {
            const Token token = node_status->lookup(i->first);
            if (not ( token == i->second || token.matches_as_string(i->second) ))
            {
              match = false;
              break;
            }
          }
        }
        if (match)
        {
          localnodes.push_back(gid);
          localnodes.push_back(((*n)->get_parent())->get_gid());
          localnodes.push_back((*n)->get_vp());
        }
      }
    }

    //get size of buffers
    std::vector<nest::int_t> n_nodes(np);
    n_nodes[Communicator::rank_] = localnodes.size();
    communicate(n_nodes);

    // Set up displacements vector.
    std::vector<int> displacements(np,0);

    for ( size_t i = 1; i < np; ++i )
      displacements.at(i) = displacements.at(i-1)+n_nodes.at(i-1);

    // Calculate sum of global connections.
    size_t n_globals =
        std::accumulate(n_nodes.begin(),n_nodes.end(), 0);
    assert(n_globals % 3 == 0);
    vector<long_t> globalnodes;
    if (n_globals != 0)
    {
      globalnodes.resize(n_globals,0L);
      communicate_Allgatherv<nest::long_t>(localnodes, globalnodes, displacements, n_nodes);
  
      //Create unflattened vector
      for ( size_t i = 0; i < n_globals -2; i +=3)
        all_nodes.push_back(NodeAddressingData(globalnodes[i],globalnodes[i+1],globalnodes[i+2]));

      //get rid of any multiple entries
      std::sort(all_nodes.begin(), all_nodes.end());
      vector<NodeAddressingData>::iterator it;
      it = std::unique(all_nodes.begin(), all_nodes.end());
      all_nodes.resize(it - all_nodes.begin());
    }
  }
  else   //on one proc or not including remote nodes
  {
    if (params->empty())
    {
      for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
        all_nodes.push_back(NodeAddressingData((*n)->get_gid(), ((*n)->get_parent())->get_gid(), (*n)->get_vp()));
    }
    else {
      //select those nodes fulfilling the key/value pairs of the dictionary
      for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
      {
        bool match = true;
        index gid = (*n)->get_gid();
        DictionaryDatum node_status = net.get_status(gid);
        for (Dictionary::iterator i = params->begin(); i != params->end(); ++i)
        {
          if (node_status->known(i->first))
          {
            const Token token = node_status->lookup(i->first);
            if (not ( token == i->second || token.matches_as_string(i->second) ))
            {
              match = false;
              break;
            }
          }
        }
        if (match)
          all_nodes.push_back(NodeAddressingData((*n)->get_gid(), ((*n)->get_parent())->get_gid(), (*n)->get_vp()));
      }
    }
    std::sort(all_nodes.begin(),all_nodes.end());
  }
}



#else //HAVE_MPI

template <typename NodeListType>
void nest::Communicator::communicate(const NodeListType& local_nodes, vector<NodeAddressingData>& all_nodes, bool)
{
  for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
        all_nodes.push_back(NodeAddressingData((*n)->get_gid(), ((*n)->get_parent())->get_gid(), (*n)->get_vp()));
  std::sort(all_nodes.begin(),all_nodes.end());
}

template <typename NodeListType>
void nest::Communicator::communicate(const NodeListType& local_nodes, vector<NodeAddressingData>& all_nodes,
                                     Network& net, DictionaryDatum params, bool)
{

  if (params->empty())
  {
    for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
      all_nodes.push_back(NodeAddressingData((*n)->get_gid(), ((*n)->get_parent())->get_gid(), (*n)->get_vp()));
  }
  else {
    //select those nodes fulfilling the key/value pairs of the dictionary
    for ( typename NodeListType::iterator n = local_nodes.begin(); n != local_nodes.end(); ++n )
    {
      bool match = true;
      index gid = (*n)->get_gid();
      DictionaryDatum node_status = net.get_status(gid);
      for (Dictionary::iterator i = params->begin(); i != params->end(); ++i)
      {
        if (node_status->known(i->first))
        {
          const Token token = node_status->lookup(i->first);
          if (not ( token == i->second || token.matches_as_string(i->second) ))
          {
            match = false;
            break;
          }
        }
      }
      if (match)
        all_nodes.push_back(NodeAddressingData((*n)->get_gid(), ((*n)->get_parent())->get_gid(), (*n)->get_vp()));
    }
  }
  std::sort(all_nodes.begin(),all_nodes.end());
}

#endif
