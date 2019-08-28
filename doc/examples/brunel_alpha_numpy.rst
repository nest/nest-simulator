.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_brunel_alpha_numpy.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_brunel_alpha_numpy.py:

Random balanced network (alpha synapses) connected with NumPy
-------------------------------------------------------------------

This script simulates an excitatory and an inhibitory population on
the basis of the network used in [1]

In contrast to brunel-alpha-nest.py, this variant uses NumPy to draw
the random connections instead of NEST's builtin connection routines.

When connecting the network customary synapse models are used, which
allow for querying the number of created synapses. Using spike
detectors the average firing rates of the neurons in the populations
are established. The building as well as the simulation time of the
network are recorded.

References
~~~~~~~~~~~~~~

.. [1] Brunel N, Dynamics of Sparsely Connected Networks of Excitatory and
       Inhibitory Spiking Neurons, Journal of Computational Neuroscience 8,
       183-208 (2000).

See Also
~~~~~~~~~~

brunel-alpha-nest.py

:Authors:



Import all necessary modules for simulation, analysis and plotting. Scipy
should be imported before nest.


.. code-block:: default


    from scipy.optimize import fsolve

    import nest
    import nest.raster_plot

    import numpy
    from numpy import exp

    import time


Definition of functions used in this example. First, define the Lambert W
function implemented in SLI. The second function computes the maximum of
the postsynaptic potential for a synaptic input current of unit amplitude (
1 pA) using the Lambert W function. Thus function will later be used to
calibrate the synaptic weights


.. code-block:: default



    def LambertWm1(x):
        nest.ll_api.sli_push(x)
        nest.ll_api.sli_run('LambertWm1')
        y = nest.ll_api.sli_pop()
        return y


    def ComputePSPnorm(tauMem, CMem, tauSyn):
        a = (tauMem / tauSyn)
        b = (1.0 / tauSyn - 1.0 / tauMem)

        # time of maximum
        t_max = 1.0 / b * (-LambertWm1(-exp(-1.0 / a) / a) - 1.0 / a)

        # maximum of PSP for current of unit amplitude
        return (exp(1.0) / (tauSyn * CMem * b) *
                ((exp(-t_max / tauMem) - exp(-t_max / tauSyn)) / b -
                 t_max * exp(-t_max / tauSyn)))

    nest.ResetKernel()


Assigning the current time to a variable in order to determine the build
time of the network.


.. code-block:: default


    startbuild = time.time()


Assigning the simulation parameters to variables.


.. code-block:: default


    dt = 0.1    # the resolution in ms
    simtime = 1000.0  # Simulation time in ms
    delay = 1.5    # synaptic delay in ms


Definition of the parameters crucial for asynchronous irregular firing of
the neurons.


.. code-block:: default


    g = 5.0  # ratio inhibitory weight/excitatory weight
    eta = 2.0  # external rate relative to threshold rate
    epsilon = 0.1  # connection probability


Definition of the number of neurons in the network and the number of neuron
 recorded from


.. code-block:: default


    order = 2500
    NE = 4 * order  # number of excitatory neurons
    NI = 1 * order  # number of inhibitory neurons
    N_neurons = NE + NI   # number of neurons in total
    N_rec = 50      # record from 50 neurons


Definition of connectivity parameter


.. code-block:: default


    CE = int(epsilon * NE)  # number of excitatory synapses per neuron
    CI = int(epsilon * NI)  # number of inhibitory synapses per neuron
    C_tot = int(CI + CE)      # total number of synapses per neuron


Initialization of the parameters of the integrate and fire neuron and the
synapses. The parameter of the neuron are stored in a dictionary. The
synaptic currents are normalized such that the amplitude of the PSP is J.


