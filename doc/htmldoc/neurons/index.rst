.. _neurons_index:

All about neurons in NEST
=========================


.. grid:: 1 1 3 3
   :gutter: 1

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white


       |neuron|  Neuron types in NEST
       ^^^

       This page highlights the numerous types of neuron models and their various mechanisms available in NEST, along with guides to specific
       implementations

       * :ref:`types_neurons`

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white


       |python| Using neurons in PyNEST scripts
       ^^^

       * :ref:`Manipulating nodes (neurons and devices) <node_handles>`: understand basic functionality of nodes
       * :ref:`param_ex`: explore how to use parameter objects in NEST

       How to handle neurons as Python objects in NEST scripts.
       PyNEST scripts, handling neurons, Python

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white


       |math| Find a specific model
       ^^^


       For details on individual models, please take alook at our model directory, where you can
       select various tags and refine the results to your choosing, or look up our A-Z list.

       * :doc:`/models/index`



----


.. _types_neurons:

Types of neurons
-----------------

.. grid::
   :gutter: 1

   .. grid-item-card::
     :columns: 3
     :link: spiking_neurons
     :link-type: ref

     .. image:: /static/img/spiking_neurons.svg


   .. grid-item-card::
     :columns: 3
     :link: multistate
     :link-type: ref


     .. image:: /static/img/multistate.svg

   .. grid-item-card::
     :columns: 3
     :link: rate_neurons
     :link-type: ref


     .. image:: /static/img/rate_neurons.svg

   .. grid-item-card::
     :columns: 3
     :link: neuron_astrocyte
     :link-type: ref


     .. image:: /static/img/astrocyte.svg

.. _spiking_neurons:

Spiking neurons
---------------


Geometry
~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item-card::
      :columns: 3

      .. image:: /static/img/point_neurons.svg

   .. grid-item::
      :columns: 9

      The most common type of neuron model in NEST.
      Point neurons are simplified models of biological neurons that represent
      the neuron as a single point where all inputs are processed,
      without considering their complex structure (such as dendrites and axons).

.. grid:: 1 2 2 2

   .. grid-item-card::
      :columns: 3

      .. image:: /static/img/mc_neurons.svg

   .. grid-item::
      :columns: 9

      Multi-compartmental neurons

      Neurons are subdivided into multiple compartments, like soma, dendrite, and axon, in which inputs can be received
      across all components.
      .. NEAT

      .. dropdown:: Multi-compartment neurons

         * iaf_cond_alpha_mc – Multi-compartment conductance-based leaky integrate-and-fire neuron model
         * pp_cond_exp_mc_urbanczik – Two-compartment point process neuron with conductance-based synapses
         * cm_default – A neuron model with user-defined dendrite structure. Currently, AMPA, GABA or AMPA+NMDA receptors.




Spiking mechanism
~~~~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item-card::
      :columns: 3

      .. image::  /static/img/integrate_and_fire.svg

   .. grid-item::
      :columns: 9

      .. tab-set::

        .. tab-item:: General info
          :selected:

          The neuron's membrane potential integrates incoming synaptic inputs over time.
          When the membrane potential reaches a certain threshold,
          the neuron "fires" an action potential

        .. tab-item:: Integration details

          * Exact integration guide?
          * IAF_integration_singularity

.. grid:: 1 2 2 2

   .. grid-item-card::
      :columns: 3

      .. image:: /static/img/parrot_neurons.svg

   .. grid-item::  Parrot neurons
      :columns: 9

      Neurons that repeat incoming spikes. Applications:

      .. dropdown:: Parrot neurons

        parrot_neuron – Neuron that repeats incoming spikes

        parrot_neuron_ps – Neuron that repeats incoming spikes - precise spike timing version

.. grid:: 1 2 2 2

   .. grid-item-card::
      :columns: 3

      .. image::  /static/img/hodgkinhuxley.svg

   .. grid-item::
      :columns: 9

      .. tab-set::

          .. tab-item:: General info

            Hodgkin-Huxley model provides a detailed representation of the electrical
            activity of neurons by explicitly modeling the ionic currents through
            voltage-gated sodium and potassium channels, along with a leak current.
            This model is particularly useful for studying the detailed mechanisms of action potential generation and propagation,

          .. tab-item:: Integration details

            * hh_details

