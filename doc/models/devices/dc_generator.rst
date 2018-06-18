dc\_generator
======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    dc_generator - provides DC input current

**Examples:**
::

    The dc current can be altered in the following way:  
      /dc_generator Create /dc_gen Set   % Creates a dc_generator, which is a node  
      dc_gen GetStatus info  % View properties (amplitude is 0)  
      dc_gen << /amplitude 1500. >> SetStatus  
      dc_gen GetStatus info    % amplitude is now 1500.0  
       
      

**Description:**
::

    The DC-Generator provides a constant DC Input  
      to the connected node. The unit of the current is pA.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      amplitude  double    - Amplitude of current in pA  
       
      

**Sends:**
::

    CurrentEvent  
       
      

**Remarks:**
::

    The dc_generator is rather inefficient, since it needs to  
      send the same current information on each time step. If you  
      only need a constant bias current into a neuron, you should  
      set it directly in the neuron, e.g., dc_generator.  
       
      

**Author:**
::

    docu by Sirko Straube  
       
      

**SeeAlso:**

-  `Device <../cc/Device.html>`__
-  `StimulatingDevice <../cc/StimulatingDevice.html>`__

**Source:**
::

    ./dc_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
