From NEST 2.x to NEST 3.0
=========================

.. contents:: Here you'll find
   :local:
   :depth: 2

.. seealso::

  See the :doc:`../nest-3/nest2_vs_3` to see a full list of functions that have changed

NEST 3 introduces a number of new features and concepts, and some changes
to the user interface that are not backwards compatible. One big change is
the removal of subnets and all functions based on subnets. To organize
neurons you should instead use the powerful GIDCollections, which will be
presented below. Other big features include Connectome objects to
efficiently work with connections, Parameter objects, and changes to how
nodes with spatial information are defined and how to work with them.

This guide is based exclusively on PyNEST.

What's new?
------------

.. _gid:

GIDCollections
~~~~~~~~~~~~~~

In NEST 3.0, when you create a population with
``nest.Create()``, a GIDCollection is returned instead of a list. And when you
connect two populations, you provide ``nest.Connect()`` with two
populations in the form of GIDCollections.

.. pull-quote::

   In **many use cases**, the addition of GIDCollections requires **no changes** to the
   scripts.

But instead of working with lists of GIDs you are working with
GIDCollections.

GIDCollections are compact and efficient containers containing the global
ID representations of nodes.

``GIDCollections`` support:

-  :ref:`Iteration <iterating>`
-  :ref:`Slicing <slicing>`
-  :ref:`Indexing <indexing>`
-  :ref:`Getting the size <get_size>` ``len``
-  :ref:`Conversion to and from lists <converting_lists>`
-  :ref:`Joining of two non-overlapping GIDCollections <joining>`
-  :ref:`Testing whether one GIDCollection is equal to another <testing_equality>` (contains the
   same GIDs)
-  :ref:`Testing of membership <testing_membership>`
-  :ref:`get_param` parameters
-  :ref:`set_param` parameters
-  :ref:`Parametrization <param_ex>`  with spatial, random, distributions, math, and logic parameters

A GIDCollection is created by

- creating new nodes
- combining two or more GIDCollections
- slicing a GIDCollection
- providing a list of GIDs, but only GIDs of existing nodes

The GIDs in a GIDCollection are sorted automatically. All GIDs in a
GIDCollection are unique, so a GID can occur at most once per
GIDCollection.

A GIDCollection can be either primitive or composite. A primitive
GIDCollection is contiguous in that it represents a continuous range of
GIDs. It is also homogeneous in that all GIDs refer to nodes of the same
type, i.e., they have the same model. A composite GIDCollection consists of
several primitive GIDCollections that either have different models, or
where the GIDs are not continuous.


  +---------------------------------------------+----------------------------------------------+
  | NEST 2.x                                    | NEST 3.0                                     |
  +=============================================+==============================================+
  |                                             |                                              |
  | ::                                          | ::                                           |
  |                                             |                                              |
  |     # A list of 10 GIDs is returned         |     # A GIDCollection object is returned     |
  |     nrns = nest.Create('iaf_psc_alpha', 10) |     nrns = nest.Create('iaf_psc_alpha', 10)  |
  |                                             |                                              |
  |     # Use lists as arguments in Connect     |     # Use GIDCollection objects as arguments |
  |     nest.Connect(nrns, nrns)                |     # in Connect                             |
  |                                             |     nest.Connect(nrns, nrns)                 |
  |                                             |                                              |
  +---------------------------------------------+----------------------------------------------+

.. _GID_support:

GIDCollections support the following operations:

Printing
    A compact representation of information about the GIDCollection can be printed.



   >>>  nrns = nest.Create('iaf_psc_alpha', 10)
   >>>  print(nrns)
        GIDCollection(metadata=None, model=iaf_psc_alpha, size=10, first=1, last=10)

.. _get_size:

Getting the size
    You can easily get the number of GIDs in the GIDCollection with

   >>>  len(nrns)
        10
.. _indexing:

Indexing
    Indexing returns a new GIDCollection with a single GID



   >>>  print(nrns[3])
        GIDCollection(metadata=None, model=iaf_psc_alpha, size=1, first=3)

.. _slicing:

Slicing
    A GIDCollection can be sliced in the same way one would slice a list,
    with ``start:stop:step`` inside brackets


    >>>  print(nrns[2:9:3])
         GIDCollection(metadata=None,
                       model=iaf_psc_alpha, size=2, first=3, last=9, step=3)

