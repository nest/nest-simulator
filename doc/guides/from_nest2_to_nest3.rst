From NEST 2.x to NEST 3.0
=========================

.. contents:: On this page, you'll find
   :local:
   :depth: 2


NEST 3.0 introduces a more direct approach to accessing node properties and handling connections. The changes will allow you to
perform operations that were not possible in previous versions.

.. seealso::

  See the :doc:`../nest-3/nest2_vs_3` to see a full list of functions that have changed


..   NEST 3.0 introduces a number of new features and concepts, and some changes
   to the user interface that are not backwards compatible. One big change is
   the :ref:`removal of subnets <subnet_rm>` and all functions based on subnets. To organize
   neurons, we now use the powerful :ref:`NodeCollections <gid>`, which will be
   presented below. Other big features include :ref:`SynapseCollection` objects to
   efficiently work with connections, :ref:`parameter objects <param_ex>`, and changes to how
   nodes with :ref:`spatial information <topo_changes>` are defined and how to work with them.

This guide is based exclusively on PyNEST.

What's new?
------------

.. _gid:

New functionality for node handles (neurons and devices)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In NEST 3.0, ``nest.Create()`` returns a *NodeCollection* object instead of a list of global IDs.
This provides a more compact and flexible container for node handles.


.. note::

   In **many use cases**, you will not need to make any changes to your scripts in NEST 3.0, unless you have used
   topology or subnets.


NodeCollection supports the following functionality

-  :ref:`Indexing <indexing>`
-  :ref:`Iteration <iterating>`
-  :ref:`Slicing <slicing>`
-  :ref:`Getting the size <get_size>` ``len``
-  :ref:`Conversion to and from lists <converting_lists>`
-  :ref:`Composing two non-overlapping NodeCollections <composing>`
-  :ref:`Testing for equality <testing_equality>` (contains the
   same node IDs)
-  :ref:`Testing of membership <testing_membership>`
-  :ref:`get_param` and  :ref:`set_param` to access node properties, or use :ref:`direct attributes <direct_attributes>` (dot notation e.g.)
-  :ref:`Parametrization <param_ex>`  with spatial, random, distributions, math, and logic parameters


  +---------------------------------------------+----------------------------------------------+
  | NEST 2.x                                    | NEST 3.0                                     |
  +=============================================+==============================================+
  |                                             |                                              |
  | ::                                          | ::                                           |
  |                                             |                                              |
  |     # A list of 10 GIDs is returned         |     # A NodeCollection object is returned    |
  |     nrns = nest.Create('iaf_psc_alpha', 10) |     nrns = nest.Create('iaf_psc_alpha', 10)  |
  |                                             |                                              |
  |     # Use lists as arguments in Connect     |     # Use NodeCollection objects as          |
  |     nest.Connect(nrns, nrns)                |     # arguments in Connect                   |
  |                                             |     nest.Connect(nrns, nrns)                 |
  |                                             |                                              |
  +---------------------------------------------+----------------------------------------------+

.. _GID_support:


Printing
    A compact representation of information about the NodeCollection can be printed


   >>>  nrns = nest.Create('iaf_psc_alpha', 10)
   >>>  print(nrns)
        NodeCollection(metadata=None, model=iaf_psc_alpha, size=10, first=1, last=10)

.. _indexing:

Indexing
    Indexing returns a new NodeCollection with a single node



   >>>  print(nrns[3])
        NodeCollection(metadata=None, model=iaf_psc_alpha, size=1, first=3)

.. _iterating:

Iteration
    You can iterate the nodes in a NodeCollection and receive a single element NodeCollection

     >>>   for node in nrns:
     >>>       print(node.global_id)
           1
           2
           3
           4
           5
           6
           7
           8
           9
           10

.. _slicing:

Slicing
    A NodeCollection can be sliced in the same way one would slice a list,
    with ``start:stop:step`` inside brackets


    >>>  print(nrns[2:9:3])
         NodeCollection(metadata=None,
                       model=iaf_psc_alpha, size=2, first=3, last=9, step=3)


.. _get_size:

Getting the size
    You can easily get the number of nodes in the NodeCollection with

   >>>  len(nrns)
        10

.. _converting_lists:

