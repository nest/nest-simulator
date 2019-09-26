NEST 2.X vs. NEST 3 conversion reference guide
==================================================

* This conversion guide provides the changes to functions or their output between NEST 2.x and NEST 3

* Functions not mentioned are unchanged

* The new terms for NEST 3 are marked in :green:`green`.


.. seealso::

  You can find code examples of changes in NEST 3 :doc:`in our NEST-3 overview <nest3_overview>`

.. _node_ref:

Nodes
~~~~~

+---------------------------------+---------------------------------+
| NEST 2.x                        | NEST 3.0                        |
+=================================+=================================+
| nest.Create(model, n=1, params= | nest.Create(model, n=1, params= |
| None) *returns*                 | None) *returns*                 |
| list                            | :green:`nest.GIDCollection`     |
+---------------------------------+---------------------------------+
| nest.GetLid(gid) *returns*      |                                 |
| list                            |                                 |
+---------------------------------+---------------------------------+

.. _conn_ref:

Connection
~~~~~~~~~~

+---------------------------------------------+---------------------------------------------+
| NEST 2.x                                    | NEST 3.0                                    |
+=============================================+=============================================+
| nest.GetConnections(list=None,              | nest.GetConnections(                        |
| list=None, synapse_model=None,              | :green:`nest.GIDcollection` =None,          |
| synapse_label=None)                         | :green:`nest.GIDcollection` =None,          |
| *returns* numpy.array                       | synapse_model=None, synapse_label=None)     |
|                                             | *returns* :green:`nest.Connectome`          |
+---------------------------------------------+---------------------------------------------+
| nest.Connect(list, list, conn_spec          | nest.Connect(:green:`nest.GIDCollection`,   |
| =None, syn_spec=None, model=None)           | :green:`nest.GIDCollection`, conn_spec=     |
|                                             | None, syn_spec=None,                        |
|                                             | :green:`return_connectome` =False           |
|                                             | *In syn_spec* *the synapse model*           |
|                                             | *is given by* *synapse_model,*              |
|                                             | *not model.*                                |
+---------------------------------------------+---------------------------------------------+
| nest.DataConnect(pre, params=None,          |                                             |
| model="static_synapse")                     |                                             |
+---------------------------------------------+---------------------------------------------+
| nest.CGConnect(list, list, cg,              | nest.CGConnect(:green:`nest.GIDCollection`, |
| parameter_map=None, model='static           | :green:`nest.GIDCollection`, cg,            |
| _synapse')                                  | parameter_map=None,                         |
|                                             | :green:`synapse_model` ='static_synapse')   |
+---------------------------------------------+---------------------------------------------+
| nest.DisconnectOneToOne(int, int,           |                                             |
| syn_spec)                                   |                                             |
+---------------------------------------------+---------------------------------------------+
| nest.Disconnect(list, list, conn_spec=      | nest.Disconnect(:green:`nest.GIDCollection`,|
| 'one_to_one', syn_spec='static_synapse')    | :green:`nest.GIDCollection`, conn_spec=     |
|                                             | 'one_to_one', syn_spec='static_synapse')    |
|                                             |                                             |
+---------------------------------------------+---------------------------------------------+

.. _subnet_ref:

Subnets
~~~~~~~

**The subnets module is deprecated!**

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
| nest.GetLeaves(subnet, properties      | :green:`nest.GIDCollection` will contain   |
| =None, local_only=False)               | all nodes                                  |
+----------------------------------------+--------------------------------------------+
| nest.GetNodes(subnets, properties      | :green:`nest.GIDCollection` will contain   |
| =None, local_only=False)               | all nodes                                  |
+----------------------------------------+--------------------------------------------+
| nest.GetChildren(subnets, properties   | :green:`nest.GIDCollection` will contain   |
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

Info
~~~~

+---------------------------------------+--------------------------------------------+
| NEST 2.x                              | NEST 3.0                                   |
+=======================================+============================================+
| nest.SetStatus(list/tuple,            | nest.SetStatus(:green:`nest.GIDCollection`,|
| params, val=None)                     | params, val=None) *Can*                    |
|                                       | *also use* nodes.set(params) *or*          |
|                                       | conns.set(params)                          |
+---------------------------------------+--------------------------------------------+
| nest.GetStatus(list/tuple,            | nest.GetStatus(:green:`nest.GIDCollection`,|
| keys=None)                            | keys=None) *Can*                           |
|                                       | *also use* nodes.get(keys=None) *or*       |
|                                       | conns.get(keys=None)                       |
+---------------------------------------+--------------------------------------------+

.. _topo_ref:


Topology
~~~~~~~~

Topology is now integrated into NEST and no longer a separate module.


+------------------------------------------------+----------------------------------------------------+
| NEST 2.x                                       | NEST 3.0                                           |
+================================================+====================================================+
| tp.CreateLayer(specs) *returns*                | :green:`nest.Create`\ (model, params=None,         |
| tuple of int(s)                                | positions=nest.spatial.free/grid)                  |
|                                                | *returns*                                          |
|                                                | :green:`nest.GIDCollection` NOTE:                  |
|                                                | *Composite layers no longer*                       |
|                                                | *possible.*                                        |
+------------------------------------------------+----------------------------------------------------+
| tp.ConnectLayers(list, list,                   | :green:`nest.Connect`\ (\                          |
| projections)                                   | :green:`nest.GIDCollection`,                       |
|                                                | :green:`nest.GIDCollection`, conn_spec= None,      |
|                                                | syn_spec=None, :green:`return_connectome` = False) |
+------------------------------------------------+----------------------------------------------------+
|                                                | :green:`layer_GIDCollection.spatial`               |
+------------------------------------------------+----------------------------------------------------+
| tp.GetLayer(nodes) *returns*                   |                                                    |
| tuple                                          |                                                    |
+------------------------------------------------+----------------------------------------------------+
| tp.GetElement(layers, location)                |                                                    |
| *returns*                                      |                                                    |
| tuple                                          |                                                    |
+------------------------------------------------+----------------------------------------------------+
| tp.GetPosition(tuple) *returns*                | :green:`nest`.GetPosition(\                        |
| tuple of tuple(s)                              | :green:`nest.GIDCollection`) *returns*             |
|                                                | tuple or                                           |
|                                                | tuple of tuple(s)                                  |
+------------------------------------------------+----------------------------------------------------+
| tp.Displacement(from_arg, to_arg)              | :green:`nest`.Displacement(from_arg, to_arg)       |
| from_arg:                                      | *from_arg:*                                        |
| tuple/list of int(s) / tuple/list              | :green:`nest.GIDCollection` *or* tuple/list        |
| of tuples/lists of floats]                     | with tuple(s)/list(s) of floats                    |
| to_arg:                                        | *to_arg:*                                          |
| tuple/list of int(s) *returns*                 | :green:`nest.GIDCollection` *returns*              |
| tuple                                          | tuple                                              |
+------------------------------------------------+----------------------------------------------------+
| tp.Distance(from_arg, to_arg)                  | :green:`nest`.Distance(from_arg, to_arg)           |
| from_arg:                                      | *from_arg:*                                        |
| [tuple/list of ints / tuple/list               | :green:`nest.GIDCollection` *or* tuple/list        |
| with tuples/lists of floats]                   | with tuple(s)/list(s) of floats                    |
| to:arg:                                        | *to_arg:*                                          |
| tuple/list of ints *returns*                   | :green:`nest.GIDCollection` *returns*              |
| tuple                                          | tuple                                              |
+------------------------------------------------+----------------------------------------------------+
| tp.FindNearestElement(tuple/list,              | :green:`nest`.FindNearestElement(\                 |
| locations, find_all=True)                      | :green:`nest.GIDCollection`, locations,            |
| *returns*                                      | find_all=True) *returns*                           |
| tuple                                          | tuple                                              |
+------------------------------------------------+----------------------------------------------------+
| tp.DumpLayerNodes(tuple, outname)              | :green:`nest`.DumpLayerNodes(\                     |
|                                                | :green:`nest.GIDCollection`, outname)              |
+------------------------------------------------+----------------------------------------------------+
| tp.DumpLayerConnections(tuple,                 | :green:`nest`.DumpLayerConnections(                |
| synapse_model, outname)                        | :green:`nest.GIDCollection`,                       |
|                                                | :green:`nest.GIDCollection`, synapse_model,        |
|                                                | outname)                                           |
+------------------------------------------------+----------------------------------------------------+
| tp.FindCenterElement(tuple)                    | :green:`nest`.FindCenterElement(\                  |
| *returns*                                      | :green:`nest.GIDCollection`) *returns*             |
| tuple                                          | :green:`int`                                       |
+------------------------------------------------+----------------------------------------------------+
| tp.GetTargetNodes(tuple, tuple,                | :green:`nest`.GetTargetNodes(tuple,                |
| tgt_model=None, syn_model=None)                | :green:`nest.GIDCollection`, syn_model=None)       |
| *returns*                                      | *returns*                                          |
| tuple of list(s) of int(s)                     | tuple of list(s) of int(s)                         |
+------------------------------------------------+----------------------------------------------------+
| tp.GetTargetPositions(tuple, tuple,            | :green:`nest`.GetTargetPositions(\                 |
| tgt_model=None, syn_model=None)                | :green:`nest.GIDCollection`,                       |
| *returns*                                      | :green:`nest.GIDCollection`, syn_model=None)       |
| tuple of tuple(s) of tuple(s)                  | *returns* list of list(s) of tuple(s) of           |
| of floats                                      | floats                                             |
+------------------------------------------------+----------------------------------------------------+
| tp.SelectNodesByMask(tuple, anchor,            | :green:`nest`.SelectNodesByMaks(\                  |
| mask_obj) *returns*                            | :green:`nest.GIDCollection`, anchor, mask_obj)     |
| list                                           | *returns*                                          |
|                                                | list                                               |
+------------------------------------------------+----------------------------------------------------+
| tp.PlotLayer(tuple, fig=None,                  | :green:`nest`.PlotLayer(\                          |
| nodecolor='b', nodesize=20)                    | :green:`nest.GIDCollection`, fig=None,             |
| *returns*                                      | nodecolor ='b', nodesize=20) *returns*             |
| matplotlib.figure.Figure                       | matplotlib.figure.Figure                           |
| object                                         | object                                             |
+------------------------------------------------+----------------------------------------------------+
| tp.PlotTargets(int, tuple, tgt_model=          | :green:`nest`.PlotTargets(\                        |
| =None, syn_type=None, fig=None,                | :green:`nest.GIDCollection`,                       |
| mask=None, kernel=None, src_color=             | :green:`nest.GIDCollection`, syn_type=             |
| 'red', src_size=50, tgt_color=                 | None, fig=None, mask=None, kernel=                 |
| 'blue', tgt_size=20, mask_color                | None, src_color='red', src_size=                   |
| ='red', kernel_color='red')                    | 50, tgt_color='blue', tgt_size=                    |
| *returns*                                      | '20, mask_color='red', kernel_color='red')         |
| matplotlib.figure.Figure                       | *returns* matplotlib.figure.Figure                 |
| object                                         | object                                             |
+------------------------------------------------+----------------------------------------------------+
| tp.PlotKernel(ax, int, mask, ke                | :green:`nest`.PlotKernel(ax,                       |
| rn=None, mask_color='red', kernel              | :green:`nest.GIDCollection`, mask, kern=None,      |
| _color='red')                                  | mask_color='red', kernel_color='red')              |
+------------------------------------------------+----------------------------------------------------+

.. _connrules:

Connection rules
^^^^^^^^^^^^^^^^

====================================== =================================================
NEST 2.x                               NEST 3
====================================== =================================================
convergent                             pairwise_bernoulli *and* use_on_source=True
convergent *and* num_connections       fixed_indegree
divergent                              pairwise_bernoulli
divergent *and* num_connections        fixed_outdegree
====================================== =================================================

Models
~~~~~~

No Change

Simulation
~~~~~~~~~~

No Change

Parallel Computing
~~~~~~~~~~~~~~~~~~

No Change

Parameters
~~~~~~~~~~

Parameters can now be used to set node and connection parameters.

.. note::

    Check out the section on :ref:`param_ex` for example usage

.. _random_param:

:green:`random`
^^^^^^^^^^^^^^^^

+-------+------------------------------------------------------------+
| NEST  | NEST 3.0                                                   |
| 2.x   |                                                            |
+=======+============================================================+
|       | nest.random.exponential(scale=1.0) *returns*               |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+
|       | nest.random.lognormal(mean=0.0, sigma=1.0, min=None, max=N |
|       | one, dimension=None) *returns*                             |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+
|       | nest.random.normal(loc=0.0, scale=1.0, min=None, max=None, |
|       | redraw=False) *returns*                                    |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+
|       | nest.random.uniform(min=0.0, max=1.0) *returns*            |
|       | nest.Parameter                                             |
+-------+------------------------------------------------------------+

.. _spatial_param:

:green:`spatial`
^^^^^^^^^^^^^^^^^

+-------+----------------------------------------------------------------+
| NEST  | NEST 3.0                                                       |
| 2.x   |                                                                |
+=======+================================================================+
|       | nest.spatial.dimension_distance.x  nest.spatial.dimension      |
|       | _distance.y  nest.spatial.dimension_distance.z                 |
|       | *returns*                                                      |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.distance *returns* nest.Parameter                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.free(pos, extent=None, edge_wrap=False,           |
|       | num_dimensions=None) *returns*                                 |
|       | nest.Parameter                                                 |
+-------+----------------------------------------------------------------+
|       | nest.spatial.grid(rows, columns, depth=None, center=None,      |
|       | extent=None, edge_wrap=False) *returns*                        |
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

+---------+--------------------------------+
| NEST 2.x| NEST 3.0                       |
+=========+================================+
|         |  nest.math.exp(nest.Parameter) |
|         |  nest.math.sin(nest.Parameter) |
|         |  nest.math.cos(nest.Parameter) |
+---------+--------------------------------+

.. _logic_param:

:green:`logic`
^^^^^^^^^^^^^^

+-------+------------------------------------------------------------------+
| NEST  | NEST 3.0                                                         |
| 2.x   |                                                                  |
+=======+==================================================================+
|       | nest.logic.conditional(condition, param_if_true, param_if_false) |
|       | *returns*                                                        |
|       | nest.Parameter                                                   |
+-------+------------------------------------------------------------------+

.. _distr_param:

:green:`distributions`
^^^^^^^^^^^^^^^^^^^^^^^^

+-------+------------------------------------------------------------+
| NEST  | NEST 3.0                                                   |
| 2.x   |                                                            |
+=======+============================================================+
|       | nest.distributions.exponential(nest.Parameter| a=1.0| tau= |
|       | 1.0)                                                       |
+-------+------------------------------------------------------------+
|       | nest.distributions.gaussian(nest.Parameter, p_center=1.0,  |
|       | mean=0.0, std_deviation=1.0)                               |
+-------+------------------------------------------------------------+
|       | nest.distributions.gaussian2D(nest.Parameter, y, p_center= |
|       | 1.0, mean_x=0.0, mean_y=0.0, std_deviation_x=1.0,          |
|       | std_deviation_y=1.0, rho=0.0)                              |
+-------+------------------------------------------------------------+
|       | nest.distributions.gamma(nest.Parameter, alpha=1.0, theta= |
|       | 1.0)                                                       |
+-------+------------------------------------------------------------+


