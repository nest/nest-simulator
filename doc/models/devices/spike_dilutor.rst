spike\_dilutor
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    spike_dilutor - repeats incoming spikes with a certain probability.

**Description:**
::

     
      The device repeats incoming spikes with a certain probability.  
      Targets will receive diffenrent spike trains.  
       
      

**Parameters:**
::

     
      The following parameters appear in the element's status dictionary:  
      p_copy double  - Copy probability  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      In parallel simulations, a copy of the device is present on each process  
      and spikes are collected only from local sources.  
       
      

**Author:**
::

    Adapted from mip_generator by Kunkel, Oct 2011  
      ported to Nest 2.6 by: Setareh, April 2015  
       
      

**SeeAlso:**

-  `mip\_generator <../cc/mip_generator.html>`__

**Source:**
::

    ./spike_dilutor.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
