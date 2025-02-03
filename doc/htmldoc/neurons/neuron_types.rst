.. _types_neurons:

Types of neurons
================


.. grid::
   :gutter: 1

   .. grid-item-card::
     :columns: 3
     :class-item: sd-text-center
     :link: spiking_neurons
     :link-type: ref

     .. image:: /static/img/spiking_neurons.svg



   .. grid-item-card::
     :columns: 3
     :class-item: sd-text-center
     :link: rate_neurons
     :link-type: ref


     .. image:: /static/img/rate_neurons.svg


   .. grid-item-card::
     :columns: 3
     :class-item: sd-text-center
     :link: multistate
     :link-type: ref


     .. image:: /static/img/multistate.svg


   .. grid-item-card::
     :columns: 3
     :class-item: sd-text-center
     :link: neuron_astrocyte
     :link-type: ref


     .. image:: /static/img/astrocyte.svg


.. grid::
   :gutter: 1

   .. grid-item::
     :columns: 12

     For details on neuron update algorthims see: :ref:`neuron_update`

.. _spiking_neurons:

Spiking neurons
---------------


Geometry
~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2

      .. image:: /static/img/point_neurons_nn.svg

   .. grid-item:: Point neurons
      :columns: 4

      The most common type of neuron model in NEST.
      Point neurons are simplified models of biological neurons that represent
      the neuron as a single point where all inputs are processed,
      without considering the complex neuronal morphology (such as dendrites and axons).


   .. grid-item::
      :columns: 2

      .. image:: /static/img/mc_neurons_nn.svg

   .. grid-item:: Multi-compartment neurons
      :columns: 4

      Neurons are subdivided into multiple compartments that can represent different parts of the neuronal morphology,
      like soma, basal and apical dendrites. Inputs can be received in all compartments and are mediated across
      compartments via electric coupling.

      .. NEAT

      .. dropdown:: Multi-compartment neurons

         {% for items in tag_dict %}
         {% if items.tag == "compartmental model" %}
         {% for item in items.models | sort %}
         * :doc:`/models/{{ item | replace(".html", "") }}`
         {% endfor %}
         {% endif %}
         {% endfor %}






Spiking mechanism
~~~~~~~~~~~~~~~~~


Spiking neuron models process synaptic inputs and generate discrete output events, which are called action
potentials or spikes.
The mechanisms by which these spikes are generated can be classified with the following distinctions.


Hard threshold
^^^^^^^^^^^^^^

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image::  /static/img/integrate_and_fire_nn.svg

   .. grid-item::
      :columns: 10

      .. tab-set::

        .. tab-item:: General info
          :selected:

          When the membrane potential reaches a certain threshold,
          the neuron deterministically  "fires" an action potential
          Neuron models iwth hard threshold do not contain intrinsic dynamics that produce the upswing of a spike. The downswing is realized
          is by an artificial reset mechanism



            .. dropdown:: Hard threshold

                  * iaf_*
                  * glif_*
                  * amat_ / mat

        .. tab-item:: Technical details

          * :ref:`exact_integration`
          * :doc:`/model_details/IAF_Integration_Singularity`

Soft threshold
^^^^^^^^^^^^^^

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image::  /static/img/hodgkinhuxley_nn.svg

   .. grid-item::
      :columns: 10

      .. tab-set::

          .. tab-item:: General info
            :selected:

            Neurons with a soft threshold model aspects of the voltage dependent conductances that underlie the
            biophysics of spike generation. Models either produce dynamics, which mimic the upswing of a spike or
            the whole whole spike wave form


            .. dropdown:: Soft threshold

               * hh_cond_beta_gap_traub – Hodgkin-Huxley neuron with gap junction support and beta function synaptic conductances
               * hh_cond_exp_traub – Hodgkin-Huxley model for Brette et al (2007) review
               * hh_psc_alpha – Hodgkin-Huxley neuron model
               * hh_psc_alpha_clopath – Hodgkin-Huxley neuron model with support for Clopath plasticity
               * hh_psc_alpha_gap – Hodgkin-Huxley neuron model with gap-junction support
               * Hill and Tononi?
               * Izhikevich
               * Adaptive exponential integrate-and-fire (AEIF)

          .. tab-item:: Technical details

            * :doc:`/model_details/hh_details`
            * :doc:`/model_details/HillTononiModels`


Stochastic
^^^^^^^^^^


.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image::  /static/img/point_process_nn.svg

   .. grid-item::
      :columns: 10

      Stochastic spiking models do not produce deterministic spike events: instead spike times are stochastic and described
      by a point process, with an underlying time dependent firing rate that is determined by the membrane potential of a neuron.
      Elevated membrane potential with respect to the resting potential increaes the likelihood of output spikes.

      .. dropdown::  Stochastic

        * gif_*
        * pp_cond_exp_mc_urbanczik – Two-compartment point process neuron with conductance-based synapses
        * pp_psc_delta – Point process neuron with leaky integration of delta-shaped PSCs

