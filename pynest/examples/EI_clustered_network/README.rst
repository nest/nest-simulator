EI-clustered circuit model
==========================

This is PyNEST implementation of the EI-clustered circuit model described by Rostami et al. :footcite:p:`Rostami2020`.

.. figure:: EI_clustered_network_schematic.png
   :alt: EI-clustered circuit model.

   Schematic of the EI-clustered circuit model. The network consists of `n_clusters` with one excitatory and one inhibitory population each.

Citing this code
----------------

If you use this code, we ask you to cite the paper by Rostami et al. :footcite:p:`Rostami2020` and the NEST release on Zenodo.

File structure
--------------

* :doc:`run_simulation.py <run_simulation>`: an example script to try out the EI-clustered circuit model
* :doc:`network.py <network>`: the main ``Network`` class with functions to build and simulate the network
* :doc:`helper.py <helper>`: helper functions for calculation of synaptic weights and currents and plot function for raster plots
* :doc:`network_params.py <network_params>`: network and neuron parameters
* :doc:`stimulus_params.py <stimulus_params>`: parameters for optional external stimulation
* :doc:`sim_params.py <sim_params>`: simulation parameters

Running the simulation
----------------------

.. code-block:: bash

   python run_simulation_EI.py

A raster plot of the network activity is saved as ``clustered_ei_raster.png``.

The code can be parallelized by using multiple threads during the NEST simulation.
This can be done by setting the parameter ``n_vp`` in the ``run_simulation_EI.py`` script.

Contributions to this PyNEST model implementation
-------------------------------------------------

2023: initial version of code and documentation by Felix J. Schmitt, Vahid Rostami and Martin Nawrot.

Acknowledgments
---------------

Funding for the study by Rostami et al. :footcite:p:`Rostami2020`: This work was supported by the German Research Foundation (DFG),
in parts through the Collaborative Research Center ’Motor Control in Health and Disease’
(DFG-SFB 1451, Project-ID 431549029) and under the Institutional Strategy of the University of Cologne within the
German Excellence Initiative (DFG-ZUK 81/1) and in parts through the DFG graduate school
’Neural Circuit Analysis’ (DFG-RTG 1960, ID 365082554) and through the European Union’s Horizon 2020 Framework
Programme for Research and Innovation under grant agreement number 945539 (Human Brain Project SGA3).
The figure is created with BioRender.com.

Other implementations of the EI-clustered model
-----------------------------------------------

A `GeNN version <https://github.com/nawrotlab/SNN_GeNN_Nest>`__  by Felix J. Schmitt, Vahid Rostami and Martin Nawrot :footcite:p:`Schmitt2023`.

References
----------

.. footbibliography::
