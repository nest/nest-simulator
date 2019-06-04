iaf\_cond\_alpha
=========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_cond_alpha - Simple conductance based leaky integrate-and-fire neuron  
      model.

**Description:**
::

     
      iaf_cond_alpha is an implementation of a spiking neuron using IAF dynamics with  
      conductance-based synapses. Incoming spike events induce a post-synaptic change  
      of conductance modelled by an alpha function. The alpha function  
      is normalised such that an event of weight 1.0 results in a peak current of 1 nS  
      at t = tau_syn.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      E_L   double - Leak reversal potential in mV.  
      C_m     double - Capacity of the membrane in pF  
      t_ref   double - Duration of refractory period in ms.  
      V_th  double - Spike threshold in mV.  
      V_reset   double   - Reset potential of the membrane in mV.  
      E_ex    double - Excitatory reversal potential in mV.  
      E_in  double - Inhibitory reversal potential in mV.  
      g_L   double - Leak conductance in nS;  
      tau_syn_ex double - Rise time of the excitatory synaptic alpha function in ms.  
      tau_syn_in double  - Rise time of the inhibitory synaptic alpha function in ms.  
      I_e     double - Constant input current in pA.  
       
      

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
       
      Kuhn, Aertsen, Rotter (2004) Neuronal Integration of Synaptic Input in  
      the Fluctuation- Driven Regime. Jneurosci 24(10) 2345-2356  
       
      

**Author:**
::

    Schrader, Plesser  
       
      

**SeeAlso:**

-  `iaf\_cond\_exp <../cc/iaf_cond_exp.html>`__
-  `iaf\_cond\_alpha\_mc <../cc/iaf_cond_alpha_mc.html>`__

**Source:**
::

    ./iaf_cond_alpha.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
