.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_if_curve.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_if_curve.py:

IF curve example
----------------------

This example illustrates how to measure the I-F curve of a neuron.
The program creates a small group of neurons and injects a noisy current
:math:`I(t) = I_mean + I_std*W(t)`
where :math:`W(t)` is a white noise process.
The programm systematically drives the current through a series of  values in
the two-dimensional `(I_mean, I_std)` space and measures the firing rate of
the neurons.

In this example, we measure the I-F curve of the adaptive exponential
integrate and fire neuron (``aeif_cond_exp``), but any other neuron model that
accepts current inputs is possible. The model and its parameters are
supplied when the IF_curve object is created.



.. code-block:: default


    import numpy
    import nest
    import shelve


Here we define which model and the neuron parameters to use for measuring
the transfer function.


.. code-block:: default


    model = 'aeif_cond_exp'
    params = {'a': 4.0,
              'b': 80.8,
              'V_th': -50.4,
              'Delta_T': 2.0,
              'I_e': 0.0,
              'C_m': 281.0,
              'g_L': 30.0,
              'V_reset': -70.6,
              'tau_w': 144.0,
              't_ref': 5.0,
              'V_peak': -40.0,
              'E_L': -70.6,
              'E_ex': 0.,
              'E_in': -70.}


    class IF_curve():

        t_inter_trial = 200.  # Interval between two successive measurement trials
        t_sim = 1000.         # Duration of a measurement trial
        n_neurons = 100       # Number of neurons
        n_threads = 4         # Nubmer of threads to run the simulation

        def __init__(self, model, params=False):
            self.model = model
            self.params = params
            self.build()
            self.connect()

        def build(self):
            #######################################################################
            #  We reset NEST to delete information from previous simulations
            # and adjust the number of threads.

            nest.ResetKernel()
            nest.SetKernelStatus({'local_num_threads': self.n_threads})

            #######################################################################
            # We set the default parameters of the neuron model to those
            # defined above and create neurons and devices.

            if self.params:
                nest.SetDefaults(self.model, self.params)
            self.neuron = nest.Create(self.model, self.n_neurons)
            self.noise = nest.Create('noise_generator')
            self.spike_detector = nest.Create('spike_detector')

        def connect(self):
            #######################################################################
            # We connect the noisy current to the neurons and the neurons to
            # the spike detectors.

            nest.Connect(self.noise, self.neuron, 'all_to_all')
            nest.Connect(self.neuron, self.spike_detector, 'all_to_all')

        def output_rate(self, mean, std):
            self.build()
            self.connect()

            #######################################################################
            # We adjust the parameters of the noise according to the current
            # values.

            nest.SetStatus(self.noise, [{'mean': mean, 'std': std, 'start': 0.0,
                                         'stop': 1000., 'origin': 0.}])

            # We simulate the network and calculate the rate.

            nest.Simulate(self.t_sim)
            rate = nest.GetStatus(self.spike_detector, 'n_events')[0] * 1000.0 \
                / (1. * self.n_neurons * self.t_sim)
            return rate

        def compute_transfer(self, i_mean=(400.0, 900.0, 50.0),
                             i_std=(0.0, 600.0, 50.0)):
            #######################################################################
            # We loop through all possible combinations of `(I_mean, I_sigma)`
            # and measure the output rate of the neuron.

            self.i_range = numpy.arange(*i_mean)
            self.std_range = numpy.arange(*i_std)
            self.rate = numpy.zeros((self.i_range.size, self.std_range.size))
            nest.set_verbosity('M_WARNING')
            for n, i in enumerate(self.i_range):
                print('I  =  {0}'.format(i))
                for m, std in enumerate(self.std_range):
                    self.rate[n, m] = self.output_rate(i, std)


    transfer = IF_curve(model, params)
    transfer.compute_transfer()


After the simulation is finished we store the data into a file for
later analysis.


.. code-block:: default


    dat = shelve.open(model + '_transfer.dat')
    dat['I_mean'] = transfer.i_range
    dat['I_std'] = transfer.std_range
    dat['rate'] = transfer.rate
    dat.close()


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_if_curve.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: if_curve.py <if_curve.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: if_curve.ipynb <if_curve.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
