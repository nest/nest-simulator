Command: stdp\_triplet\_synapse
===============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    stdp_triplet_synapse - Synapse type with spike-timing dependent  
      plasticity (triplets).

**Description:**
::

     
      stdp_triplet_synapse is a connection with spike time dependent  
      plasticity accounting for spike triplet effects (as defined in [1]).  
       
      STDP examples:  
      pair-based   Aplus_triplet = Aminus_triplet = 0.0  
      triplet    Aplus_triplet = Aminus_triplet = 1.0  
       
      

**Parameters:**
::

     
      tau_plus    double - time constant of short presynaptic trace  
       - (tau_plus of [1])  
      tau_plus_triplet   double   - time constant of long presynaptic trace  
        - (tau_x of [1])  
      Aplus   double - weight of pair potentiation rule  
       - (A_plus_2 of [1])  
      Aplus_triplet    double - weight of triplet potentiation rule  
        - (A_plus_3 of [1])  
      Aminus   double - weight of pair depression rule  
      (A_minus_2 of [1])  
      Aminus_triplet   double - weight of triplet depression rule  
      - (A_minus_3 of [1])  
      Wmax    double - maximum allowed weight  
       
      States:  
      Kplus   double: pre-synaptic trace (r_1 of [1])  
      Kplus_triplet   double: triplet pre-synaptic trace (r_2 of [1])  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] J.-P. Pfister & W. Gerstner (2006) Triplets of Spikes in a Model  
      of Spike Timing-Dependent Plasticity.  The Journal of Neuroscience  
      26(38):9673-9682; doi:10.1523/JNEUROSCI.1425-06.2006  
       
      Notes:  
         - Presynaptic traces r_1 and r_2 of [1] are stored in the connection as  
      Kplus and Kplus_triplet and decay with time-constants tau_plus and  
      tau_plus_triplet, respectively.  
       - Postsynaptic traces o_1 and o_2 of [1] are acquired from the post-synaptic  
      neuron states Kminus_ and triplet_Kminus_ which decay on time-constants  
      tau_minus and tau_minus_triplet, respectively. These two time-constants  
      can be set as properties of the postsynaptic neuron.  
        - This version implements the 'all-to-all' spike interaction of [1]. The  
      'nearest-spike' interaction of [1] can currently not be implemented  
      without changing the postsynaptic archiving-node (clip the traces to a  
      maximum of 1).  
       
      

**Author:**
::

    Abigail Morrison, Eilif Muller, Alexander Seeholzer, Teo Stocco  
      Adapted by: Philipp Weidel  
      

**FirstVersion:**
::

    Nov 2007  
      

**SeeAlso:**

-  `stdp\_triplet\_synapse\_hpc <../cc/stdp_triplet_synapse_hpc.html>`__
-  `synapsedict <../cc/synapsedict.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./stdp_triplet_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
