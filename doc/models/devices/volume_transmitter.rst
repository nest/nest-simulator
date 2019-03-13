volume\_transmitter
============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    volume_transmitter - Node used in combination with neuromodulated synaptic  
      plasticity. It collects all spikes emitted by the population of neurons  
      connected to the volume transmitter and transmits the signal to a user-specific  
      subset of synapses.

**Examples:**
::

     
      /volume_transmitter Create /vol Set  
      /iaf_psc_alpha Create /pre_neuron Set  
      /iaf_psc_alpha Create /post_neuron Set  
      /iaf_psc_alpha Create /neuromod_neuron Set  
      /stdp_dopamine_synapse  << /vt vol >>  SetDefaults  
      neuromod_neuron vol Connect  
      pre_neuron post_neuron /stdp_dopamine_synapse Connect  
       
      

**Description:**
::

     
      The volume transmitter is used in combination with neuromodulated  
      synaptic plasticty, plasticity that depends not only on the activity  
      of the pre- and the postsynaptic neuron but also on a non-local  
      neuromodulatory third signal. It collects the spikes from all neurons  
      connected to the volume transmitter and delivers the spikes to a  
      user-specific subset of synapses.  It is assumed that the  
      neuromodulatory signal is a function of the spike times of all spikes  
      emitted by the population of neurons connected to the volume  
      transmitter.  The neuromodulatory dynamics is calculated in the  
      synapses itself. The volume transmitter interacts in a hybrid  
      structure with the neuromodulated synapses. In addition to the  
      delivery of the neuromodulatory spikes triggered by every pre-synaptic  
      spike, the neuromodulatory spike history is delivered in discrete time  
      intervals of a manifold of the minimal synaptic delay. In order to  
      insure the link between the neuromodulatory synapses and the volume  
      transmitter, the volume transmitter is passed as a parameter when a  
      neuromodulatory synapse is defined. The implementation is based on the  
      framework presented in [1].  
       
      

**Parameters:**
::

     
      deliver_interval   - time interval given in d_min time steps, in which  
      the volume signal is delivered from the volume  
      transmitter to the assigned synapses  
       
      

**Receives:**
::

    SpikeEvent  
       
      

**Remarks:**
::

    major changes to update function after code revision in Apr 2013 (SK)  
      

**References:**
::

     
      [1] Potjans W, Morrison A and Diesmann M (2010). Enabling functional  
      neural circuit simulations with distributed computing of  
      neuromodulated plasticity.  
      Front. Comput. Neurosci. 4:141. doi:10.3389/fncom.2010.00141  
       
      

**Author:**
::

    Wiebke Potjans, Abigail Morrison  
      

**SeeAlso:**

-  `stdp\_dopamine\_synapse <../cc/stdp_dopamine_synapse.html>`__

**Source:**
::

    ./volume_transmitter.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
