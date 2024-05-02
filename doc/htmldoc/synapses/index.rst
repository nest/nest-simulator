.. _synapse_index:

All about synapses and connections in NEST
==========================================

Guides on using synapses in NEST
--------------------------------

.. toctree::
  :maxdepth: 1
  :glob:

  *


.. dropdown:: List of synapse models
   :color: info

   {% for items in tag_dict %}
   {% if items.tag == "synapse" %}
   {% for item in items.models | sort %}
   * :doc:`/models/{{ item | replace(".html", "") }}`
   {% endfor %}
   {% endif %}
   {% endfor %}

Naming conventions for synapse models
-------------------------------------

Synapse models include the word synapse or connection as the last word in the model name.

Synapse models may begin with the author name (e.g., ``clopath_synapse``) or process (e.g., ``stdp_synapse``).
