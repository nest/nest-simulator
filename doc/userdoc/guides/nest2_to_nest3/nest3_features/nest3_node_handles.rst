.. _nodeid:

New functionality for node handles (neurons and devices)
========================================================

In NEST 3.0, ``nest.Create()`` returns a *NodeCollection* object instead of a list of global IDs.
This provides a more compact and flexible way for handling nodes.

In most use cases, you will not need to make many changes to your scripts in NEST 3.0, unless you have used **topology** or **subnets**.

NodeCollection supports the following functionality:

-  :ref:`Indexing <indexing>`
-  :ref:`Iteration <iterating>`
-  :ref:`Slicing <slicing>`
-  :ref:`Getting the size <get_size>` ``len``
-  :ref:`Conversion to and from lists <converting_lists>`
-  :ref:`Composing two non-overlapping NodeCollections <composing>`
-  :ref:`Testing for equality <testing_equality>`
-  :ref:`Testing of membership <testing_membership>`
-  Access to node properties with :ref:`get() <get_param>` and  :ref:`set() <set_param>` or by using :ref:`direct attributes <direct_attributes>`
-  :ref:`Parametrization <param_ex>`  with spatial, random, distributions, math, and logic parameters

  +-----------------------------------------------+------------------------------------------------+
  | NEST 2.x                                      | NEST 3.0                                       |
  +===============================================+================================================+
  |                                               |                                                |
  | ::                                            | ::                                             |
  |                                               |                                                |
  |     # A list of 10 GIDs is returned           |     # A NodeCollection object is returned      |
  |     neurons = nest.Create('iaf_psc_alpha', 10)|     neurons = nest.Create('iaf_psc_alpha', 10) |
  |                                               |                                                |
  |     # Use lists as arguments in Connect       |     # Use NodeCollection objects as            |
  |     nest.Connect(neurons, neurons)            |     # arguments in Connect                     |
  |                                               |     nest.Connect(neurons, neurons)             |
  |                                               |                                                |
  +-----------------------------------------------+------------------------------------------------+

.. _nodeID_support:

NodeCollections support the following operations:

Printing
   A compact representation of information about the NodeCollection can be printed

   >>>  neurons = nest.Create('iaf_psc_alpha', 10)
   >>>  print(neurons)
        NodeCollection(metadata=None, model=iaf_psc_alpha, size=10, first=1, last=10)

.. _indexing:

Indexing
   Indexing returns a new NodeCollection with a single node

   >>>  print(neurons[3])
        NodeCollection(metadata=None, model=iaf_psc_alpha, size=1, first=3)

   NodeCollections support array indexing. Array indexing is done by passing a list or tuple of
   indices when indexing. A NodeCollection with the node IDs at the chosen indices is then returned.
   Note that all indices must be strictly ascending and unique because all node IDs in a NodeCollection must be unique.

   >>>  print(neurons[[1, 2, 5, 6]])
        NodeCollection(metadata=None,
                       model=iaf_psc_alpha, size=2, first=2, last=3;
                       model=iaf_psc_alpha, size=2, first=6, last=7)


   One may also pass a list or tuple of Booleans, where the returned NodeCollection contains the `True` elements
   of the list or tuple. The length of the list of tuple of Booleans must be equal to the length of the NodeCollection.

   >>>  print(neurons[[True, True, True, True, False, False, True, True, True, True]])
        NodeCollection(metadata=None,
                       model=iaf_psc_alpha, size=4, first=1, last=4;
                       model=iaf_psc_alpha, size=4, first=7, last=10)

.. _iterating:

Iteration
    You can iterate the nodes in a NodeCollection and receive a single element NodeCollection

     >>>   for node in neurons:
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

    >>>  print(neurons[2:9:3])
         NodeCollection(metadata=None,
                        model=iaf_psc_alpha, size=2, first=3, last=9, step=3)


.. _get_size:

Getting the size
    You can easily get the number of nodes in the NodeCollection with

   >>>  len(neurons)
        10

.. _converting_lists:

