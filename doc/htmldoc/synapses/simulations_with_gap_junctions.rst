.. _sim_gap_junctions:

Simulations with gap junctions
==============================


Simulations with gap junctions are supported by the Hodgkin-Huxley
neuron model ``hh_psc_alpha_gap``. The synapse model to create a
gap-junction connection is named ``gap_junction``. Unlike chemical
synapses gap junctions are bidirectional connections. In order to create
**one** accurate gap-junction connection **two** NEST connections are
required: For each created connection a second connection with the exact
same parameters in the opposite direction is required. NEST provides the
possibility to create both connections with a single call to
:py:func:`.Connect` via the ``make_symmetric`` flag (default value:
``False``) of the connection dictionary:

.. code:: python

    import nest

    a = nest.Create("hh_psc_alpha_gap")
    b = nest.Create("hh_psc_alpha_gap")
    gap_weight = 0.5
    syn_dict = {"synapse_model": "gap_junction", "weight": gap_weight}
    conn_dict = {"rule": "one_to_one", "make_symmetric": True}
    # Create gap junction between neurons a and b
    nest.Connect(a, b, conn_dict, syn_dict)

In this case the reverse connection is created internally. In order to
prevent the creation of incomplete or non-symmetrical gap junctions the
creation of gap junctions is restricted to

-  ``one_to_one`` connections with ``"make_symmetric": True``
-  ``all_to_all`` connections with equal source and target populations
   and default or scalar parameters

Create random connections
-------------------------

NEST random connection rules like ``fixed_total_number``,
``fixed_indegree`` etc. cannot be employed for the creation of gap
junctions. Therefore random connections have to be created on the Python
level with e.g. the ``random`` module of the Python Standard Library:

.. code:: python

    import nest
    import random
    import numpy as np

    # total number of neurons
    n_neuron = 100

    # total number of gap junctions
    n_gap_junction = 3000

    gap_weight = 0.5
    n = nest.Create("hh_psc_alpha_gap", n_neuron)
    n_list = n.tolist()

    random.seed(0)

    # draw n_gap_junction pairs of random samples
    connections = np.random.choice(n_list, [n_gap_junction, 2])

    for source_node_id, target_node_id in connections:
        nest.Connect(
            nest.NodeCollection([source_node_id]),
            nest.NodeCollection([target_node_id]),
            {"rule": "one_to_one", "make_symmetric": True},
            {"synapse_model": "gap_junction", "weight": gap_weight},
            )


As each gap junction contributes to the total number of gap-junction
connections of two neurons, it is hardly possible to create networks
with a fixed number of gap junctions per neuron. With the above script
it is however possible to control the approximate number of gap
junctions per neuron. For example, if one desires ``gap_per_neuron = 60`` the
total number of gap junctions should be chosen as
``n_gap_junction = n_neuron * gap_per_neuron / 2``.

.. note::

  The (necessary) drawback of creating the random connections on
  the Python level is the serialization of the connection procedure in
  terms of computation time and memory in distributed simulations. Each
  compute node participating in the simulation needs to draw the identical
  full set of random numbers and temporarily represent the total
  connectivity in variable ``m``. Therefore it is advisable to use the
  internal random connection rules of NEST for the creation of connections
  whenever possible. For more details see Hahne et al. [1]_

Adjust settings of iterative solution scheme
--------------------------------------------

For simulations with gap junctions, NEST uses an iterative solution
scheme based on a numerical method called Jacobi waveform relaxation.
The default settings of the iterative method are based on numerical
results, benchmarks, and previous experience with gap-junction
simulations [2]_.
and should only be changed with proper knowledge of the method. In
general the following parameters can be set via kernel parameters:

.. code:: python

    nest.use_wfr = True
    nest.wfr_comm_interval = 1.0
    nest.wfr_tol = 0.0001
    nest.wfr_max_iterations = 15
    nest.wfr_interpolation_order = 3

For a detailed description of the parameters and their function see
[3]_, Table 2.

.. seealso::

   * :doc:`/auto_examples/gap_junctions_inhibitory_network`
   * :doc:`/auto_examples/gap_junctions_two_neurons`

References
----------

.. [1] Hahne J, et al. 2016. Including Gap Junctions into Distributed Neuronal Network Simulations.
       In: Amunts K, Grandinetti L, Lippert T, Petkov N. (eds) Brain-Inspired Computing.
       BrainComp 2015. Lecture  Notes in Computer Science(), vol 10087. Springer, Cham.
       https://doi.org/10.1007/978-3-319-50862-7_4

.. [2] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M 2015.
       A unified framework for spiking and gap-junction interactions in distributed neuronal network simulations.
       Frontiers in Neuroinformatics. 9
       https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2015.00022

.. [3] Hahne J, Dahmen D , Schuecker J, Frommer A, Bolten M, Helias M, Diesmann M. 2017.
       Integration of Continuous-Time Dynamics in a Spiking Neural Network Simulator.
       Frontiers in Neuroinformatics. 11.
       https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2017.00034
