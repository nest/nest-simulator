hh\_psc\_alpha\_gap
============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    hh_psc_alpha_gap - Hodgkin Huxley neuron model with gap-junction support.

**Description:**
::

     
       
      hh_psc_alpha_gap is an implementation of a spiking neuron using the  
      Hodkin-Huxley formalism. In contrast to hh_psc_alpha the implementation  
      additionally supports gap junctions.  
       
       
      (1) Post-syaptic currents  
      Incoming spike events induce a post-synaptic change of current modelled  
      by an alpha function. The alpha function is normalised such that an event of  
      weight 1.0 results in a peak current of 1 pA.  
       
      (2) Spike Detection  
      Spike detection is done by a combined threshold-and-local-maximum search: if  
      there is a local maximum above a certain threshold of the membrane potential,  
      it is considered a spike.  
       
      (3) Gap Junctions  
      Gap Junctions are implemented by a gap current of the form g_ij( V_i  - V_j).  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      E_L   double - Resting membrane potential in mV.  
      g_L  double - Leak conductance in nS.  
      C_m    double - Capacity of the membrane in pF.  
      tau_syn_ex double - Rise time of the excitatory synaptic alpha function in ms.  
      tau_syn_in double  - Rise time of the inhibitory synaptic alpha function in ms.  
      E_Na    double - Sodium reversal potential in mV.  
      g_Na  double - Sodium peak conductance in nS.  
      E_K     double - Potassium reversal potential in mV.  
      g_Kv1  double - Potassium peak conductance in nS.  
      g_Kv3    double - Potassium peak conductance in nS.  
      Act_m    double - Activation variable m  
      Act_h    double - Activation variable h  
      Inact_n   double    - Inactivation variable n  
      I_e    double - Constant external input current in pA.  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    SpikeEvent, GapJunctionEvent, CurrentEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent, GapJunctionEvent  
       
      

**References:**
::

     
       
      Spiking Neuron Models:  
      Single Neurons, Populations, Plasticity  
      Wulfram Gerstner, Werner Kistler,  Cambridge University Press  
       
      Mancilla, J. G., Lewis, T. J., Pinto, D. J.,  
      Rinzel, J., and Connors, B. W.,  
      Synchronization of electrically coupled pairs  
      of inhibitory interneurons in neocortex,  
      J. Neurosci. 27, 2058-2073 (2007),  
      doi: 10.1523/JNEUROSCI.2715-06.2007 (parameters taken from here)  
       
      Hodgkin, A. L. and Huxley, A. F.,  
      A Quantitative Description of Membrane Current  
      and Its Application to Conduction and Excitation in Nerve,  
      Journal of Physiology, 117, 500-544 (1952)  
       
      Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,  
      Bolten, M., Frommer, A. and Diesmann, M.,  
      A unified framework for spiking and gap-junction interactions  
      in distributed neuronal network simulations,  
      Front. Neuroinform. 9:22. (2015),  
      doi: 10.3389/fninf.2015.00022  
       
      

**Author:**
::

    Jan Hahne, Moritz Helias, Susanne Kunkel  
      

**SeeAlso:**

-  `hh\_psc\_alpha <../cc/hh_psc_alpha.html>`__
-  `hh\_cond\_exp\_traub <../cc/hh_cond_exp_traub.html>`__
-  `gap\_junction <../cc/gap_junction.html>`__

**Source:**
::

    ./hh_psc_alpha_gap.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