Conversion to and from lists
    NodeCollections can be converted to lists of node IDs


    >>>  nrns.tolist()
         [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    And you can create a NodeCollection by providing a list, tuple, numpy array or range of node IDs

    >>>  print(nest.NodeCollection([2, 3, 4, 8]))
         NodeCollection(metadata=None,
                        model=iaf_psc_alpha, size=3, first=2, last=4;
                        model=iaf_psc_alpha, size=1, first=8)
    >>>  print(nest.NodeCollection(range(1,4)))
         NodeCollection(metadata=None, model=iaf_psc_alpha, size=3, first=1, last=3)

    Note however that the nodes have to be already created. If any
    of the node IDs refer to nodes that are not created, an error is thrown.

.. _composing:

Composing
    When composing two NodeCollections, NEST tries to concatenate the
    two into a single NodeCollection.


    >>>  nrns = nest.Create('iaf_psc_alpha', 10)
    >>>  nrns_2 = nest.Create('iaf_psc_alpha', 3)
    >>>  print(nrns + nrns_2)
         NodeCollection(metadata=None, model=iaf_psc_alpha, size=13, first=1, last=13)

    If the node IDs are not continuous or the models are different, a composite will be created:

    >>>  nrns_3 = nest.Create('iaf_psc_delta', 3)
    >>>  print(nrns + nrns_3)
         NodeCollection(metadata=None,
                        model=iaf_psc_alpha, size=10, first=1, last=10;
                        model=iaf_psc_delta, size=3, first=14, last=16)

    Note that composing NodeCollections that overlap or that contain metadata
    (see section on :ref:`spatially distributed nodes <topo_changes>`) is impossible.

.. _testing_equality:

Test of equality
    You can test if two NodeCollections are equal, i.e. that they contain the same node IDs

    >>>  nrns == nrns_2
         False
    >>>  nrns_2 == nest.NodeCollection([11, 12, 13])
         True

.. _testing_membership:

Test of membership
    You can test if a NodeCollection contains a certain ID

    >>>  2 in nrns
         True
    >>>  11 in nrns
         False

.. _direct_attributes:

Direct attributes
    You can directly get and set parameters of your NodeCollection
    
    >>> nrns.V_m = [-70., -60., -50., -40., -30., -20., -10., -20., -30., -40.]
    >>> nrns.V_m
        (-70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, -20.0, -30.0, -40.0)
    >>> nrns.C_m = 111.
    >>> nrns.C_m 
        (111.0, 111.0, 111.0, 111.0, 111.0, 111.0, 111.0, 111.0, 111.0, 111.0)

    If your nodes are spatially distributed (see :ref:`spatially distributed nodes <topo_changes>`),
    you can also get the spatial properties of the nodes

    >>> spatial_nodes.spatial
        {'center': (0.0, 0.0),
         'edge_wrap': False,
         'extent': (1.0, 1.0),
         'network_size': 4,
         'shape': (2, 2)}


.. _get_param:

Get the node status
~~~~~~~~~~~~~~~~~~~~~~

``get()`` returns the parameters in the collection. You can call ``get()`` in
several ways.

To get all parameters in the collection, use ``get()`` without any function arguments.
This returns a dictionary with tuples.

>>>    nodes_exp = nest.Create('iaf_psc_exp', 5)
>>>    nodes_exp[:3].get()
       {'archiver_length': (0, 0, 0),
        'beta_Ca': (0.001, 0.001, 0.001),
        'C_m': (250.0, 250.0, 250.0),
        'Ca': (0.0, 0.0, 0.0),
        'delta': (0.0, 0.0, 0.0),
        'E_L': (-70.0, -70.0, -70.0),
        'element_type': ('neuron', 'neuron', 'neuron'),
        'frozen': (False, False, False),
        'global_id': (11, 12, 13),
        'I_e': (0.0, 0.0, 0.0),
        'local': (True, True, True),
        'model': ('iaf_psc_exp', 'iaf_psc_exp', 'iaf_psc_exp'),
        'node_uses_wfr': (False, False, False),
        'post_trace': (0.0, 0.0, 0.0),
        'recordables': (('I_syn_ex',
          'I_syn_in',
          'V_m',
          'weighted_spikes_ex',
          'weighted_spikes_in'),
         ('I_syn_ex', 'I_syn_in', 'V_m', 'weighted_spikes_ex', 'weighted_spikes_in'),
         ('I_syn_ex', 'I_syn_in', 'V_m', 'weighted_spikes_ex', 'weighted_spikes_in')),
        'rho': (0.01, 0.01, 0.01),
        'supports_precise_spikes': (False, False, False),
        'synaptic_elements': ({}, {}, {}),
        't_ref': (2.0, 2.0, 2.0),
        't_spike': (-1.0, -1.0, -1.0),
        'tau_Ca': (10000.0, 10000.0, 10000.0),
        'tau_m': (10.0, 10.0, 10.0),
        'tau_minus': (20.0, 20.0, 20.0),
        'tau_minus_triplet': (110.0, 110.0, 110.0),
        'tau_syn_ex': (2.0, 2.0, 2.0),
        'tau_syn_in': (2.0, 2.0, 2.0),
        'thread': (0, 0, 0),
        'thread_local_id': (-1, -1, -1),
        'V_m': (-70.0, -70.0, -70.0),
        'V_reset': (-70.0, -70.0, -70.0),
        'V_th': (-55.0, -55.0, -55.0),
        'vp': (0, 0, 0)}

To get specific parameters in the collection, use
``get([parameter_name_1, parameter_name_2, ... , parameter_name_n])``.

Get the parameters `V_m` and `V_reset` of all nodes

>>>    nodes = nest.Create('iaf_psc_alpha', 10, {'V_m': -55.})
>>>    nodes.get(['V_m', 'V_reset'])
       {'V_m': (-55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0),
        'V_reset': (-70.0,
         -70.0,
         -70.0,
         -70.0,
         -70.0,
         -70.0,
         -70.0,
         -70.0,
         -70.0,
         -70.0)}

To get a specific parameter from the collection, you can use ``get(parameter_name)``.
This will return a tuple with the values of that parameter.

>>>    nodes.get('t_ref')
       (2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0)

If you have a single-node NodeCollection, ``get()`` will return a dictionary with
single values or a single value, depending on how it is called.

>>>    nodes[0].get(['V_m', 'V_reset'])
       {'V_m': -55.0, 'V_reset': -70.0}
>>>    nodes[0].get('t_ref')
       2.0

To select fields at a deeper hierarchy level, use ``get(parameter_name, property_name)``,
this will return an array. You can also use ``get(parameter_name, [property_name_1, ..., property_name_n])``
and get a dictionary with arrays.

>>>    sd = nest.Create('spike_detector')
>>>    sd.get('events', 'senders')
       array([], dtype=int64)

Lastly, you can specify the output format (pandas, JSON currently implemented). The
output format can be specified for all the different ``get()`` versions above.

>>>    nodes[0].get(['V_m', 'V_reset'], output='json')
       '{"V_m": -55.0, "V_reset": -70.0}'


.. _set_param:

Set the node status
~~~~~~~~~~~~~~~~~~~~~~~

``set()`` sets the values of a parameter by iterating over each node.

As with ``get()``, you can set parameters in different ways.

To set several parameters at once, use ``nodes.set(parameter_dict)``, where the
keyword of the parameter_dict is the parameter name. The value could be a list
the size of the NodeCollection, a single value, or a ``nest.Parameter``.

>>> nodes[:3].set({'V_m': [-70., -80., -90.], 'C_m': 333.})

You could also set a single parameter by using ``nodes.set(parameter_name=parameter)``.
As parameter, you can either send in a single value, a list the size of the NodeCollection,
or a ``nest.Parameter``

>>> nodes.set(t_ref=3.0)
>>> nodes[:3].set(t_ref=[3.0, 4.0, 5.0])
>>> nodes.set(t_ref=nest.random.uniform())

Note that some parameters, like `global_id`, cannot be set.


.. _SynapseCollection:

New functionality for handling connections (synapses)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Just like a NodeCollection is a container for node IDs, a SynapseCollection is a
container for connections. In NEST 3, when you call ``GetConnections()`` a
SynapseCollection is returned. SynapseCollections support a lot of the same operations
as NodeCollections.

``SynapseCollection`` supports:

-  :ref:`Indexing <conn_indexing>`
-  :ref:`Iteration <conn_iterating>`
-  :ref:`Slicing <conn_slicing>`
-  :ref:`Getting the size <conn_size>` ``len``
-  :ref:`Testing for equality <conn_testing_equality>`
-  :ref:`Get connection parameters <conn_get>`
-  :ref:`Set connection parameters <conn_set>`
-  :ref:`Setting and getting attributes directly <conn_direct_attributes>`
-  :ref:`Iterator of sources and targets <conn_s_t_iterator>`

.. seealso::

    You can find a :doc:`full example <../examples/SynapseCollection>` in our example network page


Printing
    Printing a SynapseCollection produces a table of source and target node IDs

    >>>  nest.Connect(nodes[:2], nodes[:2])
    >>>  synColl = nest.GetConnections()
    >>>  print(synColl)
         *--------*-------------*
         | source | 1, 1, 2, 2, |
         *--------*-------------*
         | target | 1, 2, 1, 2, |
         *--------*-------------*

.. _conn_indexing:

Indexing
    Indexing returns a single connection SynapseCollection.

    >>>  print(synColl[1])
         *--------*----*
         | source | 1, |
         *--------*----*
         | target | 9, |
         *--------*----*


.. _conn_iterating:

Iteration
    A SynapseCollection can be iterated, yielding a single connection SynapseCollections.

    >>>  for conn in synColl:
    >>>      print(conn.source)
         1
         1
         2
         2


.. _conn_slicing:

Slicing
    A SynapseCollection can be sliced with ``start:stop:step`` inside brackets

    >>>  print(synColl[0:3:2])
         *--------*-------*
         | source | 1, 2, |
         *--------*-------*
         | target | 1, 1, |
         *--------*-------*

.. _conn_size:

Getting the size
    We can get the number of connections in the SynapseCollection with

    >>>  len(synColl)
         4


.. _conn_testing_equality:

Test of equality
    Two SynapseCollections can be tested for equality, i.e. that they contain the same connections.

    >>>  synColl == synColl
         True
    >>>  synColl[:2] == synColl[2:]
         False


.. _conn_get:

Getting connection parameters
    Just as with NodeCollection, you can get parameters of the connections with
    ``get()``. The same function arguments as for :ref:`NodeCollections get() <get_param>`
    applies here. The returned values also follow the same rules. 

    If you call ``get()`` without any input variables, a dictionary with all parameters is
    returned as a list if number of connections is bigger than 1 and a single integer if
    number of connections is 1.

    >>>  synColl.get()
         {'delay': [1.0, 1.0, 1.0, 1.0],
          'port': [0, 1, 2, 3],
          'receptor': [0, 0, 0, 0],
          'sizeof': [32, 32, 32, 32],
          'source': [1, 1, 2, 2],
          'synapse_id': [0, 0, 0, 0],
          'synapse_model': ['static_synapse',
           'static_synapse',
           'static_synapse',
           'static_synapse'],
          'target': [1, 2, 1, 2],
          'target_thread': [0, 0, 0, 0],
          'weight': [1.0, 1.0, 1.0, 1.0]}

    Calling ``get(parameter_name)`` will return a list of parameter values, while
    ``get([parameter_name_1, ... , parameter_name_n])`` returns a dictionary with
    the values.

    >>>  synColl.get('weight')
         [1.0, 1.0, 1.0, 1.0]

    >>>  synColl[2].get(['source', 'target'])
         {'source': 2, 'target': 1}

    It is also possible to select an alternative output format with the
    ``output`` keyword. Currently it is possible to get the output in a
    json format, or a Pandas format (if Pandas is installed).

.. _conn_set:

Setting connection parameters
    Likewise, you can set the parameters of connections in the SynapseCollection.
    Again the same rules as with ``set()`` on NodeCollection applies, see :ref:`NodeCollection set() <set_param>`
    for more.

    If you want to set several parameters at once, use ``set(parameter_dictionary)``.
    You can use a single value, a list, or a ``nest.Parameter`` as values. If a single value is given,
    the value is set on all connections.

    >>>  synColl.set({'weight': [1.5, 2.0, 2.5, 3.0], 'delay': 2.0})

    Updating a single parameter is done by calling ``set(parameter_name=parameter_value)``.
    Again you can use a single value, a list, or a ``nest.Parameter`` as value.

    >>>  synColl.set(weight=3.7)

    >>>  synColl.set(weight=[4.0, 4.5, 5.0, 5.5])

    Note that some parameters, like `source` and `target`, cannot be set.

.. _conn_direct_attributes:

Setting and getting attributes directly
    You can also directly get and set parameters of your SynapseCollection
    
    >>>  synColl.weight = 5.
    >>>  synColl.weight
         [5.0, 5.0, 5.0, 5.0]
    >>>  synColl.delay = [5.1, 5.2, 5.3, 5.4]
    >>>  synColl.delay
         [5.1, 5.2, 5.3, 5.4]

    If you use a list to set the parameter, the list needs to be the same length
    as the SynapseCollection.

.. _conn_s_t_iterator:

Iterator of sources and targets
    Calling ``SynapseCollection.sources()`` or ``SynapseCollection.targets()`` returns an
    iterator over the source IDs or target IDs, respectively.

    >>>  print([s*3 for s in synColl.sources()])
         [3, 3, 6, 6]

.. _param_ex:

Parametrization
~~~~~~~~~~~~~~~~

NEST 3.0 introduces *Parameter objects*, i.e., objects that represent values
drawn from a random distribution or values based on various spatial node
parameters. Parameters can be used to set node status, to create positions
in topology (see :ref:`Topology section <topo_changes>` below), and to define connection
probabilities, weights and delays. The Parameters can be combined in
different ways, and they can be used with some mathematical functions that
are provided by NEST.

The following parameters and functionalities are provided:

-  :ref:`Random parameters <random_ex>`
-  :ref:`Spatial parameters <spatial_ex>`
-  :ref:`Spatially distributed parameters <distrib_ex>`
-  :ref:`Mathematical functions <math_ex>`
-  :ref:`Clipping, redrawing, and conditional parameters <logic>`
-  :ref:`Combination of parameters <combine_ex>`


.. _random_ex:

Random parameters
^^^^^^^^^^^^^^^^^
The random module contains random distributions that can be used to set node
and connection parameters, as well as positions for spatially distributed nodes.

  +--------------------------------------------------+--------------------------------------------+
  | Parameter                                        | Description                                |
  +==================================================+============================================+
  |  ::                                              |                                            |                                  
  |                                                  |                                            |
  |     nest.random.uniform(min=0.0, max=1.0)        | Draws samples based on a                   |
  |                                                  | uniform distribution.                      |
  +--------------------------------------------------+--------------------------------------------+
  |  ::                                              |                                            |
  |                                                  |                                            |
  |     nest.random.normal(mean=0.0, std=1.0)        | Draws samples based on a                   |
  |                                                  | normal distribution.                       |
  +--------------------------------------------------+--------------------------------------------+
  |  ::                                              |                                            |
  |                                                  |                                            |
  |     nest.random.exponential(beta=1.0)            | Draws samples based on a                   |
  |                                                  | exponential distribution.                  |
  +--------------------------------------------------+--------------------------------------------+
  |  ::                                              |                                            |
  |                                                  |                                            |
  |     nest.random.lognormal(mean=0.0, std=1.0)     | Draws samples based on a                   |
  |                                                  | lognormal distribution.                    |
  +--------------------------------------------------+--------------------------------------------+

For every value to be generated, samples are drawn from a distribution. The distribution uses
NEST's random number generator, and are therefore thread-safe. Note that
arguments can be passed to each of the distributions above to control the parameters of the
distribution.

.. code-block:: ipython

    nest.ResetKernel()

    n = nest.Create('iaf_psc_alpha', 10000, {'V_m': nest.random.normal(mean=-60., std=10.)})

    gids = n.global_id
    v_m = n.get('V_m')
    fig, ax = plt.subplots(figsize=(12, 6),
                           gridspec_kw={'width_ratios':
                                        [3, 1]},
                           ncols=2,
                           sharey=True)
    ax[0].plot(gids, v_m, '.', alpha=0.5, ms=3.5)
    ax[0].set_xlabel('GID');
    ax[1].hist(v_m, bins=40, orientation='horizontal');
    ax[1].set_xlabel('num. nodes');
    ax[0].set_ylabel('V_m');


.. image:: ../nest-3/NEST3_13_0.png


.. _spatial_ex:

Spatial parameters
^^^^^^^^^^^^^^^^^^
  The spatial module contains parameters related to spatial positions of the
  nodes.

  To create spatially distributed nodes (see section on :ref:`spatially distributed nodes <topo_changes>` for more),
  use ``ǹest.spatial.grid()`` or ``nest.spatial.free``.

  +----------------------------------------------------+-------------------------------------------------------+
  | Parameter                                          | Description                                           |
  +====================================================+=======================================================+
  |  ::                                                |                                                       |
  |                                                    | Create spatially positioned nodes distributed on a    |
  |     nest.spatial.grid(shape, center=None,          | grid with dimensions given by `shape`.                |
  |         extent=None, edge_wrap=False)              |                                                       |
  +----------------------------------------------------+-------------------------------------------------------+
  |  ::                                                |                                                       |
  |                                                    | Create spatially positioned nodes distributed freely  |
  |     nest.spatial.free(pos, extent=None,            | in space with dimensions given by `pos` or            |
  |         edge_wrap=False, num_dimensions=None)      | `num_dimensions`.                                     |
  |                                                    |                                                       |
  +----------------------------------------------------+-------------------------------------------------------+

  .. code-block:: ipython

    grid_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=[10, 8]))
    nest.PlotLayer(grid_nodes);

  .. image:: ../nest-3/NEST3_23_0.png


  .. code-block:: ipython

    free_nodes = nest.Create('iaf_psc_alpha', 100,
                             positions=nest.spatial.free(nest.random.uniform(min=0., max=10.),
                                                         num_dimensions=2))
    nest.PlotLayer(free_nodes);

  .. image:: ../nest-3/NEST3_24_0.png


  After you have created your spatially distributed nodes, you can use spatial parameters to set
  node or connection parameters.

  +----------------------------------+-------------------------------------------------------------------------+
  | Parameter                        | Description                                                             |
  +==================================+=========================================================================+
  |  ::                              |                                                                         |
  |                                  |                                                                         |
  |     nest.spatial.pos.x           | | Position of a neuron, on the x, y, and z axis.                        |
  |     nest.spatial.pos.y           | | Can be used to set node properties, but not for connecting.           |
  |     nest.spatial.pos.z           |                                                                         |
  +----------------------------------+-------------------------------------------------------------------------+
  |  ::                              |                                                                         |
  |                                  |                                                                         |
  |     nest.spatial.source_pos.x    | | Position of the source neuron, on the x, y, and z axis.               |
  |     nest.spatial.source_pos.y    | | Can only be used when connecting.                                     |
  |     nest.spatial.source_pos.z    |                                                                         |
  +----------------------------------+-------------------------------------------------------------------------+
  |  ::                              |                                                                         |
  |                                  |                                                                         |
  |     nest.spatial.target_pos.x    |                                                                         |
  |     nest.spatial.target_pos.y    | | Position of the target neuron, on the x, y, and z axis.               |
  |     nest.spatial.target_pos.z    | | Can only be used when connecting.                                     |
  +----------------------------------+-------------------------------------------------------------------------+
  |  ::                              |                                                                         |
  |                                  |                                                                         |
  |     nest.spatial.distance        | | Distance between two nodes. Can only be used when connecting.         |
  +----------------------------------+-------------------------------------------------------------------------+
  |  ::                              |                                                                         |
  |                                  |                                                                         |
  |     nest.spatial.distance.x      |                                                                         |
  |     nest.spatial.distance.y      | | Distance on the x, y and z axis between the source and target neuron. |
  |     nest.spatial.distance.z      | | Can only be used when connecting.                                     |
  +----------------------------------+-------------------------------------------------------------------------+

  These Parameters represent positions of neurons or distances between two
  neurons. To set node parameters, only the node position can be used. The
  others can only be used when connecting.


  .. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free([[x, 0.5*x] for x in np.linspace(0, 1.0, 10000)])
    spatial_nodes = nest.Create('iaf_psc_alpha', positions=positions)

    parameter = -60 + nest.spatial.pos.x + (0.4 * nest.spatial.pos.x * nest.random.normal())
    spatial_nodes.set({'V_m': parameter})

    node_pos = np.array(nest.GetPosition(spatial_nodes))
    node_pos[:,1]
    v_m = spatial_nodes.get('V_m');

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(node_pos[:,0], v_m, '.', ms=3.5)
    ax.set_xlabel('Node position on x-axis')
    ax.set_ylabel('V_m');

  .. image:: ../nest-3/NEST3_25_0.png

  NEST provides some functions to help create distributions based on for
  example the distance between two neurons.



