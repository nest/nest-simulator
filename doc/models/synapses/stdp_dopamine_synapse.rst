Command: stdp\_dopamine\_synapse
================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    stdp_dopamine_synapse - Synapse type for dopamine-modulated  
      spike-timing dependent plasticity.

**Examples:**
::

     
      /volume_transmitter Create /vol Set  
      /iaf_psc_alpha Create /pre_neuron Set  
      /iaf_psc_alpha Create /post_neuron Set  
      /iaf_psc_alpha Create /neuromod_neuron Set  
      /stdp_dopamine_synapse  << /vt vol >>  SetDefaults  
      neuromod_neuron vol Connect  
      pre_neuron post_neuron /stdp_dopamine_synapse Connect  
       
      

**Description:**
::

     
      stdp_dopamine_synapse is a connection to create synapses with  
      dopamine-modulated spike-timing dependent plasticity (used as a  
      benchmark model in [1], based on [2]). The dopaminergic signal is a  
      low-pass filtered version of the spike rate of a user-specific pool  
      of neurons. The spikes emitted by the pool of dopamine neurons are  
      delivered to the synapse via the assigned volume transmitter. The  
      dopaminergic dynamics is calculated in the synapse itself.  
       
      

**Parameters:**
::

     
      Common properties:  
      vt   long   - ID of volume_transmitter collecting the spikes  
      from the pool of dopamine releasing neurons and  
      transmitting the spikes to the synapse. A value of  
      -1 indicates that no volume transmitter has been  
      assigned.  
      A_plus   double  - Amplitude of weight change for facilitation  
      A_minus   double  - Amplitude of weight change for depression  
      tau_plus  double    - STDP time constant for facilitation in ms  
      tau_c    double - Time constant of eligibility trace in ms  
      tau_n     double - Time constant of dopaminergic trace in ms  
      b    double - Dopaminergic baseline concentration  
      Wmin   double - Minimal synaptic weight  
      Wmax   double - Maximal synaptic weight  
       
      Individual properties:  
      c   double - eligibility trace  
      n    double - neuromodulator concentration  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
         - based on an earlier version by Wiebke Potjans  
      - major changes to code after code revision in Apr 2013  
       
      

**References:**
::

     
      [1] Potjans W, Morrison A and Diesmann M (2010). Enabling  
      functional neural circuit simulations with distributed  
      computing of neuromodulated plasticity.  
      Front. Comput. Neurosci. 4:141. doi:10.3389/fncom.2010.00141  
      [2] Izhikevich, E.M. (2007). Solving the distal reward problem  
      through linkage of STDP and dopamine signaling. Cereb. Cortex,  
      17(10), 2443-2452.  
       
      

**Author:**
::

    Susanne Kunkel  
      

**SeeAlso:**

-  `volume\_transmitter <../cc/volume_transmitter.html>`__

**Source:**
::

    ./stdp_dopa_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
