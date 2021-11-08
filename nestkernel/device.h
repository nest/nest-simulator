/*
 *  device.h
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

#ifndef DEVICE_H
#define DEVICE_H


// Includes from nestkernel:
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * @defgroup Devices
 * This group comprises stimulation and recording devices.
 */

/**
 * Class implementing common interface and properties common for all devices.
 *
 * This class provides a common interface for all derived device classes.
 * Each class derived from Node and implementing a device, should have a
 * member derived from class Device. This member will contribute the
 * implementation of device specific properties.
 *
 * This class manages the properties common to all devices, namely
 * origin, start and stop of the time window during which the device
 * is active and the optional device label. The precise semantics of
 * when the device is active depend on the type of device and are
 * defined in subclasses.
 *
 * @ingroup Devices
 *
 * @author HEP 2002-07-22, 2008-03-21, 2008-06-20
 */
class Device
{
public:
  Device();
  Device( const Device& n );
  virtual ~Device()
  {
  }

  /** Reset dynamic state to that of model. */
  virtual void
  init_state()
  {
  }

  /** Reset buffers. */
  virtual void
  init_buffers()
  {
  }

  /** Set internal variables before calls to SimulationManager::run() */
  virtual void calibrate();

  virtual void get_status( DictionaryDatum& ) const;
  virtual void set_status( const DictionaryDatum& );

  /**
   *  Returns true if the device is active at the given time stamp.
   *  Semantics are implemented by subclasses.
   */
  virtual bool is_active( Time const& T ) const = 0;

  /**
   * Return lower limit in steps.
   * @todo Should be protected, but is temporarily public
   *       to solve inheritance problems in AnalogSamplingDevice.
   */
  long get_t_min_() const;

  /**
   * Return upper limit in steps.
   * @todo Should be protected, but is temporarily public
   *       to solve inheritance problems in AnalogSamplingDevice.
   */
  long get_t_max_() const;

  Time const& get_origin() const;
  Time const& get_start() const;
  Time const& get_stop() const;

private:
  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    //! Origin of device time axis, relative to network time. Defaults to 0.
    Time origin_;

    //!< Start time, relative to origin. Defaults to 0.
    Time start_;

    //!< Stop time, relative to origin. Defaults to "infinity".
    Time stop_;

    Parameters_(); //!< Sets default parameter values

    //! Copy and recalibrate parameter set
    Parameters_( const Parameters_& );

    Parameters_& operator=( const Parameters_& );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dictionary

  private:
    //! Update given Time parameter including error checking
    static void update_( const DictionaryDatum&, const Name&, Time& );
  };


  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    /**
     * Time step of device activation.
     * t_min_ = origin_ + start_, in steps.
     * @note This is an auxiliary variable that is initialized to -1 in the
     * constructor and set to its proper value by calibrate. It should NOT
     * be returned by get_parameters().
     */
    long t_min_;

    /**
     * Time step of device deactivation.
     * t_max_ = origin_ + stop_, in steps.
     * @note This is an auxiliary variable that is initialized to -1 in the
     * constructor and set to its proper value by calibrate. It should NOT
     * be returned by get_parameters().
     */
    long t_max_;
  };

  // ----------------------------------------------------------------

  Parameters_ P_;
  Variables_ V_;
};

} // namespace

inline void
nest::Device::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
}

inline void
nest::Device::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline nest::Time const&
nest::Device::get_origin() const
{
  return P_.origin_;
}

inline nest::Time const&
nest::Device::get_start() const
{
  return P_.start_;
}

inline nest::Time const&
nest::Device::get_stop() const
{
  return P_.stop_;
}

inline long
nest::Device::get_t_min_() const
{
  return V_.t_min_;
}

inline long
nest::Device::get_t_max_() const
{
  return V_.t_max_;
}

#endif // DEVICE_H