.. code-block:: default


    tauSyn = 0.5  # synaptic time constant in ms
    tauMem = 20.0  # time constant of membrane potential in ms
    CMem = 250.0  # capacitance of membrane in in pF
    theta = 20.0  # membrane threshold potential in mV
    neuron_params = {"C_m": CMem,
                     "tau_m": tauMem,
                     "tau_syn_ex": tauSyn,
                     "tau_syn_in": tauSyn,
                     "t_ref": 2.0,
                     "E_L": 0.0,
                     "V_reset": 0.0,
                     "V_m": 0.0,
                     "V_th": theta}
    J = 0.1        # postsynaptic amplitude in mV
    J_unit = ComputePSPnorm(tauMem, CMem, tauSyn)
    J_ex = J / J_unit  # amplitude of excitatory postsynaptic current
    J_in = -g * J_ex    # amplitude of inhibitory postsynaptic current


Definition of threshold rate, which is the external rate needed to fix the
membrane potential around its threshold, the external firing rate and the
rate of the poisson generator which is multiplied by the in-degree CE and
converted to Hz by multiplication by 1000.


.. code-block:: default


    nu_th = (theta * CMem) / (J_ex * CE * numpy.exp(1) * tauMem * tauSyn)
    nu_ex = eta * nu_th
    p_rate = 1000.0 * nu_ex * CE


Configuration of the simulation kernel by the previously defined time
resolution used in the simulation. Setting "print_time" to True prints the
already processed simulation time as well as its percentage of the total
simulation time.


.. code-block:: default


    nest.SetKernelStatus({"resolution": dt, "print_time": True,
                          "overwrite_files": True})

    print("Building network")


Configuration of the model `iaf_psc_alpha` and `poisson_generator` using
SetDefaults(). This function expects the model to be the inserted as a
string and the parameter to be specified in a dictionary. All instances of
theses models created after this point will have the properties specified
in the dictionary by default.


.. code-block:: default


    nest.SetDefaults("iaf_psc_alpha", neuron_params)
    nest.SetDefaults("poisson_generator", {"rate": p_rate})


Creation of the nodes using `Create`. We store the returned handles in
variables for later reference. Here the excitatory and inhibitory, as well
as the poisson generator and two spike detectors. The spike detectors will
later be used to record excitatory and inhibitory spikes.


.. code-block:: default


    nodes_ex = nest.Create("iaf_psc_alpha", NE)
    nodes_in = nest.Create("iaf_psc_alpha", NI)
    noise = nest.Create("poisson_generator")
    espikes = nest.Create("spike_detector")
    ispikes = nest.Create("spike_detector")


Configuration of the spike detectors recording excitatory and inhibitory
spikes using `SetStatus`, which expects a list of node handles and a list
of parameter dictionaries. Setting the variable "to_file" to True ensures
that the spikes will be recorded in a .gdf file starting with the string
assigned to label. Setting "withtime" and "withgid" to True ensures that
each spike is saved to file by stating the gid of the spiking neuron and
the spike time in one line.


.. code-block:: default


    nest.SetStatus(espikes, [{"label": "brunel-py-ex",
                              "withtime": True,
                              "withgid": True,
                              "to_file": True}])

    nest.SetStatus(ispikes, [{"label": "brunel-py-in",
                              "withtime": True,
                              "withgid": True,
                              "to_file": True}])

    print("Connecting devices")


Definition of a synapse using `CopyModel`, which expects the model name of
a pre-defined synapse, the name of the customary synapse and an optional
parameter dictionary. The parameters defined in the dictionary will be the
default parameter for the customary synapse. Here we define one synapse for
 the excitatory and one for the inhibitory connections giving the
previously defined weights and equal delays.


.. code-block:: default


    nest.CopyModel("static_synapse", "excitatory",
                   {"weight": J_ex, "delay": delay})
    nest.CopyModel("static_synapse", "inhibitory",
                   {"weight": J_in, "delay": delay})


Connecting the previously defined poisson generator to the excitatory and
inhibitory neurons using the excitatory synapse. Since the poisson
generator is connected to all neurons in the population the default rule (
'all_to_all') of Connect() is used. The synaptic properties are inserted
via syn_spec which expects a dictionary when defining multiple variables or
 a string when simply using a pre-defined synapse.


