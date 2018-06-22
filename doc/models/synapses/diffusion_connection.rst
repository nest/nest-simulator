diffusion\_connection
==============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    diffusion_connection - Synapse type for instantaneous rate connections  
      between neurons of type siegert_neuron.

**Description:**
::

    diffusion_connection is a connector to create  
      instantaneous connections between neurons of type siegert_neuron. The  
      connection type is identical to type rate_connection_instantaneous  
      for instantaneous rate connections except for the two parameters  
      drift_factor and diffusion_factor substituting the parameter weight.  
       
      These two factor origin from the mean-field reduction of networks of  
      leaky-integrate-and-fire neurons. In this reduction the input to the  
      neurons is characterized by its mean and its variance. The mean is  
      obtained by a sum over presynaptic activities (e.g as in eq.28 in  
      [1]), where each term of the sum consists of the presynaptic activity  
      multiplied with the drift_factor. Similarly, the variance is obtained  
      by a sum over presynaptic activities (e.g as in eq.29 in [1]), where  
      each term of the sum consists of the presynaptic activity multiplied  
      with the diffusion_factor. Note that in general the drift and  
      diffusion factors might differ from the ones given in eq. 28 and 29.,  
      for example in case of a reduction on the single neuron level or in  
      case of distributed in-degrees (see discussion in chapter 5.2 of [1])  
       
      The values of the parameters delay and weight are ignored for  
      connections of this type.  
       
      

**Transmits:**
::

    DiffusionConnectionEvent  
       
      

**References:**
::

     
       
      [1] Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,  
      Bolten, M., Helias, M. and Diesmann, M. (2017).  
      Integration of Continuous-Time Dynamics in a  
      Spiking Neural Network Simulator.  
      Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034  
       
      

**Author:**
::

    David Dahmen, Jan Hahne, Jannis Schuecker  
      

**SeeAlso:**

-  `siegert\_neuron <../cc/siegert_neuron.html>`__
-  `rate\_connection\_instantaneous <../cc/rate_connection_instantaneous.html>`__

**Source:**
::

    ./diffusion_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
