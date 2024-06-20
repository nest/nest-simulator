.. _device_index:

All about devices in NEST
=========================


Guides on using devices in NEST
-------------------------------

.. grid:: 1 1 2 2
    :gutter: 1

    .. grid-item-card::  Stimulate the network
       :class-title: sd-d-flex-row sd-align-minor-center
       :link: stimulate_network
       :link-type: ref

    .. grid-item-card:: Get data from simulation
       :class-title: sd-d-flex-row sd-align-minor-center
       :link: record_simulations
       :link-type: ref


.. toctree::
  :maxdepth: 1
  :glob:
  :hidden:

  *


.. dropdown:: List of devices
   :color: info

   {% for items in tag_dict %}
   {% if items.tag == "device" %}
   {% for item in items.models | sort %}
   * :doc:`/models/{{ item | replace(".html", "") }}`
   {% endfor %}
   {% endif %}
   {% endfor %}

Naming conventions for devices
------------------------------

A device name should represent its physical counterpart - like a multimeter is ``multimeter``.  In general, the term ``recorder`` is used for devices
that store the output (e.g., spike times or synaptic strengths over time) of other nodes and make it accessible to the user. The term  ``generator`` is used for devices that provide input into the simulation.
