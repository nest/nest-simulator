aeif\_cond\_alpha\_RK5
===============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    aeif_cond_alpha_RK5 - Conductance based exponential integrate-and-fire  
      neuron model according to Brette and Gerstner (2005)

**Description:**
::

     
      aeif_cond_alpha_RK5 is the adaptive exponential integrate and fire neuron  
      according to Brette and Gerstner (2005).  
      Synaptic conductances are modelled as alpha-functions.  
       
      This implementation uses a 5th order Runge-Kutta solver with adaptive stepsize  
      to integrate the differential equation (see Numerical Recipes 3rd Edition,  
      Press et al. 2007, Ch. 17.2).  
       
      The membrane potential is given by the following differential equation:  
      C dV/dt= -g_L(V-E_L)+g_L*Delta_T*exp((V-V_T)/Delta_T)-g_e(t)(V-E_e)  
      -g_i(t)(V-E_i)-w +I_e  
       
      and  
       
      tau_w * dw/dt= a(V-E_L) -w  
       
      

**Parameters:**
::

     
      C_m     double - Capacity of the membrane in pF  
      t_ref   double - Duration of refractory period in ms.  
      V_reset   double - Reset value for V_m after a spike. In mV.  
      E_L  double - Leak reversal potential in mV.  
      g_L     double - Leak conductance in nS.  
      I_e    double - Constant external input current in pA.  
       
      Spike adaptation parameters:  
      a  double - Subthreshold adaptation in nS.  
      b   double - Spike-triggered adaptation in pA.  
      Delta_T   double    - Slope factor in mV  
      tau_w   double - Adaptation time constant in ms  
      V_th    double - Spike initiation threshold in mV  
      V_peak    double - Spike detection threshold in mV.  
       
      Synaptic parameters:  
      E_ex     double - Excitatory reversal potential in mV.  
      tau_syn_ex double    - Rise time of excitatory synaptic conductance in ms (alpha  
      function).  
      E_in  double - Inhibitory reversal potential in mV.  
      tau_syn_in double    - Rise time of the inhibitory synaptic conductance in ms  
      (alpha function).  
       
      Numerical integration parameters:  
      HMIN    double - Minimal stepsize for numerical integration in ms  
      (default 0.001ms).  
      MAXERR     double - Error estimate tolerance for adaptive stepsize control  
      (steps accepted if err<=MAXERR). In mV.  
      Note that the error refers to the difference between the  
      4th and 5th order RK terms. Default 1e-10 mV.  
       
      Authors: Stefan Bucher, Marc-Oliver Gewaltig.  
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

    Brette R and Gerstner W (2005) Adaptive Exponential  
      Integrate-and-Fire Model as an Effective Description of  
      Neuronal Activity. J Neurophysiol 94:3637-3642  
       
      

**SeeAlso:**

-  `iaf\_cond\_alpha <../cc/iaf_cond_alpha.html>`__
-  `aeif\_cond\_exp <../cc/aeif_cond_exp.html>`__
-  `aeif\_cond\_alpha <../cc/aeif_cond_alpha.html>`__

**Source:**
::

    ./aeif_cond_alpha_RK5.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
