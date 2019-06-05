iaf\_psc\_delta
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_psc_delta - Leaky integrate-and-fire neuron model.

**Description:**
::

     
       
      iaf_psc_delta is an implementation of a leaky integrate-and-fire model  
      where the potential jumps on each spike arrival.  
       
      The threshold crossing is followed by an absolute refractory period  
      during which the membrane potential is clamped to the resting potential.  
       
      Spikes arriving while the neuron is refractory, are discarded by  
      default. If the property "refractory_input" is set to true, such  
      spikes are added to the membrane potential at the end of the  
      refractory period, dampened according to the interval between  
      arrival and end of refractoriness.  
       
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
       
      The iaf_psc_delta is the standard model used to check the consistency  
      of the nest simulation kernel because it is at the same time complex  
      enough to exhibit non-trivial dynamics and simple enough compute  
      relevant measures analytically.  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      E_L   double - Resting membrane potential in mV.  
      C_m  double - Capacitance of the membrane in pF  
      tau_m    double - Membrane time constant in ms.  
      t_ref    double - Duration of refractory period in ms.  
      V_th  double - Spike threshold in mV.  
      V_reset   double   - Reset potential of the membrane in mV.  
      I_e     double - Constant input current in pA.  
      V_min    double - Absolute lower value for the membrane potential in mV  
       
      refractory_input bool   - If true, do not discard input during  
      refractory period. Default: false.  
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      Author:  September 1999, Diesmann, Gewaltig  
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
       
      The present implementation uses individual variables for the  
      components of the state vector and the non-zero matrix elements of  
      the propagator.  Because the propagator is a lower triangular matrix  
      no full matrix multiplication needs to be carried out and the  
      computation can be done "in place" i.e. no temporary state vector  
      object is required.  
       
      The template support of recent C++ compilers enables a more succinct  
      formulation without loss of runtime performance already at minimal  
      optimization levels. A future version of iaf_psc_delta will probably  
      address the problem of efficient usage of appropriate vector and  
      matrix objects.  
       
       
      

**References:**
::

     
      [1] Rotter S & Diesmann M (1999) Exact digital simulation of time-invariant  
      linear systems with applications to neuronal modeling. Biologial Cybernetics  
      81:381-402.  
      [2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space  
      analysis of synchronous spiking in cortical neural networks.  
      Neurocomputing 38-40:565-571.  
       
      

**SeeAlso:**

-  `iaf\_psc\_alpha <../cc/iaf_psc_alpha.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_psc\_delta\_canon <../cc/iaf_psc_delta_canon.html>`__

**Source:**
::

    ./iaf_psc_delta.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
