.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_lin_rate_ipn_network.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_lin_rate_ipn_network.py:

Network of linear rate neurons
-----------------------------------

This script simulates an excitatory and an inhibitory population
of ``lin_rate_ipn`` neurons with delayed excitatory and instantaneous
inhibitory connections. The rate of all neurons is recorded using
a multimeter. The resulting rate for one excitatory and one
inhibitory neuron is plotted.



.. code-block:: default


    import nest
    import pylab
    import numpy


Assigning the simulation parameters to variables.


.. code-block:: default


    dt = 0.1  # the resolution in ms
    T = 100.0  # Simulation time in ms


Definition of the number of neurons


.. code-block:: default


    order = 50
    NE = int(4 * order)  # number of excitatory neurons
    NI = int(1 * order)  # number of inhibitory neurons
    N = int(NE+NI)       # total number of neurons


Definition of the connections


.. code-block:: default



    d_e = 5.   # delay of excitatory connections in ms
    g = 5.0  # ratio inhibitory weight/excitatory weight
    epsilon = 0.1  # connection probability
    w = 0.1/numpy.sqrt(N)  # excitatory connection strength

    KE = int(epsilon * NE)  # number of excitatory synapses per neuron (outdegree)
    KI = int(epsilon * NI)  # number of inhibitory synapses per neuron (outdegree)
    K_tot = int(KI + KE)  # total number of synapses per neuron
    connection_rule = 'fixed_outdegree'  # connection rule


Definition of the neuron model and its neuron parameters


.. code-block:: default


    neuron_model = 'lin_rate_ipn'  # neuron model
    neuron_params = {'linear_summation': True,
                     # type of non-linearity (not affecting linear rate models)
                     'tau': 10.0,
                     # time constant of neuronal dynamics in ms
                     'mu': 2.0,
                     # mean input
                     'sigma': 5.
                     # noise parameter
                     }



Configuration of the simulation kernel by the previously defined time
resolution used in the simulation. Setting ``print_time`` to True prints
the already processed simulation time as well as its percentage of the
total simulation time.


.. code-block:: default


    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": dt, "use_wfr": False,
                          "print_time": True,
                          "overwrite_files": True})

    print("Building network")


Configuration of the neuron model using ``SetDefaults``.


.. code-block:: default


    nest.SetDefaults(neuron_model, neuron_params)


Creation of the nodes using ``Create``.


.. code-block:: default


    n_e = nest.Create(neuron_model, NE)
    n_i = nest.Create(neuron_model, NI)



To record from the rate neurons a ``multimeter`` is created and the parameter
``record_from`` is set to `rate` as well as the recording interval to `dt`


.. code-block:: default


    mm = nest.Create('multimeter', params={'record_from': ['rate'],
                                           'interval': dt})


Specify synapse and connection dictionaries:
Connections originating from excitatory neurons are associatated
with a delay `d` (``rate_connection_delayed``).
Connections originating from inhibitory neurons are not associatated
with a delay (``rate_connection_instantaneous``).


.. code-block:: default


    syn_e = {'weight': w, 'delay': d_e, 'synapse_model': 'rate_connection_delayed'}
    syn_i = {'weight': -g*w, 'synapse_model': 'rate_connection_instantaneous'}
    conn_e = {'rule': connection_rule, 'outdegree': KE}
    conn_i = {'rule': connection_rule, 'outdegree': KI}


Connect rate units


.. code-block:: default


    nest.Connect(n_e, n_e, conn_e, syn_e)
    nest.Connect(n_i, n_i, conn_i, syn_i)
    nest.Connect(n_e, n_i, conn_i, syn_e)
    nest.Connect(n_i, n_e, conn_e, syn_i)


Connect recording device to rate units


.. code-block:: default


    nest.Connect(mm, n_e+n_i)


Simulate the network


.. code-block:: default


    nest.Simulate(T)


Plot rates of one excitatory and one inhibitory neuron


.. code-block:: default


    data = nest.GetStatus(mm)[0]['events']
    rate_ex = data['rate'][numpy.where(data['senders'] == n_e[0].get('global_id'))]
    rate_in = data['rate'][numpy.where(data['senders'] == n_i[0].get('global_id'))]
    times = data['times'][numpy.where(data['senders'] == n_e[0].get('global_id'))]

    pylab.figure()
    pylab.plot(times, rate_ex, label='excitatory')
    pylab.plot(times, rate_in, label='inhibitory')
    pylab.xlabel('time (ms)')
    pylab.ylabel('rate (a.u.)')
    pylab.show()


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_lin_rate_ipn_network.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: lin_rate_ipn_network.py <lin_rate_ipn_network.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: lin_rate_ipn_network.ipynb <lin_rate_ipn_network.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
