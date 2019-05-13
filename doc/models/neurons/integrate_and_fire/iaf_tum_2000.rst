iaf\_tum\_2000
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_tum_2000 - Leaky integrate-and-fire neuron model with exponential  
      PSCs.

**Description:**
::

     
       
      iaf_tum_2000 is an implementation of a leaky integrate-and-fire model  
      with exponential shaped postsynaptic currents (PSCs) according to [1].  
      The postsynaptic currents have an infinitely short rise time.  
      In particular, this model allows setting an absolute and relative  
      refractory time separately, as required by [1].  
       
      The threshold crossing is followed by an absolute refractory period (tau_abs)  
      during which the membrane potential is clamped to the resting potential.  
      During the total refractory period, the membrane potential evolves,  
      but the neuron will not emit a spike, even if the membrane potential  
      reaches threshold. The total refractory time must be larger or equal to  
      the absolute refractory time. If equal, the refractoriness of the model  
      if equivalent to the other models of NEST.  
       
      The linear subthreshold dynamics is integrated by the Exact  
      Integration scheme [2]. The neuron dynamics is solved on the time  
      grid given by the computation step size. Incoming as well as emitted  
      spikes are forced to that grid.  
       
      An additional state variable and the corresponding differential  
      equation represents a piecewise constant external current.  
       
      The general framework for the consistent formulation of systems with  
      neuron like dynamics interacting by point events is described in  
      [2]. A flow chart can be found in [3].  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      E_L   double - Resting membrane potential in mV.  
      C_m  double - Capacity of the membrane in pF  
      tau_m   double - Membrane time constant in ms.  
      tau_syn_ex   double - Time constant of postsynaptic excitatory currents in ms  
      tau_syn_in   double   - Time constant of postsynaptic inhibitory currents in ms  
      t_ref_abs   double    - Duration of absolute refractory period (V_m = V_reset)  
      in ms.  
      t_ref_tot   double  - Duration of total refractory period (no spiking) in ms.  
      V_m    double - Membrane potential in mV  
      V_th  double - Spike threshold in mV.  
      V_reset     double - Reset membrane potential after a spike in mV.  
      I_e  double - Constant input current in pA.  
      t_spike  double - Point in time of last spike in ms.  
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      If tau_m is very close to tau_syn_ex or tau_syn_in, the model  
      will numerically behave as if tau_m is equal to tau_syn_ex or  
      tau_syn_in, respectively, to avoid numerical instabilities.  
      For details, please see IAF_Neruons_Singularity.ipynb in  
      the NEST source code (docs/model_details).  
       
      

**References:**
::

     
      [1] Misha Tsodyks, Asher Uziel, and Henry Markram (2000) Synchrony Generation  
      in Recurrent Networks with Frequency-Dependent Synapses, The Journal of  
      Neuroscience, 2000, Vol. 20 RC50 p. 1-5  
      [2] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear  
      systems with applications to neuronal modeling. Biologial Cybernetics  
      81:381-402.  
      [3] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space  
      analysis of synchronous spiking in cortical neural networks.  
      Neurocomputing 38-40:565-571.  
       
      

**Author:**
::

    Moritz Helias 

**FirstVersion:**
::

    March 2006  
      

**Source:**
::

    ./iaf_tum_2000.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