.. _distrib_ex:

Spatial distribution functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The spatial_distributions module contains random distributions that take a spatial
parameter as input and applies the distribution on the parameter. They are used
for spatially distributed nodes.

  +---------------------------------------------------+----------------------------------------------------------------------------------------------------+
  | Parameter                                         | Description                                                                                        |
  +===================================================+====================================================================================================+
  |  ::                                               | .. math::                                                                                          |
  |                                                   |      p(x) = \exp{\left(-\frac{x}{\beta}\right)}                                                    |
  |     nest.spatial_distributions.exponential(x,     |                                                                                                    |
  |         beta=1.0)                                 |                                                                                                    |
  +---------------------------------------------------+----------------------------------------------------------------------------------------------------+
  |  ::                                               | .. math::                                                                                          |
  |                                                   |      p(x) = \exp{\left(-\frac{(x-\mu)^2}{2\sigma^2}\right)}                                        |
  |     nest.spatial_distributions.gaussian(x,        |                                                                                                    |
  |         mean=0.0, std=1.0)                        |                                                                                                    |
  +---------------------------------------------------+----------------------------------------------------------------------------------------------------+
  |  ::                                               | .. math::                                                                                          |
  |                                                   |      p(x) = \exp{\left(-\frac{\frac{(x-\mu_x)^2}{2\sigma_x^2} - \frac{(y-\mu_y)^2}{2\sigma_y^2} +  |
  |     nest.spatial_distributions.gaussian2D(x, y,   |      2\rho\frac{(x-\mu_x)(y-\mu_y)}{2\sigma_x*\sigma_y}}{2(1-\rho^2)}\right)}                      |
  |         mean_x=0.0, mean_y=0.0, std_x=1.0,        |                                                                                                    |
  |         std_y=1.0, rho=0.0)                       |                                                                                                    |
  +---------------------------------------------------+----------------------------------------------------------------------------------------------------+
  |  ::                                               | .. math::                                                                                          |
  |                                                   |      p(x) = \frac{x^{\kappa-1}e^{-\frac{x}{\theta}}}{\theta^\kappa\Gamma(\kappa)}                  |
  |     nest.spatial_distributions.gamma(x, kappa=1.0,|                                                                                                    |
  |         theta=1.0)                                |                                                                                                    |
  +---------------------------------------------------+----------------------------------------------------------------------------------------------------+

