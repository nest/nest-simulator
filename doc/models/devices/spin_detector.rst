spin\_detector
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    spin_detector - Device for detecting binary states in neurons.

**Description:**
::

     
      The spin_detector is a recording device. It is used to decode and  
      record binary states from spiking activity from a single neuron, or  
      from multiple neurons at once. A single spike signals the 0 state, two  
      spikes at the same time signal the 1 state. If a neuron is in the 0 or  
      1 state and emits the spiking activity corresponding to the same  
      state, the same state is recorded again.  Therefore, it is not only  
      the transitions that are recorded. Data is recorded in memory or to  
      file as for all RecordingDevices. By default, GID, time, and binary  
      state (0 or 1) for each decoded state is recorded. The state can be  
      accessed from ['events']['weight'].  
       
      The spin_detector can also record binary state times with full  
      precision from neurons emitting precisely timed spikes. Set  
      /precise_times to true to achieve this. If there are precise models  
      and /precise_times is not set, it will be set to True at the start of  
      the simulation and /precision will be increased to 15 from its default  
      value of 3.  
       
      Any node from which binary states are to be recorded, must be  
      connected to the spin_detector using the Connect command. Any  
      connection weight and delay will be ignored for that connection.  
       
      Simulations progress in cycles defined by the minimum delay. During  
      each cycle, the spin_detector records (stores in memory or writes to  
      screen/file) the states during the previous cycle. As a consequence,  
      any state information that was decoded during the cycle immediately  
      preceding the end of the simulation time will not be recorded. Setting  
      the /stop parameter to at the latest one min_delay period before the  
      end of the simulation time ensures that all binary states desired to  
      be recorded, are recorded.  
       
      states are not necessarily written to file in chronological order.  
       
      

**Receives:**
::

    SpikeEvent  
       
      

**SeeAlso:**

-  `spike\_detector <../cc/spike_detector.html>`__
-  `Device <../cc/Device.html>`__
-  `RecordingDevice <../cc/RecordingDevice.html>`__

**Source:**
::

    ./spin_detector.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
