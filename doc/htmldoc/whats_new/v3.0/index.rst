.. _release_3.0:

What's new in NEST 3.0
======================

With the release of NEST 3.0, we introduce a ton of new features to improve how you create and manage simulations.

NEST 3.0 introduces a more direct approach to accessing :ref:`neuron and device properties <node_handles>` and
to :ref:`interacting with synapses <handling_connections>`. You can now read and write properties of nodes and
connections with the functions ``get()`` and ``set()`` or by direct member access (e.g., ``neuron.V_m = -55.0``).
:ref:`Parameter objects <param_ex>` make mathematical and probabilistic functions available to pick
neuron properties, create spatial positions, define connection probabilities, and much more. In addition, it is now way
easier to perform operations such as slicing, iterating, and tests for equality on collections of neurons and synapses.

NEST 3.0 replaces the old random number generator library :ref:`with a new one <simpler_rngs>`
based on the C++ Standard Library. The new library also provides generators from the Random 123 library, including
crypotgraphic generators. Most notably, you can now much more easily seed all generators and change the type of random
number generator you are using.

We have improved how :ref:`recordings from simulations <record_sims>` are handled, making the
infrastructure more modular and extensible. In addition to the previously supported recording methods, a new backend for
SIONlib is now available. The interface for :ref:`stimulation devices <stimulation_backends>` can now handle
data from external sources, such as other simulators.

The Topology Module is no longer a separate module; it is integrated within PyNEST, and now referred to as support for
:ref:`spatially-structured networks <spatial_networks>`.

:ref:`NEST Server <nest_server>` is a novel backend to NEST that allows to run simulations via a RESTful API.
Instead of directly importing ``nest`` into your Python session, the code that controls the simulation is sent over HTTP
to NEST Server in this use-case.

With NEST 3.0, we no longer support Python 2. Check out our :ref:`Installation instructions <install_nest>`
for more information on the dependencies.

To explore what NEST 3.0 has to offer in more detail, check out the links below!


----

.. toctree::
   :maxdepth: 1

   New features in NEST 3.0 <features/index>
   Old versus new syntax <refguide_nest2_nest3>


.. seealso::

    `Release notes on Github <https://github.com/nest/nest-simulator/releases/>`_
