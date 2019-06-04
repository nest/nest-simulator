aeif\_cond\_alpha\_multisynapse
========================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    aeif_cond_alpha_multisynapse - Conductance based adaptive exponential  
      integrate-and-fire neuron model according  
      to Brette and Gerstner (2005) with  
      multiple synaptic rise time and decay  
      time constants, and synaptic conductance  
      modeled by an alpha function.

**Examples:**
::

     
       
      import nest  
      import numpy as np  
       
      neuron = nest.Create('aeif_cond_alpha_multisynapse')  
      nest.SetStatus(neuron, {"V_peak": 0.0, "a": 4.0, "b":80.5})  
      nest.SetStatus(neuron, {'E_rev':[0.0, 0.0, 0.0, -85.0],  
      'tau_syn':[1.0, 5.0, 10.0, 8.0]})  
       
      spike = nest.Create('spike_generator', params = {'spike_times':  
      np.array([10.0])})  
       
      voltmeter = nest.Create('voltmeter', 1, {'withgid': True})  
       
      delays=[1.0, 300.0, 500.0, 700.0]  
      w=[1.0, 1.0, 1.0, 1.0]  
      for syn in range(4):  
      nest.Connect(spike, neuron, syn_spec={'model': 'static_synapse',  
      'receptor_type': 1 + syn,  
      'weight': w[syn],  
      'delay': delays[syn]})  
       
      nest.Connect(voltmeter, neuron)  
       
      nest.Simulate(1000.0)  
      dmm = nest.GetStatus(voltmeter)[0]  
      Vms = dmm["events"]["V_m"]  
      ts = dmm["events"]["times"]  
      import pylab  
      pylab.figure(2)  
      pylab.plot(ts, Vms)  
      pylab.show()  
       
      

**Description:**
::

     
       
      aeif_cond_alpha_multisynapse is a conductance-based adaptive exponential  
      integrate-and-fire neuron model. It allows an arbitrary number of synaptic  
      time constants. Synaptic conductance is modeled by an alpha function, as  
      described by A. Roth and M.C.W. van Rossum in Computational Modeling Methods  
      for Neuroscientists, MIT Press 2013, Chapter 6.  
       
      The time constants are supplied by an array, "tau_syn", and the pertaining  
      synaptic reversal potentials are supplied by the array "E_rev". Port numbers  
      are automatically assigned in the range from 1 to n_receptors.  
      During connection, the ports are selected with the property "receptor_type".  
       
      The membrane potential is given by the following differential equation:  
       
      C dV/dt = -g_L(V-E_L) + g_L*Delta_T*exp((V-V_T)/Delta_T) + I_syn_tot(V, t)  
        - w + I_e  
       
      where  
       
      I_syn_tot(V,t) = \sum_i g_i(t) (V   - E_{rev,i}) ,  
       
      the synapse i is excitatory or inhibitory depending on the value of E_{rev,i}  
      and the differential equation for the spike-adaptation current w is:  
       
      tau_w * dw/dt = a(V   - E_L)  - w  
       
      When the neuron fires a spike, the adaptation current w <- w + b.  
       
      

**Parameters:**
::

     
      C_m     double - Capacity of the membrane in pF  
      t_ref   double - Duration of refractory period in ms.  
      V_reset   double - Reset value for V_m after a spike. In mV.  
      E_L  double - Leak reversal potential in mV.  
      g_L     double - Leak conductance in nS.  
      I_e    double - Constant external input current in pA.  
      Delta_T   double   - Slope factor in mV  
      V_th    double - Spike initiation threshold in mV  
      V_peak    double - Spike detection threshold in mV.  
       
      Adaptation parameters:  
      a  double - Subthreshold adaptation in nS.  
      b   double - Spike-triggered adaptation in pA.  
      tau_w    double - Adaptation time constant in ms  
       
      Synaptic parameters  
      E_rev   double vector  - Reversal potential in mV.  
      tau_syn   double vector - Time constant of synaptic conductance in ms  
       
      Integration parameters  
      gsl_error_tol  double  - This parameter controls the admissible error of the  
      GSL integrator. Reduce it if NEST complains about  
      numerical instabilities.  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      author: Hans Ekkehard Plesser, based on aeif_cond_beta_multisynapse  
      

**Sends:**
::

    SpikeEvent  
       
      

**SeeAlso:**

-  `aeif\_cond\_alpha\_multisynapse <../cc/aeif_cond_alpha_multisynapse.html>`__

**Source:**
::

    ./aeif_cond_alpha_multisynapse.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
