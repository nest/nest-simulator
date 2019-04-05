cont\_delay\_synapse
=============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    cont_delay_synapse - Synapse type for continuous delays

**Description:**
::

     
      cont_delay_synapse relaxes the condition that NEST only implements delays  
      which are an integer multiple of the time step h. A continuous delay is  
      decomposed into an integer part (delay_) and a double (delay_offset_) so  
      that the actual delay is given by  delay_*h - delay_offset_. This can be  
      combined with off-grid spike times.  
       
      Example:  
      << /resolution 1.0 >> SetKernelStatus  
       
      /sg /spike_generator << /precise_times true /spike_times [ 2.0 5.5 ] >> Create  
      def  
      /n  /iaf_psc_delta_canon Create def  
      /sd /spike_detector << /precise_times true /record_to [ /memory ] >> Create  
      def  
       
      /cont_delay_synapse << /weight 100. /delay 1.7 >> SetDefaults  
      sg n /cont_delay_synapse Connect  
      n sd Connect  
       
      10 Simulate  
       
      sd GetStatus /events/times :: ==   %  --> <. 3.7 7.2 .>  
       
      

**Transmits:**
::

    SpikeEvent, RateEvent, CurrentEvent, ConductanceEvent,  
      DoubleDataEvent  
       
      

**Remarks:**
::

     
      All delays set by the normal NEST Connect function will be rounded, even when  
      using cont_delay_connection. To set non-grid delays, you must either  
       
      1) set the delay as synapse default, as in the example above  
      2) set the delay for each synapse after the connections have been created,  
      e.g.,  
       
      sg n 100. 1.0 /cont_delay_synapse Connect  
      << /source [ sg ] /synapse_model /cont_delay_synapse >> GetConnections  
      { << /delay 1.7 >> SetStatus }  
      forall  
       
      Alternative 1) is much more efficient, but all synapses then will have the  
      same delay.  
      Alternative 2) is slower, but allows individual delay values.  
       
      Continuous delays cannot be shorter than the simulation resolution.  
       
      

**References:**
::

    none  
      

**Author:**
::

    Abigail Morrison  
      

**FirstVersion:**
::

    June 2007  
      

**SeeAlso:**

-  `synapsedict <../cc/synapsedict.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__
-  `iaf\_psc\_alpha\_canon <../cc/iaf_psc_alpha_canon.html>`__

**Source:**
::

    ./cont_delay_connection.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
