/*
 *  target_data.h
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

#ifndef TARGET_DATA_H
#define TARGET_DATA_H

// C++ includes:
#include <limits>

// Includes from nestkernel:
#include "nest_types.h"
#include "target.h"

namespace nest
{

/**
 * Used to communicate part of the connection infrastructure from
 * post- to presynaptic side. These are the elements of the MPI
 * buffers.
 * SeeAlso: SpikeData
 */
class TargetDataBase
{
private:
  static const unsigned int default_marker_ = 0;
  static const unsigned int complete_marker_ = 1;
  static const unsigned int end_marker_ = 2;
  static const unsigned int invalid_marker_ = 3;

  unsigned int source_lid_ : 19; //!< local id of presynaptic neuron
  unsigned int source_tid_ : 10; //!< thread index of presynaptic neuron
  unsigned int marker_ : 2;
  bool is_primary_ : 1;

protected:
  TargetDataBase();

public:
  ~TargetDataBase();
  void reset_marker();
  void set_complete_marker();
  void set_end_marker();
  void set_invalid_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
  bool is_invalid_marker() const;
  void set_source_lid( const index source_lid );
  void set_source_tid( const thread source_tid );
  index get_source_lid() const;
  thread get_source_tid() const;
  void set_is_primary( const bool is_primary );
  bool is_primary() const;
};

inline TargetDataBase::TargetDataBase()
  : source_lid_( 0 )
  , source_tid_( 0 )
  , marker_( default_marker_ )
  , is_primary_( true )
{
}

inline TargetDataBase::~TargetDataBase()
{
}

inline void
TargetDataBase::reset_marker()
{
  marker_ = default_marker_;
}

inline void
TargetDataBase::set_complete_marker()
{
  marker_ = complete_marker_;
}

inline void
TargetDataBase::set_end_marker()
{
  marker_ = end_marker_;
}

inline void
TargetDataBase::set_invalid_marker()
{
  marker_ = invalid_marker_;
}

inline bool
TargetDataBase::is_complete_marker() const
{
  return marker_ == complete_marker_;
}

inline bool
TargetDataBase::is_end_marker() const
{
  return marker_ == end_marker_;
}

inline bool
TargetDataBase::is_invalid_marker() const
{
  return marker_ == invalid_marker_;
}

inline void
TargetDataBase::set_source_lid( const index source_lid )
{
  assert( source_lid < 1048576 );
  source_lid_ = source_lid;
}

inline void
TargetDataBase::set_source_tid( const thread source_tid )
{
  assert( source_tid < 1024 );
  source_tid_ = source_tid;
}

inline index
TargetDataBase::get_source_lid() const
{
  return source_lid_;
}

inline thread
TargetDataBase::get_source_tid() const
{
  return source_tid_;
}

inline void
TargetDataBase::set_is_primary( const bool is_primary )
{
  is_primary_ = is_primary;
}

inline bool
TargetDataBase::is_primary() const
{
  return is_primary_;
}

class TargetData : public TargetDataBase
{
private:
  unsigned int lcid_ : 27;
  unsigned int tid_ : 10;
  unsigned int syn_id_ : 8;

public:
  TargetData();
  void set_lcid( const index lcid );
  index get_lcid() const;
  void set_tid( const thread tid );
  thread get_tid() const;
  void set_syn_id( const synindex syn_id );
  synindex get_syn_id() const;
};

inline TargetData::TargetData()
  : TargetDataBase()
  , lcid_( 0 )
  , tid_( 0 )
  , syn_id_( 0 )
{
}

inline void
TargetData::set_lcid( const index lcid )
{
  lcid_ = lcid;
}

inline index
TargetData::get_lcid() const
{
  return lcid_;
}

inline void
TargetData::set_tid( const thread tid )
{
  tid_ = tid;
}

inline thread
TargetData::get_tid() const
{
  return tid_;
}

inline void
TargetData::set_syn_id( const synindex syn_id )
{
  syn_id_ = syn_id;
}

inline synindex
TargetData::get_syn_id() const
{
  return syn_id_;
}

class SecondaryTargetData : public TargetDataBase
{
private:
  unsigned int send_buffer_pos_;
  unsigned char syn_id_;

public:
  SecondaryTargetData();
  void set_send_buffer_pos( const size_t pos );
  size_t get_send_buffer_pos() const;
  void set_syn_id( const synindex syn_id );
  synindex get_syn_id() const;
};

inline SecondaryTargetData::SecondaryTargetData()
  : TargetDataBase()
  , send_buffer_pos_( std::numeric_limits< unsigned int >::max() - 1 )
  , syn_id_( std::numeric_limits< unsigned char >::max() - 1 )
{
}

inline void
SecondaryTargetData::set_send_buffer_pos( const size_t pos )
{
  assert( pos < std::numeric_limits< unsigned int >::max() );
  send_buffer_pos_ = pos;
}

inline size_t
SecondaryTargetData::get_send_buffer_pos() const
{
  return send_buffer_pos_;
}

inline void
SecondaryTargetData::set_syn_id( const synindex syn_id )
{
  assert( syn_id < std::numeric_limits< unsigned char >::max() );
  syn_id_ = syn_id;
}

inline synindex
SecondaryTargetData::get_syn_id() const
{
  return syn_id_;
}

} // namespace nest

#endif // TARGET_DATA_H
