.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_Potjans_2014_network_params.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_Potjans_2014_network_params.py:


Pynest microcircuit parameters
------------------------------

Network parameters for the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016


.. code-block:: default


    import numpy as np


    def get_mean_delays(mean_delay_exc, mean_delay_inh, number_of_pop):
        """ Creates matrix containing the delay of all connections.

        Parameters
        ----------
        mean_delay_exc
            Delay of the excitatory connections.
        mean_delay_inh
            Delay of the inhibitory connections.
        number_of_pop
            Number of populations.

        Returns
        -------
        mean_delays
            Matrix specifying the mean delay of all connections.

        """

        dim = number_of_pop
        mean_delays = np.zeros((dim, dim))
        mean_delays[:, 0:dim:2] = mean_delay_exc
        mean_delays[:, 1:dim:2] = mean_delay_inh
        return mean_delays


    def get_std_delays(std_delay_exc, std_delay_inh, number_of_pop):
        """ Creates matrix containing the standard deviations of all delays.

        Parameters
        ----------
        std_delay_exc
            Standard deviation of excitatory delays.
        std_delay_inh
            Standard deviation of inhibitory delays.
        number_of_pop
            Number of populations in the microcircuit.

        Returns
        -------
        std_delays
            Matrix specifying the standard deviation of all delays.

        """

        dim = number_of_pop
        std_delays = np.zeros((dim, dim))
        std_delays[:, 0:dim:2] = std_delay_exc
        std_delays[:, 1:dim:2] = std_delay_inh
        return std_delays


    def get_mean_PSP_matrix(PSP_e, g, number_of_pop):
        """ Creates a matrix of the mean evoked postsynaptic potential.

        The function creates a matrix of the mean evoked postsynaptic
        potentials between the recurrent connections of the microcircuit.
        The weight of the connection from L4E to L23E is doubled.

        Parameters
        ----------
        PSP_e
            Mean evoked potential.
        g
            Relative strength of the inhibitory to excitatory connection.
        number_of_pop
            Number of populations in the microcircuit.

        Returns
        -------
        weights
            Matrix of the weights for the recurrent connections.

        """
        dim = number_of_pop
        weights = np.zeros((dim, dim))
        exc = PSP_e
        inh = PSP_e * g
        weights[:, 0:dim:2] = exc
        weights[:, 1:dim:2] = inh
        weights[0, 2] = exc * 2
        return weights


    def get_std_PSP_matrix(PSP_rel, number_of_pop):
        """ Relative standard deviation matrix of postsynaptic potential created.

        The relative standard deviation matrix of the evoked postsynaptic potential
        for the recurrent connections of the microcircuit is created.

        Parameters
        ----------
        PSP_rel
            Relative standard deviation of the evoked postsynaptic potential.
        number_of_pop
            Number of populations in the microcircuit.

        Returns
        -------
        std_mat
            Matrix of the standard deviation of postsynaptic potentials.

        """
        dim = number_of_pop
        std_mat = np.zeros((dim, dim))
        std_mat[:, :] = PSP_rel
        return std_mat


    net_dict = {
        # Neuron model.
        'neuron_model': 'iaf_psc_exp',
        # The default recording device is the spike_detector. If you also
        # want to record the membrane potentials of the neurons, add
        # 'voltmeter' to the list.
        'rec_dev': ['spike_detector'],
        # Names of the simulated populations.
        'populations': ['L23E', 'L23I', 'L4E', 'L4I', 'L5E', 'L5I', 'L6E', 'L6I'],
        # Number of neurons in the different populations. The order of the
        # elements corresponds to the names of the variable 'populations'.
        'N_full': np.array([20683, 5834, 21915, 5479, 4850, 1065, 14395, 2948]),
        # Mean rates of the different populations in the non-scaled version
        # of the microcircuit. Necessary for the scaling of the network.
        # The order corresponds to the order in 'populations'.
        'full_mean_rates':
            np.array([0.971, 2.868, 4.746, 5.396, 8.142, 9.078, 0.991, 7.523]),
        # Connection probabilities. The first index corresponds to the targets
        # and the second to the sources.
        'conn_probs':
            np.array(
                [[0.1009, 0.1689, 0.0437, 0.0818, 0.0323, 0., 0.0076, 0.],
                 [0.1346, 0.1371, 0.0316, 0.0515, 0.0755, 0., 0.0042, 0.],
                 [0.0077, 0.0059, 0.0497, 0.135, 0.0067, 0.0003, 0.0453, 0.],
                 [0.0691, 0.0029, 0.0794, 0.1597, 0.0033, 0., 0.1057, 0.],
                 [0.1004, 0.0622, 0.0505, 0.0057, 0.0831, 0.3726, 0.0204, 0.],
                 [0.0548, 0.0269, 0.0257, 0.0022, 0.06, 0.3158, 0.0086, 0.],
                 [0.0156, 0.0066, 0.0211, 0.0166, 0.0572, 0.0197, 0.0396, 0.2252],
                 [0.0364, 0.001, 0.0034, 0.0005, 0.0277, 0.008, 0.0658, 0.1443]]
        ),
        # Number of external connections to the different populations.
        # The order corresponds to the order in 'populations'.
        'K_ext': np.array([1600, 1500, 2100, 1900, 2000, 1900, 2900, 2100]),
        # Factor to scale the indegrees.
        'K_scaling': 0.1,
        # Factor to scale the number of neurons.
        'N_scaling': 0.1,
        # Mean amplitude of excitatory postsynaptic potential (in mV).
        'PSP_e': 0.15,
        # Relative standard deviation of the postsynaptic potential.
        'PSP_sd': 0.1,
        # Relative inhibitory synaptic strength (in relative units).
        'g': -4,
        # Rate of the Poissonian spike generator (in Hz).
        'bg_rate': 8.,
        # Turn Poisson input on or off (True or False).
        'poisson_input': True,
        # Delay of the Poisson generator (in ms).
        'poisson_delay': 1.5,
        # Mean delay of excitatory connections (in ms).
        'mean_delay_exc': 1.5,
        # Mean delay of inhibitory connections (in ms).
        'mean_delay_inh': 0.75,
        # Relative standard deviation of the delay of excitatory and
        # inhibitory connections (in relative units).
        'rel_std_delay': 0.5,
        # Initial conditions for the membrane potential, options are:
        # 'original': uniform mean and std for all populations.
        # 'optimized': population-specific mean and std, allowing a reduction of
        # the initial activity burst in the network.
        # Choose either 'original' or 'optimized'.
        'V0_type': 'original',
        # Parameters of the neurons.
        'neuron_params': {
            # Membrane potential average for the neurons (in mV).
            'V0_mean': {'original': -58.0,
                        'optimized': [-68.28, -63.16, -63.33, -63.45,
                                      -63.11, -61.66, -66.72, -61.43]},
            # Standard deviation of the average membrane potential (in mV).
            'V0_sd': {'original': 10.0,
                      'optimized': [5.36, 4.57, 4.74, 4.94,
                                    4.94, 4.55, 5.46, 4.48]},
            # Reset membrane potential of the neurons (in mV).
            'E_L': -65.0,
            # Threshold potential of the neurons (in mV).
            'V_th': -50.0,
            # Membrane potential after a spike (in mV).
            'V_reset': -65.0,
            # Membrane capacitance (in pF).
            'C_m': 250.0,
            # Membrane time constant (in ms).
            'tau_m': 10.0,
            # Time constant of postsynaptic excitatory currents (in ms).
            'tau_syn_ex': 0.5,
            # Time constant of postsynaptic inhibitory currents (in ms).
            'tau_syn_in': 0.5,
            # Time constant of external postsynaptic excitatory current (in ms).
            'tau_syn_E': 0.5,
            # Refractory period of the neurons after a spike (in ms).
            't_ref': 2.0}
    }

    updated_dict = {
        # PSP mean matrix.
        'PSP_mean_matrix': get_mean_PSP_matrix(
            net_dict['PSP_e'], net_dict['g'], len(net_dict['populations'])
        ),
        # PSP std matrix.
        'PSP_std_matrix': get_std_PSP_matrix(
            net_dict['PSP_sd'], len(net_dict['populations'])
        ),
        # mean delay matrix.
        'mean_delay_matrix': get_mean_delays(
            net_dict['mean_delay_exc'], net_dict['mean_delay_inh'],
            len(net_dict['populations'])
        ),
        # std delay matrix.
        'std_delay_matrix': get_std_delays(
            net_dict['mean_delay_exc'] * net_dict['rel_std_delay'],
            net_dict['mean_delay_inh'] * net_dict['rel_std_delay'],
            len(net_dict['populations'])
        ),
    }


    net_dict.update(updated_dict)


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_Potjans_2014_network_params.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: network_params.py <network_params.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: network_params.ipynb <network_params.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
