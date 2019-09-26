An overview of NEST 3
======================

.. contents:: Here you'll find
   :local:
   :depth: 2

.. seealso::

  See the :doc:`nest2_vs_3` to see a full list of functions that have changed


What's new?
------------

.. _gid:

GIDCollections
~~~~~~~~~~~~~~~~~~~~

- Functions, such as ``Create`` and ``Connect``,  now take or return ``GIDcollections`` instead of lists

The GIDcollections are very powerful; they replace subnets and will simplify scripts.

``GIDCollections`` support:

-  Iteration
-  Slicing
-  Indexing
-  Conversion to and from lists
-  Concatenation of two non-overlapping ``GIDCollection``\ s
-  Testing whether one ``GIDCollection`` is equal to another (contains the
   same GIDs)
-  Testing of membership
-  ``len``
-  :ref:`get_param` parameters
-  :ref:`set_param` parameters
-  spatial parameters on GIDCollections with spatial metadata
   with :ref:`spatial_ex`


Examples:
^^^^^^^^^^

>>> nodes_alpha = nest.Create('iaf_psc_alpha', 10)
>>> print(nodes_alpha)
    GIDCollection(metadata=None, model=iaf_psc_alpha, size=10, first=1, last=10)


>>> print(nodes_alpha[2:8])
    GIDCollection(metadata=None, model=iaf_psc_alpha, size=6, first=3, last=8)


>>> print(nodes_alpha[::2])
    GIDCollection(metadata=None, model=iaf_psc_alpha, size=5, first=1, last=9, step=2)


>>>    nodes_exp = nest.Create('iaf_psc_exp', 5)
>>>    nodes = nodes_alpha + nodes_exp
>>>    print(nodes)
       GIDCollection(metadata=None,
                  model=iaf_psc_alpha, size=10, first=1, last=10;
                  model=iaf_psc_exp, size=5, first=11, last=15)


.. code:: python

   nest.ResetKernel()

   # Create 80 exitatory neurons
   ex_nodes = nest.Create('iaf_psc_alpha', 80)

   # Create 20 inibitory neurons
   in_nodes = nest.Create('iaf_psc_exp', 20)

   # Total nodes
   nodes = ex_nodes + in_nodes

   # Inspect collection
   for gid, modelid in nodes.items():
       print(gid, modelid)

   # set randomly distributed membrane potential on the exitatory nodes
   ex_nodes.set({'V_m': nest.random.uniform(65., 85.)})

   # get all parameters of all the nodes
   print(nodes.get())

   # Create noise and spike detector
   noise = nest.Create('poisson_generator', 1, {'rate': 800.})
   sd = nest.Create('spike_detector')

   # Connect
   nest.Connect(ex_nodes, ex_nodes,
                {'rule': 'fixed_indegree', 'indegree': 8},
                {'synapse_model': 'static_synapse', 'weight': 0.1})
   nest.Connect(ex_nodes, in_nodes,
                {'rule': 'fixed_indegree', 'indegree': 4},
                {'synapse_model': 'static_synapse', 'weight': 0.1})
   nest.Connect(in_nodes, ex_nodes,
                {'rule': 'fixed_indegree', 'indegree': 5},
                {'synapse_model': 'static_synapse', 'weight': -1.0})
   nest.Connect(in_nodes, in_nodes,
                {'rule': 'fixed_indegree', 'indegree': 8},
                {'synapse_model': 'static_synapse', 'weight': -1.0})

   # Connect noise to all the nodes
   nest.Connect(noise, nodes)

   # Connect spike detector to every other node
   nest.Connect(nodes[::2], sd)

   # Simulate for 1 sec
   nest.Simulate(1000)

   # Get spike information
   print(sd.get('events', ['senders', 'times']))

For more information regarding GIDCollections see the document on :doc:`GIDCollections`.

.. _connectome:

Connectome
~~~~~~~~~~

-  ``nest.GetConnections`` returns a ``Connectome`` object  instead of a numpy array

``Connectome`` supports:

-  Iteration
-  Test for equality
-  ``len``
-  :ref:`get_param` parameters
-  :ref:`set_param` parameters

Examples
^^^^^^^^

.. code-block:: ipython

    nest.ResetKernel()

    n = nest.Create('iaf_psc_alpha', 200)
    nest.Connect(n, n, syn_spec={'weight': nest.random.exponential(scale=0.4)})

    conns = nest.GetConnections()
    weights = conns.get('weight')

>>>    print(weights[:10])
       [0.21088282805971265, 0.15657555664733017, 0.6554309097531537, 0.37681366198069244, 0.7558248149006221, 0.4509586111884833, 0.0849105474425321, 1.5868739883995078, 0.04972731121045684, 0.2983155067483565]


.. code:: python

   nest.ResetKernel()

   # Create nodes and connect
   nodes = nest.Create('iaf_psc_alpha', 10)

   nest.Connect(nodes, nodes, 'one_to_one')

   # Get Connectome and set weight distribution
   conns = nest.GetConnections()
   conns.set('weight', [1., 2., 3., 4., 5., 6., 7., 8., 9. ,10.])

   # Simulate
   nest.Simulate(100.)

   # Get sources and weights
   print(conns.get(['source', 'weight']))


.. _get_param:

get()
~~~~~~

``nodes.get`` Returns all parameters in the collection in a dictionary
with lists.


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


* ``nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n])``

>>>    nodes = nest.Create('iaf_psc_alpha', 10, {'V_m': -55.})
>>>    nodes.get(['V_m', 'V_reset'])
       {'V_m': (-55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0),
        'V_reset': (-65.0,
         -64.0,
         -63.0,
         -62.0,
         -61.0,
         -60.0,
         -59.0,
         -58.0,
         -57.0,
         -56.0)}

>>>    grid_layer.get('V_m')
       (-70.0, -70.0, -70.0, -70.0)




You can also specify the output format (pandas, JSON currently
implemented):

* ``nodes.get(output)``
* ``nodes.get(parameter_name, output)``
* ``nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n], output)``
* ``nodes.get(parameter_name, property_name, output)``
* ``nodes.get(parameter_name, [property_name_1, ... , property_name_n], output)``

Definitions
^^^^^^^^^^^^

.. glossary::

 nodes.get
     Returns all parameters in the collection in a dictionary with lists.

 nodes.get(parameter_name)
     Returns the parameter given by ``parameter_name`` as list or int/float.

 nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n])
     Returns the parameters in the collection given by the parameter names as a dictionary with lists.

 nodes.get(parameter_name, property_name)
     Hierarchical addressing.
     Returns the parameter of ``parameter_name`` given by ``property_name``
     as list or int/float.

 nodes.get(parameter_name, [property_name_1, ... , property_name_n])
     Hierarchical addressing. Returns the parameters of ``parameter_name``
     given by property names as a dictionary with list.

.. _set_param:

set()
~~~~~~

* ``nodes.set(parameter_name, parameter_value)``
* ``nodes.set(parameter_name, [parameter_val_1, parameter_val_2, ... , parameter_val_n])``
* ``nodes.set(parameter_dict)``
* ``nodes.set([parameter_dict_1, parameter_dict_2, ... , parameter_dict_n])``

Examples
^^^^^^^^

>>>    nodes.set({'V_reset': [-65.0 + n for n in range(10)]})
>>>    nodes.get(['V_m', 'V_reset'])
       {'V_m': (-55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0, -55.0),
        'V_reset': (-65.0,
         -64.0,
         -63.0,
         -62.0,
         -61.0,
         -60.0,
         -59.0,
         -58.0,
         -57.0,
         -56.0)}

.. _param_ex:

Parametrization
~~~~~~~~~~~~~~~~~

.. _random_ex:

random
^^^^^^^

-  ``nest.random.exponential``
-  ``nest.random.lognormal``
-  ``nest.random.normal``
-  ``nest.random.uniform``

