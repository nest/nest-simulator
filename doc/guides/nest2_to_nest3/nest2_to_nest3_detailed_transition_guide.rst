NEST 3.0: Detailed transition guide
===================================

* This transition guide provides the changes to functions or their output between PyNEST 2.x and PyNEST 3.0

* Functions not mentioned are unchanged

* Terms that changed for NEST 3.0 are marked in :green:`green`

* **Please note that NEST 3.0 no longer supports Python 2**

.. contents:: Here you'll find
   :local:
   :depth: 2

.. seealso::

  To see code examples of the key changes, check out our :doc:`nest2_to_nest3_overview` guide.

.. _setverbosity:

Suppress output on startup
~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------+----------------------------------+
| NEST 2.x                              | NEST 3.0                         |
+=======================================+==================================+
| export DELAY_PYNEST_INIT = 1          | export :green:`PYNEST_QUIET = 1` |
|                                       |                                  |
| import nest                           | import nest                      |
|                                       |                                  |
| nest.ll_api.init(["nest", "--quiet"]) |                                  |
+---------------------------------------+----------------------------------+


.. _node_ref:

Functions related to creation and retrieval of nodes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------+-------------------------------------+
| NEST 2.x                        | NEST 3.0                            |
+=================================+=====================================+
| nest.Create(model, n=1, params= | nest.Create(model, n=1, params=     |
| None) *returns*                 | None) *returns*                     |
| list                            | :darkgreen:`nest.NodeCollection`    |
+---------------------------------+-------------------------------------+
| nest.GetLid(gid) *returns*      | :green:`nest.GetLocalNodeConnection(|
| list                            | nest.NodeConnection)`               |
|                                 | *returns the MPI local nodes*       |
|                                 | *in a new* nest.NodeCollection.     |
+---------------------------------+-------------------------------------+

.. _conn_ref:

