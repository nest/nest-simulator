.. _topo_changes:

Overview of spatially-structured networks
=========================================

-  All topology functions are now part of ``nest`` and not
   ``nest.topology``
-  You can use the ``Create`` and ``Connect`` functions for spatial  networks, the same as you would for non-spatial
   networks
-  All former topology functions that used to take a layer ID, now take a NodeCollection
-  All former topology functions that used to return node/layer IDs now return a NodeCollection

.. note::

   See the reference section :ref:`topo_ref` in our conversion guide for all changes made to functions


Create spatially distributed nodes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Spatially distributed nodes can now be created using the standard ``nest.Create()`` function.
The arguments of this function have been changed to unify the creation of populations with and without spatial
information. To create nodes with spatial positions, ``nest.Create()`` must be provided with the
``positions`` argument

::

    spatial_nodes = nest.Create(model, positions=spatial_data)

where ``spatial_data`` can be one of the following

- ``nest.spatial.grid()``
    This creates nodes on a grid, with a prescribed number of rows and
    columns, and, if specified, an extent and center. It can be easier to think of the
    grid as being defined by number of elements in x-direction and y-direction instead of
    thinking of rows and columns. Some example grid spatial nodes
    specifications:

    ::

        nest.spatial.grid(shape=[5, 4], extent=[2., 3.])  # 5x4 grid in a 2x3 square
        nest.spatial.grid(shape=[4, 5], center=[1., 1.])  # 4x5 grid in the default 1x1 square, with shifted center
        nest.spatial.grid(shape=[4, 5], edge_wrap=True)  # 4x5 grid with periodic boundary conditions
        nest.spatial.grid(shape=[2, 3, 4])  # 3D 2x3x4 grid

- ``nest.spatial.free()``
    This creates nodes positioned freely in space. The first argument to
    ``nest.spatial.free()`` can either be a NEST parameter that generates
    the positions, or an explicit list of positions. Some example free
    spatial nodes specifications:

    ::

        nest.spatial.free([[5., 1.], [4., 2.], [3., 3.]])  # Three nodes with explicit positions

        nest.spatial.free(nest.random.lognormal(),  # Positions generated from a lognormal distribution
                          num_dimensions=2)         # in 2D

        nest.spatial.free(nest.random.uniform(),  # Positions generated from a uniform distribution
                          num_dimensions=3,       # in 3D
                          edge_wrap=True)         # with periodic boundary conditions

Note the following

- For positions generated from NEST parameters, the number of neurons
  has to be provided in ``nest.Create()`` with the ``n`` argument.
- The extent is calculated from the positions of the nodes, but can be
  set explicitly.
- If possible, NEST tries to deduce the number of dimensions. But if
  the positions are generated from NEST parameters, and there is no
  extent defined, the number of dimensions has to be provided.

  ::

      spatial_nodes = nest.Create('iaf_psc_alpha', n=5,
                                  positions=nest.spatial.free(nest.random.uniform(),
                                                              num_dimensions=3))


Spatially positioned nodes are no longer subnets, as subnets have been removed, but
are rather NodeCollections with metadata. These NodeCollections behave as normal
NodeCollections with two exceptions:

- They cannot be merged, as concatenating NodeCollections with metadata is
  not supported.
- When setting the status of nodes and connecting spatially distributed NodeCollections you can
  use spatial information as parameters.

The second point means that we can use masks and position dependent
parameters when connecting, and it is possible to set parameters of nodes
based on their positions. We can for example set the membrane potential to
a value based on the nodes' position on the x-axis:

::

    snodes = nest.Create('iaf_psc_alpha', 10
                         positions=nest.spatial.free(
                             nest.random.uniform(min=-10., max=10.), num_dimensions=2))
    snodes.V_m=-60. + nest.spatial.pos.x


