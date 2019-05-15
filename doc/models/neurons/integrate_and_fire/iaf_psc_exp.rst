iaf\_psc\_exp
======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_psc_exp - Leaky integrate-and-fire neuron model with exponential  
      PSCs.

**Description:**
::

     
      iaf_psc_expp is an implementation of a leaky integrate-and-fire model  
      with exponential shaped postsynaptic currents (PSCs) according to [1].  
      Thus, postsynaptic currents have an infinitely short rise time.  
       
      The threshold crossing is followed by an absolute refractory period (t_ref)  
      during which the membrane potential is clamped to the resting potential  
      and spiking is prohibited.  
       
      The linear subthresold dynamics is integrated by the Exact  
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
      t_ref  double - Duration of refractory period (V_m = V_reset) in ms.  
      V_m   double - Membrane potential in mV  
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
      For details, please see IAF_Neruons_Singularity.ipynb in the  
      NEST source code (docs/model_details).  
       
      iaf_psc_exp can handle current input in two ways: Current input  
      through receptor_type 0 are handled as stepwise constant current  
      input as in other iaf models, i.e., this current directly enters  
      the membrane potential equation. Current input through  
      receptor_type 1, in contrast, is filtered through an exponential  
      kernel with the time constant of the excitatory synapse,  
      tau_syn_ex. For an example application, see [4].  
       
      

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
      [4] Schuecker J, Diesmann M, Helias M (2015) Modulated escape from a  
      metastable state driven by colored noise.  
      Physical Review E 92:052119  
       
      

**Author:**
::

    Moritz Helias 

**FirstVersion:**
::

    March 2006  
      

**SeeAlso:**

-  `iaf\_psc\_exp\_ps <../cc/iaf_psc_exp_ps.html>`__

**Source:**
::

    ./iaf_psc_exp.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