.. _joining:

Joining
    When joining two GIDCollections, NEST tries to concatenate the
    primitives into a single primitive.


    >>>  nrns_2 = nest.Create('iaf_psc_alpha', 3)
    >>>  print(nrns + nrns_2)
         GIDCollection(metadata=None, model=iaf_psc_alpha, size=13, first=1, last=13)

    If the GIDs are not continuous or the models are different, a composite will be created:

    >>>  nrns_3 = nest.Create('iaf_psc_delta', 3)
    >>>  print(nrns + nrns_3)
         GIDCollection(metadata=None,
                      model=iaf_psc_alpha, size=10, first=1, last=10;
                      model=iaf_psc_delta, size=3, first=14, last=16)

    Note that joining GIDCollections that overlap or that contain metadata
    (see section on Topology) is impossible.

.. _iterating:

Iteration
    You can iterate the GIDs in a GIDCollection

     >>>   for gid in nrns:
     >>>       print(gid)
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

    You can also iterate ``nrns.items()``, which yields tuples containing
    the GID and the model ID.

.. _converting_lists:

Conversion to and from lists
    GIDCollections can be converted to lists of GIDs


    >>>  list(nrns)
         [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    And you can create a GIDCollection by providing a list of GIDs

    >>>  print(nest.GIDCollection([2, 3, 4, 8]))
         GIDCollection(metadata=None,
                      model=iaf_psc_alpha, size=3, first=2, last=4;
                      model=iaf_psc_alpha, size=1, first=8)

    Note however that the nodes have to already have been created. If any
    of the GIDs refer to nodes that are not created, an error is thrown.


.. _testing_equality:

Test of equality
    You can test if two GIDCollections are equal, i.e. that they contain the same GIDs

    >>>  nrns == nrns_2
         False
    >>>  nrns_2 == nest.GIDCollection([11, 12, 13])
         True

.. _testing_membership:

Test of membership
    You can test if a GIDCollection contains a certain GID

    >>>  2 in nrns
         True
    >>>  11 in nrns
         False

.. _get_param:

get()
~~~~~~

Getting node status

``get`` Returns all parameters in the collection in a dictionary
with lists.

Get the parameters of the first 3 nodes

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

Get the parameters `V_m` and `V_reset` of all nodes

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



You can also specify the output format (pandas, JSON currently
implemented):

* ``nodes.get(output)``
* ``nodes.get(parameter_name, output)``
* ``nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n], output)``
* ``nodes.get(parameter_name, property_name, output)``
* ``nodes.get(parameter_name, [property_name_1, ... , property_name_n], output)``

.. _set_param:

set()
~~~~~~

* ``nodes.set(parameter_name, parameter_value)``
* ``nodes.set(parameter_name, [parameter_val_1, parameter_val_2, ... , parameter_val_n])``
* ``nodes.set(parameter_dict)``
* ``nodes.set([parameter_dict_1, parameter_dict_2, ... , parameter_dict_n])``

We can set the the values of a parameter by iterating over each node

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

    We can get the status of the nodes in the GIDCollection. Getting the
    status with a single parameter returns a tuple with the values of that
    parameter for all nodes.


    >>>  nrns.get('V_m')
         (-70.0, -70.0, -70.0, -70.0, -70.0, -70.0, -70.0, -70.0,
         -70.0, -70.0)

    If more than one parameter is provided, e.g.

    ::

        nrns.get(['C_m', 'V_m'])

    a dictionary is returned with parameters as keys and tuples
    of the values. To get all parameters in a dictionary, call
    ``nrns.get()`` without any arguments. Selecting fields at a deeper
    hierarchy level is also possible

    ::

        multimeter.get('events', 'senders')  # returns an array of sender GIDs
        multimeter.get('events', ['senders', 'times'])  # returns a dictionary with arrays

    It is possible to select an alternative output format with the
    ``output`` keyword. Currently it is possible to get the output in a
    json format, or a Pandas format (if Pandas is installed).

    ::

        nrns.get(output='json')  # returns a string in json format
        nrns.get(output='pandas')  # returns a Pandas DataFrame