.. grid::

   .. grid-item::
     :class: sd-align-major-end
     :columns: 9
     :margin: 0 0 auto 0

     .. dropdown:: Hodgkin Huxley neurons

       * hh_cond_beta_gap_traub – Hodgkin-Huxley neuron with gap junction support and beta function synaptic conductances
       * hh_cond_exp_traub – Hodgkin-Huxley model for Brette et al (2007) review
       * hh_psc_alpha – Hodgkin-Huxley neuron model
       * hh_psc_alpha_clopath – Hodgkin-Huxley neuron model with support for Clopath plasticity
       * hh_psc_alpha_gap – Hodgkin-Huxley neuron model with gap-junction support


.. grid:: 1 2 2 2

   .. grid-item-card::
      :columns: 3

      .. image::  /static/img/point_process.svg

   .. grid-item::
      :columns: 9

      a spike-response model with escape-noise?

      .. dropdown:: Point process

        pp_cond_exp_mc_urbanczik – Two-compartment point process neuron with conductance-based synapses

        pp_psc_delta – Point process neuron with leaky integration of delta-shaped PSCs




Input mechanism
~~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item-card::
     :columns: 3

     .. image::  /static/img/current_based.svg

   .. grid-item::
     :columns: 9

     Synaptic inputs are represented as fixed currents (I) injected into the neuron.
     These currents are independent of the membrane potential
     and are directly added to the neuron's equation governing voltage dynamics.
     Computationally less intensive than conductance-based models.

.. grid:: 1 2 2 2

   .. grid-item-card::
     :columns: 3

     .. image::  /static/img/conductance_based.svg

   .. grid-item::
     :columns: 9

     Synaptic inputs are represented as changes in membrane conductance (`g`).
     These changes depend on the opening and closing of ion channels,
     which are often modeled based on voltage or neurotransmitter binding.
     These models capture more realistic synaptic behavior, as they account for the varying impact of
     synaptic inputs depending on the membrane potential, which can change over time.

Adaptation mechanism
~~~~~~~~~~~~~~~~~~~~

.. grid::

  .. grid-item-card::
      :columns: 3

      .. image:: /static/img/adaptive_threshold.svg

  .. grid-item::

      .. tab-set::

        .. tab-item:: General info

            Unlike a fixed threshold, an adaptive threshold increases temporarily following each spike and
            gradually returns to its baseline value over time. This mechanism models phenomena
            such as spike-frequency adaptation, where a neuron's responsiveness decreases with sustained
            high-frequency input, allowing for more realistic simulations of neuronal behavior.

            .. dropdown:: Adaptive threshold

                  * aeif_cond_alpha – Conductance based exponential integrate-and-fire neuron model
                  * aeif_cond_alpha_astro – Conductance based exponential integrate-and-fire neuron model with support for neuron-astrocyte interactions
                  * aeif_cond_alpha_multisynapse – Conductance based adaptive exponential integrate-and-fire neuron model
                  * aeif_cond_beta_multisynapse – Conductance based adaptive exponential integrate-and-fire neuron model
                  * aeif_cond_exp – Conductance based exponential integrate-and-fire neuron model
                  * aeif_psc_alpha – Current-based exponential integrate-and-fire neuron model
                  * aeif_psc_delta – Current-based adaptive exponential integrate-and-fire neuron model with delta synapse
                  * aeif_psc_delta_clopath – Adaptive exponential integrate-and-fire neuron
                  * aeif_psc_exp – Current-based exponential integrate-and-fire neuron model
                  * amat2_psc_exp – Non-resetting leaky integrate-and-fire neuron model with exponential PSCs and adaptive threshold
                  * mat2_psc_exp – Non-resetting leaky integrate-and-fire neuron model with exponential PSCs and adaptive threshold
                  * ht_neuron – Neuron model after Hill & Tononi (2005)


        .. tab-item:: Implementation details

           * aeif_models_implementation

Precise spike timing
~~~~~~~~~~~~~~~~~~~~

