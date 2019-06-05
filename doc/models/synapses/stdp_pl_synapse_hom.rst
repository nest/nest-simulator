stdp\_pl\_synapse\_hom
===============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    stdp_pl_synapse_hom - Synapse type for spike-timing dependent  
      plasticity with power law implementation using homogeneous parameters, i.e.  
      all synapses have the same parameters.

**Description:**
::

     
      stdp_pl_synapse is a connector to create synapses with spike time  
      dependent plasticity (as defined in [1]).  
       
       
      

**Parameters:**
::

     
      tau_plus  double   - Time constant of STDP window, potentiation in ms  
      (tau_minus defined in post-synaptic neuron)  
      lambda   double  - Learning rate  
      alpha    double - Asymmetry parameter (scales depressing increments as  
      alpha*lambda)  
      mu  double - Weight dependence exponent, potentiation  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      The parameters can only be set by SetDefaults and apply to all synapses of  
      the model.  
       
      

**References:**
::

     
      [1] Morrison et al. (2007) Spike-timing dependent plasticity in balanced  
      random networks. Neural Computation.  
       
      

**Author:**
::

    Abigail Morrison  
      

**FirstVersion:**
::

    May 2007  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `tsodyks\_synapse <../cc/tsodyks_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./stdp_pl_connection_hom.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
