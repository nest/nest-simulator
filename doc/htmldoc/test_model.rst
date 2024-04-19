Model directory
===============

.. literalinclude:: static/js/bad_filter_model.js


----



Find a model
------------

NEST has over 100 models, choose an option for finding the one you need!

.. grid:: 1 1 3 3

   .. grid-item-card::

      Filter models with our :ref:`model_selector`

   .. grid-item-card::

      Get :ref:`full_list` of models

   .. grid-item-card::

      Learn more about

      * neuron models

      * synapse models

      * device models

      * Creating and customizing your own models

.. _model_selector:

Model selector
--------------

Select a tag to display corresponding models. By selecting multiple tags, you can refine your search to models that match all selected tags.

.. raw:: html

    <div id="tag-container">
        <!-- Tags will be populated here by JavaScript -->
    </div>
    <h3>Filtered model list </h3>
    <div id="model-list"></div>
    <script src="_static/js/filter_models.js"></script>


----


.. _full_list:

Complete A-Z list of models
----------------------------

.. dropdown::  Show/Hide list
   :animate: fade-in-slide-down


   {% for key, values in tag_dict.items() | sort %}

   * :doc:`/{{ key | replace(".html", "") }}` : {%for value in values %} {{ value }} {% endfor %}
   {% endfor %}