Note that ``x`` and ``y`` are ``nest.Parameter`` types.

With these functions, you can, for example, recreate a Gaussian kernel as a parameter:


  +------------------------------------------------+-----------------------------------------------------------------+
  | NEST 2.x                                       | NEST 3.0                                                        |
  +------------------------------------------------+-----------------------------------------------------------------+
  |  ::                                            |  ::                                                             |
  |                                                |                                                                 |
  |     kernel = {"gaussian": {"p_center": 2.0,    |     param = 2*nest.spatial_distributions.gaussian(              |
  |                            "sigma": 1.0}}      |         nest.spatial.distance, std=1.0)                         |
  |                                                |                                                                 |
  +------------------------------------------------+-----------------------------------------------------------------+


.. code-block:: ipython

    nest.ResetKernel()

    N = 21
    middle_node = N//2

    positions = nest.spatial.free([[x, 0.] for x in np.linspace(0, 1.0, N)])
    spatial_nodes = nest.Create('iaf_psc_alpha', positions=positions)

    parameter = nest.spatial_distributions.exponential(nest.spatial.distance, beta_Ca=0.15)

    # Iterate connection to get statistical connection data
    for _ in range(2000):
        nest.Connect(spatial_nodes[middle_node], spatial_nodes,
                     conn_spec={'rule': 'pairwise_bernoulli',
                                'p': parameter})

    targets = nest.GetConnections().get('target')

    fig, ax = plt.subplots(figsize=(12, 6))
    bars = ax.hist(targets, bins=N, edgecolor='black', linewidth=1.2)

    plt.xticks(bars[1] + 0.5,np.arange(1, N+1))
    ax.set_title('Connections from node with GID {}'.format(spatial_nodes[middle_node].get('global_id')))
    ax.set_xlabel('Target GID')
    ax.set_ylabel('Num. connections');

.. image:: ../nest-3/NEST3_34_0.png



.. _math_ex:

Mathematical functions
^^^^^^^^^^^^^^^^^^^^^^

  +----------------------------+---------------------------------------------+
  | Parameter                  | Description                                 |
  +============================+=============================================+
  | ::                         |                                             |
  |                            |                                             |
  |     nest.random.exp(x)     | | Calculates the exponential of a Parameter |
  +----------------------------+---------------------------------------------+
  | ::                         |                                             |
  |                            |                                             |
  |     nest.random.cos(x)     | | Calculates the cosine of a Parameter      |
  +----------------------------+---------------------------------------------+
  | ::                         |                                             |
  |                            |                                             |
  |     nest.random.sin(x)     | | Calculates the sine of a Parameter        |
  +----------------------------+---------------------------------------------+

The mathematical functions take a parameter object as argument, and return
a new parameter which applies the mathematical function on the Parameter
given as argument.

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free([[x, 0.5*x] for x in np.linspace(0, 1.0, 100)])
    spatial_nodes = nest.Create('iaf_psc_alpha', positions=positions)

    parameter = -60 + nest.math.exp(nest.spatial.pos.x**4)
    # Also available:
    #   - nest.math.sin()
    #   - nest.math.cos()

    spatial_nodes.set({'V_m': parameter})

    node_pos = np.array(nest.GetPosition(spatial_nodes))
    node_pos[:,1]
    v_m = spatial_nodes.get('V_m');

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(node_pos[:,0], v_m, '.', ms=6.5)
    ax.set_xlabel('Node position on x-axis')
    ax.set_ylabel('V_m');



