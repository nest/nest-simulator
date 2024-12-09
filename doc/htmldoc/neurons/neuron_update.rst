.. _neuron_update:

Neuron update algorithm
=======================

.. seealso::

   Description of neuron types available in NEST

   * :ref:`types_neurons`

Example of flowchart and algorithm for ``iaf_psc_exp``
------------------------------------------------------


This flowchart and associated algorithms can be applied to other models as well, with only
slight modifications.



.. grid::
   :gutter: 1

   .. grid-item::
      :columns: 5

      **Differential equations defining the model**

      .. math::

         \frac{dV(t)}{dt}=-\frac{V(t)-E_{L}}{\tau_{m}}+\frac{I_{syn}+I_{e}}{C_{m}}

      .. math::

         \tau_{syn}\frac{dI_{syn}(t)}{dt}=-I_{syn}(t)+\tau_{syn}\sum_{j}w_{j}\sum_{k}\delta(t-t_{j}^{k}-d_{j})


   .. grid-item::
      :columns: 7

      **Solution via exponential integration with propagators**

      .. math::

         P_{V,1}(t)	=\exp\left(-\frac{t}{\tau_{m}}\right) \\
         P_{V,2}(t)	=1-\exp\left(-\frac{t}{\tau_{m}}\right) \\
         P_{I,1}(t)	=\exp\left(-\frac{t}{\tau_{syn}}\right) \\
         P_{I,2}(t)	=1-\exp\left(-\frac{t}{\tau_{syn}}\right)

Update dynamics in :math:`\Delta t`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. grid::
   :gutter: 1

   .. grid-item::
    :columns: 7
    :class: sd-text-center

    .. image:: ../static/img/flowchart_mixedfont.png
      :width: 90%


   .. grid-item::
    :outline:
    :columns: 4

     :math:`I_{syn}`:  synaptic input current(s)

     :math:`V`:  membrane potential

     :math:`r`:  refractoriness timer

     :math:`\Theta`:  spike generation threshold

     :math:`V_{reset}`:  reset potential

     :math:`\tau_r`:  refractoriness duration

     :math:`t`:  time

     :math:`\Delta t`:  time resolution

Rate neurons
------------


.. grid::

   .. grid-item::
      :columns: 6

        .. image:: /static/img/rate_neuron_workflow.png


Compartmental neurons
---------------------

.. grid::

   .. grid-item::
      :columns: 6

        .. image:: /static/img/cm_default_workflow.png

Binary neurons
--------------

.. grid::

   .. grid-item::
      :columns: 6

        .. image:: /static/img/binary_workflow.png
