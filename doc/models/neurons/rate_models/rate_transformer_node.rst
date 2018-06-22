rate\_transformer\_node
================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    rate_transformer_node - Rate neuron that sums up incoming rates  
      and applies a nonlinearity specified via the template.

**Description:**
::

     
       
      The rate transformer node simply sums up all incoming rates and applies  
      the nonlinearity specified in the function input of the template class.  
      An important application is to provide the possibility to  
      apply different nonlinearities to different incoming connections of the  
      same rate neuron by connecting the sending rate neurons to the  
      rate transformer node and connecting the rate transformer node to the  
      receiving rate neuron instead of using a direct connection.  
      Please note that for instantaneous rate connections the rate arrives  
      one time step later at the receiving rate neurons as with a direct connection.  
       
      

**Parameters:**
::

     
      Only the parameters from the class Nonlinearities can be set in the  
      status dictionary.  
       
      

**Receives:**
::

    InstantaneousRateConnectionEvent, DelayedRateConnectionEvent  
       
      

**Sends:**
::

    InstantaneousRateConnectionEvent, DelayedRateConnectionEvent  
       
      

**Remarks:**
::

     
       
         - Weights on connections from and to the rate_transformer_node  
      are handled as usual.  
         - Delays are honored on incoming and outgoing connections.  
       
      

**Author:**
::

    Mario Senden, Jan Hahne, Jannis Schuecker  
      

**FirstVersion:**
::

    November 2017 

**Source:**
::

    ./rate_transformer_node.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
