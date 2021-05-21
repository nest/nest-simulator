.. _SynapseCollection:

New functionality for handling connections (synapses)
=====================================================

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

.. note::

  We now make consistent use of term ``synapse_model`` throughout NEST 3 in reference to synapse models.
  Most importantly, this will change your ``Connect`` call, where instead of passing the synapse
  model with the ``model`` key, you should now use the ``synapse_model`` key.

    >>>  nrns = nest.Create('iaf_psc_alpha', 3)
    >>>  nest.Connect(nrns, nrns, 'one_to_one', syn_spec={'synapse_model': 'stdp_synapse'})

  Similarly, the dictionary ``GetDefaults`` returns an entry called ``synapsemodel``, which is now called
  ``synapse_model``.

.. seealso::

    You can find a :doc:`full SynapseCollection example <../../../auto_examples/synapsecollection>` on our example
    network page.

Printing
    Printing a SynapseCollection produces a table with source and target node IDs, synapse model, weight and delay.
    If your SynapseCollection has more than 36 elements, only the first and last 15 connections are displayed.
    To print all, first set ``print_all = True`` on your SynapseCollection.

    >>>  nest.Connect(nodes[:2], nodes[:2])
    >>>  synapses = nest.GetConnections()
    >>>  print(synapses)
          source   target   synapse model   weight   delay
         -------- -------- --------------- -------- -------
               1        1  static_synapse    1.000   1.000
               1        2  static_synapse    1.000   1.000
               2        1  static_synapse    1.000   1.000
               2        2  static_synapse    1.000   1.000

    >>> synapses.print_all = True

.. _conn_indexing:


Indexing
    Indexing returns a single connection SynapseCollection.

    >>>  print(synapses[1])
          source   target   synapse model   weight   delay
         -------- -------- --------------- -------- -------
               1        2  static_synapse    1.000   1.000

.. _conn_iterating:

Iteration
    A SynapseCollection can be iterated, yielding a single connection SynapseCollection per iteration.

    >>>  for conn in synapses:
    >>>      print(conn.source)
         1
         1
         2
         2

.. _conn_s_t_iterator:

Iterator of sources and targets
    Calling ``SynapseCollection.sources()`` or ``SynapseCollection.targets()`` returns an
    iterator over the source IDs or target IDs, respectively.

    >>>  print([s for s in synapses.sources()])
         [1, 1, 2, 2]


.. _conn_slicing:

Slicing
    A SynapseCollection can be sliced with ``start:stop:step`` inside brackets

    >>>  print(synapses[0:3:2])
         source   target   synapse model   weight   delay
        -------- -------- --------------- -------- -------
              1        1  static_synapse    1.000   1.000
              2        1  static_synapse    1.000   1.000

.. _conn_size:

Getting the size
    We can get the number of connections in the SynapseCollection with

    >>>  len(synapses)
         4

.. _conn_testing_equality:

Test of equality
    Two SynapseCollections can be tested for equality, i.e. that they contain the same connections.

    >>>  synapses == synapses
         True
    >>>  synapses[:2] == synapses[2:]
         False

.. _conn_direct_attributes:

Setting and getting attributes directly
    You can also directly get and set parameters of your SynapseCollection

    >>>  synapses.weight = 5.0
    >>>  synapses.weight
         [5.0, 5.0, 5.0, 5.0]
    >>>  synapses.delay = [5.1, 5.2, 5.3, 5.4]
    >>>  synapses.delay
         [5.1, 5.2, 5.3, 5.4]

    If you use a list to set the parameter, the list needs to be the same length
    as the SynapseCollection.

    For :ref:`spatially distributed <topo_changes>` sources and targets, you can access the distance between
    the source-target pairs by calling `distance` on your SynapseCollection.

    >>>  synapses.distance
         (0.47140452079103173,
          0.33333333333333337,
          0.4714045207910317,
          0.33333333333333337,
          3.925231146709438e-17,
          0.33333333333333326,
          0.4714045207910317,
          0.33333333333333326,
          0.47140452079103157)


.. _conn_get:

Getting connection parameters
    Just as with NodeCollection, you can get parameters of the connections with
    ``get()``. The same function arguments as for :ref:`NodeCollections get() <get_param>`
    apply here. The returned values also follow the same rules.

    If you call ``get()`` without any arguments, a dictionary with all parameters is
    returned. If there is only a single connection in the SynapseCollection, the dictionary contains plain values,
    whereas if there is more than one connection, the dictionary contains lists of values.

    >>>  synapses.get()
         {'delay': [1.0, 1.0, 1.0, 1.0],
          'port': [0, 1, 2, 3],
          'receptor': [0, 0, 0, 0],
          'sizeof': [32, 32, 32, 32],
          'source': [1, 1, 2, 2],
          'synapse_id': [0, 0, 0, 0],
          'synapse_model': ['static_synapse','static_synapse','static_synapse','static_synapse'],
          'target': [1, 2, 1, 2],
          'target_thread': [0, 0, 0, 0],
          'weight': [1.0, 1.0, 1.0, 1.0]}

    Calling ``get(parameter_name)`` will return a list of parameter values, while
    ``get([parameter_name_1, ... , parameter_name_n])`` returns a dictionary with
    the values.

    >>>  synapses.get('weight')
         [1.0, 1.0, 1.0, 1.0]

    >>>  synapses[2].get(['source', 'target'])
         {'source': 2, 'target': 1}

    It is also possible to select an alternative output format with the
    ``output`` keyword. Currently, it is possible to get the output as
    JSON or as a Pandas dataframe (if Pandas is installed).

