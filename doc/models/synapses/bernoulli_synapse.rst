bernoulli\_synapse
===========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    bernoulli_synapse - Static synapse with stochastic transmission.

**Description:**
::

     
      Spikes are transmitted by bernoulli_synapse following a Bernoulli trial with  
      success probability p_transmit. This synaptic mechanism was inspired by the  
      results described in [1] of greater transmission probability for stronger  
      excitatory connections and it was previously applied in [2] and [3].  
       
      bernoulli_synapse does not support any kind of plasticity. It simply stores  
      the parameters target, weight, transmission probability, delay and  
      receiver port for each connection.  
       
      

**Parameters:**
::

     
      p_transmit double  - Transmission probability, must be between 0 and 1  
       
      

**Transmits:**
::

    SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,  
      DoubleDataEvent, DataLoggingRequest  
       
      

**References:**
::

     
       
      [1] Sandrine Lefort, Christian Tomm, J.-C. Floyd Sarria, Carl C.H. Petersen,  
      The Excitatory Neuronal Network of the C2 Barrel Column in Mouse Primary  
      Somatosensory Cortex, Neuron, Volume 61, Issue 2, 29 January 2009, Pages  
      301-316, DOI: 10.1016/j.neuron.2008.12.020.  
       
      [2] Jun-nosuke Teramae, Yasuhiro Tsubo & Tomoki Fukai, Optimal spike-based  
      communication in excitable networks with strong-sparse and weak-dense links,  
      Scientific Reports 2, Article number: 485 (2012), DOI: 10.1038/srep00485  
       
      [3] Yoshiyuki Omura, Milena M. Carvalho, Kaoru Inokuchi, Tomoki Fukai, A  
      Lognormal Recurrent Network Model for Burst Generation during Hippocampal  
      Sharp Waves, Journal of Neuroscience 28 October 2015, 35 (43) 14585-14601,  
      DOI: 10.1523/JNEUROSCI.4944-14.2015 

**Author:**
::

    Susanne Kunkel, Maximilian Schmidt, Milena Menezes Carvalho  
       
      

**FirstVersion:**
::

    June 2017  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__
-  `static\_synapse\_hom\_w <../cc/static_synapse_hom_w.html>`__

**Source:**
::

    ./bernoulli_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
