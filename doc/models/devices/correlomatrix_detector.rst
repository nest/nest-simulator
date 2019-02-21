correlomatrix\_detector
================================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    correlomatrix_detector - Device for measuring the covariance matrix  
      from several inputs

**Description:**
::

    The correlomatrix_detector is a recording device. It is used to  
      record spikes from several pools of spike inputs and calculates the  
      covariance matrix of inter-spike intervals (raw auto and cross correlation)  
      binned to bins of duration delta_tau. The histogram is only recorded for  
      non-negative time lags. The negative part can be obtained by the symmetry of  
      the covariance matrix C(t) = C^T(-t).  
      The result can be obtained via GetStatus under the key /count_covariance.  
      In parallel it records a weighted histogram, where the connection weight are  
      used to weight every count, which is available under the key /covariance.  
      Both are matrices of size N_channels x N_channels, with each entry C_ij being  
      a vector of size tau_max/delta_tau + 1 containing the (weighted) histogram  
      for non-negative time lags.  
       
      The bins are centered around the time difference they represent, and are  
      left-closed and right-open in the lower triangular part of the matrix. On the  
      diagonal and in the upper triangular part the intervals are left-open and  
      right-closed. This ensures proper counting of events at the border of bins,  
      allowing consistent integration of a histogram over negative and positive  
      time lags by stacking two parts of the histogram  
      (C(t)=[C[i][j][::-1],C[j][i][1:]]).  
      In this case one needs to exclude C[j][i][0] to avoid counting the zero-lag  
      bin twice.  
       
      The correlomatrix_detector has a variable number of inputs which can be set  
      via SetStatus under the key N_channels. All incoming connections to a  
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
      delta_tau  double    - bin width in ms. This has to be an odd multiple of the  
      resolution, to allow the symmetry between positive and  
      negative time-lags.  
      tau_max   double    - one-sided width in ms. In the lower triagnular part  
      events with differences in [0, tau_max+delta_tau/2)  
      are counted. On the diagonal and in the upper  
      triangular part events with differences in  
      (0, tau_max+delta_tau/2]  
       
      N_channels long     - The number of pools. This defines the range of  
      receptor_type. Default is 1.  
      Setting N_channels clears count_covariance, covariance  
      and n_events.  
       
      covariance    matrix of double vectors, read-only    - raw, weighted  
      auto/cross  
      correlation counts  
      count_covariance  matrix of long vectors, read-only   - raw, auto/cross  
      correlation counts  
      n_events    integer vector     - number of events  
      from all sources.  
       
      

**Receives:**
::

    SpikeEvent  
       
      

**Remarks:**
::

    This recorder does not record to file, screen or memory in the usual  
      sense.  
       
      Example:  
      /s1 /spike_generator Create def  
      /s2 /spike_generator Create def  
      s1 << /spike_times [ 1.0 1.5 2.7 4.0 5.1 ] >> SetStatus  
      s2 << /spike_times [ 0.9 1.8 2.1 2.3 3.5 3.8 4.9 ] >> SetStatus  
      /cm /correlomatrix_detector Create def  
      cm << /N_channels 2 /delta_tau 0.5 /tau_max 2.5 >> SetStatus  
      s1 cm << /receptor_type 0 >> Connect  
      s2 cm << /receptor_type 1 >> Connect  
      10 Simulate  
      cm [/n_events] get ==   --> [# 5 7 #]  
      cm [/count_covariance] get ==  --> [[<# 5 1 2 2 0 2 #> <# 3 4 1 3 3 0 #>]  
      [<# 3 2 6 1 2 2 #> <# 9 3 4 6 1 2 #>]]  
      cm << /N_channels 2 >> SetStatus  
      cm [/count_covariance] get ==  --> [[<# 0 0 0 0 0 0 #> <# 0 0 0 0 0 0 #>]  
      [<# 0 0 0 0 0 0 #> <# 0 0 0 0 0 0 #>]]  
       
      

**Availability:**
::

    NEST 

**Author:**
::

    Dmytro Grytskyy  
      Jakob Jordan  
      

**FirstVersion:**
::

    2013/02/27  
      

**SeeAlso:**

-  `correlation\_detector <../cc/correlation_detector.html>`__
-  `spike\_detector <../cc/spike_detector.html>`__
-  `Device <../cc/Device.html>`__
-  `PseudoRecordingDevice <../cc/PseudoRecordingDevice.html>`__

**Source:**
::

    ./correlomatrix_detector.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
