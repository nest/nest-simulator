multimeter
===================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    multimeter - Device to record analog data from neurons.

**Synopsis:**
::

    multimeter Create  
       
      

**Examples:**
::

     
      SLI ] /iaf_cond_alpha Create /n Set  
      SLI ] n /recordables get ==  
      [/V_m /g_ex /g_in /t_ref_remaining]  
      SLI ] /multimeter Create /mm Set  
      SLI ] mm << /interval 0.5 /record_from [/V_m /g_ex /g_in] >> SetStatus  
      SLI ] mm n Connect  
      SLI ] 10 Simulate  
      SLI ] mm /events get info  
      --------------------------------------------------  
      Name  Type    Value  
      --------------------------------------------------  
      g_ex   doublevectortype     
      g_in  doublevectortype     
      senders   intvectortype     
      times   doublevectortype     
      t_ref_remaining   doublevectortype     
      V_m   doublevectortype     
      rate  doublevectortype     
      --------------------------------------------------  
      Total number of entries: 6  
       
       
      

**Description:**
::

     
      A multimeter records a user-defined set of state variables from connected nodes  
      to memory, file or stdout.  
       
      The multimeter must be configured with the list of variables to record  
      from, otherwise it will not record anything. The /recordables property  
      of a neuron model shows which quantities can be recorded with a multimeter.  
      A single multimeter should only record from neurons of the same basic  
      type (e.g. /iaf_cond_alpha and any user-defined models derived from it  
      using CopyModel). If the defaults or status dictionary of a model neuron  
      does not contain a /recordables entry, it is not ready for use with  
      multimeter.  
       
      By default, multimeters record values once per ms. Set the parameter /interval  
      to change this. The recording interval cannot be smaller than the resolution.  
       
      Results are returned in the /events entry of the status dictionary. For  
      each recorded quantity, a vector of doubles is returned. The vector has the  
      same name as the /recordable. If /withtime is set, times are given in the  
      /times vector in /events.  
       
      Accumulator mode:  
      Multimeter can operate in accumulator mode. In this case, values for all  
      recorded variables are added across all recorded nodes (but kept separate in  
      time). This can be useful to record average membrane potential in a population.  
       
      To activate accumulator mode, either set /to_accumulator to true, or set  
      /record_to [ /accumulator ].  In accumulator mode, you cannot record to file,  
      to memory, to screen, with GID or with weight. You must activate accumulator  
      mode before simulating. Accumulator data is never written to file. You must  
      extract it from the device using GetStatus.  
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary:  
      interval  double - Recording interval in ms  
      record_from  array   - Array containing the names of variables to record  
      from, obtained from the /recordables entry of the  
      model from which one wants to record  
       
      

**Sends:**
::

    DataLoggingRequest  
       
      

**Remarks:**
::

     
         - The set of variables to record and the recording interval must be set  
      BEFORE the multimeter is connected to any node, and cannot be changed  
      afterwards.  
        - A multimeter cannot be frozen.  
         - If you record with multimeter in accumulator mode and some of the nodes  
      you record from are frozen and others are not, data will only be collected  
      from the unfrozen nodes. Most likely, this will lead to confusing results,  
      so you should not use multimeter with frozen nodes.  
       
      

**Author:**
::

    Hans Ekkehard Plesser, Barna Zajzon (added offset support March 2017)  
       
       
      

**FirstVersion:**
::

    2009-04-01  
       
      

**SeeAlso:**

-  `Device <../cc/Device.html>`__
-  `RecordingDevice <../cc/RecordingDevice.html>`__

**Source:**
::

    ./multimeter.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