Conversion to and from lists
    NodeCollections can be converted to lists of node IDs

    >>>  neurons.tolist()
         [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    And you can create a NodeCollection by providing a list, tuple, NumPy array or range of node IDs

    >>>  print(nest.NodeCollection([2, 3, 4, 8]))
         NodeCollection(metadata=None,
                        model=iaf_psc_alpha, size=3, first=2, last=4;
                        model=iaf_psc_alpha, size=1, first=8)
    >>>  print(nest.NodeCollection(range(1,4)))
         NodeCollection(metadata=None, model=iaf_psc_alpha, size=3, first=1, last=3)

    Note however that the nodes have to be already created. If any
    of the node IDs refer to a non existing node, an error is thrown. Additionally each node ID can
    only occur once and the list of node IDs must be sorted in ascending order.

.. _composing:

Composing
    When composing two NodeCollections, NEST tries to concatenate the
    two into a single NodeCollection.

    >>>  neurons = nest.Create('iaf_psc_alpha', 10)
    >>>  neurons_2 = nest.Create('iaf_psc_alpha', 3)
    >>>  print(neurons + neurons_2)
         NodeCollection(metadata=None, model=iaf_psc_alpha, size=13, first=1, last=13)

    If the node IDs are not continuous or the models are different, a composite will be created:

    >>>  neurons_3 = nest.Create('iaf_psc_delta', 3)
    >>>  print(neurons + neurons_3)
         NodeCollection(metadata=None,
                        model=iaf_psc_alpha, size=10, first=1, last=10;
                        model=iaf_psc_delta, size=3, first=14, last=16)

    Note that composing NodeCollections that overlap or that contain metadata
    (see section on :ref:`spatially distributed nodes <topo_changes>`) is not supported.

.. _testing_equality:

Test of equality
    You can test if two NodeCollections are equal, i.e. that they contain the same node IDs and model(s)

    >>>  neurons == neurons_2
         False
    >>>  neurons_2 == nest.NodeCollection([11, 12, 13])
         True

.. _testing_membership:

Test of membership
    You can test if a NodeCollection contains a certain ID

    >>>  2 in neurons
         True
    >>>  11 in neurons
         False

.. _direct_attributes:

Direct attributes
    You can directly get and set parameters of your NodeCollection

    >>> neurons.V_m = [-70., -60., -50., -40., -30., -20., -10., -20., -30., -40.]
    >>> neurons.V_m
        (-70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, -20.0, -30.0, -40.0)
    >>> neurons.C_m = 111.
    >>> neurons.C_m
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
~~~~~~~~~~~~~~~~~~~

``get()`` returns the parameters in the collection. You can call ``get()`` in
several ways.

To get all parameters in the collection, use ``get()`` without any function arguments.
This returns a dictionary with tuples. If the NodeCollection is a single-element NodeCollection,
``get()`` returns a dictionary with single values.

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

>>>    sr = nest.Create('spike_recorder')
>>>    sr.get('events', 'senders')
       array([], dtype=int64)

Lastly, you can specify the output format (Pandas dataframes [`pandas`] and `JSON` [`json`] for now). The
output format can be specified for all the different ``get()`` versions above.

>>>    nodes[0].get(['V_m', 'V_reset'], output='json')
       '{"V_m": -55.0, "V_reset": -70.0}'


.. _set_param:

Set node properties
~~~~~~~~~~~~~~~~~~~

``set()`` sets the values of a parameter by iterating over each node.

As with ``get()``, you can set parameters in different ways.

To set several parameters at once, use ``nodes.set(parameter_dict)``, where the
keys of the parameter_dict are the parameter names. The values could be a list
the size of the NodeCollection, a single value, or a ``nest.Parameter``. For more info see our
page on :doc:`parametrization`.

::

 nodes[:3].set({'V_m': [-70., -80., -90.], 'C_m': 333.})

You could also set a single parameter by using ``nodes.set(parameter_name=parameter)``.
As parameter, you can either send in a single value, a list the size of the NodeCollection,
or a ``nest.Parameter``

::

 nodes.set(t_ref=3.0)
 nodes[:3].set(t_ref=[3.0, 4.0, 5.0])
 nodes.set(t_ref=nest.random.uniform())

Note that some parameters, like `global_id`, cannot be set. The documentation of a specific model
will point out which parameters can be set and which are read-only.


Dictionary with lists when setting parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is now possible to use a dictionary with lists when setting node parameters
with ``Create()``, ``set()`` or ``SetStatus()``. The values of the lists will
be distributed across the nodes. The way to do this previously was to apply a
list of dictionaries. This is still possible.

The values in the dictionary can also be single values; the value will
then be applied to each node. You can mix and match as you want; the dictionary
can contain lists and single values at the same time.

::

    pop = nest.Create("iaf_psc_alpha", 2, params= {"I_e": [200.0, 150.0], "tau_m": 20.0, "V_m": [-77.0, -66.0]})

    print(pop.get(["I_e", "tau_m", "V_m"]))