Setting node status
    In the same way as we can ``get`` the status of nodes in a
    GIDCollection, we can also ``set`` the status.

    ::

        nrns.set('V_m', -55.)  # sets V_m of all nodes
        nrns.set('V_m', [-50., -51., ...])  # sets different V_m for each node
        nrns.set({'V_m': -55., 'C_m': 150.})  # sets V_m and C_m of all nodes

We can create a composite GIDCollection (i.e., a non-contiguous or non-homogenous GIDCollection) from a list

    >>>  gc = nest.GIDCollection([1, 3, 7])
    >>>  print(gc)
         GIDCollection(metadata=None,
               model=iaf_psc_alpha, size=1, first=1;
               model=iaf_psc_alpha, size=1, first=3;
               model=iaf_psc_alpha, size=1, first=7)

.. _connectome:

Connectome
~~~~~~~~~~

``Connectome`` supports:

-  :ref:`Iteration <conn_iterating>`
-  :ref:`Indexing <conn_indexing>`
-  :ref:`Slicing <conn_slicing>`
-  :ref:`Testing for equality <conn_testing_equality>`
-  :ref:`Getting the size <conn_size>` ``len``
-  :ref:`get_param` parameters
-  :ref:`set_param` parameters

.. seealso::

    You can find a :doc:`full example <../examples/connectome>` in our example network page

Just like a GIDCollection is a container for GIDs, a Connectome is a
container for connections. In NEST 3, when you call ``GetConnections()`` a
Connectome is returned. Connectomes support a lot of the same operations
as GIDCollections:

Printing
    Printing a Connectome produces a table of source and target GIDs

    >>>  connectome = nest.GetConnections()
    >>>  print(connectome)
         *--------*-------------*
         | source | 1, 1, 2, 2, |
         *--------*-------------*
         | target | 1, 2, 1, 2, |
         *--------*-------------*

.. _conn_size:

Getting the size
    We can get the number of connections in the Connectome with


.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    layer = nest.Create('iaf_psc_alpha', 10, positions=positions)

    nest.Connect(layer, layer)
    connectome = nest.GetConnections()

>>>    len(connectome)
       100

.. _conn_indexing:

Indexing
    Indexing returns a Connectome with a single connection.

    >>>  print(connectome[1])
         *--------*----*
         | source | 1, |
         *--------*----*
         | target | 9, |
         *--------*----*

.. _conn_slicing:

Slicing
    A Connectome can be sliced with ``start:stop:step`` inside brackets

   >>>  print(connectome[0:3:2])
        *--------*-------*
        | source | 1, 1, |
        *--------*-------*
        | target | 10, 8,|
        *--------*-------*

.. _conn_interating:

Iteration
    A Connectome can be iterated, yielding single connection Connectomes.

.. _conn_testing_equality:

Test of equality
    Two Connectomes can be tested for equality, i.e. that they contain the same connections.

.. _conn_get:

Getting connection parameters
    We can get the parameters of the connections in the Connectome. The
    structure of the returned values follows the same rules as ``get()``
    for GIDCollections.

    ::

        connectome.get()  # Returns a dictionary of all parameters
        >>> connectome[0].get('weight')  # Returns the weight value of the first connection
            1.0
        connectome.get('delay')  # Returns a list of delays
        connectome.get(['weight', 'delay'])  # Returns a dictionary with weights and delays

    It is also possible to select an alternative output format with the
    ``output`` keyword. Currently it is possible to get the output in a
    json format, or a Pandas format (if Pandas is installed).

    ::

        connectome.get(output='json')  # returns a string in json format
        connectome.get(output='pandas')  # returns a Pandas DataFrame

.. _conn_set:

Setting connection parameters
    Likewise, we can set the parameters of connections in the Connectome

    ::

        connectome.set('delay', 2.0)  # Sets all delays to 2.0
        connectome.set('delay', [1.0, 2.0, 3.0, 4.0])  # Sets specific delays for each connection
        connectome.set({'weight': 1.5, 'delay': 2.0})  # Sets all weights to 1.5 and all delays to 2.0

Getting an iterator over the sources or targets
    Calling ``connectome.source()`` or ``connectome.target()`` returns an
    iterator over the source GIDs or target GIDs, respectively.

.. _param_ex:

Parameterization
~~~~~~~~~~~~~~~~