.. image:: ../nest-3/NEST3_27_0.png

.. _logic:

Clipping, redraw, and conditionals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  +----------------------------------------------------+-----------------------------------------------------+
  | Parameter                                          | Description                                         |
  +====================================================+=====================================================+
  | ::                                                 |                                                     |
  |                                                    |                                                     |
  |     nest.math.min(x, value)                        | If a value from the Parameter is above a threshold, |
  |                                                    | the value is replaced with the value of the         |
  |                                                    | threshold.                                          |
  +----------------------------------------------------+-----------------------------------------------------+
  | ::                                                 |                                                     |
  |                                                    |                                                     |
  |     nest.math.max(x, value)                        | If a value from the Parameter is beneath a          |
  |                                                    | threshold, the value is replaced with the value of  |
  |                                                    | the threshold.                                      |
  +----------------------------------------------------+-----------------------------------------------------+
  | ::                                                 |                                                     |
  |                                                    |                                                     |
  |     nest.math.redraw(x, min, max)                  | If a value from the Parameter is outside of the     |
  |                                                    | limits given, the value is redrawn. Throws an error |
  |                                                    | if a suitable value is not found after a certain    |
  |                                                    | number of redraws.                                  |
  +----------------------------------------------------+-----------------------------------------------------+
  | ::                                                 |                                                     |
  |                                                    |                                                     |
  |     nest.logic.conditional(x, val_true, val_false) | Given a condition, yields one value or another      |
  |                                                    | based on if the condition evaluates to true or      |
  |                                                    | false.                                              |
  +----------------------------------------------------+-----------------------------------------------------+

Note that ``x`` is a ``nest.Parameter``.

The ``nest.math.min()`` and ``nest.math.max()`` functions are used to clip
a Parameter. Essentially they work like the standard ``min()`` and
``max()`` functions, ``nest.math.min()`` yielding the smallest of two
values, and ``nest.math.max()`` yielding the largest of two values.

::

    # This yields values between 0.0 and 0.5, where values from the
    # distribution that are above 0.5 gets set to 0.5.
    nest.math.min(nest.random.uniform(), 0.5)

    # This yields values between 0.5 and 1.0, where values from the
    # distribution that are below 0.5 gets set to 0.5.
    nest.math.max(nest.random.uniform(), 0.5)

    # This yields values between 0.2 and 0.7, where values from the
    # distribution that are smaller than 0.2 or larger than 0.7 gets
    # redrawn from the distribution.
    nest.math.redraw(nest.random.uniform(), min=0.2, max=0.7)

The ``nest.logic.conditional()`` function works like an ``if``/``else``
statement. Three arguments are required:

- The first argument is a condition.
- The second argument is the resulting value or Parameter evalued if the
  condition evaluates to true.
- The third argument is the resulting value or Parameter evalued if the
  condition evaluates to false.

