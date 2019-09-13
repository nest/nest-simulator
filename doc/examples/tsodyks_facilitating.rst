.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_tsodyks_facilitating.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_tsodyks_facilitating.py:

Tsodyks facilitating example
--------------------------------

This scripts simulates two neurons. One is driven with dc-input and
connected to the other one with a facilitating tsodyks synapse. The
membrane potential trace of the second neuron is recorded.

This example reproduces figure 1B of [1]_
This example is analog to ``tsodyks_depressing.py``, except that
different synapse parameters are used. Here, a small facilitation
parameter ``U`` causes a slow saturation of the synaptic efficacy
(Eq. 2.2), enabling a facilitating behavior.

References
~~~~~~~~~~~~

.. [1] Tsodyks M, Pawelzik K, Markram H (1998). Neural networks with dynamic synapses. Neural
       computation, http://dx.doi.org/10.1162/089976698300017502

See Also
~~~~~~~~~~

:doc:`tsodyks_depressing`

First, we import all necessary modules for simulation and plotting.


.. code-block:: default


    import nest
    import nest.voltage_trace
    import pylab
    from numpy import exp


Second, the simulation parameters are assigned to variables. The neuron
and synapse parameters are stored into a dictionary.


.. code-block:: default


    h = 0.1    # simulation step size (ms)
    Tau = 40.    # membrane time constant
    Theta = 15.    # threshold
    E_L = 0.     # reset potential of membrane potential
    R = 1.     # membrane resistance (GOhm)
    C = Tau / R  # Tau (ms)/R in NEST units
    TauR = 2.     # refractory time
    Tau_psc = 1.5    # time constant of PSC (= Tau_inact)
    Tau_rec = 130.   # recovery time
    Tau_fac = 530.   # facilitation time
    U = 0.03   # facilitation parameter U
    A = 1540.  # PSC weight in pA
    f = 20. / 1000.  # frequency in Hz converted to 1/ms
    Tend = 1200.  # simulation time
    TIstart = 50.    # start time of dc
    TIend = 1050.  # end time of dc
    I0 = Theta * C / Tau / (1 - exp(-(1 / f - TauR) / Tau))  # dc amplitude

    neuron_param = {"tau_m": Tau,
                    "t_ref": TauR,
                    "tau_syn_ex": Tau_psc,
                    "tau_syn_in": Tau_psc,
                    "C_m": C,
                    "V_reset": E_L,
                    "E_L": E_L,
                    "V_m": E_L,
                    "V_th": Theta}

    syn_param = {"tau_psc": Tau_psc,
                 "tau_rec": Tau_rec,
                 "tau_fac": Tau_fac,
                 "U": U,
                 "delay": 0.1,
                 "weight": A,
                 "u": 0.0,
                 "x": 1.0}


Third, we reset the kernel and set the resolution using ``SetKernelStatus``.


.. code-block:: default


    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": h})


Fourth, the nodes are created using ``Create``. We store the returned
handles in variables for later reference.


.. code-block:: default


    neurons = nest.Create("iaf_psc_exp", 2)
    dc_gen = nest.Create("dc_generator")
    volts = nest.Create("voltmeter")


Fifth, the ``iaf_psc_exp`` neurons, the ``dc_generator`` and the ``voltmeter``
are configured using ``SetStatus``, which expects a list of node handles and
a parameter dictionary or a list of parameter dictionaries.


.. code-block:: default


    nest.SetStatus(neurons, neuron_param)
    nest.SetStatus(dc_gen, {"amplitude": I0, "start": TIstart, "stop": TIend})
    nest.SetStatus(volts, {"label": "voltmeter", "interval": 1.})


Sixth, the ``dc_generator`` is connected to the first neuron
(`neurons[0]`) and the `voltmeter` is connected to the second neuron
(`neurons[1]`). The command `Connect` has different variants. Plain
``Connect`` just takes the handles of pre- and post-synaptic nodes and
uses the default values for weight and delay. Note that the connection
direction for the ``voltmeter`` reflects the signal flow in the simulation
kernel, because it observes the neuron instead of receiving events from it.


.. code-block:: default


    nest.Connect(dc_gen, neurons[0])
    nest.Connect(volts, neurons[1])


Seventh, the first neuron (`neurons[0]`) is connected to the second
neuron (`neurons[1]`).  The command ``CopyModel`` copies the
``tsodyks_synapse`` model to the new name ``syn`` with parameters
``syn_param``.  The manually defined model ``syn`` is used in the
connection routine via the ``syn_spec`` parameter.


.. code-block:: default


    nest.CopyModel("tsodyks_synapse", "syn", syn_param)
    nest.Connect(neurons[0], neurons[1], syn_spec="syn")


Finally, we simulate the configuration using the command ``Simulate``,
where the simulation time `Tend` is passed as the argument.  We plot the
target neuron's membrane potential as function of time.


.. code-block:: default


    nest.Simulate(Tend)
    nest.voltage_trace.from_device(volts)


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_tsodyks_facilitating.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: tsodyks_facilitating.py <tsodyks_facilitating.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: tsodyks_facilitating.ipynb <tsodyks_facilitating.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