Functions related to connection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------------+-----------------------------------------------+
| NEST 2.x                                    | NEST 3.0                                      |
+=============================================+===============================================+
| nest.GetConnections(list=None,              | nest.GetConnections(                          |
| list=None, synapse_model=None,              | :green:`nest.NodeCollection` =None,           |
| synapse_label=None)                         | :green:`nest.NodeCollection` =None,           |
| *returns* numpy.array                       | synapse_model=None, synapse_label=None)       |
|                                             | *returns* :darkgreen:`nest.SynapseCollection` |
+---------------------------------------------+-----------------------------------------------+
| nest.Connect(list, list, conn_spec          | nest.Connect(:green:`nest.NodeCollection`,    |
| =None, syn_spec=None, model=None)           | :green:`nest.NodeCollection`, conn_spec=      |
|                                             | None, syn_spec=None,                          |
|                                             | :green:`return_SynapseCollection`\ =False)    |
|                                             |                                               |
|                                             | *In syn_spec the synapse model is given by*   |
|                                             | *the* synapse_model *key, where before it*    |
|                                             | *was* model                                   |
+---------------------------------------------+-----------------------------------------------+
| nest.DataConnect(pre, post)                 | *Use* nest.Connect(np.array, np.array,        |
|                                             | conn_spec="one_to_one")                       |
+---------------------------------------------+-----------------------------------------------+
| nest.CGConnect(list, list, cg,              | nest.CGConnect(:green:`nest.NodeCollection`,  |
| parameter_map=None, model='static           | :green:`nest.NodeCollection`, cg,             |
| _synapse')                                  | parameter_map=None,                           |
|                                             | :green:`synapse_model` ='static_synapse')     |
+---------------------------------------------+-----------------------------------------------+
| nest.DisconnectOneToOne(int, int,           | nest.Disconnect(:green:`nest.NodeCollection`, |
| syn_spec)                                   | :green:`nest.NodeCollection`,                 |
|                                             | syn_spec='static_synapse')                    |
+---------------------------------------------+-----------------------------------------------+
| nest.Disconnect(list, list, conn_spec=      | nest.Disconnect(:green:`nest.NodeCollection`, |
| 'one_to_one', syn_spec='static_synapse')    | :green:`nest.NodeCollection`, conn_spec=      |
|                                             | 'one_to_one', syn_spec='static_synapse')      |
|                                             |                                               |
+---------------------------------------------+-----------------------------------------------+

.. _subnet_ref:

Functions related to subnets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**The subnet model is removed in NEST 3.0!**

+----------------------------------------+--------------------------------------------+
| NEST 2.x                               | NEST 3.0                                   |
+========================================+============================================+
| nest.PrintNetwork(depth=1, subnet      | :green:`nest.PrintNodes()`                 |
| =None)                                 |                                            |
+----------------------------------------+--------------------------------------------+
| nest.CurrentSubnet()                   |                                            |
+----------------------------------------+--------------------------------------------+
| nest.ChangeSubnet(subnet)              |                                            |
+----------------------------------------+--------------------------------------------+
| nest.GetLeaves(subnet, properties      | :green:`nest.NodeCollection` will contain  |
| =None, local_only=False)               | all nodes                                  |
+----------------------------------------+--------------------------------------------+
| nest.GetNodes(subnets, properties      | GetNodes(properties={}, local_only=False)  |
| =None, local_only=False)               | *returns* :darkgreen:`nest.NodeCollection` |
+----------------------------------------+--------------------------------------------+
| nest.GetChildren(subnets, properties   | :green:`nest.NodeCollection` will contain  |
| =None, local_only=False)               | all nodes                                  |
+----------------------------------------+--------------------------------------------+
| nest.GetNetwork(gid, depth)            |                                            |
+----------------------------------------+--------------------------------------------+
| nest.BeginSubnet(label=None, params    |                                            |
| =None)                                 |                                            |
+----------------------------------------+--------------------------------------------+
| nest.EndSubnet()                       |                                            |
+----------------------------------------+--------------------------------------------+
| nest.LayoutNetwork(model, dim,         | *Use*                                      |
| label=None, params=None)               | nest.Create(model, n=1, params=None,       |
|                                        | positions=None)                            |
+----------------------------------------+--------------------------------------------+

.. _info_ref:

Functions related to setting and getting parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------+---------------------------------------------+
| NEST 2.x                              | NEST 3.0                                    |
+=======================================+=============================================+
| nest.SetStatus(list/tuple,            | nest.SetStatus(:green:`nest.NodeCollection`,|
| params, val=None)                     | params, val=None) *Can*                     |
|                                       | *also use* :green:`nodes.set(params)`,      |
|                                       | :green:`nodes.parameter = value`,           |
|                                       | :green:`conns.set(params)` *or*             |
|                                       | :green:`conns.parameter = value`            |
+---------------------------------------+---------------------------------------------+
| nest.GetStatus(list/tuple,            | nest.GetStatus(:green:`nest.NodeCollection`,|
| keys=None)                            | keys=None) *Can*                            |
|                                       | *also use* :green:`nodes.get(keys=None)`,   |
|                                       | :green:`nodes.parameter`,                   |
|                                       | :green:`conns.get(keys=None)` *or*          |
|                                       | :green:`conns.parameter`                    |
+---------------------------------------+---------------------------------------------+

.. _topo_ref:


Function related to spatially distributed nodes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Spatial structure, formerly provided by the Topology module, is now integrated into NEST and is no longer
a separate module.

+------------------------------------------------+----------------------------------------------------+
| NEST 2.x                                       | NEST 3.0                                           |
+================================================+====================================================+
| tp.CreateLayer(specs) *returns*                | :green:`nest.Create`\ (model, params=None,         |
| tuple of int(s)                                | positions=nest.spatial.free/grid)                  |
|                                                | *returns*                                          |
|                                                | :darkgreen:`nest.NodeCollection` NOTE:             |
|                                                | *Composite layers no longer*                       |
|                                                | *possible.*                                        |
+------------------------------------------------+----------------------------------------------------+
| tp.ConnectLayers(list, list,                   | :green:`nest.Connect`\ (\                          |
| projections)                                   | :green:`nest.NodeCollection`,                      |
|                                                | :green:`nest.NodeCollection`, conn_spec=None,      |
|                                                | syn_spec=None, :green:`return_SynapseCollection`   |
|                                                | =False)                                            |
+------------------------------------------------+----------------------------------------------------+
|                                                | :green:`spatial_NodeCollection.spatial`            |
|                                                | *returns*                                          |
|                                                | *Dictionary with spatial properties*               |
+------------------------------------------------+----------------------------------------------------+
| tp.GetLayer(nodes) *returns*                   | :green:`nest.NodeCollection` will represent the    |
| tuple                                          | spatially distributed nodes                        |
+------------------------------------------------+----------------------------------------------------+
| tp.GetElement(layers, location)                | :green:`nest.NodeCollection` will contain all nodes|
| *returns*                                      |                                                    |
| tuple                                          |                                                    |
+------------------------------------------------+----------------------------------------------------+
| tp.GetPosition(tuple) *returns*                | :green:`nest`.GetPosition(\                        |
| tuple of tuple(s)                              | :green:`nest.NodeCollection`) *returns*            |
|                                                | tuple or                                           |
|                                                | tuple of tuple(s)                                  |
+------------------------------------------------+----------------------------------------------------+
| tp.Displacement(from_arg, to_arg)              | :green:`nest`.Displacement(from_arg, to_arg)       |
| from_arg:                                      | *from_arg:*                                        |
| tuple/list of int(s) / tuple/list              | :green:`nest.NodeCollection` *or* tuple/list       |
| of tuples/lists of floats]                     | with tuple(s)/list(s) of floats                    |
| to_arg:                                        | *to_arg:*                                          |
| tuple/list of int(s) *returns*                 | :green:`nest.NodeCollection` *returns*             |
| tuple                                          | tuple                                              |
+------------------------------------------------+----------------------------------------------------+
| tp.Distance(from_arg, to_arg)                  | :green:`nest`.Distance(from_arg, to_arg)           |
| from_arg:                                      | *from_arg:*                                        |
| [tuple/list of ints / tuple/list               | :green:`nest.NodeCollection` *or* tuple/list       |
| with tuples/lists of floats]                   | with tuple(s)/list(s) of floats                    |
| to:arg:                                        | *to_arg:*                                          |
| tuple/list of ints *returns*                   | :green:`nest.NodeCollection` *returns*             |
| tuple                                          | tuple                                              |
+------------------------------------------------+----------------------------------------------------+
| tp.FindNearestElement(tuple/list,              | :green:`nest`.FindNearestElement(\                 |
| locations, find_all=True)                      | :green:`nest.NodeCollection`, locations,           |
| *returns*                                      | find_all=True) *returns*                           |
| tuple                                          | :darkgreen:`nest.NodeCollection`                   |
+------------------------------------------------+----------------------------------------------------+
| tp.DumpLayerNodes(tuple, outname)              | :green:`nest`.DumpLayerNodes(\                     |
|                                                | :green:`nest.NodeCollection`, outname)             |
+------------------------------------------------+----------------------------------------------------+
| tp.DumpLayerConnections(tuple,                 | :green:`nest`.DumpLayerConnections(                |
| synapse_model, outname)                        | :green:`nest.NodeCollection`,                      |
|                                                | :green:`nest.NodeCollection`, synapse_model,       |
|                                                | outname)                                           |
+------------------------------------------------+----------------------------------------------------+
| tp.FindCenterElement(tuple)                    | :green:`nest`.FindCenterElement(\                  |
| *returns*                                      | :green:`nest.NodeCollection`) *returns*            |
| tuple                                          | :darkgreen:`nest.NodeCollection`                   |
+------------------------------------------------+----------------------------------------------------+
| tp.GetTargetNodes(tuple, tuple,                | :green:`nest`.GetTargetNodes(\                     |
| tgt_model=None, syn_model=None)                | :green:`nest.NodeCollection`,                      |
| *returns*                                      | :green:`nest.NodeCollection`, syn_model=None)      |
| tuple of list(s) of int(s)                     | *returns* tuple of :darkgreen:`nest.NodeConnection`|
+------------------------------------------------+----------------------------------------------------+
| tp.GetTargetPositions(tuple, tuple,            | :green:`nest`.GetTargetPositions(\                 |
| tgt_model=None, syn_model=None)                | :green:`nest.NodeCollection`,                      |
| *returns*                                      | :green:`nest.NodeCollection`,                      |
| tuple of tuple(s) of tuple(s)                  | :green:`synapse_model`\ =None) *returns* list of   |
| of floats                                      | list(s) of tuple(s) of floats                      |
+------------------------------------------------+----------------------------------------------------+
| tp.SelectNodesByMask(tuple, anchor,            | :green:`nest`.SelectNodesByMaks(\                  |
| mask_obj) *returns*                            | :green:`nest.NodeCollection`, anchor, mask_obj)    |
| list                                           | *returns*                                          |
|                                                | :darkgreen:`nest.NodeConnection`                   |
+------------------------------------------------+----------------------------------------------------+
| tp.PlotLayer(tuple, fig=None,                  | :green:`nest`.PlotLayer(\                          |
| nodecolor='b', nodesize=20)                    | :green:`nest.NodeCollection`, fig=None,            |
| *returns*                                      | nodecolor ='b', nodesize=20) *returns*             |
| matplotlib.figure.Figure                       | matplotlib.figure.Figure                           |
| object                                         | object                                             |
+------------------------------------------------+----------------------------------------------------+
| tp.PlotTargets(int, tuple, tgt_model=          | :green:`nest`.PlotTargets(\                        |
| =None, syn_type=None, fig=None,                | :green:`nest.NodeCollection`,                      |
| mask=None, kernel=None, src_color=             | :green:`nest.NodeCollection`, syn_type=            |
| 'red', src_size=50, tgt_color=                 | None, fig=None, mask=None, kernel=                 |
| 'blue', tgt_size=20, mask_color                | None, src_color='red', src_size=                   |
| ='red', kernel_color='red')                    | 50, tgt_color='blue', tgt_size=                    |
| *returns*                                      | '20, mask_color='red', kernel_color='red')         |
| matplotlib.figure.Figure                       | *returns* matplotlib.figure.Figure                 |
| object                                         | object                                             |
+------------------------------------------------+----------------------------------------------------+
| tp.PlotKernel(ax, int, mask,                   | :green:`nest.PlotProbabilityParameter` (           |
| kern=None, mask_color='red',                   | :green:`nest.NodeCollection`,                      |
| kernel_color='red')                            | :green:`parameter=None`, mask=None,                |
|                                                | :green:`edges=[-0.5, 0.5, -0.5, 0.5]`,             |
|                                                | :green:`shape=[100, 100]`, ax=None,                |
|                                                | :green:`prob_cmap` ='Greens', mask_color='yellow') |
+------------------------------------------------+----------------------------------------------------+
| 'mask': {'volume':                             | 'mask': {':green:`box`'                            |
| {'lower_left': [-2., -1., -1.],                | {'lower_left': [-2., -1., -1.],                    |
| 'upper_right': [2., 1., 1.]}}                  | 'upper_right': [2., 1., 1.]}}                      |
+------------------------------------------------+----------------------------------------------------+


.. _connrules:

Spatially distributed connection rules
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

====================================== =================================================
NEST 2.x                               NEST 3.0
====================================== =================================================
convergent                             pairwise_bernoulli *and* use_on_source=True
convergent *and* num_connections       fixed_indegree
divergent                              pairwise_bernoulli
divergent *and* num_connections        fixed_outdegree
====================================== =================================================


Functions related to simulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+-------------------------+--------------------------------------------+
| NEST 2.x                | NEST 3.0                                   |
+=========================+============================================+
| nest.ResetNetwork()     | Use nest.ResetKernel() instead             |
+-------------------------+--------------------------------------------+


Functions related to models
~~~~~~~~~~~~~~~~~~~~~~~~~~~

No Change


Functions related to parallel computing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

No Change


Parameters
~~~~~~~~~~

Parameters can now be used to set node and connection parameters.

.. note::

    Check out the section on :ref:`param_ex` for example usage

.. _random_param:

:green:`random`
^^^^^^^^^^^^^^^^
The random module contains random distributions that can be used to set node
and connection parameters, as well as positions for spatially distributed nodes.

+-------+------------------------------------------------------------+
| NEST  | NEST 3.0                                                   |
| 2.x   |                                                            |
+=======+============================================================+
|       | nest.random.exponential(beta=1.0) *returns*                |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+
|       | nest.random.lognormal(mean=0.0, std=1.0) *returns*         |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+
|       | nest.random.normal(mean=0.0, std=1.0) *returns*            |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+
|       | nest.random.uniform(min=0.0, max=1.0) *returns*            |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+

.. _spatial_param:

:green:`spatial`
^^^^^^^^^^^^^^^^^
The spatial module contains parameters related to spatial positions for the
nodes.

+-------+----------------------------------------------------------------+
| NEST  | NEST 3.0                                                       |
| 2.x   |                                                                |
+=======+================================================================+
|       | nest.spatial.distance.x  nest.spatial.distance.y               |
|       | nest.spatial.distance.z                                        |
|       | *returns*                                                      |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.distance *returns* nest.Parameter                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.free(pos, extent=None, edge_wrap=False,           |
|       | num_dimensions=None) *returns*                                 |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.grid(shape, center=None, extent=None,             |
|       | edge_wrap=False) *returns*                                     |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.pos.x  nest.spatial.pos.y  nest.spatial.pos.z     |
|       | *returns*                                                      |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.source_pos.x  nest.spatial.source_pos.y           |
|       | nest.spatial.source_pos.z *returns*                            |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.target_pos.x  nest.spatial.target_pos.y           |
|       | nest.spatial.target_pos.z *returns*                            |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+

.. _math_param:

:green:`math`
^^^^^^^^^^^^^
The math module contains parameters for mathematical expressions. The mathematical
expressions all take a nest.Parameter.

+----------+--------------------------------------------+
| NEST 2.X | NEST 3.0                                   |
+==========+============================================+
|          | nest.math.exp(nest.Parameter)              |
|          | *returns* nest.Parameter                   |
+----------+--------------------------------------------+
|          | nest.math.sin(nest.Parameter)              |
|          | *returns* nest.Parameter                   |
+----------+--------------------------------------------+
|          | nest.math.cos(nest.Parameter)              |
|          | *returns* nest.Parameter                   |
+----------+--------------------------------------------+
|          | nest.math.min(nest.Parameter, value)       |
|          | *returns* nest.Parameter                   |
+----------+--------------------------------------------+
|          | nest.math.max(nest.Parameter, value)       |
|          | *returns* nest.Parameter                   |
+----------+--------------------------------------------+
|          | nest.math.redraw(nest.Parameter, min, max) |
|          | *returns* nest.Parameter                   |
+----------+--------------------------------------------+

.. _logic_param:

:green:`logic`
^^^^^^^^^^^^^^
The logic module contains logical expressions between nest.Parameter's.

+-------+------------------------------------------------------------------+
| NEST  | NEST 3.0                                                         |
| 2.x   |                                                                  |
+=======+==================================================================+
|       | nest.logic.conditional(condition, param_if_true, param_if_false) |
|       | *returns*                                                        |
|       | nest.Parameter                                                   |
+-------+------------------------------------------------------------------+

.. _distr_param:

:green:`spatial_distributions`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The spatial_distributions module contains random distributions that take a spatial
parameter as input and applies the distribution on the parameter. They are used
for spatially distributed nodes.

+-------+------------------------------------------------------------+
| NEST  | NEST 3.0                                                   |
| 2.x   |                                                            |
+=======+============================================================+
|       | nest.spatial_distributions.exponential(nest.Parameter,     |
|       | beta=1.0) *returns* nest.Parameter                         |
+-------+------------------------------------------------------------+
|       | nest.spatial_distributions.gaussian(nest.Parameter,        |
|       | mean=0.0, std=1.0) *returns* nest.Parameter                |
+-------+------------------------------------------------------------+
|       | nest.spatial_distributions.gaussian2D(nest.Parameter,      |
|       | nest.Parameter, mean_x=0.0, mean_y=0.0, std_x=1.0,         |
|       | std_y=1.0, rho=0.0) *returns* nest.Parameter               |
+-------+------------------------------------------------------------+
|       | nest.spatial_distributions.gamma(nest.Parameter, kappa=1.0 |
|       | theta=1.0) *returns* nest.Parameter                        |
+-------+------------------------------------------------------------+
