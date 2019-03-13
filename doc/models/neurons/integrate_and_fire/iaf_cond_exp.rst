iaf\_cond\_exp
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_cond_exp - Simple conductance based leaky integrate-and-fire neuron  
      model.

**Description:**
::

     
      iaf_cond_exp is an implementation of a spiking neuron using IAF dynamics with  
      conductance-based synapses. Incoming spike events induce a post-synaptic change  
      of conductance modelled by an exponential function. The exponential function  
      is normalised such that an event of weight 1.0 results in a peak conductance of  
      1 nS.  
       
      

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
      tau_syn_ex double - Time constant of the excitatory synaptic exponential  
      function in ms.  
      tau_syn_in double    - Time constant of the inhibitory synaptic exponential  
      function in ms.  
      I_e   double - Constant external input current in pA.  
       
      

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
       
      

**Author:**
::

    Sven Schrader  
       
      

**SeeAlso:**

-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_cond\_exp <../cc/iaf_cond_exp.html>`__

**Source:**
::

    ./iaf_cond_exp.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
