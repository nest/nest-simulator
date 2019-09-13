.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_precise_spiking.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_precise_spiking.py:


Comparing precise and grid-based neuron models
----------------------------------------------

In traditional time-driven simulations, spikes are constrained to the
time grid at a user-defined resolution. The precise spiking models
overcome this by handling spikes in continuous time [1]_ and [2]_.

The precise spiking neuron models in NEST include: ``iaf_psc_exp_ps``,
``iaf_psc_alpha_ps`` and ``iaf_psc_delta_ps``.
More detailed information about the precise spiking models can be
found here:
https://www.nest-simulator.org/simulations-with-precise-spike-times/

This example compares the conventional grid-constrained model and the
precise version for an integrate-and-fire neuron model with exponential
post-synaptic currents [2]_.

References
~~~~~~~~~~~

.. [1] Morrison A, Straube S, Plesser HE, Diesmann M. 2007. Exact subthreshold
       integration with continuous spike times in discrete-time neural network
       simulations. Neural Computation. 19(1):47-79.
       https://doi.org/10.1162/neco.2007.19.1.47

.. [2] Hanuschkin A, Kunkel S, Helias M, Morrison A and Diesmann M. 2010. A
       general and efficient method for incorporating precise spike times in
       globally time-driven simulations. Froniers in Neuroinformatics. 4:113.
       https://doi.org/10.3389/fninf.2010.00113


First, we import all necessary modules for simulation, analysis, and
plotting.


.. code-block:: default



    import nest
    import pylab



Second, we assign the simulation parameters to variables.


.. code-block:: default



    simtime = 100.0           # ms
    stim_current = 700.0           # pA
    resolutions = [0.1, 0.5, 1.0]  # ms



Now, we simulate the two versions of the neuron models (i.e. discrete-time:
``iaf_psc_exp``; precise: ``iaf_psc_exp_ps``) for each of the defined
resolutions. The neurons use their default parameters and we stimulate them
by injecting a current using a ``dc_generator`` device. The membrane
potential is recorded by a ``voltmeter``, the spikes are recorded by
a ``spike_detector``.  The data is stored in a dictionary for later
use.


.. code-block:: default



    data = {}

    for h in resolutions:
        data[h] = {}
        for model in ["iaf_psc_exp", "iaf_psc_exp_ps"]:
            nest.ResetKernel()
            nest.SetKernelStatus({'resolution': h})

            neuron = nest.Create(model)
            voltmeter = nest.Create("voltmeter", params={"interval": h})
            dc = nest.Create("dc_generator", params={"amplitude": stim_current})
            sd = nest.Create("spike_detector")

            nest.Connect(voltmeter, neuron)
            nest.Connect(dc, neuron)
            nest.Connect(neuron, sd)

            nest.Simulate(simtime)

            vm_status = nest.GetStatus(voltmeter, 'events')[0]
            sd_status = nest.GetStatus(sd, 'events')[0]
            data[h][model] = {"vm_times": vm_status['times'],
                              "vm_values": vm_status['V_m'],
                              "spikes": sd_status['times'],
                              "V_th": nest.GetStatus(neuron, 'V_th')[0]}



After simulation, we plot the results from the simulation. The figure
illustrates the membrane potential excursion of the two models due to
injected current simulated for 100 ms for a different timestep in each panel.
The blue line is the voltage trace of the discrete-time neuron, the red line
is that of the precise spiking version of the same model.

Please note that the temporal differences between the traces in the different
panels is caused by the different resolutions used.


.. code-block:: default



    colors = ["#3465a4", "#cc0000"]

    for v, h in enumerate(sorted(data)):
        plot = pylab.subplot(len(data), 1, v + 1)
        plot.set_title("Resolution: {0} ms".format(h))

        for i, model in enumerate(data[h]):
            times = data[h][model]["vm_times"]
            potentials = data[h][model]["vm_values"]
            spikes = data[h][model]["spikes"]
            spikes_y = [data[h][model]["V_th"]] * len(spikes)

            plot.plot(times, potentials, "-", c=colors[i], ms=5, lw=2, label=model)
            plot.plot(spikes, spikes_y, ".", c=colors[i], ms=5, lw=2)

        if v == 2:
            plot.legend(loc=4)
        else:
            plot.set_xticklabels('')


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_precise_spiking.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: precise_spiking.py <precise_spiking.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: precise_spiking.ipynb <precise_spiking.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