.. grid::

  .. grid-item-card::
      :columns: 3

      .. image:: /static/img/precise_spiking.svg

  .. grid-item::

      More computataionally heavy, but provide better resolution to spike times than a non-precise model.
      See :ref:`our guide on precise spike timing <sim_precise_spike_times>`.

      .. dropdown:: Precise spike timing

         * iaf_psc_alpha_ps – Current-based leaky integrate-and-fire neuron with alpha-shaped postsynaptic currents using regula falsi method for approximation of threshold crossing
         * iaf_psc_exp_ps – Current-based leaky integrate-and-fire neuron with exponential-shaped postsynaptic currents using regula falsi method for approximation of threshold crossing

         * iaf_psc_exp_ps_lossless – Current-based leaky integrate-and-fire neuron with exponential-shaped postsynaptic currents predicting the exact number of spikes using a state space analysis
         * iaf_psc_delta_ps – Current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents - precise spike timing version

|

----

|


.. _rate_neurons:

Rate neurons
------------

.. grid::

   .. grid-item-card::
     :columns: 3

     .. image:: /static/img/rate_neurons.svg

   .. grid-item::

     Rate neurons can approximate biologically realistic neurons but they are also used in artificial learning
     (aka recurrent neural networks RNNs)


     Averaged Activity: Rate neuron models focus on the average level of neural activity over time, rather than individual spikes.
     They are useful for studying steady-state behaviors, population dynamics, and the
     overall flow of information in neural networks without requiring detailed temporal resolution.

.. grid::

   .. grid-item-card::
      :columns: 3

      Input noise

   .. grid-item-card::
      :columns: 3

      Output noise

.. grid::

   .. grid-item-card::
      :columns: 3

      Mean-field theory

   .. grid-item::


     .. tab-set::

      .. tab-item:: General info

        Mean-field theory

        * siegert neuron

      .. tab-item:: Integration details

        * siegert_neuorn_integration

* rate_transormer_node (like parrot)

.. dropdown:: Rate neurons

   *    rate_neuron_ipn – Base class for rate model with input noise
   *    rate_neuron_opn – Base class for rate model with output noise
   *    rate_transformer_node – Rate neuron that sums up incoming rates and applies a nonlinearity specified via the template
   *    siegert_neuron – model for mean-field analysis of spiking networks
   *    sigmoid_rate – Rate neuron model with sigmoidal gain function
   *    sigmoid_rate_gg_1998 – rate model with sigmoidal gain function
   *    tanh_rate – rate model with hyperbolic tangent non-linearity
   *    threshold_lin_rate – Rate model with threshold-linear gain function
   *    gauss_rate – Rate neuron model with Gaussian gain function
   *    lin_rate – Linear rate model

|

----

|

.. _multistate:

Multi-state neurons
-------------------

.. grid::

   .. grid-item-card::
     :columns: 3

     .. image:: /static/img/multistate.svg

   .. grid-item::

     Neurons with two or three states. Simplest neuron models with threshold activation. Binary neurons have
     On / Off behavior   used in theoretical and disease theory.

.. dropdown::

  *   mcculloch_pitts_neuron – Binary deterministic neuron with Heaviside activation function
  *   erfc_neuron – Binary stochastic neuron with complementary error function as activation function
  *   ginzburg_neuron – Binary stochastic neuron with sigmoidal activation function

|

----

|

.. _neuron_astrocyte:

Neuron-astrocyte interactions
-----------------------------

.. grid::

   .. grid-item-card::
     :columns: 3

     .. image:: /static/img/astrocyte.svg

   .. grid-item::

     .. tab-set::

      .. tab-item:: General info

         Astrocytes

        .. dropdown:: Astrocyte models

             * aeif_cond_alpha_astro – Conductance based exponential integrate-and-fire neuron model with support for neuron-astrocyte interactions
             * astrocyte_lr_1994 – An astrocyte model based on Li & Rinzel (1994)
             * sic_connection – Synapse type for astrocyte-neuron connections

      .. tab-item:: Implementation details

         * astrocyte_model_implementation


Guides
------

.. toctree::
  :maxdepth: 1
  :glob:
  :hidden:

  *

