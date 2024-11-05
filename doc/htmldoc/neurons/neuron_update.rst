.. _neuron_update:

Neuron update algorithm
=======================

.. seealso::

   Description of neuron types available in NEST

   * :ref:`types_neurons`

Example of flowchart + algorithm for iaf_psc_exp
------------------------------------------------

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

      **Solution via exponential integration with propagatoras**

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

Version 2
---------


.. grid::
   :gutter: 1

   .. grid-item::
    :columns: 7
    :class: sd-text-center

    .. image:: ../static/img/flowchart_sansfont.png
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


Order of operations
-------------------



1. Subthreshold dynamics
2. Update neuron dynamics from incoming spikes
3. Test for refractoriness
4. Test for threshold
5. if spike, reset membrane potential

(the spiking neuron is an intermediate not seen outside of time step, as the state of neuron
is reset and then the info is sent around)
subthreshold dynamics are integrated throughout the alorithm ??


What about other parameters?

.. grid::

   .. grid-item::
      :columns: 6

        .. image:: /static/img/neuron_update.svg

Where ``y`` is a vector of state variables, which is updated according to the homogeneous differetial
equation, that is, in the absence of input.

Point process
-------------

.. grid::

   .. grid-item::
      :columns: 6

        .. image:: /static/img/pp_workflow.png


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


pp_psc_delta
-------------

Update membrane potential basedon input current and spikes
Update the adaptive threshold using SFA and time constants
If not in refractory period
  calculate the firing rate (Calculate instantaneous rate from transfer function)
  if firing rate positive

      draw a random number of spikes

  if spike
   set new dead time
  send spike event

   update the refractory period
  send spike events
  ? reset membrane potential, if applicable
refractory decremented
set new input current for next update cycle
log membrane potential

pp_psc_delta_mc_urbanczik
---------------------------

The neuron state is integrated over the simulation step using adaptive step size control.

Incoming spike are added to the soma and dendritic compartments
> Soma (update excitatory and inhibitory conductances)
> Dendrites (update excitatory and inhibitory currents)


If neuron not in refractory state
   calculate the firing rate based on the membrane potential
   if the firing rate is positive

      draw a random number of spikes from a poisson distribution

  if spikes occur set the refractory period and send spike events

else (if neuron in refractory state)
  decrement the refractory counter

store membrane potential (dendritic) for Urb. Senn plasticity
set new input currents

Check Refractory State: If the neuron is not in a refractory state (S_.r_ == 0):
Rate Calculation: Compute the firing rate based on the membrane potential (S_.y_[State_::V_M]).
Poisson Spike Generation: Draw a random number of spikes from a Poisson distribution if the rate is positive.
Spike Event: If spikes occur, set the refractory period (S_.r_) and send spike events.
Within Refractory Period: If the neuron is in a refractory state, decrement the refractory counter (S_.r_).
Logging and State Updates:

Urbanczik-Senn Plasticity: Store the dendritic membrane potential for plasticity rules.
Update Input Currents: Set new input currents from the current buffer for each compartment.
Log State Data: Record the state data using the logger (B_.logger_.record_data
