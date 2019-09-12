.. note::
    :class: sphx-glr-download-link-note

    Click :ref:`here <sphx_glr_download_auto_examples_multimeter_file.py>` to download the full example code
.. rst-class:: sphx-glr-example-title

.. _sphx_glr_auto_examples_multimeter_file.py:


multimeter to file example
--------------------------

This file demonstrates recording from an ``iaf_cond_alpha`` neuron using a
multimeter and writing data to file.


First, we import the necessary modules to simulate and plot this example.
The simulation kernel is put back to its initial state using ``ResetKernel``.


.. code-block:: default


    import nest
    import numpy
    import pylab

    nest.ResetKernel()


With ``SetKernelStatus``, global properties of the simulation kernel can be
specified. The following properties are related to writing to file:

* ``overwrite_files`` is set to True to permit overwriting of an existing file.
* ``data_path`` is the path to which all data is written. It is given relative
  to  the current working directory.
* 'data_prefix' allows to specify a common prefix for all data files.


.. code-block:: default


    nest.SetKernelStatus({"overwrite_files": True,
                          "data_path": "",
                          "data_prefix": ""})


For illustration, the recordables of the ``iaf_cond_alpha`` neuron model are
displayed. This model is an implementation of a spiking neuron using
integrate-and-fire dynamics with conductance-based synapses. Incoming spike
events induce a post-synaptic change of conductance modeled by an alpha
function.


.. code-block:: default


    print("iaf_cond_alpha recordables: {0}".format(
          nest.GetDefaults("iaf_cond_alpha")["recordables"]))


A neuron, a multimeter as recording device and two spike generators for
excitatory and inhibitory stimulation are instantiated. The command ``Create``
expects a model type and, optionally, the desired number of nodes and a
dictionary of parameters to overwrite the default values of the model.

 * For the neuron, the rise time of the excitatory synaptic alpha function
   (`tau_syn_ex`, in ms) and the reset potential of the membrane
   (`V_reset`, in mV) are specified.
 * For the ``multimeter``, the time interval for recording (`interval`, in
   ms) and the measures to record (membrane potential `V_m` in mV and
   excitatory and inhibitoy synaptic conductances `g_ex` and`g_in` in nS)
   are set.

 In addition, more parameters can be modified for writing to file:

 - `record_to` indicates where to put recorded data. All possible values are
   available by inspecting the keys of the `recording_backends` dictionary
   obtained from ``GetKernelStatus()``.
 - `label` specifies an arbitrary label for the device. If writing to files,
   it used in the file name instead of the model name.

 * For the spike generators, the spike times in ms (`spike_times`) are given
   explicitly.


.. code-block:: default


    n = nest.Create("iaf_cond_alpha",
                    params={"tau_syn_ex": 1.0, "V_reset": -70.0})

    m = nest.Create("multimeter",
                    params={"interval": 0.1,
                            "record_from": ["V_m", "g_ex", "g_in"],
                            "record_to": "ascii",
                            "label": "my_multimeter"})

    s_ex = nest.Create("spike_generator",
                       params={"spike_times": numpy.array([10.0, 20.0, 50.0])})
    s_in = nest.Create("spike_generator",
                       params={"spike_times": numpy.array([15.0, 25.0, 55.0])})


Next, We connect the spike generators to the neuron with ``Connect``. Synapse
specifications can be provided in a dictionary. In this example of a
conductance-based neuron, the synaptic weight ``weight`` is given in nS.
Note that the values are  positive for excitatory stimulation and negative
for inhibitor connections.


.. code-block:: default


    nest.Connect(s_ex, n, syn_spec={"weight": 40.0})
    nest.Connect(s_in, n, syn_spec={"weight": -20.0})
    nest.Connect(m, n)


A network simulation with a duration of 100 ms is started with ``Simulate``.


.. code-block:: default


    nest.Simulate(100.)


After the simulation, the recordings are obtained from the multimeter via the
key `events` of the status dictionary accessed by ``GetStatus``. `times`
contains the recording times stored for each data point.


.. code-block:: default


    events = nest.GetStatus(m)[0]["events"]
    t = events["times"]


Finally, the time courses of the membrane voltage and the synaptic
conductance are displayed.


.. code-block:: default


    pylab.clf()

    pylab.subplot(211)
    pylab.plot(t, events["V_m"])
    pylab.axis([0, 100, -75, -53])
    pylab.ylabel("membrane potential (mV)")

    pylab.subplot(212)
    pylab.plot(t, events["g_ex"], t, events["g_in"])
    pylab.axis([0, 100, 0, 45])
    pylab.xlabel("time (ms)")
    pylab.ylabel("synaptic conductance (nS)")
    pylab.legend(("g_exc", "g_inh"))


.. rst-class:: sphx-glr-timing

   **Total running time of the script:** ( 0 minutes  0.000 seconds)


.. _sphx_glr_download_auto_examples_multimeter_file.py:


.. only :: html

 .. container:: sphx-glr-footer
    :class: sphx-glr-footer-example



  .. container:: sphx-glr-download

     :download:`Download Python source code: multimeter_file.py <multimeter_file.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: multimeter_file.ipynb <multimeter_file.ipynb>`


.. only:: html

 .. rst-class:: sphx-glr-signature

    `Gallery generated by Sphinx-Gallery <https://sphinx-gallery.github.io>`_