.. .. dropdown:: integrate-and-fire (no adaptive threshold, no precise neurons)

     * eprop_iaf_adapt_bsshslm_2020 – Current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents and threshold adaptation for e-prop plasticity
     * eprop_iaf_bsshslm_2020 – Current-based leaky integrate-and-fire neuron model with delta-shaped postsynaptic currents for e-prop plasticity
     * eprop_readout_bsshslm_2020 – Current-based leaky integrate readout neuron model with delta-shaped postsynaptic currents for e-prop plasticity
     * gif_cond_exp – Conductance-based generalized integrate-and-fire neuron (GIF) model (from the Gerstner lab)
     * gif_cond_exp_multisynapse – Conductance-based generalized integrate-and-fire neuron (GIF) with multiple synaptic time constants (from the Gerstner lab)
     * gif_pop_psc_exp – Population of generalized integrate-and-fire neurons (GIF) with exponential postsynaptic currents and adaptation (from the Gerstner lab)
     * gif_psc_exp – Current-based generalized integrate-and-fire neuron (GIF) model (from the Gerstner lab)
     * gif_psc_exp_multisynapse – Current-based generalized integrate-and-fire neuron (GIF) model with multiple synaptic time constants (from the Gerstner lab)
     * glif_cond – Conductance-based generalized leaky integrate and fire (GLIF) model (from the Allen Institute)
     * glif_psc – Current-based generalized leaky integrate-and-fire (GLIF) models (from the Allen Institute)
     * glif_psc_double_alpha – Current-based generalized leaky integrate-and-fire (GLIF) models with double alpha-function (from the Allen Institute)
     * ht_neuron – Neuron model after Hill & Tononi (2005)
     * iaf_chs_2007 – Spike-response model used in Carandini et al. 2007
     * iaf_chxk_2008 – Conductance-based leaky integrate-and-fire neuron model supporting precise spike times used in Casti et al. 2008
     * iaf_cond_alpha – Simple conductance based leaky integrate-and-fire neuron model
     * iaf_cond_beta – Simple conductance based leaky integrate-and-fire neuron model
     * iaf_cond_exp – Simple conductance based leaky integrate-and-fire neuron model
     * iaf_cond_exp_sfa_rr – Conductance based leaky integrate-and-fire model with spike-frequency adaptation and relative refractory mechanisms
     * iaf_psc_alpha – Leaky integrate-and-fire model with alpha-shaped input currents
     * iaf_psc_alpha_multisynapse – Leaky integrate-and-fire neuron model with multiple ports
     * iaf_psc_delta – Leaky integrate-and-fire model with delta-shaped input currents
     * iaf_psc_exp – Leaky integrate-and-fire neuron model with exponential PSCs
     * iaf_psc_exp_htum – Leaky integrate-and-fire model with separate relative and absolute refractory period
     * iaf_psc_exp_multisynapse – Leaky integrate-and-fire neuron model with multiple ports
     * iaf_tum_2000 – Leaky integrate-and-fire neuron model with exponential PSCs and integrated short-term plasticity synapse
     * ignore_and_fire – Ignore-and-fire neuron model for generating spikes at fixed intervals irrespective of inputs
     * izhikevich – Izhikevich neuron model


.. |nav| image:: /static/img/GPS-Settings-256_nest.svg
.. |script| image:: /static/img/script_white.svg
      :scale: 20%
.. |start| image:: /static/img/start_white.svg
      :scale: 40%
.. |user| image:: /static/img/020-user.svg
.. |teacher| image:: /static/img/014-teacher.svg
.. |admin| image:: /static/img/001-shuttle.svg
.. |dev| image:: /static/img/dev_orange.svg
.. |nestml| image:: /static/img/nestml-logo.png
      :scale: 15%
.. |synapse| image:: /static/img/synapse_white.svg
.. |neuron|  image:: /static/img/neuron_white.svg
.. |glossary|  image:: /static/img/glossary_white.svg
.. |git|  image:: /static/img/git_white.svg
.. |refresh|  image:: /static/img/refresh_white.svg
.. |hpc|  image:: /static/img/hpc_white.svg
.. |random|  image:: /static/img/random_white.svg
.. |math|  image:: /static/img/math_white.svg
.. |network|  image:: /static/img/network_brain_white.svg
.. |device|  image:: /static/img/device_white.svg
.. |connect|  image:: /static/img/connect_white.svg
.. |sonata|  image:: /static/img/sonata_white.svg
.. |write|  image:: /static/img/write_nest_white.svg
      :scale: 60%
.. |parallel| image:: /static/img/parallel_white.svg
.. |simulate| image:: /static/img/simulate_white.svg
.. |interactive| image:: /static/img/interactive_white.svg
.. |python| image:: /static/img/python_white.svg
.. |gallery| image:: /static/img/gallery_white.svg