NEST 3 introduces *Parameter objects*, i.e. objects that represent values
drawn from a random distribution or values based on various spatial node
parameters. Parameters can be used to set node status, to create positions
in topology (see :ref:`Topology section <topo_changes>` below), and to define connection
probabilities, weights and delays. The Parameters can be combined in
different ways, and they can be used with some mathematical functions that
are provided by NEST.


.. _random_ex:

Random parameters
^^^^^^^^^^^^^^^^^

  +--------------------------------+-----------------------------------+
  | Parameter                      | Description                       |
  +================================+===================================+
  | ``nest.random.uniform()``      | Draws samples based on a          |
  |                                | uniform distribution.             |
  +--------------------------------+-----------------------------------+
  | ``nest.random.normal()``       | Draws samples based on a          |
  |                                | normal distribution.              |
  +--------------------------------+-----------------------------------+
  | ``nest.random.exponential()``  | Draws samples based on a          |
  |                                | exponential distribution.         |
  +--------------------------------+-----------------------------------+
  | ``nest.random.lognormal()``    | Draws samples based on a          |
  |                                | lognormal distribution.           |
  +--------------------------------+-----------------------------------+

For every value to be generated, samples are drawn from a distribution. The distribution uses
NEST's random number generator, and are therefore thread-safe. Note that
arguments can be passed to each of them to control the parameters of the
distribution.

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

Spatial parameters
^^^^^^^^^^^^^^^^^^

  +-----------------------------------------+-------------------------------------------------------------------------+
  | Parameter                               | Description                                                             |
  +=========================================+=========================================================================+
  | | ``nest.spatial.pos.x``                | | Position of a neuron, on the x, y, and z axis.                        |
  | | ``nest.spatial.pos.y``                | | Can be used to set node properties, but not for connecting.           |
  | | ``nest.spatial.pos.z``                |                                                                         |
  +-----------------------------------------+-------------------------------------------------------------------------+
  | | ``nest.spatial.source_pos.x``         | | Position of the source neuron, on the x, y, and z axis.               |
  | | ``nest.spatial.source_pos.y``         | | Can only be used when connecting.                                     |
  | | ``nest.spatial.source_pos.z``         |                                                                         |
  +-----------------------------------------+-------------------------------------------------------------------------+
  | | ``nest.spatial.target_pos.x``         |                                                                         |
  | | ``nest.spatial.target_pos.y``         | | Position of the target neuron, on the x, y, and z axis.               |
  | | ``nest.spatial.target_pos.z``         | | Can only be used when connecting.                                     |
  +-----------------------------------------+-------------------------------------------------------------------------+
  | | ``nest.spatial.distance``             | | Distance between two nodes. Can only be used when connecting.         |
  +-----------------------------------------+-------------------------------------------------------------------------+
  | | ``nest.spatial.dimension_distance.x`` |                                                                         |
  | | ``nest.spatial.dimension_distance.y`` | | Distance on the x, y and z axis between the source and target neuron. |
  | | ``nest.spatial.dimension_distance.z`` | | Can only be used when connecting.                                     |
  +-----------------------------------------+-------------------------------------------------------------------------+

  These Parameters represent positions of neurons or distances between two
  neurons. To set node parameters, only the node position can be used. The
  others can only be used when connecting.

.. code-block:: ipython

    grid_layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(rows=10, columns=8))
    nest.PlotLayer(grid_layer);


.. image:: NEST3_23_0.png


.. code-block:: ipython

    free_layer = nest.Create('iaf_psc_alpha', 100, positions=nest.spatial.free(nest.random.uniform(min=0., max=10.), num_dimensions=2))
    nest.PlotLayer(free_layer);


.. image:: NEST3_24_0.png


.. code-block:: ipython

    nest.ResetKernel()

    positions = nest.spatial.free([[x, 0.5*x] for x in np.linspace(0, 1.0, 10000)])
    layer = nest.Create('iaf_psc_alpha', positions=positions)

    parameter = -60 + nest.spatial.pos.x + (0.4 * nest.spatial.pos.x * nest.random.normal())
    layer.set({'V_m': parameter})

    node_pos = np.array(nest.GetPosition(layer))
    node_pos[:,1]
    v_m = layer.get('V_m');

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(node_pos[:,0], v_m, '.', ms=3.5)
    ax.set_xlabel('Node position on x-axis')
    ax.set_ylabel('V_m');



