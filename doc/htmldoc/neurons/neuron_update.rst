.. _neuron_update:

Neuron update algorithms
=========================



.. _sec_gen_steps:

General steps for the neuron update
-----------------------------------

Updating the state of a neuron from time `t` to some later time :math:`t+\Delta{t}` generally involves a number of distinct steps.


.. grid::
   :gutter: 1

   .. grid-item::
    :columns: 5
    :class: sd-text-right

    .. image:: ../static/img/autonomous_dynamics.svg
      :width: 50%

   .. grid-item-card::
    :columns: 7

    The autonomous dynamics describe the neuron behavior in the absence of stimulation or in the presence of constant inputs,
    and exclude all interaction events such as spikes from other neurons or devices.
    We choose an appropriate numerical solver for the dynamics, and in the case of linear differential equations,
    we use :ref:`exact integration <exact_integration>`.

   .. grid-item::
    :columns: 5
    :class: sd-text-right

    .. image:: ../static/img/incoming_events.svg
      :width: 50%

   .. grid-item-card::
    :columns: 7

    Events mimic time dependent inputs
    such as spikes, changing currents etc.

   .. grid-item::
    :columns: 5
    :class: sd-text-right

    .. image:: ../static/img/handle_exceptions.svg
      :width: 50%

   .. grid-item-card::
    :columns: 7

    Inspect the state to check if conditions
    have changed (e.g., is the neuron still refractory  or has the threshold been crossed?).

   .. grid-item::
    :columns: 5
    :class: sd-text-right

    .. image:: ../static/img/outgoing_events.svg
      :width: 50%

   .. grid-item-card::
    :columns: 7

    Increment state variables associated with the post-spike dynamics
    (such as refractoriness timers or adaptation variables), and send events (e.g. spike).

.. note::

    The exact order of the steps for the neuron update represented here depends on the neuron model,
    and in NEST, we optimize the steps to minimize computational costs.


Example of flowchart and algorithm for ``iaf_psc_alpha``
--------------------------------------------------------

Here we expand the general steps described above to explain the update steps in the
neuron model ``iaf_psc_alpha``.


.. grid::
   :gutter: 1
   :class-container: sd-text-center

   .. grid-item:: **Model Equations**
      :columns: 5

      .. math::

        \begin{flalign}
        \dot{V}(t) &=\frac{-V(t)}{\tau_{\text{m}}}+\frac{I(t) + I_{\text{ext}}(t)}{C} \\ \\
        I(t) &=\sum_{i\in\mathbb{N}, t_i\le t }\sum_{k\in S_{t_i}}\hat{\iota}_k \iota(t-t_i) \\ \\
        \iota (t) &= \frac{e}{\tau_{\text{syn}}}t e^{-t/\tau_{\text{syn}}}
        \end{flalign}

      Spike emission at time :math:`t_{i}` if :math:`V(t_{i})\geq\theta`

      For :math:`t\in(t_{i},t_{i}+\tau_{\text{r}}]`: :math:`V(t)=V_{\text{reset}}`

   .. grid-item:: **Model Parameters**
      :columns: 4

      .. list-table::

         * - :math:`\tau_{\text{m}}`
           - membrane time constant
         * - :math:`C`
           - membrane capacitance
         * - :math:`\hat{\iota}_{\text{k}}`
           - synaptic weight of presynaptic neuron k
         * - :math:`I_{\text{ext}}(t)`
           - external current
         * - :math:`\tau_{\text{syn}}`
           - synaptic time constant
         * - :math:`V_{\text{reset}}`
           - reset potential
         * - :math:`\tau_{\text{r}}`
           - refractoriness duration
         * - :math:`t`
           - time
         * - :math:`\Delta t`
           - time resolution
         * - :math:`\theta`
           - spike generation threshold

   .. grid-item:: **State variables**
      :columns: 3

      .. list-table::

         * - :math:`V`
           - membrane potential
         * - :math:`I`
           - Synaptic input currents
         * - :math:`\dot{I}`
           - temporal derivative of current
         * - :math:`r`
           - refractoriness timer



.. grid::
   :gutter: 1

   .. grid-item:: **Flowchart**
     :columns: 6

     The colors indicated on the flowchart match with the basic steps :ref:`described above <sec_gen_steps>`.

     .. image:: ../static/img/mimixedfont_neuronupdate.svg
       :width: 90%

   .. grid-item:: **Propagators for solution with exact integration**
     :columns: 6

     Propagators (`P_`)  result from the exact integration scheme explained here: :doc:`/neurons/exact-integration`.

     .. math::

        \begin{flalign}
        P_{11} & =e^{-\Delta t/\tau_{\text{syn}}}\\
        P_{21} & =\Delta t\,e^{-\Delta t/\tau_{\text{syn}}}\\
        P_{22} & =e^{-\Delta t/\tau_{\text{syn}}}\\
        P_{31} & =\frac{1}{C}\tau_{\text{eff}}e^{-\Delta t/\tau_{\text{eff}}}\left(\tau_{\text{eff}}\left[e^{-\Delta t/\tau_{\text{eff}}}-1\right]-\Delta t\right)\\
        P_{32} & =\frac{1}{C}\tau_{\text{eff}}e^{-\Delta t/\tau_{\text{syn}}}\left[e^{-\Delta t/\tau_{\text{eff}}}-1\right]\\
        P_{33} & =e^{-\Delta t/\tau_{\text{m}}}\\
        \tau_{\text{eff}} & =\tau_{\text{syn}}\tau_{\text{m}}/\left(\tau_{\text{m}}-\tau_{\text{syn}}\right)
        \end{flalign}

This flowchart and associated algorithms can be applied to other models as well, with only
slight modifications. You can find descriptions of all the neuron types available in NEST here: :ref:`types_neurons`.
