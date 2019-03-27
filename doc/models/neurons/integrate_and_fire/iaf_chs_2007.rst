iaf\_chs\_2007
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    iaf_chs_2007 - Spike-response model used in Carandini et al 2007.

**Description:**
::

     
      The membrane potential is the sum of stereotyped events: the postsynaptic  
      potentials (V_syn), waveforms that include a spike and the subsequent  
      after-hyperpolarization (V_spike) and Gaussian-distributed white noise.  
       
      The postsynaptic potential is described by alpha function where where  
      U_epsp is the maximal amplitude of the EPSP and tau_epsp is the time to  
      peak of the EPSP.  
       
      The spike waveform is described as a delta peak followed by a membrane  
      potential reset and exponential decay. U_reset is the magnitude of the  
      reset/after-hyperpolarization and tau_reset is the time constant of  
      recovery from this hyperpolarization.  
       
      The linear subthresold dynamics is integrated by the Exact  
      Integration scheme [1]. The neuron dynamics is solved on the time  
      grid given by the computation step size. Incoming as well as emitted  
      spikes are forced to that grid.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary.  
       
      tau_epsp  double - Membrane time constant in ms.  
      tau_reset    double - Refractory time constant in ms.  
      U_epsp     double - Maximum amplitude of the EPSP. Normalized.  
      U_reset     double - Reset value of the membrane potential. Normalized.  
      U_noise     double - Noise scale. Normalized.  
      noise   vector- Noise signal.  
       
      

**Receives:**
::

    SpikeEvent, DataLoggingRequest  
       
      

**Sends:**
::

    SpikeEvent  
       
      

**Remarks:**
::

     
      The way the noise term was implemented in the original model makes it  
      unsuitable for simulation in NEST. The workaround was to prepare the  
      noise signal externally prior to simulation. The noise signal,  
      if present, has to be at least as long as the simulation.  
       
      

**References:**
::

     
      [1] Carandini M, Horton JC, Sincich LC (2007) Thalamic filtering of retinal  
      spike trains by postsynaptic summation. J Vis 7(14):20,1-11.  
      [2] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear  
      systems with applications to neuronal modeling. Biologial Cybernetics  
      81:381-402.  
       
      

**Author:**
::

    Thomas Heiberg, Birgit Kriener 

**FirstVersion:**
::

    May 2012  
      

**Source:**
::

    ./iaf_chs_2007.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