.. image:: NEST3_25_0.png

  NEST provides some functions to help create distributions based on for
  example the distance between two neurons.

.. _distrib_ex:

Distribution functions
^^^^^^^^^^^^^^^^^^^^^^^^


``nest.distributions.exponential()`` takes `x`, `a`, and `tau` as arguments

.. math::

     p(x) = a e^{-\frac{x}{\tau}}


``nest.distributions.gaussian()`` `x`, `p_center`, `mean`, and `std_deviation` as arguments



.. math::
        p(x) = p_{\text{center}}  e^{-\frac
        {(x-\text{mean})^2}{2\text{std_deviation}^2}}



``nest.distributions.gaussian2D()`` takes `x`, `y`, `p_center`, `mean_x`, `mean_y`, `std_deviation_x`,
`std_deviation_y`, and `rho` as arguments


.. math::

   p(x) = p_{\text{center}}
   e^{-\frac{\frac{(x-\text{mean_x})^2}
   {\text{std_deviation_x}^2}-\frac{
   (y-\text{mean_y})^2}{\text{std_deviation_y}^2}+2
   \rho\frac{(x-\text{mean_x})(y-\text{mean_y})}
   {\text{std_deviation_x}\text{std_deviation_y}}}
   {2(1-\rho^2)}}



``nest.distributions.gamma()`` takes `x`, `alpha`, and `theta` as arguments.


 .. math:: p(x) = \frac{x^{\alpha-1}e^{-\frac{x}
            {\theta}}}{\theta^\alpha\Gamma(\alpha)}

With these functions, you can, for example, recreate a Gaussian kernel as a parameter:

  +------------------------------------------------------------+-----------------------------------------------------------------+
  | NEST 2.x                                                   | NEST 3.0                                                        |
  +------------------------------------------------------------+-----------------------------------------------------------------+
  |                                                            |                                                                 |
  |     kernel = {"gaussian": {"p_center": 1.0, "sigma": 1.0}} |     param = nest.distributions.gaussian(                        |
  |                                                            |     nest.spatial.distance, p_center=1.0, std_deviation=1.0)     |
  |                                                            |                                                                 |
  +------------------------------------------------------------+-----------------------------------------------------------------+


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

.. _math_ex:

Mathematical functions
^^^^^^^^^^^^^^^^^^^^^^

  +----------------------------+-------------------------------------------+
  | Parameter                  | Description                               |
  +----------------------------+-------------------------------------------+
  | ``nest.random.exp()``      | Calculates the exponential of a Parameter |
  +----------------------------+-------------------------------------------+
  | ``nest.random.cos()``      | Calculates the cosine of a Parameter      |
  +----------------------------+-------------------------------------------+
  | ``nest.random.sin()``      | Calculates the sine of a Parameter        |
  +----------------------------+-------------------------------------------+

The mathematical functions take a Parameter object as argument, and return
a new Parameter which applies the mathematical function on the Parameter
given as argument.

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

.. _logic:

Clipping, redraw, and conditionals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  +------------------------------+-------------------------------------------------------+
  | Parameter                    | Description                                           |
  +------------------------------+-------------------------------------------------------+
  | ``nest.math.min()``          | | If a value from the Parameter is above a threshold, |
  |                              | | the value is replaced with the value of the         |
  |                              | | threshold.                                          |
  +------------------------------+-------------------------------------------------------+
  | ``nest.math.max()``          | | If a value from the Parameter is beneath a          |
  |                              | | threshold, the value is replaced with the value of  |
  |                              | | the threshold.                                      |
  +------------------------------+-------------------------------------------------------+
  | ``nest.math.redraw()``       | | If a value from the Parameter is outside of the     |
  |                              | | limits given, the value is redrawn. Throws an error |
  |                              | | if a suitable value is not found after a certain    |
  |                              | | number of redraws.                                  |
  +------------------------------+-------------------------------------------------------+
  | ``nest.logic.conditional()`` | | Given a condition, yields one value or another      |
  |                              | | based on if the condition evaluates to true or      |
  |                              | | false.                                              |
  +------------------------------+-------------------------------------------------------+

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


Combining parameters
^^^^^^^^^^^^^^^^^^^^

