Command: tsodyks\_synapse
=========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    tsodyks_synapse - Synapse type with short term plasticity.

**Description:**
::

     
      This synapse model implements synaptic short-term depression and short-term  
      facilitation according to [1]. In particular it solves Eqs (3) and (4) from  
      this paper in an exact manner.  
       
      Synaptic depression is motivated by depletion of vesicles in the readily  
      releasable pool of synaptic vesicles (variable x in equation (3)). Synaptic  
      facilitation comes about by a presynaptic increase of release probability,  
      which is modeled by variable U in Eq (4).  
      The original interpretation of variable y is the amount of glutamate  
      concentration in the synaptic cleft. In [1] this variable is taken to be  
      directly proportional to the synaptic current caused in the postsynaptic  
      neuron (with the synaptic weight w as a proportionality constant). In order  
      to reproduce the results of [1] and to use this model of synaptic plasticity  
      in its original sense, the user therefore has to ensure the following  
      conditions:  
       
      1.) The postsynaptic neuron must be of type iaf_psc_exp or iaf_tum_2000,  
      because these neuron models have a postsynaptic current which decays  
      exponentially.  
       
      2.) The time constant of each tsodyks_synapse targeting a particular neuron  
      must be chosen equal to that neuron's synaptic time constant. In particular  
      that means that all synapses targeting a particular neuron have the same  
      parameter tau_psc.  
       
      However, there are no technical restrictions using this model of synaptic  
      plasticity also in conjunction with neuron models that have a different  
      dynamics for their synaptic current or conductance. The effective synaptic  
      weight, which will be transmitted to the postsynaptic neuron upon occurrence  
      of a spike at time t is u(t)*x(t)*w, where u(t) and x(t) are defined in  
      Eq (3) and (4), w is the synaptic weight specified upon connection.  
      The interpretation is as follows: The quantity u(t)*x(t) is the release  
      probability times the amount of releasable synaptic vesicles at time t of the  
      presynaptic neuron's spike, so this equals the amount of transmitter expelled  
      into the synaptic cleft.  
      The amount of transmitter than relaxes back to 0 with time constant tau_psc  
      of the synapse's variable y. Since the dynamics of y(t) is linear, the  
      postsynaptic neuron can reconstruct from the amplitude of the synaptic  
      impulse u(t)*x(t)*w the full shape of y(t). The postsynaptic neuron, however,  
      might choose to have a synaptic current that is not necessarily identical to  
      the concentration of transmitter y(t) in the synaptic cleft. It may realize  
      an arbitrary postsynaptic effect depending on y(t).  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      U     double - maximum probability of release [0,1]  
      tau_psc   double - time constant of synaptic current in ms  
      tau_fac   double  - time constant for facilitation in ms  
      tau_rec   double - time constant for depression in ms  
      x   double - initial fraction of synaptic vesicles in the readily  
      releasable pool [0,1]  
      y   double - initial fraction of synaptic vesicles in the synaptic  
      cleft [0,1]  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] Tsodyks, Uziel, Markram (2000) Synchrony Generation in Recurrent Networks  
      with Frequency-Dependent Synapses. Journal of Neuroscience, vol 20 RC50  
       
      

**Author:**
::

    Moritz Helias  
      

**FirstVersion:**
::

    March 2006  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_tum\_2000 <../cc/iaf_tum_2000.html>`__

**Source:**
::

    ./tsodyks_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
