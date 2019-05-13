mat2\_psc\_exp
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    mat2_psc_exp - Non-resetting leaky integrate-and-fire neuron model with  
      exponential PSCs and adaptive threshold.

**Description:**
::

     
      mat2_psc_exp is an implementation of a leaky integrate-and-fire model  
      with exponential shaped postsynaptic currents (PSCs). Thus, postsynaptic  
      currents have an infinitely short rise time.  
       
      The threshold is lifted when the neuron is fired and then decreases in a  
      fixed time scale toward a fixed level [3].  
       
      The threshold crossing is followed by a total refractory period  
      during which the neuron is not allowed to fire, even if the membrane  
      potential exceeds the threshold. The membrane potential is NOT reset,  
      but continuously integrated.  
       
      The linear subthresold dynamics is integrated by the Exact  
      Integration scheme [1]. The neuron dynamics is solved on the time  
      grid given by the computation step size. Incoming as well as emitted  
      spikes are forced to that grid.  
       
      An additional state variable and the corresponding differential  
      equation represents a piecewise constant external current.  
       
      The general framework for the consistent formulation of systems with  
      neuron like dynamics interacting by point events is described in  
      [1]. A flow chart can be found in [2].  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
       
      C_m   double - Capacity of the membrane in pF  
      E_L     double - Resting potential in mV  
      tau_m  double - Membrane time constant in ms  
      tau_syn_ex   double  - Time constant of postsynaptic excitatory currents in ms  
      tau_syn_in   double   - Time constant of postsynaptic inhibitory currents in ms  
      t_ref  double - Duration of absolute refractory period (no spiking)  
      in ms  
      V_m  double - Membrane potential in mV  
      I_e   double - Constant input current in pA  
      t_spike   double - Point in time of last spike in ms  
      tau_1    double - Short time constant of adaptive threshold in ms  
      tau_2  double - Long time constant of adaptive threshold in ms  
      alpha_1     double - Amplitude of short time threshold adaption in mV [3]  
      alpha_2   double - Amplitude of long time threshold adaption in mV [3]  
      omega  double - Resting spike threshold in mV (absolute value, not  
      relative to E_L as in [3])  
       
      The following state variables can be read out with the multimeter device:  
       
      V_m    Non-resetting membrane potential  
      V_th   Two-timescale adaptive threshold  
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      tau_m != tau_syn_{ex,in} is required by the current implementation to avoid a  
      degenerate case of the ODE describing the model [1]. For very similar values,  
      numerics will be unstable.  
       
      

**References:**
::

     
      [1] Rotter S & Diesmann M (1999) Exact simulation of  
      time-invariant linear systems with applications to neuronal  
      modeling. Biologial Cybernetics 81:381-402.  
      [2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State  
      space analysis of synchronous spiking in cortical neural  
      networks. Neurocomputing 38-40:565-571.  
      [3] Kobayashi R, Tsubo Y and Shinomoto S (2009) Made-to-order  
      spiking neuron model equipped with a multi-timescale adaptive  
      threshold. Front. Comput. Neurosci. 3:9. doi:10.3389/neuro.10.009.2009  
       
      

**Author:**
::

    Thomas Pfeil (modified iaf_psc_exp model of Moritz Helias) 

**FirstVersion:**
::

    Mai 2009  
      

**Source:**
::

    ./mat2_psc_exp.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
