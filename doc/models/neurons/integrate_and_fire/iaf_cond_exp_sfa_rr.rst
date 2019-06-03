iaf\_cond\_exp\_sfa\_rr
================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_cond_exp_sfa_rr - Simple conductance based leaky integrate-and-fire  
      neuron model.

**Description:**
::

     
      iaf_cond_exp_sfa_rr is an iaf_cond_exp_sfa_rr i.e. an implementation of a  
      spiking neuron using IAF dynamics with conductance-based synapses,  
      with additional spike-frequency adaptation and relative refractory  
      mechanisms as described in Dayan+Abbott, 2001, page 166.  
       
      As for the iaf_cond_exp_sfa_rr, Incoming spike events induce a post-synaptic  
      change  of  conductance  modelled  by an  exponential  function.  The  
      exponential function is  normalised such that an event  of weight 1.0  
      results in a peak current of 1 nS.  
       
      Outgoing spike events induce a change of the adaptation and relative  
      refractory conductances by q_sfa and q_rr, respectively.  Otherwise  
      these conductances decay exponentially with time constants tau_sfa  
      and tau_rr, respectively.  
       
      

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
      q_sfa     double - Outgoing spike activated quantal spike-frequency adaptation  
      conductance increase in nS.  
      q_rr   double - Outgoing spike activated quantal relative refractory  
      conductance increase in nS.  
      tau_sfa   double - Time constant of spike-frequency adaptation in ms.  
      tau_rr  double - Time constant of the relative refractory mechanism in ms.  
      E_sfa    double - spike-frequency adaptation conductance reversal potential in  
      mV.  
      E_rr  double - relative refractory mechanism conductance reversal potential  
      in mV.  
      I_e    double - an external stimulus current in pA.  
       
      

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
       
      Dayan, P. and Abbott, L. F. (2001). Theoretical Neuroscience, MIT Press (p166)  
       
      

**Author:**
::

    Sven Schrader, Eilif Muller  
       
      

**SeeAlso:**

-  `iaf\_cond\_exp\_sfa\_rr <../cc/iaf_cond_exp_sfa_rr.html>`__
-  `aeif\_cond\_alpha <../cc/aeif_cond_alpha.html>`__
-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_cond\_alpha <../cc/iaf_cond_alpha.html>`__

**Source:**
::

    ./iaf_cond_exp_sfa_rr.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
