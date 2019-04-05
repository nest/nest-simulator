pulsepacket\_generator
===============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    pulsepacket_generator - Generate sequence of Gaussian pulse packets.

**Description:**
::

     
      The pulsepacket_generator produces a spike train contains Gaussian pulse  
      packets centered about given  times.  A Gaussian pulse packet is  
      a given number of spikes with normal distributed random displacements  
      from the center time of the pulse.  
      It resembles the output of synfire groups of neurons.  
       
      

**Parameters:**
::

     
      pulse_times  double    - Times of the centers of pulses in ms  
      activity  int    - Number of spikes per pulse  
      sdev    double - Standard deviation of spike times in each pulse in ms  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
         - All targets receive identical spike trains.  
        - New pulse packets are generated when activity or sdev are changed.  
         - Gaussian pulse are independently generated for each given  
      pulse-center time.  
       - Both standard deviation and number of spikes may be set at any time.  
      Pulses are then re-generated with the new values.  
       
      

**SeeAlso:**

-  `spike\_generator <../cc/spike_generator.html>`__
-  `StimulatingDevice <../cc/StimulatingDevice.html>`__

**Source:**
::

    ./pulsepacket_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
