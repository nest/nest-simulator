ac\_generator
======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    ac_generator - provides AC input current

**Description:**
::

     
       
      This device produce an ac-current which are sent by a CurrentEvent. The  
      current is given by  
       
      I(t) = offset + amplitude * sin ( om * t + phi )  
       
      where  
       
      om  = 2 * pi * frequency  
      phi = phase / 180 * pi  
       
      The parameters are  
       
      amplitude   double   -  Amplitude of sine current in pA  
      offset    double -  Constant amplitude offset in pA  
      frequency   double   -  Frequency in Hz  
      phase     double -  Phase of sine current (0-360 deg)  
       
      Setting start and stop (see StimulatingDevice) only windows the current  
      as defined above. It does not shift the time axis.  
       
      

**Sends:**
::

    CurrentEvent  
       
      

**References:**
::

     
      [1] S. Rotter and M. Diesmann, Exact digital simulation of time-  
      invariant linear systems with applications to neuronal modeling,  
      Biol. Cybern. 81, 381-402 (1999)  
       
      

**Author:**
::

    Johan Hake, Spring 2003  
       
      

**SeeAlso:**

-  `Device <../cc/Device.html>`__
-  `StimulatingDevice <../cc/StimulatingDevice.html>`__
-  `dc\_generator <../cc/dc_generator.html>`__
-  `step\_current\_generator <../cc/step_current_generator.html>`__

**Source:**
::

    ./ac_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
