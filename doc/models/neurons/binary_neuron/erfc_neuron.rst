Command: erfc\_neuron
=====================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    erfc_neuron - Binary stochastic neuron with complementary error  
      function as activation function.

**Description:**
::

     
      The erfc_neuron is an implementation of a binary neuron that  
      is irregularly updated at Poisson time points. At each update  
      point the total synaptic input h into the neuron is summed up,  
      passed through a gain function g whose output is interpreted as  
      the probability of the neuron to be in the active (1) state.  
       
      The gain function g used here is  
       
      g(h) = 0.5 * erfc (( h - theta_ ) / ( sqrt( 2. ) * sigma)).  
       
      This corresponds to a McCulloch-Pitts neuron receiving additional  
      Gaussian noise with mean 0 and standard deviation sigma.  
      The time constant tau_m is defined as the mean of the  
      inter-update-interval that is drawn from an exponential  
      distribution with this parameter. Using this neuron to reproduce  
      simulations with asynchronous update (similar to [1,2]), the time  
      constant needs to be chosen as tau_m = dt*N, where dt is the simulation time  
      step and N the number of neurons in the original simulation with  
      asynchronous update. This ensures that a neuron is updated on  
      average every tau_m ms. Since in the original papers [1,2] neurons  
      are coupled with zero delay, this implementation follows that  
      definition. It uses the update scheme described in [3] to  
      maintain causality: The incoming events in time step t_i are  
      taken into account at the beginning of the time step to calculate  
      the gain function and to decide upon a transition.  In order to  
      obtain delayed coupling with delay d, the user has to specify the  
      delay d+h upon connection, where h is the simulation time step.  
       
      

**Parameters:**
::

     
      tau_m   double - Membrane time constant (mean inter-update-interval) (ms)  
      theta     double - threshold for sigmoidal activation function (mV)  
      sigma     double - 1/sqrt(2pi) x inverse of maximal slope (mV)  
       
      

**Receives:**
::

    SpikeEvent, PotentialRequest  
      

**Sends:**
::

    SpikeEvent  
      

**Remarks:**
::

     
      This neuron has a special use for spike events to convey the  
      binary state of the neuron to the target. The neuron model  
      only sends a spike if a transition of its state occurs. If the  
      state makes an up-transition it sends a spike with multiplicity 2,  
      if a down transition occurs, it sends a spike with multiplicity 1.  
      The decoding scheme relies on the feature that spikes with multiplicity  
      larger 1 are delivered consecutively, also in a parallel setting.  
      The creation of double connections between binary neurons will  
      destroy the decoding scheme, as this effectively duplicates  
      every event. Using random connection routines it is therefore  
      advisable to set the property 'multapses' to false.  
      The neuron accepts several sources of currents, e.g. from a  
      noise_generator.  
       
       
      

**References:**
::

     
      [1] Iris Ginzburg, Haim Sompolinsky. Theory of correlations in stochastic  
      neural networks (1994). PRE 50(4) p. 3171  
      [2] W. McCulloch und W. Pitts (1943). A logical calculus of the ideas  
      immanent in nervous activity. Bulletin of Mathematical Biophysics, 5:115-133.  
      [3] Abigail Morrison, Markus Diesmann. Maintaining Causality in Discrete Time  
      Neuronal Simulations. In: Lectures in Supercomputational Neuroscience,  
      p. 267. Peter beim Graben, Changsong Zhou, Marco Thiel, Juergen Kurths  
      (Eds.), Springer 2008.  
       
      

**FirstVersion:**
::

    May 2016  
      Authors: Jakob Jordan, Tobias Kuehn  
      

**SeeAlso:**

-  `mcculloch\_pitts\_neuron <../cc/mcculloch_pitts_neuron.html>`__
-  `ginzburg\_neuron <../cc/ginzburg_neuron.html>`__

**Source:**
::

    ./erfc_neuron.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
