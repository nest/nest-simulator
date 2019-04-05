ppd\_sup\_generator
============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    ppd_sup_generator - simulate the superimposed spike train of a population  
      of Poisson processes  
      with dead time.

**Description:**
::

     
       
      The ppd_sup_generator generator simulates the pooled spike train of a  
      population of neurons firing independently with Poisson process with dead  
      time statistics.  
      The rate parameter can also be sine-modulated. The generator does not  
      initialize to equilibrium in this case, initial transients might occur.  
       
      

**Parameters:**
::

     
      The following parameters appear in the element's status dictionary:  
       
      rate    double - mean firing rate of the component processes,  
      default: 0 s^-1  
      dead_time     double - minimal time between two spikes of the component  
      processes, default: 0 ms  
      n_proc   long   - number of superimposed independent component  
      processes, default: 1  
      frequency   double - rate modulation frequency, default: 0 Hz  
      relative_amplitude  double   - relative rate modulation amplitude, default: 0  
       
      

**Remarks:**
::

     
      The generator has been published in Deger, Helias, Boucsein, Rotter (2011)  
      Statistical properties of superimposed stationary spike trains,  
      Journal of Computational Neuroscience.  
      URL: http://www.springerlink.com/content/u75211r381p08301/  
      DOI: 10.1007/s10827-011-0362-8  
       
      Authors:  
      June 2009, Moritz Deger, Moritz Helias  
       
      

**SeeAlso:**

-  `gamma\_sup\_generator <../cc/gamma_sup_generator.html>`__
-  `poisson\_generator\_ps <../cc/poisson_generator_ps.html>`__
-  `spike\_generator <../cc/spike_generator.html>`__
-  `Device <../cc/Device.html>`__
-  `StimulatingDevice <../cc/StimulatingDevice.html>`__

**Source:**
::

    ./ppd_sup_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
