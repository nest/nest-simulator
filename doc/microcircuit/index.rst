Microcircuit Model
===================

This is a PyNEST implementation of the cortical microcircuit model by Potjans
and Diesmann [1]_.  The network model represents 4 layers of cortex, L2/3, L4, L5,
and L6, each consisting of 2 populations of excitatory and inhibitory neurons.
The original sli version can be found `here <https://github.com/nest/nest-simulator/tree/master/examples/nest/Potjans_2014>`__.


.. toctree::
  :maxdepth: 1

  example

  network_params
  sim_params
  stimulus_params

  api_potjans/modules

References
------------

.. [1]  Potjans TC. and Diesmann M. 2014. The cell-type specific cortical
        microcircuit: relating structure and activity in a full-scale spiking
        network model. Cerebral Cortex. 24(3):785–806. DOI: 10.1093/cercor/bhs358.

Authors: Hendrik Rothe, Hannah Bos, Sacha van Albada
May 2016

Tested configuration: This version has been tested with NEST 2.10.0,
Python 2.7.12 and 3.5.2, NumPy 1.11.2
