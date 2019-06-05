spike\_detector
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    spike_detector - Device for detecting single spikes.

**Description:**
::

     
      The spike_detector device is a recording device. It is used to record  
      spikes from a single neuron, or from multiple neurons at once. Data  
      is recorded in memory or to file as for all RecordingDevices.  
      By default, GID and time of each spike is recorded.  
       
      The spike detector can also record spike times with full precision  
      from neurons emitting precisely timed spikes. Set /precise_times to  
      achieve this. If there are precise models and /precise_times is not  
      set, it will be set to True at the start of the simulation and  
      /precision will be increased to 15 from its default value of 3.  
       
      Any node from which spikes are to be recorded, must be connected to  
      the spike detector using a normal connect command. Any connection weight  
      and delay will be ignored for that connection.  
       
      Simulations progress in cycles defined by the minimum delay. During each  
      cycle, the spike detector records (stores in memory or writes to screen/file)  
      the spikes generated during the previous cycle. As a consequence, any  
      spikes generated during the cycle immediately preceding the end of the  
      simulation time will not be recorded. Setting the /stop parameter to at the  
      latest one min_delay period before the end of the simulation time ensures that  
      all spikes desired to be recorded, are recorded.  
       
      Spike are not necessarily written to file in chronological order.  
       
      

**Receives:**
::

    SpikeEvent  
       
      

**SeeAlso:**

-  `spike\_detector <../cc/spike_detector.html>`__
-  `Device <../cc/Device.html>`__
-  `RecordingDevice <../cc/RecordingDevice.html>`__

**Source:**
::

    ./spike_detector.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
