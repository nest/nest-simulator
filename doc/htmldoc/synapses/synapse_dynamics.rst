Synapse dynamics
================

Models for synaptic dynamics are distinguished by two different
features:

#. whether they describe a current (psc) or conductance (cond)

#. the temporal response to an incoming spike.

Current-based synapses
----------------------

Current-based synapses model an input current :math:`I_{syn}(t)` that is
fed as an additive term in the equation for the subthreshold membrane
potential dynamics (:math:`V(t)`) of neurons:

.. math:: \frac{dV(t)}{dt}=f(V(t))+\frac{1}{C_{m}}\,I_{syn}(t)\,.

The effect of the synaptic input therefore is not depending on the state
(membrane potential) of the neuron. :math:`C_{m}` is the neuronal
capacitance, and the function :math:`f(V(t))` summarizes internal
membrane properties, such as leak potentials.

Conductance-based synapses
--------------------------

Conductance-based synapses model input currents indirectly via the
dynamics of a conductance :math:`g_{syn}(t)`. This conductance is
multiplied with the membrane potential :math:`V(t)` of the neuron to
yield the current :math:`g_{syn}(t)\,(V(t)-V_{r})`:

.. math:: \tau\frac{dV(t)}{dt}=f(V(t))+\frac{1}{C_{m}}\,g_{syn}(t)\,(V(t)-V_{r})\,.

The input therefore has a multiplicative effect that depends on the
state (membrane potential distance from reversal potential
:math:`V_{r}`) of the neuron.

Response dynamics
-----------------

The synaptic dynamics is specified by a kernel :math:`k(t)` that
describes the response to an incoming spike. The dynamics in general is
given by

.. math::

   \begin{aligned}
   \{I_{syn}(t),g_{syn}(t)\} & \ni x(t)=\sum_{j}w_{j}\,(k\ast s_{j})(t)\end{aligned}

where :math:`\ast` denotes a temporal convolution with presynaptic spike
trains :math:`s_{j}(t)=\sum_{k}\delta(t-t_{j}^{k})` defined by spike
times :math:`t_{j}^{k}`. :math:`w_{j}` denotes the synaptic weight to
presynaptic neuron :math:`j`

.. _delta_synapse:

Delta synapses
~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2

      .. image:: /static/img/delta_nn.svg

   .. grid-item::
      :columns: 10

      For delta synapses (delta), there is no synaptic filtering taking place.
      This corresponds to a kernel

      .. math:: k(t)=\delta(t)

      that is a delta distribution.

.. _exp_synapse:

Exponential synapses
~~~~~~~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2

      .. image:: /static/img/exp_nn.svg

   .. grid-item::
      :columns: 10
      :class: sd-d-flex-row sd-align-major-center

      .. tab-set::

       .. tab-item:: General info

          The filter kernel for exponential synapses (exp) is an exponential

          .. math:: k(t)=\exp(-t/\tau_{syn})\Theta(t)

          with Heaviside function :math:`\Theta(t)=0` for :math:`t<0` and
          :math:`\Theta(t)=1` for :math:`t\geq0`, and synaptic time constant
          :math:`\tau_{syn}`. The kernel is normalized to have a peak value
          :math:`k(0)=1`\ (TODO check if correct). The kernel corresponds to the
          solution of the ordinary first-order differential equation

          .. math:: \tau_{syn}\frac{dk(t)}{dt}=-k(t)+\tau_{syn}\delta(t)\label{eq:exp_dyn}

          with Dirac input at :math:`t=0` and initial condition
          :math:`x(-\infty)=0`.

       .. tab-item:: Technical details

          The synaptic filtering is implemented with an additional state variable
          for the synaptic current or conductance that follows the dynamics of
          `[exp_dyn] <#exp_dyn>`__ with spiking input from all presynaptic
          neurons. This dynamics is solved using exact integration (link to exact
          integration page) (ref to Rotter and Diesmann 1999).

.. _alpha_synapse:

