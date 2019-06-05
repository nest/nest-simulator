step\_current\_generator
=================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    step_current_generator - provides a piecewise constant DC input current

**Examples:**
::

     
      The current can be altered in the following way:  
      /step_current_generator Create /sc Set  
      sc << /amplitude_times [0.2 0.5] /amplitude_values [2.0 4.0] >> SetStatus  
       
      The amplitude of the DC will be 0.0 pA in the time interval [0, 0.2),  
      2.0 pA in the interval [0.2, 0.5) and 4.0 from then on.  
       
      

**Description:**
::

     
      The dc_generator provides a piecewise constant DC input to the  
      connected node(s).  The amplitude of the current is changed at the  
      specified times. The unit of the current is pA.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      amplitude_times   list of doubles    - Times at which current changes in ms  
      amplitude_values  list of doubles    - Amplitudes of step current current in  
      pA  
      allow_offgrid_times  bool    - Default false  
      If false, times will be rounded to the nearest step if they are  
      less than tic/2 from the step, otherwise NEST reports an error.  
      If true,  times are rounded to the nearest step if within tic/2  
      from the step, otherwise they are rounded up to the *end* of the  
      step.  
       
      Note:  
      Times of amplitude changes must be strictly increasing after conversion  
      to simulation time steps. The option allow_offgrid_times may be  
      useful, e.g., if you are using randomized times for current changes  
      which typically would not fall onto simulation time steps.  
       
      

**Sends:**
::

    CurrentEvent  
       
      

**Author:**
::

    Jochen Martin Eppler, Jens Kremkow  
       
      

**SeeAlso:**

-  `ac\_generator <../cc/ac_generator.html>`__
-  `dc\_generator <../cc/dc_generator.html>`__
-  `step\_current\_generator <../cc/step_current_generator.html>`__
-  `Device <../cc/Device.html>`__
-  `StimulatingDevice <../cc/StimulatingDevice.html>`__

**Source:**
::

    ./step_current_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
