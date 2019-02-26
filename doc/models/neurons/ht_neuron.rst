ht\_neuron
===================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    ht_neuron - Neuron model after Hill & Tononi (2005).

**Examples:**
::

     
         - docs/model_details/HillTononiModels.ipynb  
      - pynest/examples/intrinsic_currents_spiking.py  
      - pynest/examples/intrinsic_currents_subthreshold.py  
       
      

**Description:**
::

     
      This model neuron implements a slightly modified version of the  
      neuron model described in [1]. The most important properties are:  
       
       - Integrate-and-fire with threshold adaptive threshold.  
      - Repolarizing potassium current instead of hard reset.  
      - AMPA, NMDA, GABA_A, and GABA_B conductance-based synapses with  
      beta-function (difference of exponentials) time course.  
         - Voltage-dependent NMDA with instantaneous or two-stage unblocking [1, 2].  
      - Intrinsic currents I_h, I_T, I_Na(p), and I_KNa.  
       - Synaptic "minis" are not implemented.  
       
      Documentation and 

**Parameters:**
::

     
      V_m        - membrane potential  
      tau_m      - membrane time constant applying to all currents except  
      repolarizing K-current (see [1], p 1677)  
      t_ref     - refractory time and duration of post-spike repolarizing  
      potassium current (t_spike in [1])  
      tau_spike      - membrane time constant for post-spike repolarizing  
      potassium current  
      voltage_clamp    - if true, clamp voltage to value at beginning of simulation  
      (default: false, mainly for testing)  
      theta, theta_eq, tau_theta    - threshold, equilibrium value, time constant  
      g_KL, E_K, g_NaL, E_Na        - conductances and reversal potentials for K and  
      Na leak currents  
      {E_rev,g_peak,tau_rise,tau_decay}_{AMPA,NMDA,GABA_A,GABA_B}  
        - reversal potentials, peak conductances and  
      time constants for synapses (tau_rise/  
      tau_decay correspond to tau_1/tau_2 in the  
      paper)  
      V_act_NMDA, S_act_NMDA, tau_Mg_{fast, slow}_NMDA  
       - parameters for voltage dependence of NMDA-  
      conductance, see above  
      instant_unblock_NMDA        - instantaneous NMDA unblocking (default: false)  
      {E_rev,g_peak}_{h,T,NaP,KNa}   - reversal potential and peak conductance for  
      intrinsic currents  
      tau_D_KNa      - relaxation time constant for I_KNa  
      receptor_types     - dictionary mapping synapse names to ports on  
      neuron model  
      recordables     - list of recordable quantities  
      equilibrate     - if given and true, time-dependent activation  
      and inactivation state variables (h, m) of  
      intrinsic currents and NMDA channels are set  
      to their equilibrium values during this  
      SetStatus call; otherwise they retain their  
      present values.  
       
      Note: Conductances are unitless in this model and currents are in mV.  
       
      

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

     
      [1] S Hill and G Tononi (2005). J Neurophysiol 93:1671-1698.  
      [2] M Vargas-Caballero HPC Robinson (2003). J Neurophysiol 89:2778-2783.  
       
      

**Author:**
::

    Hans Ekkehard Plesser  
       
      

**FirstVersion:**
::

    October 2009; full revision November 2016  
       
      

**SeeAlso:**

-  `ht\_synapse <../cc/ht_synapse.html>`__

**Source:**
::

    ./ht_neuron.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
