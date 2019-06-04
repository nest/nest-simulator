iaf\_chxk\_2008
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_chxk_2008 - Conductance based leaky integrate-and-fire neuron model  
      used in Casti et al 2008.

**Description:**
::

     
      iaf_chxk_2008 is an implementation of a spiking neuron using IAF dynamics with  
      conductance-based synapses [1]. It is modeled after iaf_cond_alpha with the  
      addition of after hyper-polarization current instead of a membrane potential  
      reset. Incoming spike events induce a post-synaptic change of conductance  
      modeled by an alpha function. The alpha function is normalized such that an  
      event of weight 1.0 results in a peak current of 1 nS at t = tau_syn.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      E_L   double - Leak reversal potential in mV.  
      C_m     double - Capacity of the membrane in pF  
      V_th    double - Spike threshold in mV.  
      E_ex    double - Excitatory reversal potential in mV.  
      E_in  double - Inhibitory reversal potential in mV.  
      g_L   double - Leak conductance in nS.  
      tau_ex     double - Rise time of the excitatory synaptic alpha function in ms.  
      tau_in  double - Rise time of the inhibitory synaptic alpha function in ms.  
      I_e     double - Constant input current in pA.  
      tau_ahp   double    - Afterhyperpolarization (AHP) time constant in ms.  
      E_ahp    double - AHP potential in mV.  
      g_ahp     double - AHP conductance in nS.  
      ahp_bug   bool     - Defaults to false. If true, behaves like original  
      model implementation.  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    SpikeEvent, CurrentEvent  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] Casti A, Hayot F, Xiao Y, and Kaplan E (2008) A simple model of retina-LGN  
      transmission. J Comput Neurosci 24:235-252.  
       
      

**Author:**
::

    Heiberg  
       
      

**SeeAlso:**

-  `iaf\_cond\_alpha <../cc/iaf_cond_alpha.html>`__

**Source:**
::

    ./iaf_chxk_2008.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
