noise\_generator
=========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    noise_generator - Device to generate Gaussian white noise current.

**Description:**
::

     
      This device can be used to inject a Gaussian "white" noise current into a node.  
      The current is not really white, but a piecewise constant current with Gaussian  
      distributed amplitude. The current changes at intervals of dt. dt must be a  
      multiple of the simulation step size, the default is 1.0ms,  
      corresponding to a 1kHz cut-off.  
      Additionally a second sinusodial modulated term can be added to the standard  
      deviation of the noise.  
       
      The current generated is given by  
       
      I(t) = mean + std * N_j  for t_0 + j dt <= t < t_0 + (j-1) dt  
       
      where N_j are Gaussian random numbers with unit standard deviation and t_0 is  
      the device onset time.  
      If the modulation is added the current is given by  
       
      I(t) = mean + sqrt(std^2 + std_mod^2 * sin(omega * t + phase)) * N_j  
      for t_0 + j dt <= t < t_0 + (j-1) dt  
       
      For a detailed discussion of the properties of the noise generator, please see  
      the noise_generator.ipynb notebook included in the NEST source code  
      (docs/model_details).  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
       
      mean  double - mean value of the noise current in pA  
      std  double - standard deviation of noise current in pA  
      dt   double - interval between changes in current in ms, default 1.0ms  
      std_mod   double - modulated standard deviation of noise current in pA  
      phase  double - Phase of sine modulation (0-360 deg)  
      frequency double - Frequency of sine modulation in Hz  
       
      

**Sends:**
::

    CurrentEvent  
       
      

**Remarks:**
::

     
         - All targets receive different currents.  
        - The currents for all targets change at the same points in time.  
        - The interval between changes, dt, must be a multiple of the time step.  
         - The effect of this noise current on a neuron DEPENDS ON DT. Consider  
      the membrane potential fluctuations evoked when a noise current is  
      injected into a neuron. The standard deviation of these fluctuations  
      across an ensemble will increase with dt for a given value of std.  
      For the leaky integrate-and-fire neuron with time constant tau_m and  
      capacity C_m, membrane potential fluctuations Sigma at times t_j+delay are  
      given by  
       
      Sigma = std * tau_m / C_m * sqrt( (1-x) / (1+x) ) where x = exp(-dt/tau_m)  
       
      for large t_j. In the white noise limit, dt -> 0, one has  
       
      Sigma -> std / C_m * sqrt(dt * tau / 2).  
       
      To obtain comparable results for different values of dt, you must  
      adapt std.  
        - As the noise generator provides a different current for each of its targets,  
      the current recorded represents the instantaneous average of all the  
      currents computed. When there exists only a single target, this would be  
      equivalent to the actual current provided to that target.  
       
      

**Author:**
::

    Ported to NEST2 API 08/2007 by Jochen Eppler, updated 07/2008 by HEP 

**SeeAlso:**

-  `Device <../cc/Device.html>`__

**Source:**
::

    ./noise_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
