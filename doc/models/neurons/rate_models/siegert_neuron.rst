siegert\_neuron
========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Description:**
::

     
       
      siegert_neuron is an implementation of a rate model with the  
      non-linearity given by the gain function of the  
      leaky-integrate-and-fire neuron with delta or exponentially decaying  
      synapses [2] and [3, their eq. 25]. The model can be used for a  
      mean-field analysis of spiking networks.  
       
      The model supports connections to other rate models with zero  
      delay, and uses the secondary_event concept introduced with the  
      gap-junction framework.  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      rate  double - Rate (1/s)  
      tau     double - Time constant in ms.  
      mean  double - Additional constant input  
       
      The following parameters can be set in the status directory and are  
      used in the evaluation of the gain function. Parameters as in  
      iaf_psc_exp/delta.  
       
      tau_m   double - Membrane time constant in ms.  
      tau_syn  double - Time constant of postsynaptic currents in ms.  
      t_ref    double - Duration of refractory period in ms.  
      theta     double - Threshold relative to resting potential in mV.  
      V_reset     double - Reset relative to resting membrane potential in  
      mV.  
       
      Notes:  
       
      

**Require:**
::

    HAVE_GSL  
      

**Receives:**
::

    DiffusionConnectionEvent, DataLoggingRequest  
       
      

**Sends:**
::

    DiffusionConnectionEvent  
       
      

**References:**
::

     
       
      [1] Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,  
      Bolten, M., Helias, M. and Diesmann, M. (2017).  
      Integration of Continuous-Time Dynamics in a  
      Spiking Neural Network Simulator.  
      Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034  
       
      [2] Fourcaud, N and Brunel, N. (2002). Dynamics of the firing  
      probability of noisy integrate-and-fire neurons, Neural computation,  
      14:9, pp 2057--2110  
       
      [3] Schuecker, J., Diesmann, M. and Helias, M. (2015).  
      Modulated escape from a metastable state driven by colored noise.  
      Physical Review E 92:052119  
       
      [4] Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,  
      Bolten, M., Frommer, A. and Diesmann, M. (2015).  
      A unified framework for spiking and gap-junction interactions  
      in distributed neuronal network simulations.  
      Front. Neuroinform. 9:22. doi: 10.3389/fninf.2015.00022  
       
      

**Author:**
::

    Jannis Schuecker, David Dahmen, Jan Hahne  
      

**SeeAlso:**

-  `diffusion\_connection <../cc/diffusion_connection.html>`__

**Source:**
::

    ./siegert_neuron.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
