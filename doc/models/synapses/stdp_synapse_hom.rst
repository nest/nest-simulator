Command: stdp\_synapse\_hom
===========================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    stdp_synapse_hom - Synapse type for spike-timing dependent  
      plasticity using homogeneous parameters.

**Examples:**
::

     
      multiplicative STDP [2]  mu_plus = mu_minus = 1.0  
      additive STDP     [3]  mu_plus = mu_minus = 0.0  
      Guetig STDP   [1]  mu_plus = mu_minus = [0.0,1.0]  
      van Rossum STDP     [4]  mu_plus = 0.0 mu_minus = 1.0  
       
      

**Description:**
::

     
      stdp_synapse_hom is a connector to create synapses with spike time  
      dependent plasticity (as defined in [1]). Here the weight dependence  
      exponent can be set separately for potentiation and depression.  
       
      Parameters controlling plasticity are identical for all synapses of the  
      model, reducing the memory required per synapse considerably.  
       
      

**Parameters:**
::

     
      tau_plus   double  - Time constant of STDP window, potentiation in ms  
      (tau_minus defined in post-synaptic neuron)  
      lambda    double - Step size  
      alpha    double - Asymmetry parameter (scales depressing increments as  
      alpha*lambda)  
      mu_plus   double   - Weight dependence exponent, potentiation  
      mu_minus   double    - Weight dependence exponent, depression  
      Wmax    double - Maximum allowed weight  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      The parameters are common to all synapses of the model and must be set using  
      SetDefaults on the synapse model.  
       
      

**References:**
::

     
      [1] Guetig et al. (2003) Learning Input Correlations through Nonlinear  
      Temporally Asymmetric Hebbian Plasticity. Journal of Neuroscience  
       
      [2] Rubin, J., Lee, D. and Sompolinsky, H. (2001). Equilibrium  
      properties of temporally asymmetric Hebbian plasticity, PRL  
      86,364-367  
       
      [3] Song, S., Miller, K. D. and Abbott, L. F. (2000). Competitive  
      Hebbian learning through spike-timing-dependent synaptic  
      plasticity,Nature Neuroscience 3:9,919--926  
       
      [4] van Rossum, M. C. W., Bi, G-Q and Turrigiano, G. G. (2000).  
      Stable Hebbian learning from spike timing-dependent  
      plasticity, Journal of Neuroscience, 20:23,8812--8821  
       
      

**Author:**
::

    Moritz Helias, Abigail Morrison  
      

**FirstVersion:**
::

    March 2006  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `tsodyks\_synapse <../cc/tsodyks_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./stdp_connection_hom.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
