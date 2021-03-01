Cortical microcircuit model
===========================

This is a PyNEST implementation of the cortical microcircuit model by Potjans and Diesmann [1]_.

Here you can inspect all files belonging to this example:

* :doc:`README <cortical_microcircuit>`: documentation of this microcircuit model implementation and its usage
* :doc:`run_microcircuit.py <../auto_examples/Potjans_2014/run_microcircuit>`: an example script to try out the microcircuit
* :doc:`network.py <../auto_examples/Potjans_2014/network>`: the main Network class with functions to build and simulate the network
* :doc:`helpers.py <../auto_examples/Potjans_2014/helpers>`: helper functions for network construction, simulation and evaluation
* :doc:`network_params.py <../auto_examples/Potjans_2014/network_params>`: network and neuron parameters
* :doc:`stimulus_params.py <../auto_examples/Potjans_2014/stimulus_params>`: parameters for optional external stimulation
* :doc:`sim_params.py <../auto_examples/Potjans_2014/sim_params>`: simulation parameters
* `reference_data <https://github.com/nest/nest-simulator/tree/master/pynest/examples/Potjans_2014/reference_data>`_: reference data and figures obtained by executing run_microcircuit.py with default parameters

References
----------

.. [1]  Potjans TC. and Diesmann M. 2014. The cell-type specific cortical
        microcircuit: relating structure and activity in a full-scale spiking
        network model. Cerebral Cortex. 24(3):785–806. DOI: `10.1093/cercor/bhs358 <https://doi.org/10.1093/cercor/bhs358>`__.
