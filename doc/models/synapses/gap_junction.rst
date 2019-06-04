gap\_junction
======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    gap_junction - Synapse type for gap-junction connections.

**Description:**
::

     
      gap_junction is a connector to create gap junctions between pairs  
      of neurons. Gap junctions are bidirectional connections.  
      In order to create one accurate gap-junction connection between  
      neurons i and j two NEST connections are required: For each created  
      connection a second connection with the exact same parameters in  
      the opposite direction is required. NEST provides the possibility  
      to create both connections with a single call to Connect via  
      the make_symmetric flag:  
       
      i j << /rule /one_to_one /make_symmetric true >> /gap_junction Connect  
       
      The value of the parameter "delay" is ignored for connections of  
      type gap_junction.  
       
      

**Transmits:**
::

    GapJunctionEvent  
       
      

**References:**
::

     
       
      Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,  
      Bolten, M., Frommer, A. and Diesmann, M.,  
      A unified framework for spiking and gap-junction interactions  
      in distributed neuronal network simulations,  
      Front. Neuroinform. 9:22. (2015),  
      doi: 10.3389/fninf.2015.00022  
       
      Mancilla, J. G., Lewis, T. J., Pinto, D. J.,  
      Rinzel, J., and Connors, B. W.,  
      Synchronization of electrically coupled pairs  
      of inhibitory interneurons in neocortex,  
      J. Neurosci. 27, 2058-2073 (2007),  
      doi: 10.1523/JNEUROSCI.2715-06.2007  
       
      

**Author:**
::

    Jan Hahne, Moritz Helias, Susanne Kunkel  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `hh\_psc\_alpha\_gap <../cc/hh_psc_alpha_gap.html>`__

**Source:**
::

    ./gap_junction.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
