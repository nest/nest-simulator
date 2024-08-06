.. _neurons_index:

All about neurons in NEST
=========================


Guides on using neurons in NEST
-------------------------------

.. grid:: 1 1 2 2
   :gutter: 1

   .. grid-item::

     .. grid:: 1 1 1 1

        .. grid-item-card:: Node management (neurons and devices)

             * :ref:`node_handles`
             * :ref:`param_ex`

   .. grid-item::

     .. grid:: 1 1 1 1

        .. grid-item-card:: Exact integration
           :class-title: sd-d-flex-row sd-align-minor-center
           :link: exact_integration
           :link-type: ref

        .. grid-item-card:: Precise spike times
           :class-title: sd-d-flex-row sd-align-minor-center
           :link: sim_precise_spike_times
           :link-type: ref

.. toctree::
  :maxdepth: 1
  :glob:
  :hidden:

  *

.. dropdown:: List of neuron models
   :color: info

   {% for items in tag_dict %}
   {% if items.tag == "neuron" %}
   {% for item in items.models | sort %}
   * :doc:`/models/{{ item | replace(".html", "") }}`
   {% endfor %}
   {% endif %}
   {% endfor %}


Neuron model naming conventions
-------------------------------

Neuron model names in NEST combine abbreviations that describe the dynamics and synapse specifications for that model.
They may also include the author's name of a model based on a specific paper.

For example, the neuron model name

``iaf_cond_beta``

    corresponds to an implementation of a spiking neuron using
    integrate-and-fire dynamics with conductance-based
    synapses. Incoming spike events induce a postsynaptic change of
    conductance modeled by a beta function.

As an example for a neuron model name based on specific paper,

``hh_cond_exp_traub``

    implements a modified version of the Hodgkin Huxley neuron model
    based on Traub and Miles (1991)
