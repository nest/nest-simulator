/*
 *  target_table.h
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

#ifndef TARGET_H
#define TARGET_H

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

/**
 * A structure containing all information required to uniquely
 * identify a target neuron on a (remote) machine. Used in TargetTable
 * for presynaptic part of connection infrastructure.
 */
class Target
{
private:
  unsigned int lcid_ : 27; //!< local connection index
  unsigned int  rank_ : 20; //!< rank of target neuron
  short tid_ : 10; //!< thread index
  char syn_index_ : 6; //!< synapse-type index
  bool processed_ : 1;
public:
  Target();
  Target( const Target& target );
  Target( const thread tid, const thread rank, const synindex syn_index, const index lcid);
  void set_lcid( const size_t lcid );
  size_t get_lcid() const;
  void set_rank( const unsigned int rank );
  unsigned int get_rank() const;
  void set_tid( const unsigned int tid );
  unsigned int get_tid() const;
  void set_syn_index( const unsigned char syn_index );
  unsigned char get_syn_index() const;
  void set_processed( const bool processed );
  bool is_processed() const;
  double_t get_offset() const;
};

inline
Target::Target()
  : lcid_( 0 )
  , rank_( 0 )
  , tid_( 0 )
  , syn_index_( 0 )
  , processed_( false )
{
}

inline
Target::Target( const Target& target )
  : lcid_( target.lcid_ )
  , rank_( target.rank_ )
  , tid_( target.tid_ )
  , syn_index_( target.syn_index_ )
  , processed_( false ) // always initialize as non-processed
{
}

inline
Target::Target( const thread tid, const thread rank, const synindex syn_index, const index lcid)
  : lcid_( lcid )
  , rank_( rank )
  , tid_( tid )
  , syn_index_( syn_index )
  , processed_( false ) // always initialize as non-processed
{
  assert( lcid < 134217728 );
  assert( rank < 1048576 );
  assert( tid < 1024 );
  assert( syn_index < 64 );
}

inline void
Target::set_lcid( const size_t lcid )
{
  lcid_ = lcid;
}

inline size_t
Target::get_lcid() const
{
  return lcid_;
}

inline void
Target::set_rank( const unsigned int rank )
{
  rank_ = rank;
}

inline unsigned int
Target::get_rank() const
{
  return rank_;
}

inline void
Target::set_tid( const unsigned int tid )
{
  tid_ = tid;
}

inline unsigned int
Target::get_tid() const
{
  return tid_;
}

inline void
Target::set_syn_index( const unsigned char syn_index )
{
  syn_index_ = syn_index;
}

inline unsigned char
Target::get_syn_index() const
{
  return syn_index_;
}

inline void
Target::set_processed( const bool processed)
{
  processed_ = processed;
}

inline bool
Target::is_processed() const
{
  return processed_;
}

inline double_t
Target::get_offset() const
{
  return 0;
}

class OffGridTarget : public Target
{
private:
  double_t offset;
public:
  OffGridTarget();
  OffGridTarget( const Target& target, const double_t offset );
  double_t get_offset() const;
};

inline
OffGridTarget::OffGridTarget()
  : Target()
{
}

inline
OffGridTarget::OffGridTarget( const Target& target, const double_t offset )
  : Target( target )
  , offset( offset )
{
}

inline double_t
OffGridTarget::get_offset() const
{
  return offset;
}

} // namespace nest

#endif // TARGET_H
