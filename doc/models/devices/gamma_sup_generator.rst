gamma\_sup\_generator
==============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    gamma_sup_generator - simulate the superimposed spike train of a  
      population of gamma process.

**Description:**
::

     
       
      The gamma_sup_generator generator simulates the pooled spike train of a  
      population of neurons firing independently with gamma process statistics.  
       
      

**Parameters:**
::

     
      The following parameters appear in the element's status dictionary:  
       
      rate    double - mean firing rate of the component processes,  
      default: 0s^-1  
      gamma_shape  long     - shape paramter of component gamma processes, default: 1  
      n_proc     long   - number of superimposed independent component processes,  
      default: 1  
       
      

**Remarks:**
::

     
      The generator has been published in Deger, Helias, Boucsein, Rotter (2011)  
      Statistical properties of superimposed stationary spike trains,  
      Journal of Computational Neuroscience.  
      URL: http://www.springerlink.com/content/u75211r381p08301/  
      DOI: 10.1007/s10827-011-0362-8  
       
      

**Author:**
::

     
      Jan 2011, Moritz Deger  
       
      

**SeeAlso:**

-  `ppd\_sup\_generator <../cc/ppd_sup_generator.html>`__
-  `poisson\_generator\_ps <../cc/poisson_generator_ps.html>`__
-  `spike\_generator <../cc/spike_generator.html>`__
-  `Device <../cc/Device.html>`__
-  `StimulatingDevice <../cc/StimulatingDevice.html>`__

**Source:**
::

    ./gamma_sup_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
