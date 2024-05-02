.. _neurons_index:

All about neurons in NEST
=========================


Guides on using neurons in NEST
-------------------------------

.. toctree::
  :maxdepth: 1
  :glob:

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
