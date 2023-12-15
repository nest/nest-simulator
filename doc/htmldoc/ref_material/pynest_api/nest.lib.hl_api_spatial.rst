Spatial module
==============
 

Functions related to spatially-structured networks.

.. seealso::

   See our in depth guide to :ref:`spatial_networks` for details.


Query functions for spatial layers
----------------------------------

Syntax:
~~~~~~~

.. code-block:: Python

   nest.Distance(nodes[1], nodes[2])

.. automodule:: nest.lib.hl_api_spatial
   :members: CreateMask, Displacement, Distance, DumpLayerConnections, DumpLayerNodes, FindCenterElement, FindNearestElement, GetPosition, GetTargetNodes, GetSourceNodes, GetTargetPositions, GetSourcePositions, SelectNodesByMask
   :undoc-members:
   :show-inheritance:

Visualization functions
-----------------------

Syntax:
~~~~~~~

.. code-block:: Python

    nest.PlotLayer(layer, fig, nodecolor, nodesize)

.. automodule:: nest.lib.hl_api_spatial
   :members: PlotLayer, PlotSources, PlotTargets, PlotProbabilityParameter
   :undoc-members:
   :show-inheritance:




