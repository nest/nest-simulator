.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_clopath_synapse_spike_pairing.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_clopath_synapse_spike_pairing.py:


Clopath Rule: Spike pairing experiment
----------------------------------------

This script simulates one ``aeif_psc_delta_clopath`` neuron that is connected with
a Clopath connection [1]_. The synapse receives pairs of a pre- and a postsynaptic
spikes that are separated by either 10 ms (pre before post) or -10 ms (post
before pre). The change of the synaptic weight is measured after five of such
pairs. This experiment is repeated five times with different rates of the
sequence of the spike pairs: 10Hz, 20Hz, 30Hz, 40Hz, and 50Hz.

References
~~~~~~~~~~~

.. [1] Clopath C, Büsing L, Vasilaki E, Gerstner W (2010). Connectivity reflects coding:
       a model of voltage-based STDP with homeostasis.
       Nature Neuroscience 13:3, 344--352


.. code-block:: default


    import numpy as np
    import matplotlib.pyplot as pl
    import nest


First we specify the neuron parameters. To enable voltage dependent
prefactor ``A_LTD(u_bar_bar)`` add ``A_LTD_const: False`` to the dictionary.


.. code-block:: default


    nrn_params = {'V_m': -70.6,
                  'E_L': -70.6,
                  'C_m': 281.0,
                  'theta_minus': -70.6,
                  'theta_plus': -45.3,
                  'A_LTD': 14.0e-5,
                  'A_LTP': 8.0e-5,
                  'tau_minus': 10.0,
                  'tau_plus': 7.0,
                  'delay_u_bars': 4.0,
                  'a': 4.0,
                  'b': 0.0805,
                  'V_reset': -70.6 + 21.0,
                  'V_clamp': 33.0,
                  't_clamp': 2.0,
                  't_ref': 0.0,
                  }



Hardcoded spike times of presynaptic spike generator


.. code-block:: default


    spike_times_pre = [
        # Presynaptic spike before the postsynaptic
        [20.,  120.,  220.,  320.,  420.],
        [20.,   70.,  120.,  170.,  220.],
        [20.,   53.3,   86.7,  120.,  153.3],
        [20.,   45.,   70.,   95.,  120.],
        [20.,   40.,   60.,   80.,  100.],
        # Presynaptic spike after the postsynaptic
        [120.,  220.,  320.,  420.,  520.,  620.],
        [70.,  120.,  170.,  220.,  270.,  320.],
        [53.3,   86.6,  120.,  153.3,  186.6,  220.],
        [45.,   70.,   95.,  120.,  145.,  170.],
        [40.,   60.,   80.,  100.,  120.,  140.]]


Hardcoded spike times of postsynaptic spike generator


.. code-block:: default


    spike_times_post = [
        [10.,  110.,  210.,  310.,  410.],
        [10.,   60.,  110.,  160.,  210.],
        [10.,   43.3,   76.7,  110.,  143.3],
        [10.,   35.,   60.,   85.,  110.],
        [10.,  30.,  50.,  70.,  90.],
        [130.,  230.,  330.,  430.,  530.,  630.],
        [80.,  130.,  180.,  230.,  280.,  330.],
        [63.3,   96.6,  130.,  163.3,  196.6,  230.],
        [55.,   80.,  105.,  130.,  155.,  180.],
        [50.,   70.,   90.,  110.,  130.,  150.]]
    init_w = 0.5
    syn_weights = []
    resolution = 0.1


Loop over pairs of spike trains


.. code-block:: default


    for (s_t_pre, s_t_post) in zip(spike_times_pre, spike_times_post):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": resolution})

        # Create one neuron
        nrn = nest.Create("aeif_psc_delta_clopath", 1, nrn_params)

        # We need a parrot neuron since spike generators can only
        # be connected with static connections
        prrt_nrn = nest.Create("parrot_neuron", 1)

        # Create and connect spike generators
        spike_gen_pre = nest.Create("spike_generator", 1, {
                                    "spike_times": s_t_pre})

        nest.Connect(spike_gen_pre, prrt_nrn,
                     syn_spec={"delay": resolution})

        spike_gen_post = nest.Create("spike_generator", 1, {
                                     "spike_times": s_t_post})

        nest.Connect(spike_gen_post, nrn, syn_spec={
                     "delay": resolution, "weight": 80.0})

        # Create weight recorder
        wr = nest.Create('weight_recorder', 1)

        # Create Clopath connection with weight recorder
        nest.CopyModel("clopath_synapse", "clopath_synapse_rec",
                       {"weight_recorder": wr[0]})
        syn_dict = {"synapse_model": "clopath_synapse_rec",
                    "weight": init_w, "delay": resolution}
        nest.Connect(prrt_nrn, nrn, syn_spec=syn_dict)

        # Simulation
        simulation_time = (10.0 + max(s_t_pre[-1], s_t_post[-1]))
        nest.Simulate(simulation_time)

        # Extract and save synaptic weights
        w_events = nest.GetStatus(wr)[0]["events"]
        weights = w_events["weights"]
        syn_weights.append(weights[-1])

    syn_weights = np.array(syn_weights)
    # scaling of the weights so that they are comparable to [1]
    syn_weights = 100.0*15.0*(syn_weights - init_w)/init_w + 100.0

    # Plot results
    fig1, axA = pl.subplots(1, sharex=False)
    axA.plot([10., 20., 30., 40., 50.], syn_weights[5:], color='b', lw=2.5, ls='-',
             label="pre-post pairing")
    axA.plot([10., 20., 30., 40., 50.], syn_weights[:5], color='g', lw=2.5, ls='-',
             label="post-pre pairing")
    axA.set_ylabel("normalized weight change")
    axA.set_xlabel("rho (Hz)")
    axA.legend()
    axA.set_title("synaptic weight")

    pl.show()


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_clopath_synapse_spike_pairing.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: clopath_synapse_spike_pairing.py <clopath_synapse_spike_pairing.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: clopath_synapse_spike_pairing.ipynb <clopath_synapse_spike_pairing.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