Input mechanism
~~~~~~~~~~~~~~~

NEST supports various input mechanisms to neuron models. The majority of mechanisms are related to chemical synapses
that couple neurons witha delay, but there are also electrical synapses, which couple neurons instantaneously.

Electrical
^^^^^^^^^^

- Gap junctions are direct electrical connections between neurons. The respective membrane potentials are instantaneously
  coupled to each other.

Chemical
^^^^^^^^

Chemical synapses couple neurons in a delayed fashion, because of the conversion of electrical and chemical
signals at the synapse. This process is captured by two major classes of models in NEST that either model input currents
or conductances.

.. seealso::

   :doc:`../synapses/synapse_dynamics`

- Current-based models:


.. grid:: 1 2 2 2

   .. grid-item::
     :columns: 2
     :class: sd-d-flex-row sd-align-major-center

     .. image::  /static/img/current_based_nn.svg

   .. grid-item::
     :columns: 10

     NEST convention: ``psc`` (aka CUBA)

     Model post-synaptic responses to incoming spikes as changes in current.
     The response of the post-synaptic neuron is independent of the neuronal state.

     For more details see :ref:`synapse_dynamics`.

     .. dropdown:: Current-based neuron models

         {% for items in tag_dict %}
         {% if items.tag == "current-based" %}
         {% for item in items.models | sort %}
         * :doc:`/models/{{ item | replace(".html", "") }}`
         {% endfor %}
         {% endif %}
         {% endfor %}

- Conductance-based models


.. grid:: 1 2 2 2

   .. grid-item::
     :columns: 2
     :class: sd-d-flex-row sd-align-major-center

     .. image::  /static/img/conductance_based_nn.svg

   .. grid-item::
     :columns: 10

     NEST convention: ``cond`` (aka COBA)

     Model post-synaptic responses to incoming spikes as changes in conductances.
     The response of the post-synaptic neuron depends on the neuronal state.
     These models capture more realistic synaptic behavior, as they account for the varying impact of
     synaptic inputs depending on the membrane potential, which can change over time.


     For more details see :ref:`synapse_dynamics`.

     .. dropdown:: Conductance-based neuron models

         {% for items in tag_dict %}
         {% if items.tag == "conductance-based" %}
         {% for item in items.models | sort %}
         * :doc:`/models/{{ item | replace(".html", "") }}`
         {% endfor %}
         {% endif %}
         {% endfor %}



Post-synaptic input responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Synaptic inputs can be modeled using different kernels to represent
how the `current or conductance` changes over time after a synaptic event.


.. seealso::

   For more details see :ref:`time_dependence`

..note:: graphs - add y axis peak 1 picoampere

.. grid:: 1 2 2 2

   .. grid-item-card:: Delta
      :columns: 3
      :link: delta_synapse
      :link-type: ref

      .. image:: /static/img/delta_nn.svg

   .. grid-item-card:: Exp
      :columns: 3
      :link: exp_synapse
      :link-type: ref

      .. image:: /static/img/exp_nn.svg

   .. grid-item-card:: Alpha
      :columns: 3
      :link: alpha_synapse
      :link-type: ref

      .. image:: /static/img/alpha2.svg


   .. grid-item-card:: Beta
      :columns: 3
      :link: beta_synapse
      :link-type: ref

      .. image:: /static/img/beta2.svg



Adaptation mechanism
~~~~~~~~~~~~~~~~~~~~


.. grid::

  .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/adaptive_threshold_ nn.svg

  .. grid-item::
      :columns: 10

      .. tab-set::

        .. tab-item:: General info
            :selected:


            Unlike a fixed threshold, an adaptive threshold increases temporarily following each spike and
            gradually returns to its baseline value over time. This mechanism models phenomena
            such as spike-frequency adaptation, where a neuron's responsiveness decreases with sustained
            high-frequency input.

            .. dropdown:: Models with adaptation

              {% for items in tag_dict %}
              {% if items.tag == "adaptation" %}
              {% for item in items.models | sort %}
              * :doc:`/models/{{ item | replace(".html", "") }}`
              {% endfor %}
              {% endif %}
              {% endfor %}


        .. tab-item:: Technical details

           * :doc:`/model_details/aeif_models_implementation`



Auxilliary neurons
~~~~~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/parrot_neurons_nn.svg

   .. grid-item::
      :columns: 10

      NEST provides a number of auxilliary neuron models that can be used for specific purposes such as repeating or
      ignoring particular incoming spike patterns. Use cases for such functionality include testing or benchmarking
      simulator performance, or the creation of shared spiking input patterns to neurons.

      .. dropdown:: Auxillary neurons

        ignore-and-fire - used for benchmarking . . .

        parrot_neuron – Neuron that repeats incoming spikes

        parrot_neuron_ps – Neuron that repeats incoming spikes - precise spike timing version



Precise spike timing
~~~~~~~~~~~~~~~~~~~~


