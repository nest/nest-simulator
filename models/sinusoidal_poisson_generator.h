/*
 *  sinusoidal_poisson_generator.h
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

#ifndef SINUSOIDAL_POISSON_GENERATOR_H
#define SINUSOIDAL_POISSON_GENERATOR_H

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
     Name: sinusoidal_poisson_generator - Generates sinusoidally modulated Poisson spike trains.

     Description:
     sinusoidal_poisson_generator generates sinusoidally modulated Poisson spike trains. By default,
     each target of the generator will receive a different spike train.

     The instantaneous rate of the process is given by

         f(t) = max(0, dc + ac sin ( 2 pi freq t + phi )) >= 0

     Parameters: 
     The following parameters can be set in the status dictionary:

     dc         double - Mean firing rate in spikes/second, default: 0 s^-1
     ac         double - Firing rate modulation amplitude in spikes/second, default: 0 s^-1
     freq       double - Modulation frequency in Hz, default: 0 Hz
     phi        double - Modulation phase in radian, default: 0
  
     individual_spike_trains   bool - See note below, default: true

     Remarks:
     - If ac > dc, firing rate is cut off at zero. In this case, the mean
       firing rate will be less than dc.
     - The state of the generator is reset on calibration.
     - The generator does not support precise spike timing.
     - You can use the multimeter to sample the rate of the generator.
     - The generator will create different trains if run at different
       temporal resolutions.

     - Individual spike trains vs single spike train:
       By default, the generator sends a different spike train to each of its targets.
       If /individual_spike_trains is set to false using either SetDefaults or CopyModel
       before a generator node is created, the generator will send the same spike train
       to all of its targets.

     Receives: DataLoggingRequest

     Sends: SpikeEvent

     FirstVersion: July 2006, Oct 2009, May 2013
     Author: Hans Ekkehard Plesser
     SeeAlso: poisson_generator, sinusoidal_gamma_generator
  */

  class sinusoidal_poisson_generator: public Node
  {
    
  public:        
    
    sinusoidal_poisson_generator();
    sinusoidal_poisson_generator(const sinusoidal_poisson_generator&);

    using Node::handle;
    using Node::connect_sender;

    port check_connection(Connection&, port);

    void handle(DataLoggingRequest &);
    port connect_sender(DataLoggingRequest &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &) ;

    //! Model can be switched between proxies (single spike train) and not
    bool has_proxies() const { return not P_.individual_spike_trains_; }

    //! Allow multimeter to connect to local instances
    bool local_receiver() const { return true; } 
    
  private:
    void init_state_(const Node&);
    void init_buffers_();
    void calibrate();
    void event_hook(DSSpikeEvent&);

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

      /** Emit individual spike trains for each target, or same for all? */
      bool individual_spike_trains_;

      Parameters_();  //!< Sets default parameter values
      Parameters_(const Parameters_& );
      Parameters_& operator=(const Parameters_& p); // Copy constructor EN

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      
      /**
       * Set values from dicitonary.
       * @note State is passed so that the position can be reset if the 
       *       spike_times_ vector has been filled with new data.
       */
      void set(const DictionaryDatum&, const sinusoidal_poisson_generator&);  
    };

    // ------------------------------------------------------------
    
    /**
     * State
     */
    struct State_ {      

      double_t y_0_;      //!< Two-component oscillator state vector, see Rotter&Diesmann
      double_t y_1_;

      double_t rate_;    //!< current rate, kept for recording

      State_();  //!< Sets default state value

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      void set(const DictionaryDatum&, const Parameters_&);  //!< Set values from dicitonary
    };

    // ------------------------------------------------------------

    // These friend declarations must be precisely here.
    friend class RecordablesMap<sinusoidal_poisson_generator>;
    friend class UniversalDataLogger<sinusoidal_poisson_generator>;

    // ---------------------------------------------------------------- 

    /**
     * Buffers of the model.
     */
    struct Buffers_ {
      Buffers_(sinusoidal_poisson_generator&);
      Buffers_(const Buffers_&, sinusoidal_poisson_generator&);
      UniversalDataLogger<sinusoidal_poisson_generator> logger_;
    };

    // ------------------------------------------------------------

    struct Variables_ {
      librandom::PoissonRandomDev poisson_dev_;  //!< random deviate generator
      
      double_t h_;    //! time resolution (ms)
      double_t sin_;  //!< sin(h om) in propagator
      double_t cos_;  //!< cos(h om) in propagator
    };

    double_t get_rate_() const { return 1000.0 * S_.rate_; }

    // ------------------------------------------------------------

    StimulatingDevice<SpikeEvent> device_;
    static RecordablesMap<sinusoidal_poisson_generator> recordablesMap_;

    Parameters_ P_;
    State_      S_;
    Variables_  V_;
    Buffers_    B_;

  };

  inline
    port sinusoidal_poisson_generator::check_connection(Connection& c, port receptor_type)
    {
      if ( P_.individual_spike_trains_ )
      {
	DSSpikeEvent e;
	e.set_sender(*this);
	c.check_event(e);
	return c.get_target()->connect_sender(e, receptor_type);
      }
      else
      {
	SpikeEvent e;
	e.set_sender(*this);
	c.check_event(e);
	return c.get_target()->connect_sender(e, receptor_type);
      }
    }

  inline
    port sinusoidal_poisson_generator::connect_sender(DataLoggingRequest& dlr, 
				      port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return B_.logger_.connect_logging_device(dlr, recordablesMap_);
    }

  inline
    void sinusoidal_poisson_generator::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d);
    device_.get_status(d);
    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  inline
    void sinusoidal_poisson_generator::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors

    ptmp.set(d, *this);     // throws if BadProperty
    // We now know that ptmp is consistent. We do not write it back
    // to P_ before we are also sure that the properties to be set
    // in the parent class are internally consistent.
    device_.set_status(d);

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;

  }

} // namespace

#endif // sinusoidal_poisson_generator_H
