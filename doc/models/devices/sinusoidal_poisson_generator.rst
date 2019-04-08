sinusoidal\_poisson\_generator
=======================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    sinusoidal_poisson_generator - Generates sinusoidally modulated Poisson  
      spike trains.

**Description:**
::

     
      sinusoidal_poisson_generator generates sinusoidally modulated Poisson spike  
      trains. By default, each target of the generator will receive a different  
      spike train.  
       
      The instantaneous rate of the process is given by  
       
      f(t) = max(0, rate + amplitude sin ( 2 pi frequency t + phase * pi/180 ))  
      >= 0  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
       
      rate  double - Mean firing rate in spikes/second, default: 0 s^-1  
      amplitude  double  - Firing rate modulation amplitude in spikes/second,  
      default: 0 s^-1  
      frequency  double  - Modulation frequency in Hz, default: 0 Hz  
      phase    double - Modulation phase in degree [0-360], default: 0  
       
      individual_spike_trains   bool - See note below, default: true  
       
      

**Receives:**
::

    DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
         - If amplitude > rate, firing rate is cut off at zero. In this case, the mean  
      firing rate will be less than rate.  
        - The state of the generator is reset on calibration.  
        - The generator does not support precise spike timing.  
       - You can use the multimeter to sample the rate of the generator.  
        - The generator will create different trains if run at different  
      temporal resolutions.  
       
       - Individual spike trains vs single spike train:  
      By default, the generator sends a different spike train to each of its  
      targets. If /individual_spike_trains is set to false using either  
      SetDefaults or CopyModel before a generator node is created, the generator  
      will send the same spike train to all of its targets.  
       
      

**Author:**
::

    Hans Ekkehard Plesser  
      

**FirstVersion:**
::

    July 2006, Oct 2009, May 2013  
      

**SeeAlso:**

-  `poisson\_generator <../cc/poisson_generator.html>`__
-  `sinusoidal\_gamma\_generator <../cc/sinusoidal_gamma_generator.html>`__

**Source:**
::

    ./sinusoidal_poisson_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
