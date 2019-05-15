mip\_generator
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    mip_generator - create spike trains as described by the MIP model.

**Description:**
::

     
      The mip_generator generates correlated spike trains using an Multiple  
      Interaction Process (MIP) as described in [1]. Underlying principle is a  
      Poisson mother process with rate r, the spikes of which are copied into the  
      child processes with a certain probability p. Every node the mip_generator is  
      connected to receives a distinct child process as input, whose rate is p*r.  
      The value of the pairwise correlation coefficient of two child processes  
      created by a MIP process equals p.  
       
       
      

**Parameters:**
::

     
      The following parameters appear in the element's status dictionary:  
       
      rate    double - Mean firing rate of the mother process in Hz  
      p_copy    double - Copy probability  
      mother_rng   rng     - Random number generator of mother process  
      mother_seed  long   - Seed of RNG of mother process  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      The MIP generator may emit more than one spike through a child process  
      during a single time step, especially at high rates.  If this happens,  
      the generator does not actually send out n spikes.  Instead, it emits  
      a single spike with n-fold synaptic weight for the sake of efficiency.  
      Furthermore, note that as with the Poisson generator, different threads  
      have their own copy of a MIP generator. By using the same mother_seed  
      it is ensured that the mother process is identical for each of the  
      generators.  
       
      IMPORTANT: The mother_seed of mpi_generator must be different from any  
      seeds used for the global or thread-specific RNGs set in  
      the kernel.  
       
      

**References:**
::

     
      [1] Alexandre Kuhn, Ad Aertsen, Stefan Rotter  
      Higher-Order Statistics of Input Ensembles and the Response of Simple  
      Model Neurons  
      Neural Computation 15, 67-101 (2003)  
       
      

**Author:**
::

    May 2006, Helias  
      

**SeeAlso:**

-  `Device <../cc/Device.html>`__

**Source:**
::

    ./mip_generator.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
