izhikevich
===================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    izhikevich - Izhikevich neuron model

**Description:**
::

     
      Implementation of the simple spiking neuron model introduced by Izhikevich  
      [1]. The dynamics are given by:  
      dv/dt = 0.04*v^2 + 5*v + 140    - u + I  
      du/dt = a*(b*v  - u)  
       
      if v >= V_th:  
      v is set to c  
      u is incremented by d  
       
      v jumps on each spike arrival by the weight of the spike.  
       
      As published in [1], the numerics differs from the standard forward Euler  
      technique in two ways:  
      1) the new value of u is calculated based on the new value of v, rather than  
      the previous value  
      2) the variable v is updated using a time step half the size of that used to  
      update variable u.  
       
      This model offers both forms of integration, they can be selected using the  
      boolean parameter consistent_integration. To reproduce some results published  
      on the basis of this model, it is necessary to use the published form of the  
      dynamics. In this case, consistent_integration must be set to false. For all  
      other purposes, it is recommended to use the standard technique for forward  
      Euler integration. In this case, consistent_integration must be set to true  
      (default).  
       
       
      

**Parameters:**
::

     
      The following parameters can be set in the status dictionary.  
       
      V_m   double - Membrane potential in mV  
      U_m   double - Membrane potential recovery variable  
      V_th  double - Spike threshold in mV.  
      I_e     double - Constant input current in pA. (R=1)  
      V_min  double - Absolute lower value for the membrane potential.  
      a     double - describes time scale of recovery variable  
      b    double - sensitivity of recovery variable  
      c     double - after-spike reset value of V_m  
      d   double - after-spike reset value of U_m  
      consistent_integration  bool   - use standard integration technique  
       
       
      

**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest  
      

**Sends:**
::

    SpikeEvent  
       
      

**References:**
::

     
      [1] Izhikevich, Simple Model of Spiking Neurons,  
      IEEE Transactions on Neural Networks (2003) 14:1569-1572  
       
      

**Author:**
::

    Hanuschkin, Morrison, Kunkel  
      

**FirstVersion:**
::

    2009  
      

**SeeAlso:**

-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `mat2\_psc\_exp <../cc/mat2_psc_exp.html>`__

**Source:**
::

    ./izhikevich.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