Composite layers:
    It is no longer possible to create composite layers, i.e. layers with
    multiple nodes in each position. To reproduce this, we now create
    multiple spatially distributed NodeCollections.

      +-------------------------------------------+----------------------------------------------------------------------+
      | NEST 2.x                                  | NEST 3.0                                                             |
      +===========================================+======================================================================+
      |                                           |                                                                      |
      | ::                                        | ::                                                                   |
      |                                           |                                                                      |
      |     l = tp.CreateLayer(                   |     sn_iaf = nest.Create('iaf_psc_alpha'                             |
      |             {'rows': 1,                   |                          positions=nest.spatial.grid(                |
      |              'columns': 2,                |                              shape=[2, 1]))                          |
      |              'elements':                  |                                                                      |
      |                  ['iaf_cond_alpha',       |     sn_poi = nest.Create('poisson_generator',                        |
      |                   'poisson_generator']})  |                           positions=nest.spatial.grid(               |
      |                                           |                               shape=[3, 1]))                         |
      |     Use l when connecting, setting        |                                                                      |
      |     parameters etc.                       |     Use sn_iaf and sn_poi when connecting,                           |
      |                                           |     setting parameters etc.                                          |
      +-------------------------------------------+----------------------------------------------------------------------+


Retrieving spatial information
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To retrieve the spatial information from your nodes, spatially structured NodeCollections have
a ``.spatial`` parameter that will retrieve all spatial information as a dictionary.

>>>  spatial_nodes.spatial
     {'center': (0.41717460937798023, 0.3541409997269511, 0.5058779059909284),
      'edge_wrap': False,
      'extent': (0.6786768797785043, 0.4196595948189497, 0.8852582329884171),
      'network_size': 5,
      'positions': ((0.1951471883803606, 0.24431120231747627, 0.5770208276808262),
       (0.34431440755724907, 0.46397079713642597, 0.8201442817226052),
       (0.17783616948872805, 0.4038907829672098, 0.16324878949671984),
       (0.3796140942722559, 0.2643292499706149, 0.848507022485137),
       (0.6565130492672324, 0.38219101540744305, 0.4020354822278023))}

Note that if you have specified your positions as a NEST parameter, NEST will convert that
to a list with lists, and this is what you will get when calling ``.spatial``.


Connect spatially distributed nodes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Similar to creating nodes with spatial distributions, such nodes are now connected using the
standard ``nest.Connect()`` function. Connecting NodeCollections with
spatial data is no different from connecting NodeCollections without
metadata. In a layer-connection context, moving to the standard
``Connect()`` function brings with it some notable changes:

- Convergent and divergent specification of connection is removed, or
  rather renamed. See table below.

  ======================================= ==================================================
  NEST 2.x                                NEST 3.0
  ======================================= ==================================================
  ``convergent``                          ``pairwise_bernoulli`` with ``use_on_source=True``
  ``convergent`` with ``num_connections`` ``fixed_indegree``
  ``divergent``                           ``pairwise_bernoulli``
  ``divergent`` with ``num_connections``  ``fixed_outdegree``
  ======================================= ==================================================

  ``use_on_source`` here refers to whether the mask and connection probability
  should be applied to the source neuron or the target neuron.
  This is only required for ``pairwise_bernoulli``, as ``fixed_indegree``
  and ``fixed_outdegree`` implicitly only apply to either the source or
  target nodes.

- The connection probability specification ``kernel``  is renamed to ``p``
  to fit with ``pairwise_bernoulli``, and is only possible for the
  connection rules in the table above.

- Using a ``mask`` is only possible with the connection rules in the table
  above.

Usage examples
^^^^^^^^^^^^^^

