.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_sensitivity_to_perturbation.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_sensitivity_to_perturbation.py:


Sensitivity to perturbation
---------------------------

This script simulates a network in two successive trials, which are identical
except for one extra input spike in the second realisation (a small
perturbation). The network consists of recurrent, randomly connected excitatory
and inhibitory neurons. Its activity is driven by an external Poisson input
provided to all neurons independently. In order to ensure that the network is
reset appropriately between the trials, we do the following steps:

- resetting the network
- resetting the random network generator
- resetting the internal clock
- deleting all entries in the spike detector
- introducing a hyperpolarisation phase between the trials
  (in order to avoid that spikes remaining in the NEST memory
  after the first simulation are fed into the second simulation)


Importing all necessary modules for simulation, analysis and plotting.


.. code-block:: default



    import numpy
    import pylab
    import nest



Here we define all parameters necessary for building and simulating the
network.


.. code-block:: default


    # We start with the global network parameters.


    NE = 1000      # number of excitatory neurons
    NI = 250       # number of inhibitory neurons
    N = NE + NI    # total number of neurons
    KE = 100       # excitatory in-degree
    KI = 25        # inhibitory in-degree



Parameters specific for the neurons in the network. The  default values of
the reset potential :term:`E_L` and the spiking threshold :term:`V_th` are used to set
the limits of the initial potential of the neurons.


.. code-block:: default



    neuron_model = 'iaf_psc_delta'
    neuron_params = nest.GetDefaults(neuron_model)
    Vmin = neuron_params['E_L']   # minimum of initial potential distribution (mV)
    Vmax = neuron_params['V_th']  # maximum of initial potential distribution (mV)



Synapse parameters. Changing the weights `J` in the network can lead to
qualitatively different behaviors. If `J` is small (e.g. ``J = 0.1``), we
are likely to observe a non-chaotic network behavior (after perturbation
the network returns to its original activity). Increasing `J`
(e.g ``J = 5.5``) leads to rather chaotic activity. Given that in this
example the transition to chaos is probabilistic, we sometimes observe
chaotic behavior for small weights (e.g. ``J = 0.5``) and non-chaotic
behavior for strong weights (e.g. ``J = 5.4``).


.. code-block:: default



    J = 0.5                   # excitatory synaptic weight (mV)
    g = 6.                    # relative inhibitory weight
    delay = 0.1               # spike transmission delay (ms)


    # External input parameters.


    Jext = 0.2                # PSP amplitude for external Poisson input (mV)
    rate_ext = 6500.          # rate of the external Poisson input


    # Perturbation parameters.


    t_stim = 400.             # perturbation time (time of the extra spike)
    Jstim = Jext              # perturbation amplitude (mV)


    # Simulation parameters.


    T = 1000.                 # simulation time per trial (ms)
    fade_out = 2.*delay       # fade out time (ms)
    dt = 0.01                 # simulation time resolution (ms)
    seed_NEST = 30            # seed of random number generator in Nest
    seed_numpy = 30           # seed of random number generator in numpy



Before we build the network, we reset the simulation kernel to ensure
that previous NEST simulations in the python shell will not disturb this
simulation and set the simulation resolution (later defined
synaptic delays cannot be smaller than the simulation resolution).


.. code-block:: default



    nest.ResetKernel()
    nest.SetStatus([0], [{"resolution": dt}])



Now we start building the network and create excitatory and inhibitory nodes
and connect them. According to the connectivity specification, each neuron
is assigned random KE synapses from the excitatory population and random KI
synapses from the inhibitory population.


.. code-block:: default



    nodes_ex = nest.Create(neuron_model, NE)
    nodes_in = nest.Create(neuron_model, NI)
    allnodes = nodes_ex+nodes_in

    nest.Connect(nodes_ex, allnodes,
                 conn_spec={'rule': 'fixed_indegree', 'indegree': KE},
                 syn_spec={'weight': J, 'delay': dt})
    nest.Connect(nodes_in, allnodes,
                 conn_spec={'rule': 'fixed_indegree', 'indegree': KI},
                 syn_spec={'weight': -g*J, 'delay': dt})


Afterwards we create a :cpp:class:`poisson_generator <nest::poisson_generator>` that provides spikes (the external
input) to the neurons until time `T` is reached.
Afterwards a :cpp:class:`dc_generator <nest::dc_generator>`, which is also connected to the whole population,
provides a stong hyperpolarisation step for a short time period `fade_out`.

The `fade_out` period has to last at least twice as long as the simulation
resolution to supress the neurons from firing.


.. code-block:: default



    ext = nest.Create("poisson_generator",
                      params={'rate': rate_ext, 'stop': T})
    nest.Connect(ext, allnodes,
                 syn_spec={'weight': Jext, 'delay': dt})

    suppr = nest.Create("dc_generator",
                        params={'amplitude': -1e16, 'start': T,
                                'stop': T+fade_out})
    nest.Connect(suppr, allnodes)

    spikedetector = nest.Create("spike_detector")
    nest.Connect(allnodes, spikedetector)



We then create the :cpp:class:`spike_generator <nest::spike_generator>`, which provides the extra spike
(perturbation).


.. code-block:: default


    stimulus = nest.Create("spike_generator")
    nest.SetStatus(stimulus, {'spike_times': []})



Finally, we run the two simulations successively. After each simulation the
sender ids and spiketimes are stored in a list (`senders`, `spiketimes`).


.. code-block:: default



    senders = []
    spiketimes = []



We need to reset the network, the random number generator, and the clock of
the simulation kernel. In addition, we ensure that there is no spike left in
the spike detector.

In the second trial, we add an extra input spike at time `t_stim` to the
neuron that fires first after perturbation time `t_stim`. Thus, we make sure
that the perturbation is transmitted to the network before it fades away in
the perturbed neuron. (Single IAF-neurons are not chaotic.)


.. code-block:: default



    for trial in [0, 1]:
        nest.ResetNetwork()
        nest.SetStatus([0], [{"rng_seeds": [seed_NEST]}])
        nest.SetStatus([0], {'time': 0.0})
        nest.SetStatus(spikedetector, {'n_events': 0})

        # We assign random initial membrane potentials to all neurons

        numpy.random.seed(seed_numpy)
        Vms = Vmin + (Vmax - Vmin) * numpy.random.rand(N)
        nest.SetStatus(allnodes, "V_m", Vms)

        if trial == 1:
            id_stim = [senders[0][spiketimes[0] > t_stim][0]]
            nest.Connect(stimulus, list(id_stim),
                         syn_spec={'weight': Jstim, 'delay': dt})
            nest.SetStatus(stimulus, {'spike_times': [t_stim]})

        # Now we simulate the network and add a fade out period to discard
        # remaining spikes.

        nest.Simulate(T)
        nest.Simulate(fade_out)

        # Storing the data.

        senders += [nest.GetStatus(spikedetector, 'events')[0]['senders']]
        spiketimes += [nest.GetStatus(spikedetector, 'events')[0]['times']]


We plot the spiking activity of the network (first trial in red, second trial
in black).


.. code-block:: default


    pylab.figure(1)
    pylab.clf()
    pylab.plot(spiketimes[0], senders[0], 'ro', ms=4.)
    pylab.plot(spiketimes[1], senders[1], 'ko', ms=2.)
    pylab.xlabel('time (ms)')
    pylab.ylabel('neuron id')
    pylab.xlim((0, T))
    pylab.ylim((0, N))


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_sensitivity_to_perturbation.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: sensitivity_to_perturbation.py <sensitivity_to_perturbation.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: sensitivity_to_perturbation.ipynb <sensitivity_to_perturbation.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
