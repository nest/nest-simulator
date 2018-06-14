Command: gif\_pop\_psc\_exp
===========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    gif_pop_psc_exp - Population of generalized integrate-and-fire neurons  
      with exponential postsynaptic currents and adaptation

**Description:**
::

     
       
      This model simulates a population of spike-response model neurons with  
      multi-timescale adaptation and exponential postsynaptic currents, as  
      described in [1].  
       
      The single neuron model is defined by the hazard function  
       
      lambda_0 * exp[ ( V_m  - E_sfa ) / Delta_V ]  
       
      After each spike the membrane potential V_m is reset to V_reset. Spike  
      frequency  
      adaptation is implemented by a set of exponentially decaying traces, the  
      sum of which is E_sfa. Upon a spike, all adaptation traces are incremented  
      by the respective q_sfa each and decay with the respective time constant  
      tau_sfa.  
       
      The corresponding single neuron model is available in NEST as gif_psc_exp.  
      The default parameters, although some are named slightly different, are not  
      matched in both models due to historical reasons. See below for the parameter  
      translation.  
       
      As gif_pop_psc_exp represents many neurons in one node, it may send a lot  
      of spikes. In each time step, it sends at most one spike though, the  
      multiplicity of which is set to the number of emitted spikes. Postsynaptic  
      neurons and devices in NEST understand this as several spikes, but  
      communication effort is reduced in simulations.  
       
      This model uses a new algorithm to directly simulate the population activity  
      (sum of all spikes) of the population of neurons, without explicitly  
      representing each single neuron (see [1]). The computational cost is largely  
      independent of the number N of neurons represented. The algorithm used  
      here is fundamentally different from and likely much faster than the one  
      used in the previously added population model pp_pop_psc_delta.  
       
      Connecting two population models corresponds to full connectivity of every  
      neuron in each population. An approximation of random connectivity can be  
      implemented by connecting populations through a spike_dilutor.  
       
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      V_reset   double - Membrane potential is reset to this value in mV after a  
      spike.  
      V_T_star   double  - Threshold level of the membrane potential in mV.  
      E_L   double - Resting potential in mV  
      Delta_V   double  - Noise level of escape rate in mV.  
      C_m  double - Capacitance of the membrane in pF.  
      tau_m   double - Membrane time constant in ms.  
      t_ref    double - Duration of refractory period in ms.  
      I_e   double - Constant input current in pA.  
      N    long   - Number of neurons in the population.  
      len_kernel long      - Refractory effects are accounted for up to len_kernel  
      time steps  
      lambda_0   double    - Firing rate at threshold in 1/s.  
      tau_syn_ex double    - Time constant for excitatory synaptic currents in ms.  
      tau_syn_in double   - Time constant for inhibitory synaptic currents in ms.  
      tau_sfa   double vector     - Adaptation time constants in ms.  
      q_sfa     double vector  - Adaptation kernel amplitudes in ms.  
      BinoRand   bool   - If True, binomial random numbers are used, otherwise  
      we use Poisson distributed spike counts.  
       
       
      Parameter translation to gif_psc_exp:  
       
      gif_pop_psc_exp   gif_psc_exp  relation  
      ----------------------------------------------------  
      tau_m     g_L     tau_m = C_m / g_L  
      N     ---     use N gif_psc_exp  
       
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      Authors: Nov 2016, Moritz Deger, Tilo Schwalger, Hesam Setareh  
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
       
      [1] Towards a theory of cortical columns: From spiking neurons to  
      interacting neural populations of finite size  
      Tilo Schwalger, Moritz Deger, Wulfram Gerstner  
      PLoS Comput Biol 2017  
      https://doi.org/10.1371/journal.pcbi.1005507  
       
      

**SeeAlso:**

-  `gif\_psc\_exp <../cc/gif_psc_exp.html>`__
-  `pp\_pop\_psc\_delta <../cc/pp_pop_psc_delta.html>`__
-  `spike\_dilutor <../cc/spike_dilutor.html>`__

**Source:**
::

    ./gif_pop_psc_exp.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
