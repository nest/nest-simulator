All about NEST 3.0
==================

With the release of NEST 3.0, we introduce a ton of new features to improve how you create and manage simulations.

NEST 3.0 introduces a more direct approach to accessing :doc:`node properties (neurons and devices) <nest3_features/nest3_node_handles>`
and :doc:`handling connections (synapses) <nest3_features/nest3_handling_connections>`. The changes will allow you to more easily
perform operations such as slicing, iterating, and testing for equality.
You can now access properties or nodes and connections with the functions ``get()`` and ``set()``.
In addition, :doc:`parameter objects <nest3_features/parametrization>` can be used with mathematical functions, to create spatial positions, define connection
probabilities, and much more.

NEST 3.0 replaces the old random number generator :doc:`with a new one <../random_numbers>` based on the C++ Standard Library. We also provide generators
from the Random 123 library, including crypotgraphic generators. Now you can easily seed generators and change
the type of random number generator you are using.

We have improved how :doc:`recordings from backends <nest3_features/recording_simulations>` are handled, making the infrastructure more modular and extensible.
In addition to the previously supported backends, SIONlib is now available.
The interface for :doc:`stimulation devices <nest3_features/stimulation_backends>` can now handle data from an external source, such as another simulator.

The Topology Module is no longer a separate module; it is integrated within ``nest``, now referred to as :doc:`spatially-structured networks <../spatial/index>`.

:doc:`NEST Server <../../../nest_server>` provides a backend to run simulations that can be deployed locally or on a remote machine.
The code that controls the simulation is sent over HTTP to NEST Server, thus you do not have to directly import ``nest``.

To explore what NEST 3.0 has to offer in more detail, check out the links below!

----

.. toctree::
   :maxdepth: 1

   New features in NEST 3.0 <nest3_features/index>
   Spatially-structured networks (topology module) <../spatial/index>
   Old versus new function syntax <refguide_nest2_nest3>
   whats_removed


.. admonition:: Python 3

   With NEST 3.0, we no longer support Python 2. Running the code snippets throughout this guide requires a freshly
   started instance of Python (and sometimes pyplot from matplotlib). Check out our :doc:`Installation instructions <../../installation/linux_install>` for more
   information on the dependencies.
