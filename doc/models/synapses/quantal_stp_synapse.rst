Command: quantal\_stp\_synapse
==============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    quantal_stp_synapse - Probabilistic synapse model with short term  
      plasticity.

**Description:**
::

     
       
      This synapse model implements synaptic short-term depression and  
      short-term facilitation according to the quantal release model  
      described by Fuhrmann et al. [1] and Loebel et al. [2].  
       
      Each presynaptic spike will stochastically activate a fraction of  
      the available release sites.  This fraction is binomialy  
      distributed and the release probability per site is governed by the  
      Fuhrmann et al. (2002) model. The solution of the differential  
      equations is taken from Maass and Markram 2002 [3].  
       
      The connection weight is interpreted as the maximal weight that can  
      be obtained if all n release sites are activated.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      U     double - Maximal fraction of available resources [0,1],  
      default=0.5  
      u   double - available fraction of resources [0,1], default=0.5  
      p   double - probability that a vesicle is available, default = 1.0  
      n   long   - total number of release sites, default = 1  
      a   long   - number of available release sites, default = n  
      tau_rec   double   - time constant for depression in ms, default=800 ms  
      tau_rec   double   - time constant for facilitation in ms, default=0 (off)  
       
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] Fuhrmann, G., Segev, I., Markram, H., & Tsodyks, M. V. (2002). Coding of  
      temporal information by activity-dependent synapses. Journal of  
      neurophysiology, 87(1), 140-8.  
      [2] Loebel, A., Silberberg, G., Helbig, D., Markram, H., Tsodyks,  
      M. V, & Richardson, M. J. E. (2009). Multiquantal release underlies  
      the distribution of synaptic efficacies in the neocortex. Frontiers  
      in computational neuroscience, 3(November), 27.  
      doi:10.3389/neuro.10.027.2009  
      [3] Maass, W., & Markram, H. (2002). Synapses as dynamic memory buffers.  
      Neural networks, 15(2), 155-61.  
       
      

**Author:**
::

    Marc-Oliver Gewaltig, based on tsodyks2_synapse  
      

**FirstVersion:**
::

    December 2013  
      

**SeeAlso:**

-  `tsodyks2\_synapse <../cc/tsodyks2_synapse.html>`__
-  `synapsedict <../cc/synapsedict.html>`__
-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./quantal_stp_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
