/*
 *  dc_generator.h
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


/*BeginDocumentation
Name: dc_generator - provides DC input current

Description: The DC-Generator provides a constant DC Input
to the connected node. The unit of the current is pA.

Parameters: 
  The following parameters can be set in the status dictionary:
  amplitude  double - Amplitude of current in pA

Examples: The dc current can be altered in the following way:
   /dc_generator Create /dc_gen Set         % Creates a dc_generator, which is a node
   dc_gen GetStatus info                    % View properties (amplitude is 0)
   dc_gen << /amplitude 1500. >> SetStatus
   dc_gen GetStatus info                    % amplitude is now 1500.0

Note: The dc_generator is rather inefficient, since it needs to
      send the same current information on each time step. If you
      only need a constant bias current into a neuron, you should 
      set it directly in the neuron, e.g., dc_generator.

Sends: CurrentEvent
      
Author: docu by Sirko Straube

SeeAlso: Device, StimulatingDevice

*/ 

#ifndef DC_GENERATOR_H
#define DC_GENERATOR_H

#include <vector>
#include "nest.h"
#include "event.h"
#include "node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "stimulating_device.h"

namespace nest
{
  /**
   * DC current generator.
   *
   * @ingroup Devices
   */
  class dc_generator : public Node
  {
    
  public:        
    
    dc_generator();
    dc_generator(const dc_generator&);

    bool has_proxies() const {return false;} 


    port check_connection(Connection&, port);
    
    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

  private:

    void init_state_(const Node&);
    void init_buffers_();
    void calibrate();
    
    void update(Time const &, const long_t, const long_t);
    
    // ------------------------------------------------------------
    
    /**
     * Store independent parameters of the model.
     */
    struct Parameters_ {
      double_t    amp_;   //!< stimulation amplitude, in pA
      
      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      void set(const DictionaryDatum&);  //!< Set values from dicitonary
    };
    
    // ------------------------------------------------------------

    StimulatingDevice<CurrentEvent> device_;
    Parameters_ P_;
  };

  inline  
  port dc_generator::check_connection(Connection& c, port receptor_type)
  {
    CurrentEvent e;
    e.set_sender(*this);
    c.check_event(e);
    return c.get_target()->connect_sender(e, receptor_type);
  }
  
  inline
  void dc_generator::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    device_.get_status(d);
  }

  inline
  void dc_generator::set_status(const DictionaryDatum &d)
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

#endif /* #ifndef DC_GENERATOR_H */

