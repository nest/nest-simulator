iaf\_cond\_alpha\_mc
=============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_cond_alpha_mc - PROTOTYPE Multi-compartment conductance-based leaky  
      integrate-and-fire neuron model.

**Description:**
::

     
      THIS MODEL IS A PROTOTYPE FOR ILLUSTRATION PURPOSES. IT IS NOT YET  
      FULLY TESTED. USE AT YOUR OWN PERIL!  
       
      iaf_cond_alpha_mc is an implementation of a multi-compartment spiking  
      neuron using IAF dynamics with conductance-based synapses. It serves  
      mainly to illustrate the implementation of multicompartment models in  
      NEST.  
       
      The model has three compartments: soma, proximal and distal dendrite,  
      labeled as s, p, and d, respectively. Compartments are connected through  
      passive conductances as follows  
       
      C_m.s d/dt V_m.s = ...   - g_sp ( V_m.s  - V_m.p )  
       
      C_m.p d/dt V_m.p = ...    - g_sp ( V_m.p  - V_m.s )   - g_pd ( V_m.p  - V_m.d )  
       
      C_m.d d/dt V_m.d = ...        - g_pd ( V_m.d  - V_m.p )  
       
      A spike is fired when the somatic membrane potential exceeds threshold,  
      V_m.s >= V_th. After a spike, somatic membrane potential is clamped to  
      a reset potential, V_m.s == V_reset, for the refractory period. Dendritic  
      membrane potentials are not manipulated after a spike.  
       
      There is one excitatory and one inhibitory conductance-based synapse  
      onto each compartment, with alpha-function time course. The alpha  
      function is normalised such that an event of weight 1.0 results in a  
      peak current of 1 nS at t = tau_syn. Each compartment can also receive  
      current input from a current generator, and an external (rheobase)  
      current can be set for each compartment.  
       
      Synapses, including those for injection external currents, are addressed through  
      the receptor types given in the receptor_types entry of the state dictionary.  
      Note that in contrast to the single-compartment iaf_cond_alpha model, all  
      synaptic weights must be positive numbers!  
       
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary. Parameters  
      for each compartment are collected in a sub-dictionary; these sub-dictionaries  
      are called "soma", "proximal", and "distal", respectively. In the list below,  
      these parameters are marked with an asterisk.  
       
      V_m*    double - Membrane potential in mV  
      E_L*  double - Leak reversal potential in mV.  
      C_m*    double - Capacity of the membrane in pF  
      E_ex*   double - Excitatory reversal potential in mV.  
      E_in*     double - Inhibitory reversal potential in mV.  
      g_L*  double - Leak conductance in nS;  
      tau_syn_ex*  double   - Rise time of the excitatory synaptic alpha function in ms.  
      tau_syn_in*  double    - Rise time of the inhibitory synaptic alpha function in ms.  
      I_e*    double - Constant input current in pA.  
       
      g_sp     double - Conductance connecting soma and proximal dendrite, in nS.  
      g_pd     double - Conductance connecting proximal and distal dendrite, in  
      nS.  
      t_ref  double - Duration of refractory period in ms.  
      V_th  double - Spike threshold in mV.  
      V_reset     double - Reset potential of the membrane in mV.  
       
      Example:  
      See examples/nest/mc_neuron.py.  
       
      Remark:  
      This is a prototype for illustration which has undergone only limited testing.  
      Details of the implementation and user-interface will likely change.  
      USE AT YOUR OWN PERIL!  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
       
      Meffin, H., Burkitt, A. N., & Grayden, D. B. (2004). An analytical  
      model for the large, fluctuating synaptic conductance state typical of  
      neocortical neurons in vivo. J.  Comput. Neurosci., 16, 159-175.  
       
      Bernander, O ., Douglas, R. J., Martin, K. A. C., & Koch, C. (1991).  
      Synaptic background activity influences spatiotemporal integration in  
      single pyramidal cells.  Proc. Natl. Acad. Sci. USA, 88(24),  
      11569-11573.  
       
      

**Author:**
::

    Plesser  
       
      

**SeeAlso:**

-  `iaf\_cond\_alpha <../cc/iaf_cond_alpha.html>`__

**Source:**
::

    ./iaf_cond_alpha_mc.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
