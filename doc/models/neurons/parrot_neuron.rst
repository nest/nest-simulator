parrot\_neuron
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    parrot_neuron - Neuron that repeats incoming spikes.

**Description:**
::

     
       
      The parrot neuron simply emits one spike for every incoming spike.  
      An important application is to provide identical poisson spike  
      trains to a group of neurons. The poisson_generator sends a different  
      spike train to each of its target neurons. By connecting one  
      poisson_generator to a parrot_neuron and then that parrot_neuron to  
      a group of neurons, all target neurons will receive the same poisson  
      spike train.  
       
      

**Parameters:**
::

     
      No parameters to be set in the status dictionary.  
       
      

**Receives:**
::

    SpikeEvent  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
       
         - Weights on connection to the parrot_neuron are ignored.  
        - Weights on connections from the parrot_neuron are handled as usual.  
        - Delays are honored on incoming and outgoing connections.  
       
      Only spikes arriving on connections to port 0 will be repeated.  
      Connections onto port 1 will be accepted, but spikes incoming  
      through port 1 will be ignored. This allows setting exact pre-  
      and post-synaptic spike times for STDP protocols by connecting  
      two parrot neurons spiking at desired times by, e.g., a  
      stdp_synapse onto port 1 on the post-synaptic parrot neuron.  
       
      

**Author:**
::

    David Reichert, Abigail Morrison, Alexander Seeholzer, Hans Ekkehard  
      Plesser  
      

**FirstVersion:**
::

    May 2006 

**Source:**
::

    ./parrot_neuron.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
