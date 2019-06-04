correlospinmatrix\_detector
====================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    correlospinmatrix_detector - Device for measuring the covariance matrix  
      from several inputs

**Description:**
::

    The correlospinmatrix_detector is a recording device. It is used  
      to record correlations from binary neurons from several binary sources and  
      calculates the raw auto and cross correlation binned to bins of duration  
      delta_tau. The result can be obtained via GetStatus under the key  
      /count_covariance. The result is a tensor of rank 3 of size  
      N_channels x N_channels, with each entry C_ij being a vector of size  
      2*tau_max/delta_tau + 1 containing the histogram for the different time lags.  
       
      The bins are centered around the time difference they represent, and are  
      left-closed and right-open in the lower triangular part of the matrix. On the  
      diagonal and in the upper triangular part the intervals are left-open and  
      right-closed. This ensures proper counting of events at the border of bins.  
       
      The correlospinmatrix_detector has a variable number of inputs which can be  
      set via SetStatus under the key N_channels. All incoming connections to a  
      specified receptor will be pooled.  
       
      

**Parameters:**
::

     
      Tstart  double     - Time when to start counting events. This time should  
      be set to at least start + tau_max in order to avoid  
      edge effects of the correlation counts.  
      Tstop    double     - Time when to stop counting events. This time should be  
      set to at most Tsim    - tau_max, where Tsim is the  
      duration of simulation, in order to avoid edge effects  
      of the correlation counts.  
      delta_tau  double    - bin width in ms. This has to be a multiple of the  
      resolution.  
      tau_max   double    - one-sided width in ms. In the lower triangular part  
      events with differences in [0, tau_max+delta_tau/2)  
      are counted. On the diagonal and in the upper  
      triangular part events with differences in (0,  
      tau_max+delta_tau/2]  
      N_channels long     - The number of inputs to correlate. This defines the  
      range of receptor_type. Default is 1.  
       
      count_covariance matrix of long vectors, read-only      - raw, auto/cross  
      correlation counts  
       
      

**Receives:**
::

    SpikeEvent  
       
      

**Remarks:**
::

    This recorder does not record to file, screen or memory in the usual  
      sense. The result must be obtained by a call to GetStatus. Setting either  
      N_channels, Tstart, Tstop, tau_max or delta_tau clears count_covariance.  
       
      Example:  
       
      See also pynest/examples/correlospinmatrix_detector_two_neuron.py  
      for a script reproducing a setting studied in Fig 1 of Grinzburg &  
      Sompolinsky (1994) PRE 50(4) p. 3171.  
       
      See also examples/nest/correlospinmatrix_detector.sli for a basic  
      example in sli.  
       
      /sg1 /spike_generator Create def  
      /sg2 /spike_generator Create def  
      /sg3 /spike_generator Create def  
       
      /csd /correlospinmatrix_detector Create def  
       
      csd << /N_channels 3 /tau_max 10. /delta_tau 1.0 >> SetStatus  
       
      sg1 << /spike_times [10. 10. 16.] >> SetStatus  
      sg2 << /spike_times [15. 15. 20.] >> SetStatus  
       
       
      % one final event needed so that last down transition will be detected  
      sg3 << /spike_times [25.] >> SetStatus  
       
       
      sg1 csd << /receptor_type 0 >> Connect  
      sg2 csd << /receptor_type 1 >> Connect  
      sg3 csd << /receptor_type 2 >> Connect  
       
      100. Simulate  
       
      

**Availability:**
::

    NEST 

**Author:**
::

    Moritz Helias  
       
      

**FirstVersion:**
::

    2015/08/25  
      

**SeeAlso:**

-  `correlation\_detector <../cc/correlation_detector.html>`__
-  `correlomatrix\_detector <../cc/correlomatrix_detector.html>`__
-  `spike\_detector <../cc/spike_detector.html>`__
-  `Device <../cc/Device.html>`__
-  `PseudoRecordingDevice <../cc/PseudoRecordingDevice.html>`__

**Source:**
::

    ./correlospinmatrix_detector.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