NEST Parameters support the basic arithmetic operations. Two Parameters
can be added together, subtracted, multiplied with each other, or one can
be divided by the other. They also support being raised to the power of a
number, but they can only be raised to the power of an integer or a
floating point number. Parameters can therefore be combined in almost any
way. In fact the distribution functions in ``nest.distributions`` are just
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

Using parameters to set node properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Using Parameters makes it easy to set node properties

  +-----------------------------------------------+----------------------------------------------------+
  | NEST 2.x                                      | NEST 3.0                                           |
  +===============================================+====================================================+
  |                                               |                                                    |
  | ::                                            | ::                                                 |
  |                                               |                                                    |
  |     for gid in nrns:                          |     nrns.set('V_m', nest.random.uniform(-20., 20)) |
  |         v_m = numpy.random.uniform(-20., 20.) |                                                    |
  |         nest.SetStatus([gid], {'V_m': V_m})   |                                                    |
  |                                               |                                                    |
  |                                               |                                                    |
  +-----------------------------------------------+----------------------------------------------------+

What's changed?
----------------

.. _topo_changes:

Topology module
~~~~~~~~~~~~~~~~

-  All topology functions are now part of ``nest`` and not
   ``nest.topology``
-  You can use the ``Create`` and ``Connect`` functions for structured?? networks, same as you would for a "regular"
   network
-  ``nest.GetPosition`` -> now takes a GIDCollection instead of a list of GIDs
-  ``nest.FindCenterElement`` -> now returns ``int`` instead of
   ``tuple``

.. note::

   See the reference section :ref:`topo_ref` in our conversion guide for all changes made to functions

Much of the functionality of Topology has been moved to the standard
functions. In fact, there is no longer a Topology module in PyNEST. The
functions that are specific for Topology are now in the ``nest`` module.

Creating layers
^^^^^^^^^^^^^^^

Creating layers is now done with the standard ``nest.Create()`` function.
Arguments of layer creation have also been changed to make creating
populations with and without spatial information more unified. To create
nodes with spatial positions, ``nest.Create()`` must be provided with the
``positions`` argument

::

    layer = nest.Create(model, positions=spatial_data)

where ``spatial_data`` can be one of the following

``nest.spatial.grid()``
    This creates a grid layer, with a prescribed number of rows and
    columns, and a specified extent. Some example grid layer
    specifications:

    ::

        nest.spatial.grid(rows=5, columns=4, extent=[2., 2.])  # 5x4 grid in a 2x2 square
        nest.spatial.grid(rows=4, columns=5, center=[1., 1.])  # 4x5 grid in the default 1x1 square, with shifted center
        nest.spatial.grid(rows=4, columns=5, edge_wrap=True)  # 4x5 grid with periodic boundary conditions
        nest.spatial.grid(rows=2, columns=3, depth=4)  # 3D 2x3x4 grid

``nest.spatial.free()``
    This creates a free layer. The first argument to
    ``nest.spatial.free()`` can be either a NEST Parameter that generates
    the positions, or an explicit list of positions. Some example free
    layer specifications:

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

Topology layers are no longer subnets, as subnets have been removed, but
GIDCollections with metadata. These GIDCollections behave as normal
GIDCollections with two exceptions:

- They cannot be merged, as concatenating GIDCollections with metadata is
  not allowed.
- Setting the status of nodes and connecting layer GIDCollections can
  use spatial information as parameters.

The second point means that we can use masks and position dependent
parameters when connecting, and it is possible to set parameters of nodes
based on their positions. We can for example set the membrane potential to
a value based on the nodes' position on the x-axis:

::

    layer = nest.Create('iaf_psc_alpha', 10
                        positions=nest.spatial.free(
                            nest.random.uniform(min=-10., max=10.), num_dimensions=2))
    layer.set('V_m', -60. + nest.spatial.pos.x)

It is also no longer possible to create composite layers, i.e. layers with
multiple nodes in each position. To reproduce this, we now have to create
multiple layers.

.. TODO: Composite layer replacement recommendation/example

Connecting layers
^^^^^^^^^^^^^^^^^^

