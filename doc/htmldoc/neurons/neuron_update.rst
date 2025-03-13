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
        \frac{dV(t)}{dt} &=\frac{-V(t)}{\tau}+\frac{I(t) + I_{ext}(t)}{C} \\ \\
        I(t) &=\sum_{i\in\mathbb{N}, t_i\le t }\sum_{k\in S_{t_i}}\hat{\iota}_k \iota(t-t_i) \\ \\
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
         * - :math:`I_{ext}(t)`
           - external current
         * - :math:`\tau_{syn}`
           - synaptic time constant


.. grid::
   :gutter: 1
   :class-container: sd-text-center

   .. grid-item:: **State variables**
      :columns: 4

      .. math::

        \begin{flalign}
         * V
         * I  \\
         * \frac{dI}{dt} \\
        \end{flalign}


   .. grid-item:: **Solution via exponential integration with propagators**
      :columns: 7

      p_**  are resulting from exact integration scheme -- see page --

      .. math::

        \begin{flalign}
         P_{11}(t)	&=\exp\left(-\frac{t}{\tau_{m}}\right) \\
         P_{21}(t)	&=\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{22}(t)	&=1-\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{30}(t)	&=\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{31}(t)	&=\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{32}(t)	&=1-\exp\left(-\frac{t}{\tau_{syn}}\right)
        \end{flalign}


----

integrate V box
V(t+ \deltat) =  V(t)
+ p_33 * V(t)
+ p_32 * \iota(t)
+ p_31 * d\iota(t) / dt
-\tau / C * (p33-1) * I_{ext}(t)


integrate I and dI/dt

I(t + \deltat ) = p_22 * I(t)
+ p_21 * dI(t)/dt

dI(t + \deltat) / dt  = p_11 * dI(t)/dt


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
