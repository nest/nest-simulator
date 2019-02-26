correlation\_detector
==============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    correlation_detector - Device for evaluating cross correlation between  
      two spike sources

**Description:**
::

     
      The correlation_detector device is a recording device. It is used to record  
      spikes from two pools of spike inputs and calculates the count_histogram of  
      inter-spike intervals (raw cross correlation) binned to bins of duration  
      delta_tau. The result can be obtained via GetStatus under the key  
      /count_histogram.  
      In parallel it records a weighted histogram, where the connection weights  
      are used to weight every count. In order to minimize numerical errors the  
      Kahan summation algorithm is used when calculating the weighted histogram.  
      (http://en.wikipedia.org/wiki/Kahan_summation_algorithm)  
      Both are arrays of 2*tau_max/delta_tau+1 values containing the histogram  
      counts in the following way:  
       
      Let t_{1,i} be the spike times of source 1,  
      t_{2,j} the spike times of source 2.  
      histogram[n] then contains the sum of products of the weight w_{1,i}*w_{2,j},  
      count_histogram[n] contains 1 summed over all events with t_{2,j}-t_{1,i} in  
       
      [ n*delta_tau   - tau_max   - delta_tau/2 , n*delta_tau - tau_max + delta_tau/2 )  
       
      The bins are centered around the time difference they represent, but are  
      left-closed and right-open. This means that events with time difference  
      -tau_max-delta_tau/2 are counted in the leftmost bin, but event with  
      difference tau_max+delta_tau/2 are not counted at all.  
       
      The correlation detector has two inputs, which are selected via the  
      receptor_port of the incoming connection: All incoming connections with  
      receptor_port = 0 will be pooled as the spike source 1, the ones with  
      receptor_port = 1 will be used as spike source 2.  
       
      

**Parameters:**
::

     
      Tstart  double     - Time when to start counting events. This time should  
      be set to at least start + tau_max in order to avoid  
      edge effects of the correlation counts.  
      Tstop    double     - Time when to stop counting events. This time should be  
      set to at most Tsim    - tau_max, where Tsim is the  
      duration of simulation, in order to avoid edge effects  
      of the correlation counts.  
      delta_tau  double    - bin width in ms  
      tau_max   double      - one-sided histogram width in ms. Events with  
      differences in  
      [-tau_max-delta_tau/2, tau_max+delta_tau/2)  
      are counted.  
       
      histogram     double vector, read-only   - raw, weighted cross  
      correlation counts  
      histogram_correction double_vector, read-only  - correction factors for kahan  
      summation algorithm  
      count_histogram   long vector, read-only     - raw, cross correlation  
      counts  
      n_events     integer vector     - number of events from source  
      0 and 1. By setting n_events  
      to [0 0], the histogram is  
      cleared.  
       
      

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
      /cd /correlation_detector Create def  
      cd << /delta_tau 0.5 /tau_max 2.5 >> SetStatus  
      s1 cd << /receptor_type 0 >> Connect  
      s2 cd << /receptor_type 1 >> Connect  
      10 Simulate  
      cd [/n_events] get ==   --> [# 5 7 #]  
      cd [/histogram] get ==  --> [. 0 3 3 1 4 3 2 6 1 2 2 .]  
      cd << /reset true >> SetStatus  
      cd [/histogram] get ==  --> [. 0 0 0 0 0 0 0 0 0 0 0 .]  
       
      

**Availability:**
::

    NEST 

**Author:**
::

    Moritz Helias  
      Jakob Jordan (implemented Kahan summation algorithm) 2013/02/18  
      

**FirstVersion:**
::

    2007/5/21  
      

**SeeAlso:**

-  `spike\_detector <../cc/spike_detector.html>`__
-  `Device <../cc/Device.html>`__
-  `PseudoRecordingDevice <../cc/PseudoRecordingDevice.html>`__

**Source:**
::

    ./correlation_detector.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