.. grid::

  .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/precise_nn.svg

  .. grid-item::

      NEST convention: ``ps``

      By default, the dynamics of neuronal models are evaluated on a fixed time grid that can be specified before simulation.

      Precise neuron models instead calculate precise rather than grid-constrained spike times. These models are more
      computationally heavy,  but provide better resolution to spike times than a grid-constrained model.
      Spiking neuron networks are often chaotic systems such that an infinitesimal shift in spike time might lead to changes in
      the overall network dynamics.

      See :ref:`our guide on precise spike timing <sim_precise_spike_times>`.


      .. dropdown:: Models with precise spike times

        {% for items in tag_dict %}
        {% if items.tag == "precise" %}
        {% for item in items.models | sort %}
        * :doc:`/models/{{ item | replace(".html", "") }}`
        {% endfor %}
        {% endif %}
        {% endfor %}


----

|


.. _rate_neurons:

Rate neurons
------------

.. grid::

   .. grid-item::
     :columns: 2
     :class: sd-d-flex-row sd-align-major-center

     .. image:: /static/img/rate_neurons_nn.svg

   .. grid-item::

     Rate neurons can approximate biologically realistic neurons but they are also used in artificial neuranl networks
     (aka recurrent neural networks RNNs)

     Most rate neurons in NEST are implemented as templates based on the non-linearity and noise type.



Noise application
~~~~~~~~~~~~~~~~~

.. grid::

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      **Input noise**

      .. image:: /static/img/input_noise_nn.svg


   .. grid-item::

     ``ipn``: Noise is added to the input rate

       :doc:`/models/rate_neuron_ipn`

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      **Output noise**

      .. image:: /static/img/output_noise_nn.svg

   .. grid-item::

     ``opn``: Noise is applied to the output rate

       :doc:`/models/rate_neuron_opn`


Where is Non-linearity applied? ``linear_summation``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


The boolean parameter ``linear_summation`` determines whether the
input from different presynaptic neurons is first summed linearly and
then transformed by a nonlinearity (``True``), or if the input from
individual presynaptic neurons is first nonlinearly transformed and
then summed up (``False``). Default is ``True``.

You can set this parameter in the parameter dictionary of the rate neuron.


Rate transformer
~~~~~~~~~~~~~~~~

You can use the :doc:`rate_transformer_node </models/rate_transformer_node>` (applies a non-linearity
to a sum of incoming rates, transforming them before passing on to other nodes.)



Type of non-linearity
~~~~~~~~~~~~~~~~~~~~~~~

You can specify the type of non-linearity, which in NEST are provided as C++ templates.

The following non-linearity types are available:


 .. dropdown:: Rate models with non-linearity

  *  :doc:`/models/gauss_rate`
  *  :doc:`/models/lin_rate`
  *  :doc:`/models/sigmoid_rate`
  *  :doc:`/models/sigmoid_rate_gg_1998`
  *  :doc:`/models/tanh_rate`
  *  :doc:`/models/threshold_lin_rate`

Use rate neurons in your simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use a rate neuron, the naming convention is ``<non-linearity>_rate_<noise_type>``

For example::

   nest.Create("gauss_rate_opn")

If using the  ``rate_transformer_node``, you can use the following syntax ``rate_transformer_<non-linearity>``

Example::

    nest.Create("rate_transformer_tanh")


Mean field theory
~~~~~~~~~~~~~~~~~


.. grid::

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/siegert_neuron_nn.svg

   .. grid-item::


     .. tab-set::

      .. tab-item:: General info
        :selected:

        Mean-field theory

        * :doc:`/models/siegert_neuron`

      .. tab-item:: Technical details

        * :doc:`/model_details/siegert_neuron_integration`


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

   .. grid-item::
     :columns: 2
     :class: sd-d-flex-row sd-align-major-center

     .. image:: /static/img/multistate_nn.svg

   .. grid-item::

     Neurons with two or three states. Simplest neuron models with threshold activation. Binary neurons have
     On / Off behavior   used in theoretical and disease theory.

.. dropdown:: Multi-state neurons

    {% for items in tag_dict %}
    {% if items.tag == "binary" %}
    {% for item in items.models | sort %}
    * :doc:`/models/{{ item | replace(".html", "") }}`
    {% endfor %}
    {% endif %}
    {% endfor %}


|

----

|

.. _neuron_astrocyte:

Astrocytes
----------

.. grid::

   .. grid-item::
     :columns: 2
     :class: sd-d-flex-row sd-align-major-center

     .. image:: /static/img/astrocyte_nn.svg

   .. grid-item::

     .. tab-set::

      .. tab-item:: General info
         :selected:

         Astrocytes

         .. dropdown:: Astrocyte models

            {% for items in tag_dict %}
            {% if items.tag == "astrocyte" %}
            {% for item in items.models | sort %}
            * :doc:`/models/{{ item | replace(".html", "") }}`
            {% endfor %}
            {% endif %}
            {% endfor %}


      .. tab-item:: Technical details

         * :doc:`/model_details/astrocyte_model_implementation`




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
