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
#include "static_assert.h"
#include "target.h"

namespace nest
{
class TargetDataFields
{
private:
  unsigned int lcid_ : 27;
  unsigned int tid_ : 10;
  unsigned int syn_id_ : 8;

public:
  // Members must be set explicitly -- no defaults

  /**
   * Sets the local connection ID.
   */
  void set_lcid( const index lcid );

  /**
   * Returns the local connection ID.
   */
  index get_lcid() const;

  /**
   * Sets the target ID.
   */
  void set_tid( const thread tid );

  /**
   * Returns the target ID.
   */
  thread get_tid() const;

  /**
   * Sets the synapse-type ID.
   */
  void set_syn_id( const synindex syn_id );

  /**
   * Returns the synapse-type ID.
   */
  synindex get_syn_id() const;
};

inline void
TargetDataFields::set_lcid( const index lcid )
{
  lcid_ = lcid;
}

inline index
TargetDataFields::get_lcid() const
{
  return lcid_;
}

inline void
TargetDataFields::set_tid( const thread tid )
{
  tid_ = tid;
}

inline thread
TargetDataFields::get_tid() const
{
  return tid_;
}

inline void
TargetDataFields::set_syn_id( const synindex syn_id )
{
  syn_id_ = syn_id;
}

inline synindex
TargetDataFields::get_syn_id() const
{
  return syn_id_;
}

class SecondaryTargetDataFields
{
private:
  unsigned int send_buffer_pos_;
  unsigned char syn_id_;

public:
  // Members must be set explicitly -- no defaults
  void set_send_buffer_pos( const size_t pos );
  size_t get_send_buffer_pos() const;
  void set_syn_id( const synindex syn_id );
  synindex get_syn_id() const;
};

inline void
SecondaryTargetDataFields::set_send_buffer_pos( const size_t pos )
{
  assert( pos < std::numeric_limits< unsigned int >::max() );
  send_buffer_pos_ = pos;
}

inline size_t
SecondaryTargetDataFields::get_send_buffer_pos() const
{
  return send_buffer_pos_;
}

inline void
SecondaryTargetDataFields::set_syn_id( const synindex syn_id )
{
  assert( syn_id < std::numeric_limits< unsigned char >::max() );
  syn_id_ = syn_id;
}

inline synindex
SecondaryTargetDataFields::get_syn_id() const
{
  return syn_id_;
}

/**
 * Used to communicate part of the connection infrastructure from
 * post- to presynaptic side. These are the elements of the MPI
 * buffers.
 * SeeAlso: SpikeData
 */
class TargetData
{
  // Members must be set explicitly -- no defaults
  // Done this way to create large vector without preconstruction
  // and to handle variant fields

private:
  static const unsigned int default_marker_ = 0;
  static const unsigned int complete_marker_ = 1;
  static const unsigned int end_marker_ = 2;
  static const unsigned int invalid_marker_ = 3;

  unsigned int source_lid_ : 19; //!< local id of presynaptic neuron
  unsigned int source_tid_ : 10; //!< thread index of presynaptic neuron
  unsigned int marker_ : 2;
  bool is_primary_ : 1; //!< TargetData has TargetDataFields
                        //!< else has SecondaryTargetDataFields

public:
  //<! variant fields
  union
  {
    TargetDataFields target_data;
    SecondaryTargetDataFields secondary_data;
  };

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

//!< check legal size
typedef StaticAssert< sizeof( TargetData ) == 12 >::success
  success_target_data_size;

inline void
TargetData::reset_marker()
{
  marker_ = default_marker_;
}

inline void
TargetData::set_complete_marker()
{
  marker_ = complete_marker_;
}

inline void
TargetData::set_end_marker()
{
  marker_ = end_marker_;
}

inline void
TargetData::set_invalid_marker()
{
  marker_ = invalid_marker_;
}

inline bool
TargetData::is_complete_marker() const
{
  return marker_ == complete_marker_;
}

inline bool
TargetData::is_end_marker() const
{
  return marker_ == end_marker_;
}

inline bool
TargetData::is_invalid_marker() const
{
  return marker_ == invalid_marker_;
}

inline void
TargetData::set_source_lid( const index source_lid )
{
  assert( source_lid < 1048576 );
  source_lid_ = source_lid;
}

inline void
TargetData::set_source_tid( const thread source_tid )
{
  assert( source_tid < 1024 );
  source_tid_ = source_tid;
}

inline index
TargetData::get_source_lid() const
{
  return source_lid_;
}

inline thread
TargetData::get_source_tid() const
{
  return source_tid_;
}

inline void
TargetData::set_is_primary( const bool is_primary )
{
  is_primary_ = is_primary;
}

inline bool
TargetData::is_primary() const
{
  return is_primary_;
}
} // namespace nest

#endif // TARGET_DATA_H
