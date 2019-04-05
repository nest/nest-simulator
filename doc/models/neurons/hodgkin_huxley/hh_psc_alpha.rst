hh\_psc\_alpha
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    hh_psc_alpha - Hodgkin Huxley neuron model.

**Description:**
::

     
       
      hh_psc_alpha is an implementation of a spiking neuron using the Hodkin-Huxley  
      formalism.  
       
      (1) Post-syaptic currents  
      Incoming spike events induce a post-synaptic change of current modelled  
      by an alpha function. The alpha function is normalised such that an event of  
      weight 1.0 results in a peak current of 1 pA.  
       
       
      (2) Spike Detection  
      Spike detection is done by a combined threshold-and-local-maximum search: if  
      there is a local maximum above a certain threshold of the membrane potential,  
      it is considered a spike.  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      E_L   double - Resting membrane potential in mV.  
      g_L  double - Leak conductance in nS.  
      C_m    double - Capacity of the membrane in pF.  
      tau_ex     double - Rise time of the excitatory synaptic alpha function in ms.  
      tau_in  double - Rise time of the inhibitory synaptic alpha function in ms.  
      E_Na    double - Sodium reversal potential in mV.  
      g_Na  double - Sodium peak conductance in nS.  
      E_K     double - Potassium reversal potential in mV.  
      g_K    double - Potassium peak conductance in nS.  
      Act_m    double - Activation variable m  
      Act_h    double - Activation variable h  
      Inact_n   double    - Inactivation variable n  
      I_e    double - Constant external input current in pA.  
       
      Problems/Todo:  
       
      better spike detection  
      initial wavelet/spike at simulation onset  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      Authors: Schrader  
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
       
      Spiking Neuron Models:  
      Single Neurons, Populations, Plasticity  
      Wulfram Gerstner, Werner Kistler,  Cambridge University Press  
       
      Theoretical Neuroscience:  
      Computational and Mathematical Modeling of Neural Systems  
      Peter Dayan, L. F. Abbott, MIT Press (parameters taken from here)  
       
      Hodgkin, A. L. and Huxley, A. F.,  
      A Quantitative Description of Membrane Current  
      and Its Application to Conduction and Excitation in Nerve,  
      Journal of Physiology, 117, 500-544 (1952)  
       
      

**SeeAlso:**

-  `hh\_cond\_exp\_traub <../cc/hh_cond_exp_traub.html>`__

**Source:**
::

    ./hh_psc_alpha.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
