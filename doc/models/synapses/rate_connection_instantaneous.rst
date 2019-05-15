rate\_connection\_instantaneous
========================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    rate_connection_instantaneous - Synapse type for instantaneous rate  
      connections.

**Description:**
::

     
      rate_connection_instantaneous is a connector to create  
      instantaneous connections between rate model neurons.  
       
      The value of the parameter delay is ignored for connections of  
      this type. To create rate connections with delay please use  
      the synapse type rate_connection_delayed.  
       
      

**Transmits:**
::

    InstantaneousRateConnectionEvent  
       
      

**References:**
::

     
       
      Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,  
      Bolten, M., Helias, M. and Diesmann, M. (2017).  
      Integration of Continuous-Time Dynamics in a  
      Spiking Neural Network Simulator.  
      Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034  
       
      

**Author:**
::

    David Dahmen, Jan Hahne, Jannis Schuecker  
      

**SeeAlso:**

-  `rate\_connection\_delayed <../cc/rate_connection_delayed.html>`__
-  `rate\_neuron\_ipn <../cc/rate_neuron_ipn.html>`__
-  `rate\_neuron\_opn <../cc/rate_neuron_opn.html>`__

**Source:**
::

    ./rate_connection_instantaneous.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
