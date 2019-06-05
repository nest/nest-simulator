static\_synapse
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    static_synapse - Synapse type for static connections.

**Description:**
::

     
      static_synapse does not support any kind of plasticity. It simply stores  
      the parameters target, weight, delay and receiver port for each connection.  
       
      

**Transmits:**
::

    SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,  
      DoubleDataEvent, DataLoggingRequest  
       
      

**Remarks:**
::

    Refactored for new connection system design, March 2007  
       
      

**Author:**
::

    Jochen Martin Eppler, Moritz Helias  
       
      

**FirstVersion:**
::

    October 2005  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `tsodyks\_synapse <../cc/tsodyks_synapse.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__

**Source:**
::

    ./static_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
