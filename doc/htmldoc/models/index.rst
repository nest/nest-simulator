.. _modelsmain:


Model directory
===============


NEST has over 100 models, choose an option for finding the one you need!

.. _model_selector:

Model selector
--------------


Select a tag to display corresponding models.

By selecting multiple tags, you can refine your search to models that match all selected tags.

.. raw:: html

    <div id="tag-container">
        <!-- Tags will be populated here by JavaScript -->
    </div>
    <h3>List of models based on your selection </h3>
    <div id="model-list"></div>
    <script src="../_static/js/filter_models.js"></script>

|
|

----


.. _full_list:

Complete A-Z list of models
----------------------------

.. dropdown::  Show/Hide list
   :animate: fade-in-slide-down
   :color: info


   **Neurons**

   {% for items in tag_dict %}
   {% if items.tag == "neuron" %}
   {% for item in items.models | sort %}
   * :doc:`{{ item | replace(".html", "") }}`
   {% endfor %}
   {% endif %}
   {% endfor %}

   **Synapses**

   {% for items in tag_dict %}
   {% if items.tag == "synapse" %}
   {% for item in items.models | sort %}
   * :doc:`{{ item | replace(".html", "") }}`
   {% endfor %}
   {% endif %}
   {% endfor %}

   **Devices**

   {% for items in tag_dict %}
   {% if items.tag == "device" %}
   {% for item in items.models | sort %}
   * :doc:`{{ item | replace(".html", "") }}`
   {% endfor %}
   {% endif %}
   {% endfor %}



Learn more about . . .
-----------------------

.. grid:: 1 1 2 2
  :gutter: 1

  .. grid-item-card::

      * :doc:`neuron models </neurons/index>`

      * :doc:`synapse models </synapses/index>`

      * :doc:`devices </devices/index>`

      * :ref:`creating and customizing models with NESTML <nestml_ad>`

      * :ref:`Model terminology <model_meaning>`



----

.. _model_meaning:


What we mean by `models`
------------------------

The term `models` in the context of NEST (and the field of computational neuroscience as a whole) is used with two different meanings:

1. **Neuron and synapse models**. These consist of a set of mathematical
   equations and algorithmic components that describe the
   characteristics and behavior of biological neurons and synapses. In
   NEST, the terms neuron and synapse models are also used for the C++
   implementations of these conceptual entities. Most of the models in
   NEST are based on either peer-reviewed publications or text books
   like [1]_. This is what we mean for models in our model directory.

2. **Network models**. These models are created from individual neuron
   and synapse models using the different commands provided by the
   :ref:`PyNEST API <pynest_api>`. Examples for such network models
   are the :doc:`microcircuit model
   <../auto_examples/Potjans_2014/index>` or the `multi-area model
   <https://inm-6.github.io/multi-area-model/>`_).
   You can also explore :doc:`all our example networks </examples/index>`.

.. seealso::

  See our glossary section on :ref:`common abbreviations used for model terms <model_terms>`. It includes alternative terms commonly used in the literature.


.. _nestml_ad:

Create and customize models with NESTML
---------------------------------------

Check out :doc:`NESTML <nestml:index>`, a domain-specific language for neuron and synapse models.
NESTML enables fast prototyping of new models using an easy to understand, yet powerful syntax. This is achieved by a combination of a flexible processing toolchain
written in Python with high simulation performance through the automated generation of C++ code, suitable for use in NEST Simulator.

.. seealso::

  See the :doc:`NESTML docs for installation details <nestml:index>`.

References
~~~~~~~~~~

.. [1] Dayan P and Abbott L (2001). Theoretical Neuroscience: Computational
       and Mathematical Modeling of Neural Systems. Cambridge, MA: MIT Press.
       https://pure.mpg.de/pubman/faces/ViewItemOverviewPage.jsp?itemId=item_300


.. toctree::
    :maxdepth: 1
    :hidden:

    {% for keys in model_dict %}
    {{ keys | replace(".html", "") }}
    {% endfor %}
