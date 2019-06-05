pp\_psc\_delta
=======================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    pp_psc_delta - Point process neuron with leaky integration of
      delta-shaped PSCs.

**Description:**
::



      pp_psc_delta is an implementation of a leaky integrator, where the potential
      jumps on each spike arrival. It produces spike stochastically, and supports
      spike-frequency adaptation, and other optional features.

      Spikes are generated randomly according to the current value of the
      transfer function which operates on the membrane potential. Spike
      generation is followed by an optional dead time. Setting with_reset to
      true will reset the membrane potential after each spike.

      The transfer function can be chosen to be linear, exponential or a sum of
      both by adjusting three parameters:

      rate = Rect[ c_1 * V' + c_2 * exp(c_3 * V') ],

      where the effective potential V' = V_m    - E_sfa and E_sfa is called
      the adaptive threshold. Here Rect means rectifier:
      Rect(x) = {x if x>=0, 0 else} (this is necessary because negative rates are
      not possible).

      By setting c_3 = 0, c_2 can be used as an offset spike rate for an otherwise
      linear rate model.

      The dead time enables to include refractoriness. If dead time is 0, the
      number of spikes in one time step might exceed one and is drawn from the
      Poisson distribution accordingly. Otherwise, the probability for a spike
      is given by 1   - exp(-rate*h), where h is the simulation time step. If
      dead_time is smaller than the simulation resolution (time step), it is
      internally set to the resolution.

      Note that, even if non-refractory neurons are to be modeled, a small value
      of dead_time, like dead_time=1e-8, might be the value of choice since it
      uses faster uniform random numbers than dead_time=0, which draws Poisson
      numbers. Only for very large spike rates (> 1 spike/time_step) this will
      cause errors.

      The model can optionally include an adaptive firing threshold.
      If the neuron spikes, the threshold increases and the membrane potential
      will take longer to reach it.
      Here this is implemented by subtracting the value of the adaptive threshold
      E_sfa from the membrane potential V_m before passing the potential to the
      transfer function, see also above. E_sfa jumps by q_sfa when the neuron
      fires a spike, and decays exponentially with the time constant tau_sfa
      after (see [2] or [3]). Thus, the E_sfa corresponds to the convolution of the
      neuron's spike train with an exponential kernel.
      This adaptation kernel may also be chosen as the sum of n exponential
      kernels. To use this feature, q_sfa and tau_sfa have to be given as a list
      of n values each.

      The firing of pp_psc_delta is usually not a renewal process. For example,
      its firing may depend on its past spikes if it has non-zero adaptation terms
      (q_sfa). But if so, it will depend on all its previous spikes, not just the
      last one -- so it is not a renewal process model. However, if "with_reset"
      is True, and all adaptation terms (q_sfa) are 0, then it will reset
      ("forget") its membrane potential each time a spike is emitted, which makes
      it a renewal process model (where "rate" above is its hazard function,
      also known as conditional intensity).

      pp_psc_delta may also be called a spike-response model with escape-noise [6]
      (for vanishing, non-random dead_time). If c_1>0 and c_2==0, the rate is a
      convolution of the inputs with exponential filters -- which is a model known
      as a Hawkes point process (see [4]). If instead c_1==0, then pp_psc_delta is
      a point process generalized linear model (with the canonical link function,
      and exponential input filters) (see [5,6]).

      This model has been adapted from iaf_psc_delta. The default parameters are
      set to the mean values given in [2], which have been matched to spike-train
      recordings. Due to the many features of pp_psc_delta and its versatility,
      parameters should be set carefully and conciously.




**Parameters:**
::



      The following parameters can be set in the status dictionary.

      V_m   double - Membrane potential in mV.
      C_m  double - Capacitance of the membrane in pF.
      tau_m   double - Membrane time constant in ms.
      q_sfa    double - Adaptive threshold jump in mV.
      tau_sfa     double - Adaptive threshold time constant in ms.
      dead_time  double - Duration of the dead time in ms.
      dead_time_random  bool   - Should a random dead time be drawn after each
      spike?
      dead_time_shape   int    - Shape parameter of dead time gamma distribution.
      t_ref_remaining   double - Remaining dead time at simulation start.
      with_reset    bool   - Should the membrane potential be reset after a
      spike?
      I_e  double - Constant input current in pA.
      c_1  double - Slope of linear part of transfer function in
      Hz/mV.
      c_2    double - Prefactor of exponential part of transfer function
      in Hz.
      c_3  double - Coefficient of exponential non-linearity of
      transfer function in 1/mV.




**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest

      Author:  July 2009, Deger, Helias; January 2011, Zaytsev; May 2014, Setareh


**Sends:**
::

    SpikeEvent



**References:**
::



      [1] Multiplicatively interacting point processes and applications to neural
      modeling (2010) Stefano Cardanobile and Stefan Rotter, Journal of
      Computational Neuroscience

      [2] Predicting spike timing of neocortical pyramidal neurons by simple
      threshold models (2006) Jolivet R, Rauch A, Luescher H-R, Gerstner W.
      J Comput Neurosci 21:35-49

      [3] Pozzorini C, Naud R, Mensi S, Gerstner W (2013) Temporal whitening by
      power-law adaptation in neocortical neurons. Nat Neurosci 16: 942-948.
      (uses a similar model of multi-timescale adaptation)

      [4] Grytskyy D, Tetzlaff T, Diesmann M and Helias M (2013) A unified view
      on weakly correlated recurrent networks. Front. Comput. Neurosci. 7:131.

      [5] Deger M, Schwalger T, Naud R, Gerstner W (2014) Fluctuations and
      information filtering in coupled populations of spiking neurons with
      adaptation. Physical Review E 90:6, 062704.

      [6] Gerstner W, Kistler WM, Naud R, Paninski L (2014) Neuronal Dynamics:
      From single neurons to networks and models of cognition.
      Cambridge University Press




**SeeAlso:**

-  `pp\_pop\_psc\_delta <../cc/pp_pop_psc_delta.html>`__
-  `iaf\_psc\_delta <../cc/iaf_psc_delta.html>`__
-  `iaf\_psc\_alpha <../cc/iaf_psc_alpha.html>`__
-  `iaf\_psc\_exp <../cc/iaf_psc_exp.html>`__
-  `iaf\_psc\_delta\_canon <../cc/iaf_psc_delta_canon.html>`__

**Source:**
::

    ./pp_psc_delta.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