::

    # A heaviside step function with uniformly distributed input values.
    nest.logic.conditional(nest.random.uniform(min=-1., max=1.) < 0., 0., 1.)

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free([[x, 0.5*x] for x in np.linspace(0, 1.0, 50)])
    spatial_nodes = nest.Create('iaf_psc_alpha', positions=positions)

    spatial_nodes.set(V_m=nest.logic.conditional(nest.spatial.pos.x < 0.5,
                                                 -55 + 10*nest.spatial.pos.x,
                                                 -55))

    node_pos = np.array(nest.GetPosition(spatial_nodes))
    node_pos[:,1]
    v_m = spatial_nodes.get('V_m');

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(node_pos[:,0], v_m, 'o')
    ax.set_xlabel('Node position on x-axis')
    ax.set_ylabel('V_m');



.. image:: ../nest-3/NEST3_26_0.png


.. _combine_ex:

Combine parameters
^^^^^^^^^^^^^^^^^^^^

NEST parameters support the basic arithmetic operations. Two parameters
can be added together, subtracted, multiplied with each other, or one can
be divided by the other. They also support being raised to the power of a
number, but they can only be raised to the power of an integer or a
floating point number. Parameters can therefore be combined in almost any
way. In fact the distribution functions in ``nest.spatial_distributions`` are just
arithmetic expressions defined in Python.

Some examples:

::

    # A uniform distribution yielding values in the range (-44., -64.).
    p = -54. + nest.random.uniform(min=-10., max=10)

    # Two random distributions combined, with shifted center.
    p = 1.0 + 2 * nest.random.exponential() * nest.random.normal()

    # The node position on the x-axis, combined with a noisy y-axis component.
    p = nest.spatial.pos.x + (0.4 * nest.spatial.pos.y * nest.random.normal())

    # The quadratic distance between two nodes, with a noisy distance component.
    p = nest.spatial.distance**2 + 0.4 * nest.random.uniform() * nest.spatial.distance

Use parameters to set node properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Using parameters makes it easy to set node properties

  +-----------------------------------------------+----------------------------------------------------+
  | NEST 2.x                                      | NEST 3.0                                           |
  +===============================================+====================================================+
  |                                               |                                                    |
  | ::                                            | ::                                                 |
  |                                               |                                                    |
  |     for gid in nrns:                          |     nrns.V_m=nest.random.uniform(-20., 20)         |
  |         v_m = numpy.random.uniform(-20., 20.) |                                                    |
  |         nest.SetStatus([gid], {'V_m': V_m})   |                                                    |
  |                                               |                                                    |
  |                                               |                                                    |
  +-----------------------------------------------+----------------------------------------------------+

What's changed?
----------------

With NEST 3.0, we no longer support Python 2.

.. _param_changes:

Model parameter changes and changed functionalities
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use term synapse_model throughout:
    As all PyNEST functions that used to take the list returned by ``Create`` now use the NodeCollection
    returned by ``Create``, there shouldn't be too many changes on the PyNEST level. One important
    change though, is that we now use ``synapse_model`` throughout to reference the synapse model.

    Most importantly, this will change your ``Connect`` call, where instead of passing the synapse
    model with the ``model`` key, you should now use the ``synapse_model`` key.

    >>>  nrns = nest.Create('iaf_psc_alpha', 3)
    >>>  nest.Connect(nrns, nrns, 'one_to_one', syn_spec={'synapse_model': 'stdp_synapse'})

    Simillarly, ``GetDefaults`` used to return an entry called ``synapsemodel``. It now returns and entry
    called ``synapse_model``.

Use allow_offgrid_times throughout:
    In the model ``spike_generator``, the parameter ``allow_offgrid_spikes`` is renamed 
    ``allow_offgrid_times`` for consistency with other models.

Use unit ms instead of number of simulation steps:
    The ``structural_plasticity_update_interval`` now has the unit ms instead of
    number of simulation steps.


.. _topo_changes:

Topology module
~~~~~~~~~~~~~~~

-  All topology functions are now part of ``nest`` and not
   ``nest.topology``
-  You can use the ``Create`` and ``Connect`` functions for spatial  networks, same as you would for non-spatial
   network
-  All former topology functions that used to take a layer ID, now take a NodeCollection
-  All former topology functions that used to return node/layer IDs now return a NodeCollection

.. note::

   See the reference section :ref:`topo_ref` in our conversion guide for all changes made to functions

All of the functionality of Topology has been moved to the standard
functions. In fact, there is no longer a Topology module in PyNEST. The
functions that are specific for Topology are now in the ``nest`` module.

Create spatially distributed nodes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Creating spatially distributed nodes are now done with the standard ``nest.Create()`` function.
Arguments of node creation have also been changed to make creating
populations with and without spatial information more unified. To create
nodes with spatial positions, ``nest.Create()`` must be provided with the
``positions`` argument

::

    spatial_nodes = nest.Create(model, positions=spatial_data)

where ``spatial_data`` can be one of the following

- ``nest.spatial.grid()``
    This creates nodes on a grid, with a prescribed number of rows and
    columns, and, if specified, an extent and center. Some example grid spatial nodes
    specifications:

    ::

        nest.spatial.grid(shape=[5, 4], extent=[2., 2.])  # 5x4 grid in a 2x2 square
        nest.spatial.grid(shape=[4, 5], center=[1., 1.])  # 4x5 grid in the default 1x1 square, with shifted center
        nest.spatial.grid(shape=[4, 5], edge_wrap=True)  # 4x5 grid with periodic boundary conditions
        nest.spatial.grid(shape=[2, 3, 4])  # 3D 2x3x4 grid

- ``nest.spatial.free()``
    This creates nodes positioned freely in space. The first argument to
    ``nest.spatial.free()`` can either be a NEST Parameter that generates
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

