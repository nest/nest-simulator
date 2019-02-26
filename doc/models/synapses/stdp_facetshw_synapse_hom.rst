stdp\_facetshw\_synapse\_hom
=====================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    stdp_facetshw_synapse_hom - Synapse type for spike-timing dependent  
      plasticity using homogeneous parameters,  
      i.e. all synapses have the same parameters.

**Description:**
::

     
      stdp_facetshw_synapse is a connector to create synapses with spike-timing  
      dependent plasticity (as defined in [1]).  
      This connector is a modified version of stdp_synapse.  
      It includes constraints of the hardware developed in the FACETS (BrainScaleS)  
      project [2,3], as e.g. 4-bit weight resolution, sequential updates of groups  
      of synapses and reduced symmetric nearest-neighbor spike pairing scheme. For  
      details see [3].  
      The modified spike pairing scheme requires the calculation of tau_minus_  
      within this synapse and not at the neuron site via Kplus_ like in  
      stdp_connection_hom.  
       
      

**Parameters:**
::

     
      Common properties:  
      tau_plus     double - Time constant of STDP window, causal branch in ms  
      tau_minus_stdp  double  - Time constant of STDP window, anti-causal branch  
      in ms  
      Wmax    double - Maximum allowed weight  
       
      no_synapses     long   - total number of synapses  
      synapses_per_driver   long   - number of synapses updated at once  
      driver_readout_time     double - time for processing of one synapse row  
      (synapse line driver)  
      readout_cycle_duration    double - duration between two subsequent  
      updates of same synapse (synapse line  
      driver)  
      lookuptable_0    vector   - three look-up tables (LUT)  
      lookuptable_1   vector  
      lookuptable_2  vector  
      configbit_0    vector   - configuration bits for evaluation  
      function. For details see code in  
      function eval_function_ and [4]  
      (configbit[0]=e_cc, ..[1]=e_ca,  
      ..[2]=e_ac, ..[3]=e_aa).  
      Depending on these two sets of  
      configuration bits weights are updated  
      according LUTs (out of three: (1,0),  
      (0,1), (1,1)). For (0,0) continue  
      without reset.  
      configbit_1   vector  
      reset_pattern  vector   - configuration bits for reset behavior.  
      Two bits for each LUT (reset causal  
      and acausal). In hardware only (all  
      false; never reset) or (all true;  
      always reset) is allowed.  
       
      Individual properties:  
      a_causal     double - causal and anti-causal spike pair accumulations  
      a_acausal   double  
      a_thresh_th  double    - two thresholds used in evaluation function.  
      No common property, because variation of analog  
      synapse circuitry can be applied here  
      a_thresh_tl  double  
      synapse_id   long   - synapse ID, used to assign synapses to groups (synapse  
      drivers)  
       
      

**Transmits:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      The synapse IDs are assigned to each synapse in an ascending order (0,1,2,  
      ...) according their first presynaptic activity and is used to group synapses  
      that are updated at once. It is possible to avoid activity dependent synapse  
      ID assignments by manually setting the no_synapses and the synapse_id(s)  
      before running the simulation. The weights will be discretized after the  
      first presynaptic activity at a synapse.  
       
      Common properties can only be set on the synapse model using SetDefaults.  
       
      

**References:**
::

     
      [1] Morrison, A., Diesmann, M., and Gerstner, W. (2008).  
      Phenomenological models of synaptic plasticity based on  
      spike-timing, Biol. Cybern., 98,459--478  
       
      [2] Schemmel, J., Gruebl, A., Meier, K., and Mueller, E. (2006).  
      Implementing synaptic plasticity in a VLSI spiking neural  
      network model, In Proceedings of the 2006 International  
      Joint Conference on Neural Networks, pp.1--6, IEEE Press  
       
      [3] Pfeil, T., Potjans, T. C., Schrader, S., Potjans, W., Schemmel, J.,  
      Diesmann, M., & Meier, K. (2012).  
      Is a 4-bit synaptic weight resolution enough?  -  
      constraints on enabling spike-timing dependent plasticity in neuromorphic  
      hardware. Front. Neurosci. 6 (90).  
       
      [4] Friedmann, S. in preparation  
       
       
      

**Author:**
::

    Thomas Pfeil (TP), Moritz Helias, Abigail Morrison  
      

**FirstVersion:**
::

    July 2011  
      

**SeeAlso:**

-  `stdp\_synapse <../cc/stdp_synapse.html>`__
-  `synapsedict <../cc/synapsedict.html>`__
-  `tsodyks\_synapse <../cc/tsodyks_synapse.html>`__
-  `static\_synapse <../cc/static_synapse.html>`__

**Source:**
::

    ./stdp_connection_facetshw_hom.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
