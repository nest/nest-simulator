gif\_psc\_exp\_multisynapse
====================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    gif_psc_exp_multisynapse - Current-based generalized  
      integrate-and-fire neuron model with multiple synaptic time  
      constants according to Mensi et al. (2012) and Pozzorini et al. (2015).

**Description:**
::

     
       
      gif_psc_exp_multisynapse is the generalized integrate-and-fire neuron  
      according to Mensi et al. (2012) and Pozzorini et al. (2015), with  
      exponential shaped postsynaptic currents.  
       
      This model features both an adaptation current and a dynamic threshold for  
      spike-frequency adaptation. The membrane potential (V) is described by the  
      differential equation:  
       
      C*dV(t)/dt = -g_L*(V(t)-E_L)   - eta_1(t)  - eta_2(t)  - ...   - eta_n(t) + I(t)  
       
      where each eta_i is a spike-triggered current (stc), and the neuron model can  
      have arbitrary number of them.  
      Dynamic of each eta_i is described by:  
       
      tau_eta_i*d{eta_i}/dt = -eta_i  
       
      and in case of spike emission, its value increased by a constant (which can be  
      positive or negative):  
       
      eta_i = eta_i + q_eta_i  (in case of spike emission).  
       
      Neuron produces spikes STOCHASTICALLY according to a point process with the  
      firing intensity:  
       
      lambda(t) = lambda_0 * exp[ (V(t)-V_T(t)) / Delta_V ]  
       
      where V_T(t) is a time-dependent firing threshold:  
       
      V_T(t) = V_T_star + gamma_1(t) + gamma_2(t) + ... + gamma_m(t)  
       
      where gamma_i is a kernel of spike-frequency adaptation (sfa), and the neuron  
      model can have arbitrary number of them.  
      Dynamic of each gamma_i is described by:  
       
      tau_gamma_i*d{gamma_i}/dt = -gamma_i  
       
      and in case of spike emission, its value increased by a constant (which can be  
      positive or negative):  
       
      gamma_i = gamma_i + q_gamma_i  (in case of spike emission).  
       
      Note that in the current implementation of the model (as described in [1] and  
      [2]) the values of eta_i and gamma_i are affected immediately after spike  
      emission. However, GIF toolbox (http://wiki.epfl.ch/giftoolbox) which fits  
      the model using experimental data, requires a different set of eta_i and  
      gamma_i. It applies the jump of eta_i and gamma_i after the refractory period.  
      One can easily convert between q_eta/gamma of these two approaches:  
      q_eta_giftoolbox = q_eta_NEST * (1 - exp( -tau_ref / tau_eta ))  
      The same formula applies for q_gamma.  
       
      On the postsynapic side, there can be arbitrarily many synaptic time constants  
      (gif_psc_exp has exactly two: tau_syn_ex and tau_syn_in). This can be reached  
      by specifying separate receptor ports, each for a different time constant. The  
      port number has to match the respective "receptor_type" in the connectors.  
       
      The shape of post synaptic current is exponential.  
       
      

**Parameters:**
::

     
      C_m     double - Capacity of the membrane in pF  
      t_ref   double - Duration of refractory period in ms.  
      V_reset   double - Reset value after a spike in mV.  
      E_L   double - Leak reversal potential in mV.  
      g_L     double - Leak conductance in nS.  
      I_e    double - Constant external input current in pA.  
       
      Spike adaptation and firing intensity parameters:  
      q_stc     vector of double   - Values added to spike-triggered currents (stc)  
      after each spike emission in nA.  
      tau_stc   vector of double    - Time constants of stc variables in ms.  
      q_sfa   vector of double   - Values added to spike-frequency adaptation  
      (sfa) after each spike emission in mV.  
      tau_sfa   vector of double  - Time constants of sfa variables in ms.  
      Delta_V   double   - Stochasticity level in mV.  
      lambda_0   double  - Stochastic intensity at firing threshold V_T in 1/s.  
      V_T_star   double    - Base threshold in mV  
       
      Synaptic parameters  
      tau_syn  vector of double    - Time constants of the synaptic currents in ms.  
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
       
      [1] Mensi S, Naud R, Pozzorini C, Avermann M, Petersen CC, Gerstner W (2012)  
      Parameter extraction and classification of three cortical neuron types  
      reveals two distinct adaptation mechanisms. J. Neurophysiol., 107(6),  
      1756-1775.  
       
      [2] Pozzorini C, Mensi S, Hagens O, Naud R, Koch C, Gerstner W (2015)  
      Automated High-Throughput Characterization of Single Neurons by Means of  
      Simplified Spiking Models. PLoS Comput. Biol., 11(6), e1004275.  
       
       
      

**Author:**
::

    March 2016, Setareh  
      

**SeeAlso:**

-  `pp\_psc\_delta <../cc/pp_psc_delta.html>`__
-  `gif\_psc\_exp <../cc/gif_psc_exp.html>`__
-  `gif\_cond\_exp <../cc/gif_cond_exp.html>`__
-  `gif\_cond\_exp\_multisynapse <../cc/gif_cond_exp_multisynapse.html>`__

**Source:**
::

    ./gif_psc_exp_multisynapse.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
