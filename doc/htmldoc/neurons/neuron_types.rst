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

     In the following section, we introduce the different types of neuron models implemented in NEST, describe their dynamics
     and technical details. A generic description of the numerical update algorithm can be found here: :ref:`neuron_update`.

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

      For more information see our project :doc:`NEST-NEAT <neat:index>`, a Python library for the study, simulation and
      simplification of morphological neuron models.

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
          the neuron deterministically  "fires" an action potential.
          Neuron models with hard threshold do not contain intrinsic dynamics that produce the upswing of a spike.
          Some of the neurons in this class do not reset the membrane potential after a spike.
          Note that the threshold itself can be dynamic (see `Adaptation` section below).



            .. dropdown:: Hard threshold

               {% for items in tag_dict %}
               {% if items.tag == "hard threshold" %}
               {% for item in items.models | sort %}
               * :doc:`/models/{{ item | replace(".html", "") }}`
               {% endfor %}
               {% endif %}
               {% endfor %}

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
            the whole spike wave form.
            Some models in this class contain a hard threshold that triggers an instantaneous reset of the membrane potential.
            This threshold is needed to finish the action potential and to avoid an unbounded growth of the membrane potential.
            The action-potential initiation is not affected by this and governed by continuous dynamics.

            .. dropdown:: Soft threshold

               {% for items in tag_dict %}
               {% if items.tag == "soft threshold" %}
               {% for item in items.models | sort %}
               * :doc:`/models/{{ item | replace(".html", "") }}`
               {% endfor %}
               {% endif %}
               {% endfor %}


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
      Elevated membrane potential with respect to the resting potential increases the likelihood of output spikes.

      .. dropdown::  Stochastic

         {% for items in tag_dict %}
         {% if items.tag == "stochastic" %}
         {% for item in items.models | sort %}
         * :doc:`/models/{{ item | replace(".html", "") }}`
         {% endfor %}
         {% endif %}
         {% endfor %}


Input mechanism
~~~~~~~~~~~~~~~

NEST supports various input mechanisms to neuron models. The majority of mechanisms are related to chemical synapses
that couple neurons with a delay, but there are also electrical synapses, which couple neurons instantaneously.

Electrical
^^^^^^^^^^

:ref:`Gap junctions <sim_gap_junctions>` are direct electrical connections between neurons. The respective membrane potentials are instantaneously
coupled to each other.

Chemical
^^^^^^^^

Chemical synapses couple neurons in a delayed fashion, because of the conversion of electrical and chemical
signals at the synapse. This process is captured by two major classes of models in NEST that either model synaptic
inputs as currents or conductances.



   **Current-based (CUBA) models**


   .. grid:: 1 2 2 2

      .. grid-item::
        :columns: 2
        :class: sd-d-flex-row sd-align-major-center

        .. image::  /static/img/current_based_nn.svg

      .. grid-item::
        :columns: 10


        Model post-synaptic responses to incoming spikes as changes in current.
        The response of the post-synaptic neuron is independent of the neuronal state.
        In NEST, current-based neuron models are labeled by ``psc`` (post-synaptic currents).

        .. dropdown:: Current-based neuron models

            {% for items in tag_dict %}
            {% if items.tag == "current-based" %}
            {% for item in items.models | sort %}
            * :doc:`/models/{{ item | replace(".html", "") }}`
            {% endfor %}
            {% endif %}
          {% endfor %}

   **Conductance-based (COBA) models**


   .. grid:: 1 2 2 2

      .. grid-item::
        :columns: 2
        :class: sd-d-flex-row sd-align-major-center

        .. image::  /static/img/conductance_based_nn.svg

      .. grid-item::
        :columns: 10

        Model post-synaptic responses to incoming spikes as changes in conductances.
        The response of the post-synaptic neuron depends on the neuronal state.
        These models capture more realistic synaptic behavior, as they account for the varying impact of
        synaptic inputs depending on the membrane potential, which can change over time.
        In NEST, conductance-based neuron models are labeled by ``cond``.


        .. dropdown:: Conductance-based neuron models

            {% for items in tag_dict %}
            {% if items.tag == "conductance-based" %}
            {% for item in items.models | sort %}
            * :doc:`/models/{{ item | replace(".html", "") }}`
            {% endfor %}
            {% endif %}
            {% endfor %}