.. code-block:: default


    nest.Connect(noise, nodes_ex, syn_spec="excitatory")
    nest.Connect(noise, nodes_in, syn_spec="excitatory")


Connecting the first N_rec nodes of the excitatory and inhibitory
population to the associated spike detectors using excitatory synapses.
Here the same shortcut for the specification of the synapse as defined
above is used.


.. code-block:: default


    nest.Connect(nodes_ex[:N_rec], espikes, syn_spec="excitatory")
    nest.Connect(nodes_in[:N_rec], ispikes, syn_spec="excitatory")

    print("Connecting network")


Here, we create the connections from the excitatory neurons to all other
neurons. We exploit that the neurons have consecutive IDs, running from 1,
...,NE for the excitatory neurons and from (NE+1),...,(NE+NI) for the
inhibitory neurons.


.. code-block:: default


    numpy.random.seed(1234)

    sources_ex = numpy.random.randint(1, NE + 1, (N_neurons, CE))
    sources_in = numpy.random.randint(NE + 1, N_neurons + 1, (N_neurons, CI))


We now iterate over all neuron IDs, and connect the neuron to the sources
from our array. The first loop connects the excitatory neurons and the
second loop the inhibitory neurons.


.. code-block:: default


    for n in range(N_neurons):
        nest.Connect(list(sources_ex[n]), [n + 1], syn_spec="excitatory")

    for n in range(N_neurons):
        nest.Connect(list(sources_in[n]), [n + 1], syn_spec="inhibitory")


Storage of the time point after the buildup of the network in a variable.


.. code-block:: default


    endbuild = time.time()


Simulation of the network.


.. code-block:: default


    print("Simulating")

    nest.Simulate(simtime)


Storage of the time point after the simulation of the network in a variable.


.. code-block:: default


    endsimulate = time.time()


Reading out the total number of spikes received from the spike detector
connected to the excitatory population and the inhibitory population.


.. code-block:: default


    events_ex = nest.GetStatus(espikes, "n_events")[0]
    events_in = nest.GetStatus(ispikes, "n_events")[0]


Calculation of the average firing rate of the excitatory and the inhibitory
neurons by dividing the total number of recorded spikes by the number of
neurons recorded from and the simulation time. The multiplication by 1000.0
converts the unit 1/ms to 1/s=Hz.


.. code-block:: default


    rate_ex = events_ex / simtime * 1000.0 / N_rec
    rate_in = events_in / simtime * 1000.0 / N_rec


Reading out the number of connections established using the excitatory and
inhibitory synapse model. The numbers are summed up resulting in the total
number of synapses.


.. code-block:: default


    num_synapses = (nest.GetDefaults("excitatory")["num_connections"] +
                    nest.GetDefaults("inhibitory")["num_connections"])


Establishing the time it took to build and simulate the network by taking
the difference of the pre-defined time variables.


.. code-block:: default


    build_time = endbuild - startbuild
    sim_time = endsimulate - endbuild


Printing the network properties, firing rates and building times.


.. code-block:: default


    print("Brunel network simulation (Python)")
    print("Number of neurons : {0}".format(N_neurons))
    print("Number of synapses: {0}".format(num_synapses))
    print("       Exitatory  : {0}".format(int(CE * N_neurons) + N_neurons))
    print("       Inhibitory : {0}".format(int(CI * N_neurons)))
    print("Excitatory rate   : %.2f Hz" % rate_ex)
    print("Inhibitory rate   : %.2f Hz" % rate_in)
    print("Building time     : %.2f s" % build_time)
    print("Simulation time   : %.2f s" % sim_time)


Plot a raster of the excitatory neurons and a histogram.


.. code-block:: default


    nest.raster_plot.from_device(espikes, hist=True)


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_brunel_alpha_numpy.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: brunel_alpha_numpy.py <brunel_alpha_numpy.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: brunel_alpha_numpy.ipynb <brunel_alpha_numpy.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
