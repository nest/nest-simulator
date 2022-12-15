.. _pynest_api:

PyNEST API
==========

The ``nest`` module contains methods and attributes to control the NEST kernel.
This interface is known as PyNEST.

.. _sec:kernel_attributes:

Kernel attributes
-----------------

The NEST kernel can be controlled from the PyNEST interface by getting or
setting attributes on the ``nest`` module:

.. code-block:: python

  # set a single kernel attribute
  nest.resolution = 0.1

  # set multiple attributes at once
  nest.set(min_delay=0.1, max_delay=2.0)

  # if you have the attributes in a dictionary
  params = {'min_delay': 0.1, 'max_delay': 2.0}
  nest.set(**params)

Here is a list of attributes that can be get and/or set on the ``nest`` module:

.. autoclass:: nest.NestModule
    :members:
    :exclude-members: set_communicator

Functions related to models
-------------------------------

.. automodule:: nest.lib.hl_api_models
    :members:

Functions related to the creation and retrieval of nodes (neurons, devices)
----------------------------------------------------------------------------

.. automodule:: nest.lib.hl_api_nodes
    :members:

Functions related to setting and getting parameters
-----------------------------------------------------

.. automodule:: nest.lib.hl_api_info
    :members:

Functions related to connections
---------------------------------

.. automodule:: nest.lib.hl_api_connections
    :members:

Functions related to simulation
---------------------------------

.. automodule:: nest.lib.hl_api_simulation
    :members:

Functions related to parallel computing
----------------------------------------

.. automodule:: nest.lib.hl_api_parallel_computing
    :members:

Functions related to spatially distributed nodes
------------------------------------------------

.. note::

 This used to be a separate topology module. Now, it is integrated into NEST 3.0.

.. automodule:: nest.lib.hl_api_spatial
    :members:

Functions related to NEST types
-------------------------------

.. automodule:: nest.lib.hl_api_types
    :members:

Functions related to helper info
---------------------------------

.. automodule:: nest.lib.hl_api_helper
    :members:

Functions related to randomization
----------------------------------

.. automodule:: nest.random.hl_api_random
    :members:

Functions related to spatial distributions
------------------------------------------

.. automodule:: nest.spatial
    :members: distance

.. automodule:: nest.spatial
    :members: 

.. automodule:: nest.spatial_distributions.hl_api_spatial_distributions
    :members:

Functions related to mathematical expressions
---------------------------------------------

.. automodule:: nest.math.hl_api_math
    :members:

.. automodule:: nest.logic.hl_api_logic
    :members:
