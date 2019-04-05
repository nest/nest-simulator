iaf\_psc\_alpha
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_psc_alpha - Leaky integrate-and-fire neuron model.

**Description:**
::

     
       
      iaf_psc_alpha is an implementation of a leaky integrate-and-fire model  
      with alpha-function shaped synaptic currents. Thus, synaptic currents  
      and the resulting post-synaptic potentials have a finite rise time.  
       
      The threshold crossing is followed by an absolute refractory period  
      during which the membrane potential is clamped to the resting potential.  
       
      The linear subthresold dynamics is integrated by the Exact  
      Integration scheme [1]. The neuron dynamics is solved on the time  
      grid given by the computation step size. Incoming as well as emitted  
      spikes are forced to that grid.  
       
      An additional state variable and the corresponding differential  
      equation represents a piecewise constant external current.  
       
      The general framework for the consistent formulation of systems with  
      neuron like dynamics interacting by point events is described in  
      [1].  A flow chart can be found in [2].  
       
      Critical tests for the formulation of the neuron model are the  
      comparisons of simulation results for different computation step  
      sizes. sli/testsuite/nest contains a number of such tests.  
       
      The iaf_psc_alpha is the standard model used to check the consistency  
      of the nest simulation kernel because it is at the same time complex  
      enough to exhibit non-trivial dynamics and simple enough compute  
      relevant measures analytically.  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      E_L   double - Resting membrane potential in mV.  
      C_m  double - Capacity of the membrane in pF  
      tau_m   double - Membrane time constant in ms.  
      t_ref    double - Duration of refractory period in ms.  
      V_th  double - Spike threshold in mV.  
      V_reset   double   - Reset potential of the membrane in mV.  
      tau_syn_ex double  - Rise time of the excitatory synaptic alpha function in ms.  
      tau_syn_in double  - Rise time of the inhibitory synaptic alpha function in ms.  
      I_e     double - Constant external input current in pA.  
      V_min   double - Absolute lower value for the membrane potential.  
       
      

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

     
      [1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear  
      systems with applications to neuronal modeling. Biologial Cybernetics  
      81:381-402.  
      [2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space  
      analysis of synchronous spiking in cortical neural networks.  
      Neurocomputing 38-40:565-571.  
      [3] Morrison A, Straube S, Plesser H E, & Diesmann M (2006) Exact subthreshold  
      integration with continuous spike times in discrete time neural network  
      simulations. Neural Computation, in press  
       
      

**FirstVersion:**
::

    September 1999  
      Author:  Diesmann, Gewaltig  
      

**SeeAlso:**

-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_cond\_exp <../cc/iaf_cond_exp.html>`__

**Source:**
::

    ./iaf_psc_alpha.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