Similar to creating layers, connecting layers is now done with the
standard ``nest.Connect()`` function. Connecting GIDCollections with
spatial data is no different from connecting GIDCollections without
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

  ``use_on_source`` here refers to if the mask and connection probability
  should be applied to the source neuron instead of the target neuron.
  This is only required for ``pairwise_bernoulli``, as ``fixed_indegree``
  and ``fixed_outdegree`` implicitly states if we are using the source or
  target layer as a driver.

- The connection probability specification ``kernel``  is renamed to ``p``
  to fit with ``pairwise_bernoulli``, and is only possible for the
  connection rules in the table above.

- Using a ``mask`` is only possible with the connection rules in the table
  above.

Usage examples
~~~~~~~~~~~~~~~

A grid layer connected with Gaussian distance dependent connection
probability and rectangular mask on the target layer:

  +---------------------------------------------------------+---------------------------------------------------------+
  | NEST 2.x                                                | NEST 3.0                                                |
  +=========================================================+=========================================================+
  |                                                         |                                                         |
  | ::                                                      | ::                                                      |
  |                                                         |                                                         |
  |     l = tp.CreateLayer(                                 |     l = nest.Create('iaf_psc_alpha',                    |
  |         {'columns': nc, 'rows': nr,                     |                     positions=nest.spatial.grid(        |
  |          'elements': 'iaf_psc_alpha',                   |                         rows=nr, columns=nc,            |
  |          'extent': [2., 2.]})                           |                         extent=[2., 2.]))               |
  |                                                         |                                                         |
  |     conn_dict = {'connection_type': 'divergent',        |     conn_dict = {'rule': 'pairwise_bernoulli',          |
  |                  'kernel': {'gaussian':                 |                  'p': nest.distributions.gaussian(      |
  |                             {'p_center': 1.,            |                      nest.spatial.distance,             |
  |                              'sigma': 1.}},             |                      p_center=1., std_deviation=1.),    |
  |                  'mask': {'rectangular':                |                  'mask': {'rectangular':                |
  |                           {'lower_left': [-0.5, -0.5],  |                           {'lower_left': [-0.5, -0.5],  |
  |                            'upper_right': [0.5, 0.5]}}} |                            'upper_right': [0.5, 0.5]}}} |
  |     nest.ConnectLayers(l, l, conn_dict)                 |     nest.Connect(l, l, conn_dict)                       |
  |                                                         |                                                         |
  +---------------------------------------------------------+---------------------------------------------------------+

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
  |                  'number_of_connections': 50,                    |                  'weight': nest.random.normal(min=-1., max=1.),     |
  |                  'kernel': {'linear':                            |                  'delay': 1.5*nest.spatial.distance,                |
  |                             {'a': -0.5, 'c': 1.}},               |                  'multapses': True,                                 |
  |                  'weights': {'normal':                           |                  'autapses': False}                                 |
  |                              {'min': -1.0, 'max': 1.0}},         |     nest.Connect(l, l, conn_dict)                                   |
  |                  'delays': {'linear': {'a': 1.5, 'c': 0.}},      |                                                                     |
  |                  'allow_multapses': True,                        |                                                                     |
  |                  'allow_autapses': False}                        |                                                                     |
  |     tp.ConnectLayers(l, l, conn_dict)                            |                                                                     |
  |                                                                  |                                                                     |
  +------------------------------------------------------------------+---------------------------------------------------------------------+

What's removed?
-----------------

Subnets
~~~~~~~~~~

Subnets are gone. Instead GIDCollections should be used to organize neurons.

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
prints GID ranges and model names of the nodes in the network.

  +---------------------------------------------+---------------------------------------+
  | NEST 2.x                                    | NEST 3.0                              |
  +=============================================+=======================================+
  |                                             |                                       |
  | ::                                          | ::                                    |
  |                                             |                                       |
  |     nest.PrintNetwork(depth=2, subnet=None) |     nest.PrintNodes()                 |
  |                                             |                                       |
  | prints                                      | prints                                |
  |                                             |                                       |
  | ::                                          | ::                                    |
  |                                             |                                       |
  |     +-[0] root dim=[15]                     |      1 .. 10 iaf_psc_alpha            |
  |        |                                    |     11 .. 15 iaf_psc_exp              |
  |        +-[1]...[10] iaf_psc_alpha           |                                       |
  |        +-[11]...[15] iaf_psc_exp            |                                       |
  |                                             |                                       |
  +---------------------------------------------+---------------------------------------+