A grid layer connected with Gaussian distance dependent connection
probability and rectangular mask on the target layer:

  +---------------------------------------------------------+----------------------------------------------------------------------+
  | NEST 2.x                                                | NEST 3.0                                                             |
  +=========================================================+======================================================================+
  |                                                         |                                                                      |
  | ::                                                      | ::                                                                   |
  |                                                         |                                                                      |
  |     l = tp.CreateLayer(                                 |     l = nest.Create('iaf_psc_alpha',                                 |
  |         {'columns': nc, 'rows': nr,                     |                     positions=nest.spatial.grid(                     |
  |          'elements': 'iaf_psc_alpha',                   |                         shape=[nc, nr],                              |
  |          'extent': [2., 2.]})                           |                         extent=[2., 2.]))                            |
  |                                                         |                                                                      |
  |     conn_dict = {'connection_type': 'divergent',        |     conn_dict = {'rule': 'pairwise_bernoulli',                       |
  |                  'kernel': {'gaussian':                 |                  'p': nest.spatial_distributions.gaussian(           |
  |                             {'p_center': 1.,            |                      nest.spatial.distance,                          |
  |                              'sigma': 1.}},             |                      std=1.),                                        |
  |                  'mask': {'rectangular':                |                  'mask': {'rectangular':                             |
  |                           {'lower_left': [-0.5, -0.5],  |                           {'lower_left': [-0.5, -0.5],               |
  |                            'upper_right': [0.5, 0.5]}}} |                            'upper_right': [0.5, 0.5]}}}              |
  |     nest.ConnectLayers(l, l, conn_dict)                 |     nest.Connect(l, l, conn_dict)                                    |
  |                                                         |                                                                      |
  +---------------------------------------------------------+----------------------------------------------------------------------+

A free layer with uniformly distributed positions, connected with fixed
number of outgoing connections, linear distance dependent connection
probability and delay, and random weights from a normal distribution:

  +------------------------------------------------------------------+---------------------------------------------------------------------+
  | NEST 2.x                                                         | NEST 3.0                                                            |
  +==================================================================+=====================================================================+
  |                                                                  |                                                                     |
  | ::                                                               | ::                                                                  |
  |                                                                  |                                                                     |
  |     import numpy as np                                           |     pos = nest.spatial.free(nest.random.uniform(-1., 1.),           |
  |     pos = [[np.random.uniform(-1., 1.),                          |                             num_dimensions=2)                       |
  |             np.random.uniform(-1., 1.)] for j in range(1000)]    |     l = nest.Create('iaf_psc_alpha', 1000, positions=pos)           |
  |     l = tp.CreateLayer({'positions': pos, 'extent': [2., 2.],    |                                                                     |
  |                         'elements': 'iaf_psc_alpha'})            |     conn_dict = {'rule': 'fixed_outdegree',                         |
  |                                                                  |                  'outdegree': 50,                                   |
  |     conn_dict = {'connection_type': 'divergent',                 |                  'p': 1. - 0.5*nest.spatial.distance,               |
  |                  'number_of_connections': 50,                    |                  'weight': nest.random.normal(mean=0., std=1.),     |
  |                  'kernel': {'linear':                            |                  'delay': 1.5*nest.spatial.distance,                |
  |                             {'a': -0.5, 'c': 1.}},               |                  'allow_multapses': True,                           |
  |                  'weights': {'normal':                           |                  'allow_autapses': False}                           |
  |                              {'mean': 0.0, 'sigma': 1.0}},       |     nest.Connect(l, l, conn_dict)                                   |
  |                  'delays': {'linear': {'a': 1.5, 'c': 0.}},      |                                                                     |
  |                  'allow_multapses': True,                        |                                                                     |
  |                  'allow_autapses': False}                        |                                                                     |
  |     tp.ConnectLayers(l, l, conn_dict)                            |                                                                     |
  |                                                                  |                                                                     |
  +------------------------------------------------------------------+---------------------------------------------------------------------+

Masks
^^^^^
In NEST 3.0, the mask ``volume`` got removed, as the same mask was already available under the name ``box``.
The former was only an alias available for backward compatibility.

Retrieving distance information
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
If you have a SynapseCollection with connections from a spatially distributed network, you can retrieve the
*distance* between the source-target pairs by calling ``.distance`` on the SynapseCollection.

  ::

    s_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=[3, 1]))
    t_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=[1, 3]))
    nest.Connect(s_nodes, t_nodes)

    conns = nest.GetConnections()
    dist = conns.distance

``.distance`` will be a tuple of the same length as your SynapseCollection, where ``dist[indx]`` will be the distance
between the source-target pair at *indx*.

Calling ``.distance`` on a SynapseCollection where either the source or target, or both, are not spatially
distributed also works, and you will receive `nan` whenever one of the nodes is non-spatial.
