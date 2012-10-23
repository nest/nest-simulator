/*
 *  smp_generator.h
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

#ifndef SMP_GENERATOR_H
#define SMP_GENERATOR_H

#include "nest.h"
#include "event.h"
#include "node.h"
#include "stimulating_device.h"
#include "poisson_randomdev.h"
#include "connection.h"
#include "universal_data_logger.h"

namespace nest {

  class Network;

  /* BeginDocumentation
     Name: smp_generator - Generates sinusoidally modulated Poisson train.

     Description:
     smp_generator generates a sinusoidally modulate Poisson spike train.
     The instantaneous rate of the process is given by

     f(t) = max(0, dc + ac sin ( 2 pi freq t + phi )) >= 0

     Parameters: 
     The following parameters can be set in the status dictionary.

     dc         double - Mean firing rate in spikes/second, default: 0 s^-1
     ac         double - Firing rate modulation amplitude in spikes/second, default: 0 s^-1
     freq       double - Modulation frequency in Hz, default: 0 Hz
     phi        double - Modulation phase in radian, default: 0
  
     Remarks:
     - If ac > dc, firing rate is cut off at zero. In this case, the mean
       firing rate will be less than dc.
     - The state of the generator is reset on calibration.
     - The generator does not support precise spike timing.
     - You can use the multimeter to sample the rate of the generator.
     - The generator sends the same spike train to all of its targets.
     - The generator will create different trains if run at different
       temporal resolutions.

     Receives: DataLoggingRequest

     Sends: SpikeEvent

     FirstVersion: July 2006, Oct 2009
     Author: Hans Ekkehard Plesser
     SeeAlso: ac_generator
  */

  class smp_generator: public Node
  {
    
  public:        
    
    smp_generator();
    smp_generator(const smp_generator&);

    using Node::handle;
    using Node::connect_sender;

    port check_connection(Connection&, port);

    void handle(DataLoggingRequest &);
    port connect_sender(DataLoggingRequest &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &) ;

  private:
    void init_state_(const Node&);
    void init_buffers_();
    void calibrate();

    void update(Time const &, const long_t, const long_t);
    
    struct Parameters_ {
      /** temporal frequency in radian/ms. */
      double_t om_;
            
      /** phase in radian */
      double_t phi_;
      
      /** DC amplitude in spikes/s */
      double_t dc_;
      
      /** AC amplitude in spikes/s */
      double_t ac_;

      Parameters_();  //!< Sets default parameter values
      Parameters_(const Parameters_& );
      Parameters_& operator=(const Parameters_& p); // Copy constructor EN

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      
      /**
       * Set values from dicitonary.
       * @note State is passed so that the position can be reset if the 
       *       spike_times_ vector has been filled with new data.
       */
      void set(const DictionaryDatum&);  
    };

    // ------------------------------------------------------------
    
    /**
     * State
     */
    struct State_ {      

      double_t y_0_;      //!< Two-component oscillator state vector, see Rotter&Diesmann
      double_t y_1_;

      double_t rate_;    //!< current rate, kept for recording
      Time last_spike_;  //!< time stamp of most recent spike fired.

      State_();  //!< Sets default state value

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      void set(const DictionaryDatum&, const Parameters_&);  //!< Set values from dicitonary
    };

    // ------------------------------------------------------------

    // These friend declarations must be precisely here.
    friend class RecordablesMap<smp_generator>;
    friend class UniversalDataLogger<smp_generator>;

    // ---------------------------------------------------------------- 

    /**
     * Buffers of the model.
     */
    struct Buffers_ {
      Buffers_(smp_generator&);
      Buffers_(const Buffers_&, smp_generator&);
      UniversalDataLogger<smp_generator> logger_;
    };

    // ------------------------------------------------------------

    struct Variables_ {
      librandom::PoissonRandomDev poisson_dev_;  //!< random deviate generator
      
      double_t sin_;  //!< sin(h om) in propagator
      double_t cos_;  //!< cos(h om) in propagator
    };

    double_t get_rate_() const { return 1000.0 * S_.rate_; }

    // ------------------------------------------------------------

    StimulatingDevice<SpikeEvent> device_;
    static RecordablesMap<smp_generator> recordablesMap_;

    Parameters_ P_;
    State_      S_;
    Variables_  V_;
    Buffers_    B_;

  };

  inline
    port smp_generator::check_connection(Connection& c, port receptor_type)
    {
      SpikeEvent e;
      e.set_sender(*this);
      c.check_event(e);
      return c.get_target()->connect_sender(e, receptor_type);
    }

  inline
    port smp_generator::connect_sender(DataLoggingRequest& dlr, 
				      port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return B_.logger_.connect_logging_device(dlr, recordablesMap_);
    }

  inline
    void smp_generator::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d);
    device_.get_status(d);
    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  inline
    void smp_generator::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors

    ptmp.set(d);                       // throws if BadProperty
    // We now know that ptmp is consistent. We do not write it back
    // to P_ before we are also sure that the properties to be set
    // in the parent class are internally consistent.
    device_.set_status(d);

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;

  }

} // namespace

#endif // smp_generator_H
