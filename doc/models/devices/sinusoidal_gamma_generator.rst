sinusoidal\_gamma\_generator
=====================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    sinusoidal_gamma_generator - Generates sinusoidally modulated gamma  
      spike trains.

**Description:**
::

     
      sinusoidal_gamma_generator generates sinusoidally modulated gamma spike  
      trains. By default, each target of the generator will receive a different  
      spike train.  
       
      The instantaneous rate of the process is given by  
       
      f(t) = rate + amplitude sin ( 2 pi frequency t + phase * pi/180 )  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
       
      rate  double - Mean firing rate in spikes/second, default: 0 s^-1  
      amplitude  double  - Firing rate modulation amplitude in spikes/second,  
      default: 0 s^-1  
      frequency  double  - Modulation frequency in Hz, default: 0 Hz  
      phase    double - Modulation phase in degree [0-360], default: 0  
      order   double - Gamma order (>= 1), default: 1  
       
      individual_spike_trains   bool - See note below, default: true  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
         - The gamma generator requires 0 <= amplitude <= rate.  
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
       
      

**References:**
::

    Barbieri et al, J Neurosci Methods 105:25-37 (2001)  
      

**Author:**
::

    Hans E Plesser, Thomas Heiberg  
       
      

**FirstVersion:**
::

    October 2007, May 2013  
      

**SeeAlso:**

-  `sinusoidal\_poisson\_generator <../cc/sinusoidal_poisson_generator.html>`__
-  `gamma\_sup\_generator <../cc/gamma_sup_generator.html>`__

**Source:**
::

    ./sinusoidal_gamma_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