.. _synapse_selection:

Selecting the target synapse
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Many neuron models provide more than one synapse, most commonly an excitatory and an
inhibitory one with their own time constants (``tau_syn_ex`` and ``tau_syn_in``) and, for
conductance-based models, their own reversal potentials (``E_ex`` and ``E_in``).
How you direct an incoming spike to one synapse or the other is not the same for all
models. It is fixed by the neuron model you connect to, and neither the synapse model nor
the :py:func:`.Connect` call can change it.

NEST models use one of the following three mechanisms.

Sign of the weight
^^^^^^^^^^^^^^^^^^

Most models with an excitatory and an inhibitory synapse use the **sign of the connection
weight** to choose between them. A positive weight targets the excitatory synapse, a
negative weight the inhibitory one. Spike connections must use the default
``receptor_type`` of 0. Requesting any other receptor raises an ``UnknownReceptorType``
error when the connection is created.

The one exception is :doc:`/models/iaf_tum_2000`, where ``receptor_type`` selects the input
channel rather than the synapse. Connections from another ``iaf_tum_2000`` must use
receptor 1, and the weight sign still chooses excitatory or inhibitory. Note also that a
few models, such as :doc:`/models/iaf_psc_exp`, accept a non-zero ``receptor_type`` for
*current* input from a generator. That is unrelated to how spikes are routed.

.. code-block:: python

    n = nest.Create("iaf_cond_exp")
    pg_ex = nest.Create("poisson_generator", params={"rate": 80.0})
    pg_in = nest.Create("poisson_generator", params={"rate": 80.0})

    # positive weight -> excitatory synapse (E_ex, tau_syn_ex)
    nest.Connect(pg_ex, n, syn_spec={"weight": 1.0, "delay": 1.0})

    # negative weight -> inhibitory synapse (E_in, tau_syn_in)
    nest.Connect(pg_in, n, syn_spec={"weight": -1.0, "delay": 1.0})

What the sign controls differs between the two model classes:

* In **current-based** models, the sign selects the time constant and also sets the
  direction of the post-synaptic current. A negative weight is hyperpolarizing.

* In **conductance-based** models, the sign selects the time constant and the reversal
  potential, but only the *absolute value* of the weight sets the conductance amplitude.
  Whether the input depolarizes or hyperpolarizes the neuron then follows from the
  reversal potential relative to the membrane potential, not from the sign of the weight.

.. note::

   Describe your connections as having positive or negative weights, and do not rely on
   the behavior of a weight of exactly zero. Models are not consistent about which synapse
   a zero weight is routed to, though a zero-weight connection has no effect either way.

.. dropdown:: Models that select the synapse by weight sign

   Current-based:

   * :doc:`/models/aeif_psc_alpha`
   * :doc:`/models/aeif_psc_exp`
   * :doc:`/models/amat2_psc_exp`
   * :doc:`/models/gif_pop_psc_exp`
   * :doc:`/models/gif_psc_exp`
   * :doc:`/models/hh_psc_alpha`
   * :doc:`/models/hh_psc_alpha_clopath`
   * :doc:`/models/hh_psc_alpha_gap`
   * :doc:`/models/iaf_psc_alpha`
   * :doc:`/models/iaf_psc_alpha_ps`
   * :doc:`/models/iaf_psc_exp`
   * :doc:`/models/iaf_psc_exp_htum`
   * :doc:`/models/iaf_psc_exp_ps`
   * :doc:`/models/iaf_psc_exp_ps_lossless`
   * :doc:`/models/iaf_tum_2000`
   * :doc:`/models/mat2_psc_exp`

   Conductance-based:

   * :doc:`/models/aeif_cond_alpha`
   * :doc:`/models/aeif_cond_alpha_astro`
   * :doc:`/models/aeif_cond_exp`
   * :doc:`/models/gif_cond_exp`
   * :doc:`/models/hh_cond_beta_gap_traub`
   * :doc:`/models/hh_cond_exp_traub`
   * :doc:`/models/iaf_chxk_2008`
   * :doc:`/models/iaf_cond_alpha`
   * :doc:`/models/iaf_cond_beta`
   * :doc:`/models/iaf_cond_exp`
   * :doc:`/models/iaf_cond_exp_sfa_rr`