Alpha synapses
~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center


      .. image:: /static/img/alpha2.svg

   .. grid-item::
      :columns: 10

      .. tab-set::

       .. tab-item:: General info

          Alpha synapses (alpha) are defined by the filter kernel

          .. math:: k(t)=\frac{e}{\tau_{syn}}t\exp(-t/\tau_{syn})\Theta(t)

          with Euler number :math:`e`, Heaviside function :math:`\Theta(t)=0` for
          :math:`t<0` and :math:`\Theta(t)=1` for :math:`t\geq0`, and synaptic
          time constant :math:`\tau_{syn}`. The kernel is normalized to have a
          peak value :math:`k(\tau_{syn})=1` (TODO check if correct, it is correct
          for iaf_cond_alpha). The kernel corresponds to the solution of the
          system of ordinary differential equations

          .. math::

             \begin{aligned}
             \tau_{syn}\frac{dk(t)}{dt} & =-k(t)+e\,\kappa(t)\label{eq:alpha1}\\
             \tau_{syn}\frac{d\kappa(t)}{dt} & =-\kappa(t)+\tau_{syn}\delta(t)\label{eq:alpha2}\end{aligned}

          with Dirac input at :math:`t=0` and initial conditions
          :math:`\kappa(-\infty)=k(-\infty)=0`. The alpha kernel therefore
          represents the consecutive application of two exponential filter
          kernels.

          Note that the above system of differential equations is equivalent to
          the second-order differential equation

          .. math:: \frac{d^{2}k(t)}{dt^{2}}+(a+b)\frac{dk(t)}{dt}+(ab)k(t)=\frac{e}{\tau_{syn}}\,\delta(t)

          with :math:`a=b=1/\tau_{syn}` and initial condition :math:`k(-\infty)=0`
          and :math:`\frac{dk}{dt}(-\infty)=0` (ref Rotter Diesmann 1999). The
          solution to this equation for :math:`a=b` is called alpha function which
          gives rise to the name alpha synapse.


       .. tab-item:: Technical details

          The synaptic filtering is implemented with two additional state
          variables related to the synaptic current or conductance. These
          variables follow the dynamics of `[alpha1] <#alpha1>`__ and
          `[alpha2] <#alpha2>`__ and are solved using exact integration (link to
          exact integration page) (ref to Rotter and Diesmann 1999).

.. _beta_synapse:

Beta synapses
~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/beta2.svg

   .. grid-item::
      :columns: 10

      .. tab-set::

       .. tab-item:: General info

          Beta synapses are defined by a kernel that is the difference of two
          exponentials (TODO check how it is normalized in NEST, the description
          at
          https://nest-simulator.readthedocs.io/en/stable/models/iaf_cond_beta.html
          is strange because the kernel does not have a peak at
          :math:`t=\tau_{syn,rise}`. TODO discuss):

          .. math:: k(t)=\frac{\tau_{syn,decay}}{\tau_{syn,rise}-\tau_{syn,decay}}\left[\exp(-t/\tau_{syn,rise})-\exp(-t/\tau_{syn,decay})\right]\Theta(t)\label{eq:beta_kernel}

          This function allows for independent rise and decay times, as quantified
          by :math:`\tau_{syn,rise}` and :math:`\tau_{syn,decay}`, respectively.
          The kernel corresponds to the solution of the system of ordinary
          differential equations

          .. math::

             \begin{aligned}
             \tau_{syn,rise}\frac{dk(t)}{dt} & =-k(t)+\kappa(t)\label{eq:beta1}\\
             \tau_{syn,decay}\frac{d\kappa(t)}{dt} & =-\kappa(t)+\tau_{syn,decay}\delta(t)\label{eq:beta2}\end{aligned}

          with Dirac input at :math:`t=0` and initial conditions
          :math:`\kappa(-\infty)=k(-\infty)=0`. Note that this system of
          differential equations is equivalent to the second-order differential
          equation

          .. math:: \frac{d^{2}k(t)}{dt^{2}}+(a+b)\frac{dk(t)}{dt}+(ab)k(t)=\frac{1}{\tau_{syn,rise}}\delta(t)

          with :math:`a=1/\tau_{syn,rise}\neq b=1/\tau_{syn,decay}` and initial
          condition :math:`k(-\infty)=0` and :math:`\frac{dk}{dt}(-\infty)=0` (ref
          Rotter Diesmann 1999). For the case
          :math:`\tau_{syn,rise}=\tau_{syn,decay}` please use the alpha synapse
          model instead. Even though the limit
          :math:`\tau_{syn,rise}\rightarrow\tau_{syn,decay}` is well defined and
          coincides with the alpha synapse, there can be numerical issues as both
          numerator and denominator in the kernel `[beta_kernel] <#beta_kernel>`__
          vanish in this limit.


       .. tab-item:: Technical details

          The synaptic filtering is implemented with two additional state
          variables related to the synaptic current or conductance. These
          variables follow the dynamics of `[beta1] <#beta1>`__ and
          `[beta2] <#beta2>`__ and are solved using exact integration (link to
          exact integration page) (ref to Rotter and Diesmann 1999).

Weight dynamics
===============

Above we discussed the postsynaptic dynamics that is elicited after an
incoming spike with weight :math:`w_{j}`. Next, we study different
models for how the weight of the connection can change over time.

Static connections
------------------

Here the weight stays constant over time.

Synaptic plasticity
-------------------

LTP and LTD
~~~~~~~~~~~

STDP
~~~~

Voltage-based plasticity
~~~~~~~~~~~~~~~~~~~~~~~~

Structural plasticity
---------------------

Here the weight of existing connections not only change, but also new
connections are being formed over time and existing connections are
being removed.
