

# PyNEST 2.x vs PyNEST 3.0

Functions not mentioned are unchanged.

Most functions now take `NodeCollection` instead of lists, but, seeing as `Create()` returns a `NodeCollection`, not much needs to be done.

Functions that you have to be careful about:

- `nest.GetConnections()` returns a `SynapseCollection` object
- All topology functions are now part of `nest` and not `nest.topology`
- `nest.GetPosition` -> no longer take list of node IDs
- `nest.FindCenterElement` -> now returns `int` instead of `tuple`

### Nodes

| NEST 2.x                                                     | NEST 3.0                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `nest.Create(model, n=1, params=None)`<br /><br />returns `list` | `nest.Create(model, n=1, params=None)`<br /><br />returns `nest.NodeCollection` |
| `nest.GetLid(node_id)`<br /><br />returns `list`                 |                                                              |

### Connection

| NEST 2.x                                                     | NEST 3.0                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `nest.GetConnections(list=None, list=None, synapse_model=None, synapse_label=None)`  <br /><br />returns `numpy.array` | `nest.GetConnections(nest.NodeCollection=None, nest.NodeCollection=None,synapse_model=None, synapse_label=None)` <br /><br />returns `nest.SynapseCollection` |
| `nest.Connect(list, list, conn_spec=None, syn_spec=None, model=None)` | `nest.Connect(nest.NodeCollection, nest.NodeCollection, conn_spec=None, syn_spec=None, return_synapsecollection=False)` <br /><br />In `syn_spec` the synapse model is given by `synapse_model`, not `model`. |
| `nest.DataConnect(pre, params=None, model="static_synapse")` |                                                              |
| `nest.CGConnect(list, list, cg, parameter_map=None, model="static_synapse")` | `nest.CGConnect(nest.NodeCollection, nest.NodeCollection, cg, parameter_map=None, model="static_synapse")` |
| `nest.DisconnectOneToOne(int, int, syn_spec)`                |                                                              |
| `nest.Disconnect(list, list, conn_spec='one_to_one', syn_spec='static_synapse')` | `nest.Disconnect(nest.NodeCollection, nest.NodeCollection, conn_spec='one_to_one', syn_spec='static_synapse')` |

### Subnets

| NEST 2.x                                                       | NEST 3.0                                                   |
| -------------------------------------------------------------- | ---------------------------------------------------------- |
| `nest.PrintNetwork(depth=1, subnet=None)`                      | `nest.PrintNodes()`                                        |
| `nest.CurrentSubnet()`                                         |                                                            |
| `nest.ChangeSubnet(subnet)`                                    |                                                            |
| `nest.GetLeaves(subnet, properties=None, local_only=False)`    | `nest.NodeCollection` will contain all nodes               |
| `nest.GetNodes(subnets, properties=None, local_only=False)`    | `nest.NodeCollection` will contain all nodes.              |
| `nest.GetChildren(subnets, properties=None, local_only=False)` | `nest.NodeCollection` will contain all nodes.              |
| `nest.GetNetwork(node_id, depth)`                                  |                                                            |
| `nest.BeginSubnet(label=None, params=None)`                    |                                                            |
| `nest.EndSubnet()`                                             |                                                            |
| `nest.LayoutNetwork(model, dim, label=None, params=None)`      | Use `nest.Create(model, n=1, params=None, positions=None)` |

### Info

| NEST 2.x                                       | NEST 3.0                                                     |
| ---------------------------------------------- | ------------------------------------------------------------ |
| `nest.SetStatus(list/tuple, params, val=None)` | `nest.SetStatus(nest.NodeCollection, params, val=None)`<br /><br />Can also use `nodes.set(params)` or `conns.set(params)`  |
| `nest.GetStatus(list/tuple, keys=None)`        | `nest.GetStatus(nest.NodeCollection, keys=None)`<br /><br />Can also use `nodes.get(keys=None)` or `conns.get(keys=None)` |

### Topology

| NEST 2.x                                                     | NEST 3.0                                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `tp.CreateLayer(specs)`<br /><br />returns `tuple of int(s)`<br /><br /> | `nest.Create(model, params=None, positions=nest.spatial.free/grid)`<br /><br />returns `nest.NodeCollection`<br /><br />NB! Composite layers no longer possible. |
| `tp.ConnectLayers(list, list, projections)`                  | `nest.Connect(nest.NodeCollection, nest.NodeCollection, conn_spec=None, syn_spec=None, return_synapsecollection=False)` |
|                                                              | `layer_NodeCollection.spatial`                                |
| `tp.GetLayer(nodes)`<br /><br />returns `tuple`              |                                                              |
| `tp.GetElement(layers, location)`<br /><br />returns `tuple` |                                                              |
| `tp.GetPosition(tuple)`<br /><br />returns `tuple of tuple(s)` | `nest.GetPosition(nest.NodeCollection)`<br /><br />returns `tuple` or `tuple of tuple(s)` |
| `tp.Displacement(from_arg, to_arg)`<br /><br />`from_arg`: `tuple/list of int(s) / tuple/list of tuples/lists of floats]`<br /><br />`to_arg`: `tuple/list of int(s)`<br /><br />returns `tuple` | `nest.Displacement(from_arg, to_arg)`<br /><br />`from_arg`:  `nest.NodeCollection or tuple/list with tuple(s)/list(s) of floats`<br /><br />`to_arg`: `nest.NodeCollection`<br /><br />returns `tuple` |
| `tp.Distance(from_arg, to_arg)`<br /><br />`from_arg`: `[tuple/list of ints / tuple/list with tuples/lists of floats]`<br /><br />`to:arg`: `tuple/list of ints`<br /><br />returns `tuple` | `nest.Distance(from_arg, to_arg)`<br /><br />`from_arg`: `nest.NodeCollection or tuple/list with tuple(s)/list(s) of floats`<br /><br />`to_arg`: `nest.NodeCollection`<br /><br />returns `tuple` |
| `tp.FindNearestElement(tuple/list, locations, find_all=True)`<br /><br />returns `tuple` | `nest.FindNearestElement(nest.NodeCollection, locations, find_all=True)`<br /><br />returns `tuple` |
| `tp.DumpLayerNodes(tuple, outname)`                          | `nest.DumpLayerNodes(nest.NodeCollection, outname)`           |
| `tp.DumpLayerConnections(tuple, synapse_model, outname)`     | `nest.DumpLayerConnections(nest.NodeCollection, nest.NodeCollection, synapse_model, outname)` |
| `tp.FindCenterElement(tuple)`<br /><br />returns `tuple`     | `nest.FindCenterElement(nest.NodeCollection)`<br /><br />returns `int` |
| `tp.GetTargetNodes(tuple, tuple, tgt_model=None, syn_model=None)`<br /><br />returns `tuple of list(s) of int(s)` | `nest.GetTargetNodes(tuple, nest.NodeCollection, syn_model=None)`<br /><br />returns `tuple of list(s) of int(s)` |
| `tp.GetTargetPositions(tuple, tuple, tgt_model=None, syn_model=None)`<br /><br />returns `tuple of tuple(s) of tuple(s) of floats` | `nest.GetTargetPositions(nest.NodeCollection, nest.NodeCollection, syn_model=None)`<br /><br />returns `list of list(s) of tuple(s) of floats` |
| `tp.SelectNodesByMask(tuple, anchor, mask_obj)`<br /><br />returns `list` | `nest.SelectNodesByMaks(nest.NodeCollection, anchor, mask_obj)`<br /><br />returns `list` |
| `tp.PlotLayer(tuple, fig=None, nodecolor='b', nodesize=20)`<br /><br />returns `matplotlib.figure.Figure` object | `nest.PlotLayer(nest.NodeCollection, fig=None, nodecolor='b', nodesize=20)`<br /><br />returns`matplotlib.figure.Figure` object |
| `tp.PlotTargets(int, tuple, tgt_model=None, syn_type=None, fig=None, mask=None, kernel=None, src_color='red', src_size=50, tgt_color='blue', tgt_size=20, mask_color='red', kernel_color='red')`<br /><br />returns `matplotlib.figure.Figure` object | `nest.PlotTargets(nest.NodeCollection, nest.NodeCollection, syn_type=None, fig=None, mask=None, kernel=None, src_color='red', src_size=50, tgt_color='blue', tgt_size=20, mask_color='red', kernel_color='red')`<br /><br />returns `matplotlib.figure.Figure` object |
| `tp.PlotKernel(ax, int, mask, kern=None, mask_color='red', kernel_color='red')` | `nest.PlotKernel(ax, nest.NodeCollection, mask, kern=None, mask_color='red', kernel_color='red')` |