Receptor type
^^^^^^^^^^^^^

Multisynapse, multi-compartment and multi-receptor models instead select the synapse by
:ref:`receptor type <receptor-types>`. Each synapse or compartment has its own integer ID,
which you pass as ``receptor_type`` in the ``syn_spec`` dictionary. The sign of the weight
plays no part in the choice.

.. code-block:: python

    n = nest.Create("iaf_psc_exp_multisynapse", params={"tau_syn": [2.0, 10.0]})
    pg = nest.Create("poisson_generator", params={"rate": 80.0})

    nest.Connect(pg, n, syn_spec={"weight": 1.0, "delay": 1.0, "receptor_type": 2})

For the current-based models in this group, a negative weight gives a hyperpolarizing
current on the chosen receptor, as usual. For the conductance-based ones, a negative
weight gives a negative conductance, which is not physically meaningful. These models do
not check the sign, so use positive weights and let the reversal potential of the chosen
receptor determine the effect.

.. dropdown:: Models that select the synapse by receptor type

   * :doc:`/models/gif_psc_exp_multisynapse`
   * :doc:`/models/glif_cond`
   * :doc:`/models/glif_psc`
   * :doc:`/models/glif_psc_double_alpha`
   * :doc:`/models/ht_neuron`
   * :doc:`/models/iaf_bw_2001`
   * :doc:`/models/iaf_bw_2001_exact`
   * :doc:`/models/iaf_cond_alpha_mc`
   * :doc:`/models/iaf_psc_alpha_multisynapse`
   * :doc:`/models/iaf_psc_exp_multisynapse`
   * :doc:`/models/pp_cond_exp_mc_urbanczik`

Receptor type, with negative weights rejected
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The conductance-based multisynapse models and the compartmental model select the synapse
by receptor type as above, but additionally reject negative weights with an error when the
spike is delivered, rather than letting a negative conductance through.

.. dropdown:: Models that reject negative weights

   * :doc:`/models/aeif_cond_alpha_multisynapse`
   * :doc:`/models/aeif_cond_beta_multisynapse`
   * :doc:`/models/cm_default`
   * :doc:`/models/gif_cond_exp_multisynapse`
   * :doc:`/models/astrocyte_lr_1994`

Models with a single synapse
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Models with delta-shaped post-synaptic responses, along with the e-prop and Izhikevich
models, sum all input into one channel. There is no synapse to select. The sign of the
weight still determines whether an input depolarizes or hyperpolarizes the neuron.

.. warning::

   :doc:`/models/iaf_chs_2007` has a single synapse and **silently discards** spikes
   arriving with a negative weight. No error or warning is raised.


Post-synaptic input responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Synaptic inputs can be modeled using different kernels to represent
how the `current or conductance` changes over time after a synaptic event.
The type of post-synaptic input (`exp`, `delta`, `alpha`, `beta`) are indicated at
the end of the neuron model name (e.g., ``iaf_psc_delta``)


.. grid:: 1 2 2 2

   .. grid-item-card:: Delta
      :columns: 3

      .. image:: /static/img/delta_nn.svg

   .. grid-item-card:: Exp
      :columns: 3

      .. image:: /static/img/exp_nn.svg

   .. grid-item-card:: Alpha
      :columns: 3

      .. image:: /static/img/alpha2.svg


   .. grid-item-card:: Beta
      :columns: 3

      .. image:: /static/img/beta2.svg



Adaptation mechanism
~~~~~~~~~~~~~~~~~~~~


