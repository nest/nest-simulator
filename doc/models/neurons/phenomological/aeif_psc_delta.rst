aeif\_psc\_delta
=========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    aeif_psc_delta - Current-based adaptive exponential integrate-and-fire  
      neuron model according to Brette and Gerstner (2005) with delta synapse.

**Description:**
::

     
       
      aeif_psc_delta is the adaptive exponential integrate and fire neuron  
      according to Brette and Gerstner (2005), with post-synaptic currents  
      in the form of delta spikes.  
       
      This implementation uses the embedded 4th order Runge-Kutta-Fehlberg  
      solver with adaptive stepsize to integrate the differential equation.  
       
      The membrane potential is given by the following differential equation:  
      C dV/dt= -g_L(V-E_L)+g_L*Delta_T*exp((V-V_T)/Delta_T)+I(t)+I_e  
       
      and  
       
      tau_w * dw/dt= a(V-E_L) -W  
       
      I(t) = J Sum_k delta(t - t^k).  
       
      Here delta is the dirac delta function and k indexes incoming  
      spikes. This is implemented such that V_m will be incremented/decremented by  
      the value of J after a spike.  
       
      

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
       
      Integration parameters  
      gsl_error_tol  double - This parameter controls the admissible error of the  
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

    Mikkel Elle Lepperød adapted from aeif_psc_exp and iaf_psc_delta  
       
      

**SeeAlso:**

-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `aeif\_cond\_exp <../cc/aeif_cond_exp.html>`__
-  `aeif\_psc\_exp <../cc/aeif_psc_exp.html>`__

**Source:**
::

    ./aeif_psc_delta.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
