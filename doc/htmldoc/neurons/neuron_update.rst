.. _neuron_update:

Neuron update algorithms
=========================

.. seealso::

   Description of neuron types available in NEST

   * :ref:`types_neurons`

Generic flowchart for steps in the neuron update
------------------------------------------------


.. grid::
   :gutter: 1

   .. grid-item::
    :columns: 6
    :class: sd-text-center

    .. image:: ../static/img/generic_neuron_update.svg
      :width: 50%

   .. grid-item::
    :columns: 6

    **Autonomous dynamics:**  describes the neuron behaviour in the absence of stimulation or
    in the presence of constant inputs
    and excludes all interaction events such as spikes from other neurons or devices

    **Event** - mimic time dependent inputs such as spikes, changing currents etc.

Example of flowchart and algorithm for ``iaf_psc_alpha``
--------------------------------------------------------


This flowchart and associated algorithms can be applied to other models as well, with only
slight modifications.



.. grid::
   :gutter: 1
   :class-container: sd-text-center

   .. grid-item:: **Model Equations**
      :columns: 6

      .. math::

        \begin{flalign}
        \frac{dV}{dt} &=\frac{-V}{\tau}+\frac{I}{C} \\ \\
        I(t) &=\sum_{i\in\mathbb{N}, t_i\le t }\sum_{k\in S_{t_i}}\hat{\iota}_k \iota(t-t_i)+I_{\text{ext}} \\ \\
        \iota (t) &= \frac{e}{\tau_{syn}}t e^{-t/\tau_{\text{syn}}}.
        \end{flalign}


   .. grid-item:: **Model Parameters**
      :columns: 6

      .. list-table::

         * - :math:`\tau`
           - membrane time constant
         * - :math:`C`
           - membrane capacitance
         * - :math:`\iota_k`
           - synaptic weight of presynaptic neuron k
         * - :math:`I_{ext}`
           - external constant current
         * - :math:`\tau_{syn}`
           - synaptic time constant


.. grid::
   :gutter: 1
   :class-container: sd-text-center

   .. grid-item:: **State variables**
      :columns: 4

      .. math::

        \begin{flalign}
         y1 &=  \frac{d\iota}{dt} + \frac{\iota}{\tau_{syn}} \\
         y2 &= \iota \\
         y3 &= V
        \end{flalign}


   .. grid-item:: **Solution via exponential integration with propagators**
      :columns: 7

      .. math::

        \begin{flalign}
         P_{y1,1}(t)	&=\exp\left(-\frac{t}{\tau_{m}}\right) \\
         P_{y1,2}(t)	&=1-\exp\left(-\frac{t}{\tau_{m}}\right) \\
         P_{y2,1}(t)	&=\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{y2,2}(t)	&=1-\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{y3,1}(t)	&=\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{y3,2}(t)	&=1-\exp\left(-\frac{t}{\tau_{syn}}\right)
        \end{flalign}


Update dynamics in :math:`\Delta t`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. grid::
   :gutter: 1

   .. grid-item::
    :columns: 7
    :class: sd-text-center

    .. image:: ../static/img/mixedfont-flowchart.png
      :width: 90%


   .. grid-item::
    :columns: 5

    .. list-table::

     * - :math:`I_{syn}`
       - synaptic input current(s)
     * - :math:`V`
       - membrane potential
     * - :math:`r`
       - refractoriness timer
     * - :math:`\Theta`
       - spike generation threshold
     * - :math:`V_{reset}`
       - reset potential
     * - :math:`\tau_r`
       - refractoriness duration
     * - :math:`t`
       - time
     * - :math:`\Delta t`
       - time resolution

..      old ones

      .. math::

         \frac{dV(t)}{dt}=-\frac{V(t)-E_{L}}{\tau_{m}}+\frac{I_{syn}+I_{e}}{C_{m}}

      .. math::

         \tau_{syn}\frac{dI_{syn}(t)}{dt}=-I_{syn}(t)+\tau_{syn}\sum_{j}w_{j}\sum_{k}\delta(t-t_{j}^{k}-d_{j})
