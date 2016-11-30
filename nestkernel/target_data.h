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

// Includes from nestkernel:
#include "nest_types.h"
#include "target.h"

namespace nest
{

/**
 * Structure used to communicate part of the connection infrastructure
 * from post- to presynaptic side. These are the elements of the MPI
 * buffers.
 * SeeAlso: SpikeData
 */
class TargetDataBase
{
private:
  index lid_ : 20;  //!< local id of presynaptic neuron
  thread tid_ : 10; //!< thread index of presynaptic neuron
  unsigned int marker_ : 2;
  bool is_primary_;
  static const unsigned int complete_marker_ = 1;
  static const unsigned int end_marker_ = 2;
  static const unsigned int invalid_marker_ = 3;

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
  void set_lid( const index lid );
  void set_tid( const thread tid );
  index get_lid() const;
  thread get_tid() const;
  void is_primary( const bool is_primary );
  bool is_primary() const;
};

inline TargetDataBase::TargetDataBase()
  : lid_( 0 )
  , tid_( 0 )
  , marker_( 0 )
  , is_primary_( true )
{
}

inline TargetDataBase::~TargetDataBase()
{
}

inline void
TargetDataBase::reset_marker()
{
  marker_ = 0;
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
TargetDataBase::set_lid( const index lid )
{
  lid_ = lid;
}

inline void
TargetDataBase::set_tid( const thread tid )
{
  tid_ = tid;
}

inline index
TargetDataBase::get_lid() const
{
  return lid_;
}

inline thread
TargetDataBase::get_tid() const
{
  return tid_;
}

inline void
TargetDataBase::is_primary( const bool is_primary )
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
  Target target_;

public:
  TargetData();
  const Target& get_target() const;
  Target& get_target();
};

inline TargetData::TargetData()
  : TargetDataBase()
  , target_( Target() )
{
}

inline const Target&
TargetData::get_target() const
{
  return target_;
}

inline Target&
TargetData::get_target()
{
  return target_;
}

class SecondaryTargetData : public TargetDataBase
{
private:
  size_t send_buffer_pos_;

public:
  SecondaryTargetData();
  void set_send_buffer_pos( const size_t pos );
  size_t get_send_buffer_pos() const;
};

inline SecondaryTargetData::SecondaryTargetData()
  : TargetDataBase()
  , send_buffer_pos_( invalid_index )
{
}

inline void
SecondaryTargetData::set_send_buffer_pos( const size_t pos )
{
  send_buffer_pos_ = pos;
}

inline size_t
SecondaryTargetData::get_send_buffer_pos() const
{
  return send_buffer_pos_;
}

} // namespace nest

#endif // TARGET_DATA_H
