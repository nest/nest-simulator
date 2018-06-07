Simulations with gap junctions
==============================

**Note:** This documentation describes the usage of gap junctions in
NEST 2.12. A documentation for NEST 2.10 can be found in `Hahne et al.
2016 <http://link.springer.com/chapter/10.1007/978-3-319-50862-7_4>`__.
It is however recommended to use NEST 2.12 (or later), due to several
improvements in terms of usability.

Introduction
------------

Simulations with gap junctions are supported by the Hodgkin-Huxley
neuron model ``hh_psc_alpha_gap``. The synapse model to create a
gap-junction connection is named ``gap_junction``. Unlike chemical
synapses gap junctions are bidirectional connections. In order to create
**one** accurate gap-junction connection **two** NEST connections are
required: For each created connection a second connection with the exact
same parameters in the opposite direction is required. NEST provides the
possibility to create both connections with a single call to
``nest.Connect`` via the ``make_symmetric`` flag (default value:
``False``) of the connection dictionary:

.. code:: python

    import nest

    a = nest.Create('hh_psc_alpha_gap')
    b = nest.Create('hh_psc_alpha_gap')
    # Create gap junction between neurons a and b
    nest.Connect(a, b, {'rule': 'one_to_one', 'make_symmetric': True}, 
                       {'model': 'gap_junction', 'weight': 0.5})

In this case the reverse connection is created internally. In order to
prevent the creation of incomplete or non-symmetrical gap junctions the
creation of gap junctions is restricted to

-  ``one_to_one`` connections with ``'make_symmetric': True``
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

    n = nest.Create('hh_psc_alpha_gap', n_neuron)

    random.seed(0)

    # draw n_gap_junction pairs of random samples from the list of all
    # neurons and reshaped data into two corresponding lists of neurons
    m = np.transpose(
        [random.sample(n, 2) for _ in range(n_gap_junction)])

    # connect obtained lists of neurons both ways
    nest.Connect(m[0], m[1],
                 {'rule': 'one_to_one', 'make_symmetric': True},
                 {'model': 'gap_junction', 'weight': 0.5})

As each gap junction contributes to the total number of gap-junction
connections of two neurons, it is hardly possible to create networks
with a fixed number of gap junctions per neuron. With the above script
it is however possible to control the approximate number of gap
junctions per neuron. E.g. if one desires ``gap_per_neuron = 60`` the
total number of gap junctions should be chosen as
``n_gap_junction = n_neuron * gap_per_neuron / 2``.

**Note:** The (necessary) drawback of creating the random connections on
the Python level is the serialization of the connection procedure in
terms of computation time and memory in distributed simulations. Each
compute node participating in the simulation needs to draw the identical
full set of random numbers and temporarily represent the total
connectivity in variable ``m``. Therefore it is advisable to use the
internal random connection rules of NEST for the creation of connections
whenever possible. For more details see `Hahne et al.
2016 <http://link.springer.com/chapter/10.1007/978-3-319-50862-7_4>`__.

Adjust settings of iterative solution scheme
--------------------------------------------

For simulations with gap junctions NEST uses an iterative solution
scheme based on a numerical method called Jacobi waveform relaxation.
The default settings of the iterative method are based on numerical
results, benchmarks and previous experience with gap-junction
simulations (see `Hahne et al.
2015 <http://journal.frontiersin.org/article/10.3389/fninf.2015.00022/full>`__)
and should only be changed with proper knowledge of the method. In
general the following parameters can be set via kernel parameters:

.. code:: python

    nest.SetKernelStatus({'use_wfr': True,
                          'wfr_comm_interval': 1.0,
                          'wfr_tol': 0.0001,
                          'wfr_max_iterations': 15,
                          'wfr_interpolation_order': 3})

For a detailed description of the parameters and their function see
(`Hahne et al. 2016 <https://arxiv.org/abs/1610.09990>`__, Table 2).
