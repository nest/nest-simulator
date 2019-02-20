weight\_recorder
=========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    weight_recorder - Device for detecting single spikes.

**Description:**
::

     
      The weight_recorder device is a recording device. It is used to record  
      weights from synapses. Data is recorded in memory or to file as for all  
      RecordingDevices.  
      By default, source GID, target GID, time and weight of each spike is recorded.  
       
      In order to record only from a subset of connected synapses, the  
      weight_recorder accepts the parameters 'senders' and 'targets', with which the  
      recorded data is limited to the synapses with the corresponding source or target  
      gid.  
       
      The weight recorder can also record weights with full precision  
      from neurons emitting precisely timed spikes. Set /precise_times to  
      achieve this.  
       
      Data is not necessarily written to file in chronological order.  
       
      

**Receives:**
::

    WeightRecordingEvent  
       
      

**SeeAlso:**

-  `weight\_recorder <../cc/weight_recorder.html>`__
-  `spike\_detector <../cc/spike_detector.html>`__
-  `Device <../cc/Device.html>`__
-  `RecordingDevice <../cc/RecordingDevice.html>`__

**Source:**
::

    ./weight_recorder.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