.. code-block:: ipython

    nest.ResetKernel()

    n = nest.Create('iaf_psc_alpha', 10000, {'V_m': nest.random.normal(loc=-60., scale=10.)})

    gids = n.get('global_id')
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



.. image:: NEST3_13_0.png



.. code-block:: ipython

    nest.ResetKernel()

    n = nest.Create('iaf_psc_alpha', 10000, {'V_m': -60 + 2*nest.random.exponential() + nest.random.normal()})

    gids = n.get('global_id')
    v_m = n.get('V_m')
    fig, ax = plt.subplots(figsize=(12, 6),
                           gridspec_kw={'width_ratios': [3, 1]},
                           ncols=2,
                           sharey=True)
    ax[0].plot(gids, v_m, '.', alpha=0.5, ms=3.5)
    ax[0].set_xlabel('GID');
    ax[1].hist(v_m, bins=40, orientation='horizontal');
    ax[1].set_xlabel('num. nodes');
    ax[0].set_ylabel('V_m');



.. image:: NEST3_14_0.png


.. _spatial_ex:

spatial
^^^^^^^^

-  ``nest.spatial.dimension_distance.x``
-  ``nest.spatial.dimension_distance.y``
-  ``nest.spatial.dimension_distance.x``
-  ``nest.spatial.distance``
-  ``nest.grid``
-  ``nest.free``
-  ``nest.pos.x``, ``nest.pos.y``, ``nest.pos.z``
-  ``nest.source_pos.x``, ``nest.source_pos.y``, ``nest.source_pos.z``
-  ``nest.target_pos.x``, ``nest.target_pos.y``, ``nest.target_pos.z``

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    layer = nest.Create('iaf_psc_alpha', 10, positions=positions)
    parameter = nest.spatial.distance
    nest.Connect(layer, layer, conn_spec={'rule': 'pairwise_bernoulli',
                                          'p': parameter})
>>>    print('Num. connections:', len(nest.GetConnections()))
       Num. connections: 51

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    layer = nest.Create('iaf_psc_alpha', 10, positions=positions)

    nest.Connect(layer, layer)

>>>    len(nest.GetConnections())
       100

.. _math_ex:

math
^^^^^^

-  ``nest.math.exp``
-  ``nest.math.cos``
-  ``nest.math.sin``

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free([[x, 0.5*x] for x in np.linspace(0, 1.0, 100)])
    layer = nest.Create('iaf_psc_alpha', positions=positions)

    parameter = -60 + nest.math.exp(nest.spatial.pos.x**4)
    # Also available:
    #   - nest.math.sin()
    #   - nest.math.cos()

    layer.set({'V_m': parameter})

    node_pos = np.array(nest.GetPosition(layer))
    node_pos[:,1]
    v_m = layer.get('V_m');

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(node_pos[:,0], v_m, '.', ms=6.5)
    ax.set_xlabel('Node position on x-axis')
    ax.set_ylabel('V_m');



.. image:: NEST3_27_0.png

.. _logic_ex:

logic
^^^^^^

-  ``nest.logic.conditional``

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free([[x, 0.5*x] for x in np.linspace(0, 1.0, 50)])
    layer = nest.Create('iaf_psc_alpha', positions=positions)

    layer.set({'V_m': nest.logic.conditional(nest.spatial.pos.x < 0.5,
                                             -55 + 10*nest.spatial.pos.x,
                                             -55)})

    node_pos = np.array(nest.GetPosition(layer))
    node_pos[:,1]
    v_m = layer.get('V_m');

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(node_pos[:,0], v_m, 'o')
    ax.set_xlabel('Node position on x-axis')
    ax.set_ylabel('V_m');



.. image:: NEST3_26_0.png

.. _distr_ex:

distributions
^^^^^^^^^^^^^

-  ``nest.distributions.exponential``
-  ``nest.distributions.gaussian``
-  ``nest.distributions.gaussian2D``
-  ``nest.distributions.gamma``

