Spatial modules
===============
 

Functions related to spatially-structured networks and spatial distributions.

.. seealso::

   See our in depth guide to :ref:`spatial_networks` for details.


Query functions for spatial layers
----------------------------------

Syntax:

.. code-block:: Python

   nest.Distance(nodes[1], nodes[2])

.. automodule:: nest.lib.hl_api_spatial
   :members: CreateMask, Displacement, Distance, DumpLayerConnections, DumpLayerNodes, FindCenterElement, FindNearestElement, GetPosition, GetTargetNodes, GetSourceNodes, GetTargetPositions, GetSourcePositions, SelectNodesByMask
   :undoc-members:
   :show-inheritance:


Spatial positions functions
---------------------------

The spatial module contains parameters related to spatial positions of the nodes.

Syntax:

.. code-block:: Python

  nest.spatial.grid(shape, center,
    extent, edge_wrap)

.. automodule:: nest.spatial.hl_api_spatial
   :members:

.. automodule:: nest.spatial
   :members: distance

Spatial distribution functions
------------------------------

Functions to help create distributions based on the position of the nodes.

Syntax:

.. code-block:: Python

   nest.spatial_distributions.gamma(x, kappa, theta)

.. automodule:: nest.spatial_distributions.hl_api_spatial_distributions
   :members:
   :undoc-members:
   :show-inheritance:

Visualization functions
-----------------------

Syntax:

.. code-block:: Python

    nest.PlotLayer(layer, fig, nodecolor, nodesize)

.. automodule:: nest.lib.hl_api_spatial
   :members: PlotLayer, PlotSources, PlotTargets, PlotProbabilityParameter
   :undoc-members:
   :show-inheritance:




