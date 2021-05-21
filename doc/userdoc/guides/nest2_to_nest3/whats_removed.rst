What's removed from NEST 3.0?
=============================

Subnets
~~~~~~~

Subnets are gone. Instead NodeCollections should be used to group and organize neurons.

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
  | spike_detector                               | spike_recorder                                |
  +----------------------------------------------+-----------------------------------------------+

Furthermore, the model `iaf_tum_2000` has been renamed to `iaf_psc_exp_htum`. iaf_psc_exp_htum is
the exact same model as iaf_tum_2000, it has just been renamed to match NEST's naming conventions.

Functions
~~~~~~~~~

Some functions have also been removed. The removed functions where either related to subnets,
or they can be replaced by using other functions with indexing into a NodeCollection.

The following functions have been removed:

+----------------------+------------------------------+
| - BeginSubnet        |                              |
| - ChangeSubnet       |                              |
| - CurrentSubnet      |                              |
| - EndSubnet          |                              |
| - GetChildren        |                              |
| - GetLayer           |   See :ref:`subnet_ref`      |
| - GetLeaves          |                              |
| - GetLID             |                              |
| - GetNetwork         |                              |
| - LayoutNetwork      |                              |
+----------------------+------------------------------+
| - ResetNetwork       |  See :ref:`sim_ref`          |
+----------------------+------------------------------+
| - DataConnect        |                              |
| - DisconnectOneToOne |  See :ref:`conn_ref`         |
| - CGConnect          |                              |
+----------------------+------------------------------+
| - GetElement         |   See :ref:`topo_ref`        |
+----------------------+------------------------------+
| - RestoreNodes       | (have never existed on PyNEST|
|                      | level, it was only a SLI     |
|                      | function)                    |
+----------------------+------------------------------+

