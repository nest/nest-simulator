Command: ht\_synapse
====================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    ht_synapse - Synapse with depression after Hill & Tononi (2005).

**Description:**
::

     
      This synapse implements the depression model described in [1, p 1678].  
      See docs/model_details/HillTononi.ipynb for details.  
       
      Synaptic dynamics are given by  
       
      P'(t) = ( 1 - P ) / tau_P  
      P(T+) = (1    - delta_P) P(T-)   for T : time of a spike  
      P(t=0) = 1  
       
      w(t) = w_max * P(t)  is the resulting synaptic weight  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      tau_P   double   - synaptic vesicle pool recovery time constant [ms]  
      delta_P  double - fractional change in vesicle pool on incoming spikes  
      [unitless]  
      P  double - current size of the vesicle pool [unitless, 0 <= P <= 1]  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] S Hill and G Tononi (2005). J Neurophysiol 93:1671-1698.  
       
      

**Author:**
::

    Hans Ekkehard Plesser, based on markram_synapse  
      

**FirstVersion:**
::

    March 2009  
      

**SeeAlso:**

-  `ht\_neuron <../cc/ht_neuron.html>`__
-  `tsodyks\_synapse <../cc/tsodyks_synapse.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./ht_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
