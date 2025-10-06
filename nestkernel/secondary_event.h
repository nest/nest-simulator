/*
 *  secondary_event.h
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

#ifndef SECONDARY_EVENT_H
#define SECONDARY_EVENT_H

// Includes from nestkernel
#include "event.h"

// C++ includes
#include <set>

namespace nest
{

class SICEvent;

/**
 * Base class of secondary events. Provides interface for
 * serialization and deserialization. This event type may be
 * used to transmit data on a regular basis
 * Further information about secondary events and
 * their usage with gap junctions can be found in
 *
 * Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,
 * Bolten, M., Frommer, A. and Diesmann, M.,
 * A unified framework for spiking and gap-junction interactions
 * in distributed neuronal network simulations,
 * Front. Neuroinform. 9:22. (2015),
 * doi: 10.3389/fninf.2015.00022
 */
class SecondaryEvent : public Event
{

public:
  SecondaryEvent* clone() const override = 0;

  virtual void add_syn_id( const synindex synid ) = 0;

  //! size of event in units of unsigned int
  virtual size_t size() = 0;
  virtual std::vector< unsigned int >::iterator& operator<<( std::vector< unsigned int >::iterator& pos ) = 0;
  virtual std::vector< unsigned int >::iterator& operator>>( std::vector< unsigned int >::iterator& pos ) = 0;

  virtual const std::set< synindex >& get_supported_syn_ids() const = 0;

  virtual void reset_supported_syn_ids() = 0;
};

/**
 * This template function returns the number of uints covered by a variable of
 * type T. This function is used to determine the storage demands for a
 * variable of type T in the NEST communication buffer, which is of type
 * std::vector<unsigned int>.
 */
template < typename T >
size_t
number_of_uints_covered()
{
  size_t num_uints = sizeof( T ) / sizeof( unsigned int );
  if ( num_uints * sizeof( unsigned int ) < sizeof( T ) )
  {
    num_uints += 1;
  }
  return num_uints;
}

/**
 * This template function writes data of type T to a given position of a
 * std::vector< unsigned int >.
 * Please note that this function does not increase the size of the vector,
 * it just writes the data to the position given by the iterator.
 * The function is used to write data from SecondaryEvents to the NEST
 * communication buffer. The pos iterator is advanced during execution.
 * For a discussion on the functionality of this function see github issue #181
 * and pull request #184.
 */
template < typename T >
void
write_to_comm_buffer( T d, std::vector< unsigned int >::iterator& pos )
{
  // there is no aliasing problem here, since cast to char* invalidate strict
  // aliasing assumptions
  char* const c = reinterpret_cast< char* >( &d );

  const size_t num_uints = number_of_uints_covered< T >();
  size_t left_to_copy = sizeof( T );

  for ( size_t i = 0; i < num_uints; i++ )
  {
    memcpy( &( *( pos + i ) ), c + i * sizeof( unsigned int ), std::min( left_to_copy, sizeof( unsigned int ) ) );
    left_to_copy -= sizeof( unsigned int );
  }

  pos += num_uints;
}

/**
 * Template class for the storage and communication of a std::vector of type
 * DataType. The class provides the functionality to communicate homogeneous
 * data of type DataType. The second template type Subclass (which should be
 * chosen as the derived class itself) is used to distinguish derived classes
 * with the same DataType. This is required because of the included static
 * variables in the base class (as otherwise all derived classes with the same
 * DataType would share the same static variables).
 *
 * Technically the DataSecondaryEvent only contains iterators pointing to
 * the memory location of the std::vector< DataType >.
 *
 * Conceptually, there is a one-to-one mapping between a SecondaryEvent
 * and a ConnectorModel using it. The synindex of this particular
 * ConnectorModel is stored as first element in the static set
 * ``supported_syn_ids_`` on model registration. There are however reasons (e.g.
 * the usage of CopyModel() or the creation of the labeled synapse model
 * duplicates for pyNN) which make it necessary to register several
 * ConnectorModels with one SecondaryEvent. Therefore the synindices
 * of all these models are added to ``supported_syn_ids_``.
 */
template < typename DataType, typename Subclass >
class DataSecondaryEvent : public SecondaryEvent
{
private:
  static std::set< synindex > supported_syn_ids_;
  static size_t coeff_length_; // length of coeffarray

  union CoeffarrayBegin
  {
    std::vector< unsigned int >::iterator as_uint;
    typename std::vector< DataType >::iterator as_DataType;

    CoeffarrayBegin() {}; // need to provide default constructor due to
                          // non-trivial constructors of iterators
  } coeffarray_begin_;

  union CoeffarrayEnd
  {
    std::vector< unsigned int >::iterator as_uint;
    typename std::vector< DataType >::iterator as_DataType;

    CoeffarrayEnd() {}; // need to provide default constructor due to
                        // non-trivial constructors of iterators
  } coeffarray_end_;

public:
  /**
   * This function adds the ids of connection models that use this
   * event type. This is called when the model is registered with the
   * kernel and when the corresponded connector model is copied.
   *
   * This function needs to be a virtual function of the base class
   * DataSecondaryEvent as it is usually called from a pointer or
   * reference of type SecondaryEvent.
   *
   * See also:
   */
  void add_syn_id( const synindex synid ) override;