### Types

See below for more information about what you can do with the different types.

| NEST 2.x | NEST 3.0                                                     |
| -------- | ------------------------------------------------------------ |
|          | `nest.NodeCollection`<br /><br />Returned from `nest.Create(...)`call. |
|          | `nest.SynapseCollection`<br /><br />Returned from `nest.GetConnections(...)` call |

### Models

No Change

### Simulation

No Change

### Parallel Computing

No Change

### Parameters

Parameters can be used to set node and connection parameters.

**random**

| NEST 2.x | NEST 3.0                                                     |
| -------- | ------------------------------------------------------------ |
|          | `nest.random.exponential(beta=1.0)`<br /><br />returns `nest.Parameter` |
|          | `nest.random.lognormal(mean=0.0, std=1.0)`<br /><br />returns `nest.Parameter` |
|          | `nest.random.normal(mean=0.0, std=1.0)`<br /><br />returns `nest.Parameter` |
|          | `nest.random.uniform(min=0.0, max=1.0)`<br /><br />returns `nest.Parameter` |

**spatial**

| NEST 2.x | NEST 3.0                                                     |
| -------- | ------------------------------------------------------------ |
|          | `nest.spatial.distance.x`<br /><br />`nest.spatial.distance.y`<br /><br />`nest.spatial.distance.z`<br /><br />returns `nest.Parameter` |
|          | `nest.spatial.distance`<br /><br />returns `nest.Parameter`  |
|          | `nest.spatial.free(pos, extent=None, edge_wrap=False, num_dimensions=None)`<br /><br />returns `nest.Parameter` |
|          | `nest.spatial.grid(rows, columns, depth=None, center=None, extent=None, edge_wrap=False)`<br /><br />returns `nest.Parameter` |
|          | `nest.spatial.pos.x`<br /><br />`nest.spatial.pos.y`<br /><br />`nest.spatial.pos.z`<br /><br />returns `nest.Parameter` |
|          | `nest.spatial.source_pos.x`<br /><br />`nest.spatial.source_pos.y`<br /><br />`nest.spatial.source_pos.z`<br /><br />returns `nest.Parameter` |
|          | `nest.spatial.target_pos.x`<br /><br />`nest.spatial.target_pos.y`<br /><br />`nest.spatial.target_pos.z`<br /><br />returns `nest.Parameter` |

**math**

| NEST 2.x | NEST 3.0                        |
| -------- | ------------------------------- |
|          | `nest.math.exp(nest.Parameter)` |
|          | `nest.math.sin(nest.Parameter)` |
|          | `nest.math.cos(nest.Parameter)` |

**logic**

| NEST 2.x | NEST 3.0                                                     |
| -------- | ------------------------------------------------------------ |
|          | `nest.logic.conditional(condition, param_if_true, param_if_false)`<br /><br />returns `nest.Parameter` |

**distributions**

| NEST 2.x | NEST 3.0                                                     |
| -------- | ------------------------------------------------------------ |
|          | `nest.spatial_distributions.exponential(nest.Parameter, beta=1.0)` |
|          | `nest.spatial_distributions.gaussian(nest.Parameter, mean=0.0, std=1.0)` |
|          | `nest.spatial_distributions.gaussian2D(nest.Parameter, nest.Parameter, mean_x=0.0, mean_y=0.0, std_x=1.0, std_y=1.0, rho=0.0)` |
|          | `nest.spatial_distributions.gamma(nest.Parameter, kappa=1.0)` |





## Some new concepts:

### NodeCollection:

Supports:

- Iteration
- Slicing
- Indexing
- Conversion to and from lists
- Concatenation of two non-overlapping `NodeCollection`s
- Test whether one `NodeCollection` is equal to another (contains the same node IDs)
- Test of membership
- `len()`
- `get` parameters
- `set` parameters
- get spatial parameters on NodeCollections with spatial metadata with`spatial`

**Example**

```python
nest.ResetKernel()

# Create 80 exitatory neurons
ex_nodes = nest.Create('iaf_psc_alpha', 80)

# Create 20 inibitory neurons
in_nodes = nest.Create('iaf_psc_exp', 20)

# Total nodes
nodes = ex_nodes + in_nodes

# Inspect collection
for node_id, modelid in nodes.items():
    print(node_id, modelid)

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
```



#### get()

**Syntax `get()`:**

`nodes.get()`

`nodes.get(parameter_name)`

`nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n])`

`nodes.get(parameter_name, property_name)`

`nodes.get(parameter_name, [property_name_1, ... , property_name_n])`



You can also specify the output format (pandas, JSON currently implemented):

`nodes.get(output)`

`nodes.get(parameter_name, output)`

`nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n], output)`

`nodes.get(parameter_name, property_name, output)`

`nodes.get(parameter_name, [property_name_1, ... , property_name_n], output)`

***Description:***

`nodes.get()` Returns all parameters in the collection in a dictionary with lists.

`nodes.get(parameter_name)` Returns the parameter given by `parameter_name` as list or int/float.

`nodes.get([parameter_name_1, parameter_name_2, ... , parameter_name_n])` Returns the parameters in the collection given by the parameter names as a dictionary with lists.

`nodes.get(parameter_name, property_name)`  Hierarchical addressing. Returns the parameter of `parameter_name` given by `property_name` as list or int/float.

`nodes.get(parameter_name, [property_name_1, ... , property_name_n])`  Hierarchical addressing. Returns the parameters of `parameter_name` given by property names as a dictionary with list.

When `output` is given, the return value is given in the form of the output specified.



#### set()

**Syntax `set()`:**

`nodes.set(parameter_name, parameter_value)`

`nodes.set(parameter_name, [parameter_val_1, parameter_val_2, ... , parameter_val_n])`

`nodes.set(parameter_dict)`

`nodes.set([parameter_dict_1, parameter_dict_2, ... , parameter_dict_n])`



### SynapseCollection

Supports:

- Iteration
- Test for equality
- `len`
- `get` parameters
- `set` parameters

**Example**

```python
nest.ResetKernel()

# Create nodes and connect
nodes = nest.Create('iaf_psc_alpha', 10)

nest.Connect(nodes, nodes, 'one_to_one')

# Get SynapseCollection and set weight distribution
conns = nest.GetConnections()
conns.set('weight', [1., 2., 3., 4., 5., 6., 7., 8., 9. ,10.])

# Simulate
nest.Simulate(100.)

# Get sources and weights
print(conns.get(['source', 'weight']))
```





### Parametrization

**random**

- `nest.random.exponential()`
- `nest.random.lognormal()`
- `nest.random.normal()`
- `nest.random.uniform()`

**spatial**

- `nest.spatial.distance.x`
- `nest.spatial.distance.y`
- `nest.spatial.distance.x`
- `nest.spatial.distance`
- `nest.grid()`
- `nest.free()`
- `nest.pos.x`, `nest.pos.y`, `nest.pos.z`
- `nest.source_pos.x`, `nest.source_pos.y`, `nest.source_pos.z`
- `nest.target_pos.x`, `nest.target_pos.y`, `nest.target_pos.z`

**math**

- `nest.math.exp()`
- `nest.math.cos()`
- `nest.math.sin()`

**logic**

- `nest.logic.conditional()`

**distributions**

- `nest.spatial_distributions.exponential()`
- `nest.spatial_distributions.gaussian()`
- `nest.spatial_distributions.gaussian2D()`
- `nest.spatial_distributions.gamma()`
