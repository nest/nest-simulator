tsodyks2\_synapse
==========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    tsodyks2_synapse - Synapse type with short term plasticity.

**Description:**
::

     
      This synapse model implements synaptic short-term depression and short-term  
      facilitation according to [1] and [2]. It solves Eq (2) from [1] and  
      modulates U according to eq. (2) of [2].  
       
      This connection merely scales the synaptic weight, based on the spike history  
      and the parameters of the kinetic model. Thus, it is suitable for all types  
      of synaptic dynamics, that is current or conductance based.  
       
      The parameter A_se from the publications is represented by the  
      synaptic weight. The variable x in the synapse properties is the  
      factor that scales the synaptic weight.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      U     double - probability of release increment (U1) [0,1],  
      default=0.5  
      u     double - Maximum probability of release (U_se) [0,1],  
      default=0.5  
      x     double - current scaling factor of the weight, default=U  
      tau_rec   double  - time constant for depression in ms, default=800 ms  
      tau_fac   double   - time constant for facilitation in ms, default=0 (off)  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
       
      Under identical conditions, the tsodyks2_synapse produces  
      slightly lower peak amplitudes than the tsodyks_synapse. However,  
      the qualitative behavior is identical. The script  
      test_tsodyks2_synapse.py in the examples compares the two synapse  
      models.  
       
       
      

**References:**
::

     
      [1] Tsodyks, M. V., & Markram, H. (1997). The neural code between neocortical  
      pyramidal neurons depends on neurotransmitter release probability.  
      PNAS, 94(2), 719-23.  
      [2] Fuhrmann, G., Segev, I., Markram, H., & Tsodyks, M. V. (2002). Coding of  
      temporal information by activity-dependent synapses. Journal of  
      neurophysiology, 87(1), 140-8.  
      [3] Maass, W., & Markram, H. (2002). Synapses as dynamic memory buffers.  
      Neural networks, 15(2), 155-61.  
       
      

**Author:**
::

    Marc-Oliver Gewaltig, based on tsodyks_synapse by Moritz Helias  
      

**FirstVersion:**
::

    October 2011  
      

**SeeAlso:**

-  `tsodyks\_synapse <../cc/tsodyks_synapse.html>`__
-  `synapsedict <../cc/synapsedict.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./tsodyks2_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