  const std::set< synindex >&
  get_supported_syn_ids() const override
  {
    return supported_syn_ids_;
  }

  /**
   * Resets the vector of supported syn ids to those originally
   * registered via ModelsModule or user defined Modules, i.e.,
   * removes all syn ids created by CopyModel. This is important to
   * maintain consistency across ResetKernel, which removes all copied
   * models.
   */
  void
  reset_supported_syn_ids() override
  {
    supported_syn_ids_.clear();
  }

  static void set_coeff_length( const size_t coeff_length );

  void
  set_coeffarray( std::vector< DataType >& ca )
  {
    coeffarray_begin_.as_DataType = ca.begin();
    coeffarray_end_.as_DataType = ca.end();
    assert( coeff_length_ == ca.size() );
  }

  /**
   * The following operator is used to read the information of the
   * DataSecondaryEvent from the buffer in EventDeliveryManager::deliver_events
   */
  std::vector< unsigned int >::iterator&
  operator<<( std::vector< unsigned int >::iterator& pos ) override
  {
    // The synid can be skipped here as it is stored in a static vector

    // generating a copy of the coeffarray is too time consuming
    // therefore we save an iterator to the beginning+end of the coeffarray
    coeffarray_begin_.as_uint = pos;

    pos += coeff_length_ * number_of_uints_covered< DataType >();

    coeffarray_end_.as_uint = pos;

    return pos;
  }


  /**
   * The following operator is used to write the information of the
   * DataSecondaryEvent into the secondary_events_buffer_.
   * All DataSecondaryEvents are identified by the synid of the
   * first element in supported_syn_ids_.
   */
  std::vector< unsigned int >::iterator&
  operator>>( std::vector< unsigned int >::iterator& pos ) override
  {
    for ( auto it = coeffarray_begin_.as_DataType; it != coeffarray_end_.as_DataType; ++it )
    {
      // we need the static_cast here as the size of a stand-alone variable
      // and a std::vector entry may differ (e.g. for std::vector< bool >)
      write_to_comm_buffer( static_cast< DataType >( *it ), pos );
    }
    return pos;
  }

  size_t
  size() override
  {
    size_t s = number_of_uints_covered< synindex >();
    s += number_of_uints_covered< size_t >();
    s += number_of_uints_covered< DataType >() * coeff_length_;

    return s;
  }

  const std::vector< unsigned int >::iterator&
  begin()
  {
    return coeffarray_begin_.as_uint;
  }

  const std::vector< unsigned int >::iterator&
  end()
  {
    return coeffarray_end_.as_uint;
  }

  DataType get_coeffvalue( std::vector< unsigned int >::iterator& pos );
};

/**
 * Event for gap-junction information. The event transmits the interpolation
 * of the membrane potential to the connected neurons.
 */
class GapJunctionEvent : public DataSecondaryEvent< double, GapJunctionEvent >
{

public:
  GapJunctionEvent()
  {
  }

  void operator()() override;
  GapJunctionEvent* clone() const override;
};

/**
 * Event for rate model connections without delay. The event transmits
 * the rate to the connected neurons.
 */
class InstantaneousRateConnectionEvent : public DataSecondaryEvent< double, InstantaneousRateConnectionEvent >
{

public:
  InstantaneousRateConnectionEvent()
  {
  }

  void operator()() override;
  InstantaneousRateConnectionEvent* clone() const override;
};

/**
 * Event for rate model connections with delay. The event transmits
 * the rate to the connected neurons.
 */
class DelayedRateConnectionEvent : public DataSecondaryEvent< double, DelayedRateConnectionEvent >
{

public:
  DelayedRateConnectionEvent()
  {
  }

  void operator()() override;
  DelayedRateConnectionEvent* clone() const override;
};


/**
 * Event for diffusion connections (rate model connections for the
 * siegert_neuron). The event transmits the rate to the connected neurons.
 */
class DiffusionConnectionEvent : public DataSecondaryEvent< double, DiffusionConnectionEvent >
{
private:
  // drift factor of the corresponding connection
  double drift_factor_;
  // diffusion factor of the corresponding connection
  double diffusion_factor_;

public:
  DiffusionConnectionEvent()
  {
  }

  void operator()() override;
  DiffusionConnectionEvent* clone() const override;

  void
  set_diffusion_factor( double t ) override
  {
    diffusion_factor_ = t;
  };

  void
  set_drift_factor( double t ) override
  {
    drift_factor_ = t;
  };

  double get_drift_factor() const;
  double get_diffusion_factor() const;
};

/**
 * Event for learning signal connections. The event transmits
 * the learning signal to the connected neurons.
 */
class LearningSignalConnectionEvent : public DataSecondaryEvent< double, LearningSignalConnectionEvent >
{

public:
  LearningSignalConnectionEvent()
  {
  }

  void operator()() override;
  LearningSignalConnectionEvent* clone() const override;
};

} // namespace nest

#endif /* #ifndef SECONDARY_EVENT_H */
