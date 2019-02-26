iaf\_psc\_exp\_multisynapse
====================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_psc_exp_multisynapse - Leaky integrate-and-fire neuron model with  
      multiple ports.

**Description:**
::

     
       
      iaf_psc_exp_multisynapse is a direct extension of iaf_psc_exp.  
      On the postsynapic side, there can be arbitrarily many synaptic  
      time constants (iaf_psc_exp has exactly two: tau_syn_ex and tau_syn_in).  
       
      This can be reached by specifying separate receptor ports, each for  
      a different time constant. The port number has to match the respective  
      "receptor_type" in the connectors.  
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
       
      Author:  Plesser, adapted from iaf_psc_alpha_multisynapse  
      

**Sends:**
::

    SpikeEvent  
       
      

**SeeAlso:**

-  `iaf\_psc\_alpha <../cc/iaf_psc_alpha.html>`__
-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_cond\_exp <../cc/iaf_cond_exp.html>`__
-  `iaf\_psc\_alpha\_multisynapse <../cc/iaf_psc_alpha_multisynapse.html>`__

**Source:**
::

    ./iaf_psc_exp_multisynapse.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
