ginzburg\_neuron
=========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    ginzburg_neuron - Binary stochastic neuron with sigmoidal activation  
      function.

**Description:**
::

     
      The ginzburg_neuron is an implementation of a binary neuron that  
      is irregularly updated as Poisson time points. At each update  
      point the total synaptic input h into the neuron is summed up,  
      passed through a gain function g whose output is interpreted as  
      the probability of the neuron to be in the active (1) state.  
       
      The gain function g used here is g(h) = c1*h + c2 * 0.5*(1 +  
      tanh(c3*(h-theta))) (output clipped to [0,1]). This allows to  
      obtain affin-linear (c1!=0, c2!=0, c3=0) or sigmoidal (c1=0,  
      c2=1, c3!=0) shaped gain functions.  The latter choice  
      corresponds to the definition in [1], giving the name to this  
      neuron model.  
      The choice c1=0, c2=1, c3=beta/2 corresponds to the Glauber  
      dynamics [2], g(h) = 1 / (1 + exp(-beta (h-theta))).  
      The time constant tau_m is defined as the mean  
      inter-update-interval that is drawn from an exponential  
      distribution with this parameter. Using this neuron to reprodce  
      simulations with asynchronous update [1], the time constant needs  
      to be chosen as tau_m = dt*N, where dt is the simulation time  
      step and N the number of neurons in the original simulation with  
      asynchronous update. This ensures that a neuron is updated on  
      average every tau_m ms. Since in the original paper [1] neurons  
      are coupled with zero delay, this implementation follows this  
      definition. It uses the update scheme described in [3] to  
      maintain causality: The incoming events in time step t_i are  
      taken into account at the beginning of the time step to calculate  
      the gain function and to decide upon a transition.  In order to  
      obtain delayed coupling with delay d, the user has to specify the  
      delay d+h upon connection, where h is the simulation time step.  
       
      

**Parameters:**
::

     
      tau_m   double - Membrane time constant (mean inter-update-interval)  
      in ms.  
      theta   double - threshold for sigmoidal activation function mV  
      c1  double - linear gain factor (probability/mV)  
      c2     double - prefactor of sigmoidal gain (probability)  
      c3   double - slope factor of sigmoidal gain (1/mV)  
       
      

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
      destroy the deconding scheme, as this effectively duplicates  
      every event. Using random connection routines it is therefore  
      advisable to set the property 'multapses' to false.  
      The neuron accepts several sources of currents, e.g. from a  
      noise_generator.  
       
       
      

**References:**
::

     
      [1] Iris Ginzburg, Haim Sompolinsky. Theory of correlations in stochastic  
      neural networks (1994). PRE 50(4) p. 3171  
      [2] Hertz Krogh, Palmer. Introduction to the theory of neural computation.  
      Westview (1991).  
      [3] Abigail Morrison, Markus Diesmann. Maintaining Causality in Discrete Time  
      Neuronal  
      Simulations.  
      In: Lectures in Supercomputational Neuroscience, p. 267. Peter beim Graben,  
      Changsong Zhou, Marco Thiel, Juergen Kurths (Eds.), Springer 2008.  
       
      

**Author:**
::

    Moritz Helias  
      

**FirstVersion:**
::

    February 2010  
      

**SeeAlso:**

-  `pp\_psc\_delta <../cc/pp_psc_delta.html>`__

**Source:**
::

    ./ginzburg_neuron.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