- For positions generated from NEST Parameters, the number of neurons
  has to be provided in ``nest.Create()``.
- The extent is calculated from the positions of the nodes, but can be
  set explicitly.
- If possible, NEST tries to deduce the number of dimensions. But if
  the positions are generated from NEST Parameters, and there is no
  extent defined, the number of dimensions has to be provided.

  ::

      spatial_nodes = nest.Create('iaf_psc_alpha', n=5,
                                  positions=nest.spatial.free(nest.random.uniform(),
                                                              num_dimensions=3))


Spatially positioned nodes are no longer subnets, as subnets have been removed, but
NodeCollections with metadata. These NodeCollections behave as normal
NodeCollections with two exceptions:

- They cannot be merged, as concatenating NodeCollections with metadata is
  not allowed.
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
    snodes.set('V_m', -60. + nest.spatial.pos.x)


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
      |              'columns': 2,                |                              shape=[1, 2]))                          |
      |              'elements':                  |                                                                      |
      |                  ['iaf_cond_alpha',       |     sn_poi = nest.Create('poisson_generator',                        |
      |                   'poisson_generator']})  |                           positions=nest.spatial.grid(               |
      |                                           |                               shape=[1, 3]))                         |
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

Note that if you have specified your positions as a NEST Parameter, NEST will convert that
to a list with lists, and this is what you will get when calling ``.spatial``. 


Connect spatially distributed nodes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Similar to creating nodes with spatial distributions, connecting is now done with the
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
  and ``fixed_outdegree`` implicitly states if we are using the source or
  target nodes.

- The connection probability specification ``kernel``  is renamed to ``p``
  to fit with ``pairwise_bernoulli``, and is only possible for the
  connection rules in the table above.

- Using a ``mask`` is only possible with the connection rules in the table
  above.

Usage examples
~~~~~~~~~~~~~~~

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
  |          'elements': 'iaf_psc_alpha',                   |                         shape=[nr, nc],                              |
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
  |                             {'a': -0.5, 'c': 1.}},               |                  'multapses': True,                                 |
  |                  'weights': {'normal':                           |                  'autapses': False}                                 |
  |                              {'mean': 0.0, 'sigma': 1.0}},       |     nest.Connect(l, l, conn_dict)                                   |
  |                  'delays': {'linear': {'a': 1.5, 'c': 0.}},      |                                                                     |
  |                  'allow_multapses': True,                        |                                                                     |
  |                  'allow_autapses': False}                        |                                                                     |
  |     tp.ConnectLayers(l, l, conn_dict)                            |                                                                     |
  |                                                                  |                                                                     |
  +------------------------------------------------------------------+---------------------------------------------------------------------+


What's removed?
-----------------

.. subnet_rm::

Subnets
~~~~~~~

Subnets are gone. Instead NodeCollections should be used to organize neurons.

  +---------------------------------------------+---------------------------------------+
  | NEST 2.x                                    | NEST 3.0                              |
  +=============================================+=======================================+
  |                                             |                                       |
  | ::                                          | ::                                    |
  |                                             |                                       |
  |     net = nest.LayoutNetwork(model, dim)    |     nrns = nest.Create(model, dim)    |
  |     nrns = nest.GetLeaves(net)[0]           |                                       |
  |                                             |                                       |
  +---------------------------------------------+---------------------------------------+

Printing the network as a tree of subnets is no longer possible. The
``PrintNetwork()`` function has been replaced with ``PrintNodes()``, which
prints ID ranges and model names of the nodes in the network.

  +----------------------------------------------+---------------------------------------+
  | NEST 2.x                                     | NEST 3.0                              |
  +==============================================+=======================================+
  |                                              |                                       |
  | >>>  nest.PrintNetwork(depth=2, subnet=None) | >>>  nest.PrintNodes()                |
  |      [0] root dim=[15]                       |      1 .. 10 iaf_psc_alpha            |
  |      [1]...[10] iaf_psc_alpha                |      11 .. 15 iaf_psc_exp             |
  |      [11]...[15] iaf_psc_exp                 |                                       |
  |                                              |                                       |
  |                                              |                                       |
  +----------------------------------------------+---------------------------------------+

.. model_rm::

Models
~~~~~~

With NEST 3.0, some models have been removed. They all have alternative models that can
be used instead.

  +----------------------------------------------+-----------------------------------------------+
  | Removed model                                | Replacement model                             |
  +==============================================+===============================================+
  | iaf_neuron                                   | iaf_psc_alpha                                 |
  +----------------------------------------------+-----------------------------------------------+
  | aeif_cond_alpha_RK5                          | aeif_cond_alpha                               |
  +----------------------------------------------+-----------------------------------------------+
  | iaf_psc_alpha_presc                          | iaf_psc_alpha_ps                              |
  +----------------------------------------------+-----------------------------------------------+
  | iaf_psc_delta_canon                          | iaf_psc_delta_ps                              |
  +----------------------------------------------+-----------------------------------------------+
  | subnet                                       | no longer needed, use NodeCollection instead  |
  +----------------------------------------------+-----------------------------------------------+


.. function_rm::

Functions
~~~~~~~~~

Some functions have also been removed. The removed functions where either related to subnets,
or they can be replaced by using other functions with indexing into a NodeCollection.
The removed functions are (see also :doc:`../nest-3/nest2_vs_3` for a full list of functions that have changed):

- BeginSubnet
- ChangeSubnet
- CurrentSubnet
- DataConnect
- DisconnectOneToOne
- EndSubnet
- GetChildren
- GetElement
- GetLayer
- GetLeaves
- GetLID
- GetNetwork
- LayoutNetwork
- ResetNetwork
- RestoreNodes (have never existed on PyNEST level, it was just a SLI function)