.. _conn_set:

Setting connection parameters
    Likewise, you can set the parameters of connections in the SynapseCollection.
    Again the same rules as with ``set()`` on NodeCollection applies, see :ref:`set_param`
    for more details.

    If you want to set several parameters at once, use ``set(parameter_dictionary)``.
    You can use a single value, a list, or a ``nest.Parameter`` as values. If a single value is given,
    the value is set on all connections.

    >>>  synapses.set({'weight': [1.5, 2.0, 2.5, 3.0], 'delay': 2.0})

    Updating a single parameter is done by calling ``set(parameter_name=parameter_value)``.
    Again you can use a single value, a list, or a ``nest.Parameter`` as value.

    >>>  synapses.set(weight=3.7)

    >>>  synapses.set(weight=[4.0, 4.5, 5.0, 5.5])

    Note that some parameters, like `source` and `target`, cannot be set.  The documentation of a specific
    model will point out which parameters can be set and which are read-only.


.. _collocated_synapses

Collocated synapses
~~~~~~~~~~~~~~~~~~~
It is now possible to create connections with several synapses simultaneously. The different synapse dictionaries will
then be applied to each source-target pair. To create these collocated synapses, ``CollocatedSynapses()`` must be used
as the `syn_spec` argument of ``Connect``, instead of the usual syn_spec dictionary argument. The constructor
``CollocatedSynapses()`` takes dictionaries as arguments.

  ::

    nodes = nest.Create('iaf_psc_alpha', 3)
    syn_spec = nest.CollocatedSynapses({'weight': 4., 'delay': 1.5},
                                       {'synapse_model': 'stdp_synapse'},
                                       {'synapse_model': 'stdp_synapse', 'alpha': 3.})
    nest.Connect(nodes, nodes, conn_spec='one_to_one', syn_spec=syn_spec)

    conns = nest.GetConnections()
    print(conns.alpha)

This will create 9 connections: 3 using `static_synapse` with a `weight` of `4` and `delay` of `1.5`, and 6 using
the `stdp_synapse`. Of the 6 using `stdp_synapse`, 3 will have the default alpha value, and 3 will have an alpha of
`3.0`.

  >>> print(nest.GetKernelStatus('num_connections'))
  9

If you want to connect with different receptor types, you can do the following:

  ::

    src = nest.Create('iaf_psc_exp_multisynapse', 7)
    trgt = nest.Create('iaf_psc_exp_multisynapse', 7, {'tau_syn': [0.1 + i for i in range(7)]})

    syn_spec = nest.CollocatedSynapses({'weight': 5.0, 'receptor_type': 2},
                                       {'weight': 1.5, 'receptor_type': 7})

    nest.Connect(src, trgt, 'one_to_one', syn_spec=syn_spec)

    conns = nest.GetConnections()
    print(conns.get())

You can see how many synapse parameters you have by calling `len()` on your `CollocatedSynapses` object:

  >>> len(syn_spec)
  2

.. _connect_arrays:

New functionality for connecting arrays of node IDs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

While you should aim to use NodeCollections to create connections whenever possible,
there may be cases where you have a predefined set of pairs of pre- and postsynaptic nodes.
In those cases, it may be inefficient to convert the individual IDs in the pair to NodeCollections
to be passed to the ``Connect()`` function, especially if there are thousands or millions of
pairs to connect.

To efficiently create connections in these cases, you can pass NumPy arrays to ``Connect()``.
This variant of ``Connect()`` will create connections in a one-to-one fashion.

::

   nest.Create('iaf_psc_alpha', 10)
   # Node IDs in the arrays must address existing nodes, but may occur multiple times.
   sources = np.array([1, 5, 7, 5], dtype=np.uint64)
   targets = np.array([2, 2, 4, 4], dtype=np.uint64)
   nest.Connect(sources, targets, conn_spec="one_to_one")

You can also specify weights, delays, and receptor type for each connection as arrays.
All arrays have to have lengths equal to those of ``sources`` and ``targets``.

::

   weights = np.array([0.5, 0.5, 2., 2.])
   delays = np.array([1., 1., 2., 2.])
   syn_spec = {'weight': weights, 'delay': delays}
   nest.Connect(sources, targets, conn_spec='one_to_one', syn_spec=syn_spec)
