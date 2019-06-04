static\_synapse\_hom\_w
================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    static_synapse_hom_w - Synapse type for static connections with  
      homogeneous weight.

**Description:**
::

     
      static_synapse_hom_w does not support any kind of plasticity. It simply  
      stores the parameters delay, target, and receiver port for each connection  
      and uses a common weight for all connections.  
       
      

**Parameters:**
::

     
      No Parameters  
       
      

**Transmits:**
::

    SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,  
      DataLoggingRequest, DoubleDataEvent  
       
      

**Remarks:**
::

     
      The common weight for all connections of this model must be set by  
      SetDefaults on the model. If you create copies of this model using  
      CopyModel, each derived model can have a different weight.  
       
      

**References:**
::

     
      No References  
      

**Author:**
::

    Susanne Kunkel, Moritz Helias  
      

**FirstVersion:**
::

    April 2008  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./static_connection_hom_w.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
