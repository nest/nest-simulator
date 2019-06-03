pp\_pop\_psc\_delta
============================

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

.. raw:: html

   <div class="wrap">

**Name:**
::

    pp_pop_psc_delta - Population of point process neurons with leaky
      integration of delta-shaped PSCs.

**Description:**
::



      pp_pop_psc_delta is an effective model of a population of neurons. The
      N component neurons are assumed to be spike response models with escape
      noise, also known as generalized linear models. We follow closely the
      nomenclature of [1]. The component neurons are a special case of
      pp_psc_delta (with purely exponential rate function, no reset and no
      random dead_time). All neurons in the population share the inputs that it
      receives, and the output is the pooled spike train.

      The instantaneous firing rate of the N component neurons is defined as

      rate(t) = rho_0 * exp( (h(t)   - eta(t))/delta_u ),

      where h(t) is the input potential (synaptic delta currents convolved with
      an exponential kernel with time constant tau_m), eta(t) models the effect
      of refractoriness and adaptation (the neuron's own spike train convolved with
      a sum of exponential kernels with time constants tau_eta), and delta_u
      sets the scale of the voltages.

      To represent a (homogeneous) population of N inhomogeneous renewal process
      neurons, we can keep track of the numbers of neurons that fired a certain
      number of time steps in the past. These neurons will have the same value of
      the hazard function (instantaneous rate), and we draw a binomial random
      number for each of these groups. This algorithm is thus very similar to
      ppd_sup_generator and gamma_sup_generator, see also [2].

      However, the adapting threshold eta(t) of the neurons generally makes the
      neurons non-renewal processes. We employ the quasi-renewal approximation
      [1], to be able to use the above algorithm. For the extension of [1] to
      coupled populations see [3].

      In effect, in each simulation time step, a binomial random number for each
      of the groups of neurons has to be drawn, independent of the number of
      represented neurons. For large N, it should be much more efficient than
      simulating N individual pp_psc_delta models.

      pp_pop_psc_delta emits spike events like other neuron models, but no more
      than one per time step. If several component neurons spike in the time step,
      the multiplicity of the spike event is set accordingly. Thus, to monitor
      its output, the multiplicity of the spike events has to be taken into
      account. Alternatively, the internal variable n_events gives the number of
      spikes emitted in a time step, and can be monitored using a multimeter.

      A journal article that describes the model and algorithm in detail is
      in preparation.




**Parameters:**
::



      The following parameters can be set in the status dictionary.


      N     int    - Number of represented neurons.
      tau_m   double - Membrane time constant in ms.
      C_m  double - Capacitance of the membrane in pF.
      rho_0   double - Base firing rate in 1/s.
      delta_u   double - Voltage scale parameter in mV.
      I_e     double - Constant input current in pA.
      tau_eta  list of doubles    - time constants of post-spike kernel
      in ms.
      val_eta     list of doubles    - amplitudes of exponentials in
      post-spike-kernel in mV.
      len_kernel  double - post-spike kernel eta is truncated after
      max(tau_eta) * len_kernel.


      The parameters correspond to the ones of pp_psc_delta as follows.

      c_1  =  0.0
      c_2  =  rho_0
      c_3    =  1/delta_u
      q_sfa  =  val_eta
      tau_sfa  =  tau_eta
      I_e  =  I_e

      dead_time    =  simulation resolution
      dead_time_random =  False
      with_reset   =  False
      t_ref_remaining  =  0.0




**Receives:**
::

    SpikeEvent, CurrentEvent, DataLoggingRequest



**Sends:**
::

    SpikeEvent



**References:**
::



      [1] Naud R, Gerstner W (2012) Coding and decoding with adapting neurons:
      a population approach to the peri-stimulus time histogram.
      PLoS Compututational Biology 8: e1002711.

      [2] Deger M, Helias M, Boucsein C, Rotter S (2012) Statistical properties
      of superimposed stationary spike trains. Journal of Computational
      Neuroscience 32:3, 443-463.

      [3] Deger M, Schwalger T, Naud R, Gerstner W (2014) Fluctuations and
      information filtering in coupled populations of spiking neurons with
      adaptation. Physical Review E 90:6, 062704.




**Author:**
::

    May 2014, Setareh, Deger


**SeeAlso:**

-  `pp\_psc\_delta <../cc/pp_psc_delta.html>`__
-  `ppd\_sup\_generator <../cc/ppd_sup_generator.html>`__
-  `gamma\_sup\_generator <../cc/gamma_sup_generator.html>`__

**Source:**
::

    ./pp_pop_psc_delta.h

.. raw:: html

   </div>

+----------------------------------------+-----------------------------------------+--------------------------------------------------+
| `NEST HelpDesk <../../index.html>`__   | `Command Index <../helpindex.html>`__   | `NEST Quick Reference <../../quickref.html>`__   |
+----------------------------------------+-----------------------------------------+--------------------------------------------------+

© 2004 `The NEST Initiative <https://www.nest-initiative.org>`__
