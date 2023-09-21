.. _modelsmain:

Models in NEST
==============

What we mean by `models`
------------------------

The term `models` in the context of NEST (and the field of computational neuroscience as a whole) is used with two different meanings:

1. **Neuron and synapse models**. These consist of a set of mathematical
   equations and algorithmic components that describe the
   characteristics and behavior of biological neurons and synapses. In
   NEST, the terms neuron and synapse models are also used for the C++
   implementations of these conceptual entities. Most of the models in
   NEST are based on either peer-reviewed publications or text books
   like [1]_.
2. **Network models**. These models are created from individual neuron
   and synapse models using the different commands provided by the
   :ref:`PyNEST API <pynest_api>`. Examples for such network models
   are the :doc:`microcircuit model
   <../auto_examples/Potjans_2014/index>` or the `multi-area model
   <https://inm-6.github.io/multi-area-model/>`_). In the following
   description, we focus on neuron and synapse models and not on
   network models.

Find a model
------------

By default, NEST comes with a ton of models! Textbook standards like
integrate-and-fire and Hodgkin-Huxley-type models are available
alongside high-quality implementations of models published by the
neuroscience community.  The model directory is organized by keywords
(e.g., :doc:`adaptive threshold <index_adaptive threshold>`,
:doc:`conductance-based <index_conductance-based>`, etc.).  Models
that contain a specific keyword will be listed under that word.

In many modeling situations, the full set of models that ship with
NEST by default is not needed. To only include a subset of the models
with NEST, please have a look at the :ref:`modelset configuration
options <modelset_config>`.

.. seealso::

   Discover :doc:`all the models in our directory <index>`.

Create and customize models with NESTML
---------------------------------------

Check out :doc:`NESTML <nestml:index>`, a domain-specific language for neuron and synapse models.
NESTML enables fast prototyping of new models using an easy to understand, yet powerful syntax. This is achieved by a combination of a flexible processing toolchain
written in Python with high simulation performance through the automated generation of C++ code, suitable for use in NEST Simulator.

.. seealso::

  See the :doc:`NESTML docs for installation details <nestml:index>`.

.. note::

  NESTML is also available as part of NEST's official :ref:`docker image <docker>`.

Model naming
------------

Neuron models
~~~~~~~~~~~~~

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

Synapse models
~~~~~~~~~~~~~~

Synapse models include the word synapse as the last word in the model name.

Synapse models may begin with the author name (e.g., ``clopath_synapse``) or process (e.g., ``stdp_synapse``).

Devices
~~~~~~~

A device name should represent its physical counterpart - like a multimeter is ``multimeter``.  In general, the term ``recorder`` is used for devices
that store the output (e.g., spike times or synaptic strengths over time) of other nodes and make it accessible to the user. The term  ``generator`` is used for devices that provide input into the simulation.

.. seealso::

  See our glossary section on :ref:`common abbreviations used for model terms <model_terms>`. It includes alternative terms commonly used in the literature.

References
~~~~~~~~~~
  
.. [1] Dayan P and Abbott L (2001). Theoretical Neuroscience: Computational
       and Mathematical Modeling of Neural Systems. Cambridge, MA: MIT Press.
       https://pure.mpg.de/pubman/faces/ViewItemOverviewPage.jsp?itemId=item_300
