
#ifndef NEST_ENUM_TYPES_H
#define NEST_ENUM_TYPES_H

#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace nest
{

namespace nest_enum_types
{
enum class RecrodingDeviceType
{
  UNSETREC = -1,
  MULTIMETER,
  SPIKE_RECORDER,
  SPIN_DETECTOR,
  WEIGHT_RECORDER
};

enum class StimulationDeviceType
{
  UNSETSTIM = -1,
  CURRENT_GENERATOR,
  SPIKE_GENERATOR,
  DOUBLE_DATA_GENERATOR,
  DELAYED_RATE_CONNECTION_GENERATOR,
};

struct DeviceType
{
private:
  StimulationDeviceType stim;
  RecrodingDeviceType rec;
  bool type_is_resolved;

public:
  DeviceType()
    : stim( StimulationDeviceType::UNSETSTIM )
    , rec( RecrodingDeviceType::UNSETREC )
    , type_is_resolved( false )
  {
  }

  DeviceType( StimulationDeviceType value )
    : stim( value )
    , rec( RecrodingDeviceType::UNSETREC )
    , type_is_resolved( true )
  {
  }

  DeviceType( RecrodingDeviceType value )
    : stim( StimulationDeviceType::UNSETSTIM )
    , rec( value )
    , type_is_resolved( true )
  {
  }

  DeviceType( const DeviceType& type )
    : stim( type.stim )
    , rec( type.rec )
    , type_is_resolved( type.type_is_resolved )
  {
  }

  void
  set( StimulationDeviceType value )
  {
    if ( type_is_resolved )
    {
      throw std::runtime_error(
        "Type is already resolved to StimulationDevice, can not change "
        "underlying type!" );
    }
    else
    {
      stim = value;
      type_is_resolved = true;
    }
  }

  void
  set( RecrodingDeviceType value )
  {
    if ( type_is_resolved )
    {
      throw std::runtime_error(
        "Type is already resolved to RecrodingDevice, can not change "
        "underlying type!" );
    }
    else
    {
      rec = value;
      type_is_resolved = true;
    }
  }

  template < typename EnumT >
  bool
  equals( EnumT value )
  {
    if constexpr ( std::is_same< EnumT, StimulationDeviceType >::value )
    {
      if ( type_is_resolved && stim != StimulationDeviceType::UNSETSTIM && stim == value )
        return true;
      else
        return false;
    }

    if constexpr ( std::is_same< EnumT, RecrodingDeviceType >::value )
    {
      if ( type_is_resolved && rec != RecrodingDeviceType::UNSETREC && rec == value )
        return true;
      else
        return false;
    }
    return false;
  }

  int
  get_value()
  {
    if ( type_is_resolved && stim != StimulationDeviceType::UNSETSTIM )
    {
      return static_cast< int >( stim );
    }

    if ( type_is_resolved && rec != RecrodingDeviceType::UNSETREC )
    {
      return static_cast< int >( rec );
    }

    return -1;
  }
};

}; // namespace nest_enum_types

} // namespace nest


#endif