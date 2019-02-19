hh\_cond\_exp\_traub
=============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    hh_cond_exp_traub - Hodgin Huxley based model, Traub modified.

**Description:**
::

     
       
      hh_cond_exp_traub is an implementation of a modified Hodkin-Huxley model  
       
      (1) Post-synaptic currents  
      Incoming spike events induce a post-synaptic change of conductance modeled  
      by an exponential function. The exponential function is normalized such that an  
      event of weight 1.0 results in a peak current of 1 nS.  
       
      (2) Spike Detection  
      Spike detection is done by a combined threshold-and-local-maximum search: if  
      there is a local maximum above a certain threshold of the membrane potential,  
      it is considered a spike.  
       
      Problems/Todo:  
      Only the channel variables m,h,n are implemented. The original  
      contains variables called y,s,r,q and \chi.  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      V_T   double - Voltage offset that controls dynamics. For default  
      parameters, V_T = -63mV results in a threshold around  
      -50mV.  
      E_L    double - Leak reversal potential in mV.  
      C_m     double - Capacity of the membrane in pF.  
      g_L    double - Leak conductance in nS.  
      tau_syn_ex double - Time constant of the excitatory synaptic exponential  
      function in ms.  
      tau_syn_in double    - Time constant of the inhibitory synaptic exponential  
      function in ms.  
      t_ref     double - Duration of refractory period in ms.  
      E_ex  double - Excitatory synaptic reversal potential in mV.  
      E_in     double - Inhibitory synaptic reversal potential in mV.  
      E_Na     double - Sodium reversal potential in mV.  
      g_Na  double - Sodium peak conductance in nS.  
      E_K     double - Potassium reversal potential in mV.  
      g_K    double - Potassium peak conductance in nS.  
      I_e  double - External input current in pA.  
       
      

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

     
       
      Traub, R.D. and Miles, R. (1991) Neuronal Networks of the Hippocampus.  
      Cambridge University Press, Cambridge UK.  
       
      

**Author:**
::

    Schrader  
       
      

**SeeAlso:**

-  `hh\_psc\_alpha <../cc/hh_psc_alpha.html>`__

**Source:**
::

    ./hh_cond_exp_traub.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
