aeif\_cond\_exp
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    aeif_cond_exp - Conductance based exponential integrate-and-fire neuron  
      model according to Brette and Gerstner (2005).

**Description:**
::

     
       
      aeif_cond_exp is the adaptive exponential integrate and fire neuron  
      according to Brette and Gerstner (2005), with post-synaptic  
      conductances in the form of truncated exponentials.  
       
      This implementation uses the embedded 4th order Runge-Kutta-Fehlberg  
      solver with adaptive stepsize to integrate the differential equation.  
       
      The membrane potential is given by the following differential equation:  
      C dV/dt= -g_L(V-E_L)+g_L*Delta_T*exp((V-V_T)/Delta_T)-g_e(t)(V-E_e)  
      -g_i(t)(V-E_i)-w +I_e  
       
      and  
       
      tau_w * dw/dt= a(V-E_L) -W  
       
       
      Note that the spike detection threshold V_peak is automatically set to  
      V_th+10 mV to avoid numerical instabilites that may result from  
      setting V_peak too high.  
       
      

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
      V_t     double - Spike initiation threshold in mV  
      V_peak    double - Spike detection threshold in mV.  
       
      Synaptic parameters  
      E_ex  double - Excitatory reversal potential in mV.  
      tau_syn_ex double    - Rise time of excitatory synaptic conductance in ms (exp  
      function).  
      E_in    double - Inhibitory reversal potential in mV.  
      tau_syn_in double    - Rise time of the inhibitory synaptic conductance in ms  
      (exp function).  
       
      Integration parameters  
      gsl_error_tol  double   - This parameter controls the admissible error of the  
      GSL integrator. Reduce it if NEST complains about  
      numerical instabilities.  
       
      

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

    Brette R and Gerstner W (2005) Adaptive Exponential  
      Integrate-and-Fire Model as an Effective Description of  
      Neuronal Activity. J Neurophysiol 94:3637-3642  
       
      

**Author:**
::

    Adapted from aeif_cond_alpha by Lyle Muller; full revision by Tanguy  
      Fardet on December 2016  
       
      

**SeeAlso:**

-  `iaf\_cond\_exp <../cc/iaf_cond_exp.html>`__
-  `aeif\_cond\_alpha <../cc/aeif_cond_alpha.html>`__

**Source:**
::

    ./aeif_cond_exp.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