.. code-block:: ipython

    nest.ResetKernel()

    N = 21
    middle_node = N//2

    positions = nest.spatial.free([[x, 0.] for x in np.linspace(0, 1.0, N)])
    layer = nest.Create('iaf_psc_alpha', positions=positions)

    parameter = nest.distributions.exponential(nest.spatial.distance, a=1.0, tau=0.15)

    # Iterate connection to get statistical connection data
    for _ in range(2000):
        nest.Connect(layer[middle_node], layer,
                     conn_spec={'rule': 'pairwise_bernoulli',
                                'p': parameter})

    targets = nest.GetConnections().get('target')

    fig, ax = plt.subplots(figsize=(12, 6))
    bars = ax.hist(targets, bins=N, edgecolor='black', linewidth=1.2)

    plt.xticks(bars[1] + 0.5,np.arange(1, N+1))
    ax.set_title('Connections from node with GID {}'.format(layer[middle_node].get('global_id')))
    ax.set_xlabel('Target GID')
    ax.set_ylabel('Num. connections');



.. image:: NEST3_34_0.png

What's changed?
----------------

.. _topo_changes:

Topology module
~~~~~~~~~~~~~~~~

-  All topology functions are now part of ``nest`` and not
   ``nest.topology``
-  ``nest.GetPosition`` -> now takes a GIDCollection instead of a list of GIDs
-  ``nest.FindCenterElement`` -> now returns ``int`` instead of
   ``tuple``

.. note::

   See the reference section :ref:`topo_ref` in our conversion guide for all changes made to functions

Examples
^^^^^^^^

>>>    grid_layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(rows=2, columns=2, center=[1., 1.]))
>>>    free_layer = nest.Create('iaf_psc_alpha', 4, positions=nest.spatial.free([[1., 1.], [2., 2.], [3., 3.], [4., 4.]]))
>>>    print(grid_layer)
       GIDCollection(metadata=spatial, model=iaf_psc_alpha, size=4, first=1, last=4)


>>>    grid_layer.spatial
       {'network_size': 4,
        'center': (1.0, 1.0),
        'columns': 2,
        'edge_wrap': False,
        'extent': (1.0, 1.0),
        'rows': 2}

>>>   free_layer.spatial
      {'network_size': 4,
       'center': (2.5, 2.5),
       'edge_wrap': False,
       'extent': (3.2, 3.2),
       'positions': ((1.0, 1.0), (2.0, 2.0), (3.0, 3.0), (4.0, 4.0))}

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    layer = nest.Create('iaf_psc_alpha', 10, positions=positions)

    nest.Connect(layer, layer)

>>>    len(nest.GetConnections())
       100

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    layer = nest.Create('iaf_psc_alpha', 10, positions=positions)

    nest.Connect(layer, layer, conn_spec={'rule': 'fixed_indegree', 'indegree': 2})

>>>    print('Num. connections:', len(nest.GetConnections()))
       Num. connections: 20

.. code-block:: ipython

    free_layer = nest.Create('iaf_psc_alpha', 100, positions=nest.spatial.free(nest.random.uniform(min=0., max=10.), num_dimensions=2))
    nest.PlotLayer(free_layer);



.. image:: NEST3_24_0.png


.. _conn_changes:

Connection rules
~~~~~~~~~~~~~~~~~

====================================== =================================================
NEST 2.x                                NEST 3.0
====================================== =================================================
`convergent`                           `pairwise_bernoulli` and `use_on_source=True`
`convergent` and `num_connections`     `fixed_indegree`
`divergent`                            `pairwise_bernoulli`
`divergent` and `num_connections`      `fixed_outdegree`
====================================== =================================================

.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    layer = nest.Create('iaf_psc_alpha', 10, positions=positions)

    nest.Connect(layer, layer, conn_spec={'rule': 'fixed_indegree', 'indegree': 2})

>>>    print('Num. connections:', len(nest.GetConnections()))
       Num. connections: 20



What's removed?
----------------

Subnets module
~~~~~~~~~~~~~~~~~~

SiblingContainers
~~~~~~~~~~~~~~~~~

.. seealso::

  See :doc:`nest2_vs_3` to see a full list of functions that have changed