.. grid::

  .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/adaptive_threshold_nn.svg

  .. grid-item::
      :columns: 10

      .. tab-set::

        .. tab-item:: General info
            :selected:


            Unlike a fixed threshold, an adaptive threshold increases temporarily following each spike and
            gradually returns to its baseline value over time. This mechanism models phenomena
            such as spike-frequency adaptation, where a neuron's responsiveness decreases with sustained
            high-frequency input. A different mechanism to implement similar adaptation behavior is via a spike-triggered hyperpolarizing adaptation current.

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



Auxiliary neurons
~~~~~~~~~~~~~~~~~~

.. grid:: 1 2 2 2

   .. grid-item::
      :columns: 2
      :class: sd-d-flex-row sd-align-major-center

      .. image:: /static/img/parrot_neurons_nn.svg

   .. grid-item::
      :columns: 10

      NEST provides a number of auxiliary neuron models that can be used for specific purposes such as repeating or
      ignoring particular incoming spike patterns. Use cases for such functionality include testing or benchmarking
      simulator performance, or the creation of shared spiking input patterns to neurons.

      .. dropdown:: Auxiliary neurons

        ignore-and-fire – Used for benchmarking

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

      By default, the dynamics of neuronal models are evaluated on a fixed time grid that can be specified before simulation.

      Precise neuron models instead calculate precise rather than grid-constrained spike times. These models are more
      computationally heavy,  but provide better resolution to spike times than a grid-constrained model.
      Spiking neuron networks are often chaotic systems such that an infinitesimal shift in spike time might lead to changes in
      the overall network dynamics.
      In NEST, we label models with precise spike times with  ``ps``.

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

     Rate neurons can approximate biologically realistic neurons but they are also used in artificial neuronal networks
     (also known as recurrent neural networks RNNs).

     Most rate neurons in NEST are implemented as templates based on the non-linearity and noise type.

Type of non-linearity
~~~~~~~~~~~~~~~~~~~~~~~

You can specify the type of non-linearity, which in NEST are provided as C++ templates.

The following non-linearity types are available:


 .. dropdown:: Templates with non-linearity

  *  :doc:`/models/gauss_rate`
  *  :doc:`/models/lin_rate`
  *  :doc:`/models/sigmoid_rate`
  *  :doc:`/models/sigmoid_rate_gg_1998`
  *  :doc:`/models/tanh_rate`
  *  :doc:`/models/threshold_lin_rate`

Where is Non-linearity applied?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


The boolean parameter ``linear_summation`` determines whether the
input from different presynaptic neurons is first summed linearly and
then transformed by a nonlinearity (``True``), or if the input from
individual presynaptic neurons is first nonlinearly transformed and
then summed up (``False``). Default is ``True``.

You can set this parameter in the parameter dictionary of the rate neuron.



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




Use rate neurons in your simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The rate models are instantiated from the templates.
To use a rate neuron, the naming convention is ``<non-linearity>_rate_<noise_type>``

For example::

   nest.Create("gauss_rate_opn")

If using the  ``rate_transformer_node`` (see :ref:`below <rate_transformer>`), you can use the following syntax ``rate_transformer_<non-linearity>``

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

        Rate models can also be used as mean-field descriptions for the population-rate dynamics of spiking networks.

        * :doc:`/models/siegert_neuron`

      .. tab-item:: Technical details

        * :doc:`/model_details/siegert_neuron_integration`

.. _rate_transformer:

Rate transformer
~~~~~~~~~~~~~~~~

If you want to transform rates in a non-linear manner, but do not want any neuronal dynamics processing these rates, then you can use the rate transformer node.

The :doc:`rate_transformer_node </models/rate_transformer_node>` applies a non-linearity
to a sum of incoming rates, transforming them before passing on to other nodes.



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

     Neurons with two or three discrete states. These are the simplest neuron models with threshold activation. Binary neurons have
     On / Off behavior used in theoretical neuroscience and disease theory.

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

         NEST offers an astrocyte model for interactions with neurons, including
         ``TripartiteConnect`` to support the creation of a pre-synaptic, a post-synaptic and a third-factor population.

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
