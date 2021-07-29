All about NEST 3.0
==================

With the release of NEST 3.0, we introduce a ton of new features to improve how you create and manage simulations.

NEST 3.0 introduces a more direct approach to accessing :doc:`neuron and device properties <features/node_handles>` and
to :doc:`interacting with synapses <features/handling_connections>`. You can now read and write properties of nodes and
connections with the functions ``get()`` and ``set()`` or by direct member access (e.g., ``neuron.V_m = -55.0``).
:doc:`Parameter objects <features/parametrization>` make mathematical and probabilistic functions available to pick
neuron properties, create spatial positions, define connection probabilities, and much more. In addition, it is now way
easier to perform operations such as slicing, iterating, and tests for equality on collections of neurons and synapses.

NEST 3.0 replaces the old random number generator library :doc:`with a new one <features/random_number_generators>`
based on the C++ Standard Library. The new library also provides generators from the Random 123 library, including
crypotgraphic generators. Most notably, you can now much more easily seed all generators and change the type of random
number generator you are using.

We have improved how :doc:`recordings from simulations <features/recording_simulations>` are handled, making the
infrastructure more modular and extensible. In addition to the previously supported recording methods, a new backend for
SIONlib is now available. The interface for :doc:`stimulation devices <features/stimulation_backends>` can now handle
data from external sources, such as other simulators.

The Topology Module is no longer a separate module; it is integrated within PyNest, and now referred to as support for
:doc:`spatially-structured networks <../../guides/spatially_structured_networks>`.

:doc:`NEST Server <../../../nest_server>` is a novel backend to NEST that allows to run simulations via a RESTful API.
Instead of directly importing ``nest`` into your Python session, the code that controls the simulation is sent over HTTP
to NEST Server in this use-case.

With NEST 3.0, we no longer support Python 2. Check out our :doc:`Installation instructions <../../installation/index>`
for more information on the dependencies.

To explore what NEST 3.0 has to offer in more detail, check out the links below!

----

.. toctree::
   :maxdepth: 1

   New features in NEST 3.0 <features/index>
   Old versus new syntax <refguide_nest2_nest3>
