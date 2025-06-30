.. _synapse_index:

All about synapses and connections in NEST
==========================================

Guides on using synapses in NEST
--------------------------------


.. grid:: 1 1 2 2

  .. grid-item-card:: Managing coonnections

      * :ref:`connectivity_concepts`
      * :ref:`connection_generator`
      * :ref:`synapse_spec`

  .. grid-item-card:: Weight normalization
      :class-title: sd-d-flex-row sd-align-minor-center
      :link: weight_normalization
      :link-type: ref

  .. grid-item-card:: Gap Junctions
      :class-title: sd-d-flex-row sd-align-minor-center
      :link: sim_gap_junctions
      :link-type: ref

  .. grid-item-card:: Connection functionality
      :class-title: sd-d-flex-row sd-align-minor-center
      :link: handling_connections
      :link-type: ref

.. toctree::
  :maxdepth: 1
  :glob:
  :hidden:

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
