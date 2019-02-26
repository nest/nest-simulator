sigmoid\_rate
======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    sigmoid_rate - rate model with sigmoidal gain function

**Description:**
::

     
       
      sigmoid_rate is an implementation of a nonlinear rate model with input  
      function input(h) = g / ( 1. + exp( -beta * ( h - theta ) ) ).  
      Input transformation can either be applied to individual inputs  
      or to the sum of all inputs.  
       
      The model supports connections to other rate models with either zero or  
      non-zero delay, and uses the secondary_event concept introduced with  
      the gap-junction framework.  
       
      

**Parameters:**
::

     
       
      The following parameters can be set in the status dictionary.  
       
      rate  double - Rate (unitless)  
      tau    double - Time constant of rate dynamics in ms.  
      mean     double - Mean of Gaussian white noise.  
      std  double - Standard deviation of Gaussian white noise.  
      g  double - Gain parameter.  
      beta   double - Slope parameter.  
      theta     double - Threshold.  
      linear_summation   bool    - Specifies type of non-linearity (see above).  
      rectify_output    bool   - Switch to restrict rate to values >= 0.  
       
      Note:  
      The boolean parameter linear_summation determines whether the  
      input from different presynaptic neurons is first summed linearly and  
      then transformed by a nonlinearity (true), or if the input from  
      individual presynaptic neurons is first nonlinearly transformed and  
      then summed up (false). Default is true.  
       
      

**Receives:**
::

    InstantaneousRateConnectionEvent, DelayedRateConnectionEvent,  
      DataLoggingRequest  
       
      

**Sends:**
::

    InstantaneousRateConnectionEvent, DelayedRateConnectionEvent  
       
      

**References:**
::

     
       
      [1] Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,  
      Bolten, M., Helias, M. and Diesmann, M. (2017).  
      Integration of Continuous-Time Dynamics in a  
      Spiking Neural Network Simulator.  
      Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034  
       
      [2] Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,  
      Bolten, M., Frommer, A. and Diesmann, M. (2015).  
      A unified framework for spiking and gap-junction interactions  
      in distributed neuronal network simulations.  
      Front. Neuroinform. 9:22. doi: 10.3389/fninf.2015.00022  
       
      

**Author:**
::

    Mario Senden, Jan Hahne, Jannis Schuecker  
      

**SeeAlso:**

-  `rate\_connection\_instantaneous <../cc/rate_connection_instantaneous.html>`__
-  `rate\_connection\_delayed <../cc/rate_connection_delayed.html>`__

**Source:**
::

    ./sigmoid_rate.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <http://www.nest-initiative.org>`__
