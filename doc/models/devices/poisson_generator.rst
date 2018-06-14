poisson\_generator
===========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    poisson_generator - simulate neuron firing with Poisson processes  
      statistics.

**Description:**
::

     
      The poisson_generator simulates a neuron that is firing with Poisson  
      statistics, i.e. exponentially distributed interspike intervals. It will  
      generate a _unique_ spike train for each of it's targets. If you do not want  
      this behavior and need the same spike train for all targets, you have to use a  
      parrot neuron inbetween the poisson generator and the targets.  
       
      

**Parameters:**
::

     
      The following parameters appear in the element's status dictionary:  
       
      rate    double - mean firing rate in Hz  
      origin   double    - Time origin for device timer in ms  
      start   double - begin of device application with resp. to origin in ms  
      stop    double - end of device application with resp. to origin in ms  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      A Poisson generator may, especially at high rates, emit more than one  
      spike during a single time step. If this happens, the generator does  
      not actually send out n spikes. Instead, it emits a single spike with  
      n-fold synaptic weight for the sake of efficiency.  
       
      The design decision to implement the Poisson generator as a device  
      which sends spikes to all connected nodes on every time step and then  
      discards the spikes that should not have happened generating random  
      numbers at the recipient side via an event hook is twofold.  
       
      On one hand, it leads to the saturation of the messaging network with  
      an enormous amount of spikes, most of which will never get delivered  
      and should not have been generated in the first place.  
       
      On the other hand, a proper implementation of the Poisson generator  
      needs to provide two basic features: (a) generated spike trains  
      should be IID processes w.r.t. target neurons to which the generator  
      is connected and (b) as long as virtual_num_proc is constant, each  
      neuron should receive an identical Poisson spike train in order to  
      guarantee reproducibility of the simulations across varying machine  
      numbers.  
       
      Therefore, first, as Network::get_network().send sends spikes to all the  
      recipients, differentiation has to happen in the hook, second, the  
      hook can use the RNG from the thread where the recipient neuron sits,  
      which explains the current design of the generator. For details,  
      refer to:  
       
      http://ken.brainworks.uni-freiburg.de/cgi-bin/mailman/private/nest_developer/2011-January/002977.html  
       
      

**SeeAlso:**

-  `poisson\_generator\_ps <../cc/poisson_generator_ps.html>`__
-  `Device <../cc/Device.html>`__
-  `parrot\_neuron <../cc/parrot_neuron.html>`__

**Source:**
::

    ./poisson_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
