vogels\_sprekeler\_synapse
===================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    vogels_sprekeler_synapse - Synapse type for symmetric spike-timing  
      dependent  
      plasticity with constant depression.

**Description:**
::

     
      vogels_sprekeler_synapse is a connector to create synapses with symmetric  
      spike time dependent plasticity and constant depression (as defined in [1]).  
      The learning rule is symmetric, i.e., the synapse is strengthened  
      irrespective of the order of the pre and post-synaptic spikes. Each  
      pre-synaptic spike also causes a constant depression of the synaptic weight  
      which differentiates this rule from other classical stdp rules.  
       
      

**Parameters:**
::

     
      tau     double - Time constant of STDP window, potentiation in ms  
      Wmax  double - Maximum allowed weight  
      eta     double - learning rate  
      alpha    double - constant depression (= 2 * tau * target firing rate in  
      [1])  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] Vogels et al. (2011) Inhibitory Plasticity Balances Excitation and  
      Inhibition in Sensory Pathways and Memory Networks.  
      Science Vol. 334, Issue 6062, pp. 1569-1573  
      DOI: 10.1126/science.1211095  
       
      

**Author:**
::

    Ankur Sinha  
      

**FirstVersion:**
::

    January 2016  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__

**Source:**
::

    ./vogels_sprekeler_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
