Getting started
===============

Have you already :doc:`installed NEST <installation/index>`?

Then let's look at how to create a neural network simulation!

NEST is a command line tool for simulating neural networks.
A NEST simulation tries to follow the logic of an electrophysiological
experiment - the difference being it takes place inside the computer
rather than in the physical world.

You can use NEST interactively from the Python prompt, from within
IPython or in a Jupyter Notebook.  The latter is helpful when you are
exploring PyNEST, trying to learn a new functionality or debugging a
routine.

How does it work?
-----------------

Let's start with a basic script to simulate a simple neural network.

Run Python or a Jupyter Notebook and try out this example simulation in NEST:

Import required packages:

.. code-block:: python

   import nest
   import nest.voltage_trace
   import matplotlib.pyplot as plt
   nest.ResetKernel()

Create the neuron models you want to simulate:

.. code-block:: python

   neuron = nest.Create('iaf_psc_exp')

Create the devices to stimulate or observe the neurons in the simulation:

.. code-block:: python

   spikegenerator = nest.Create('spike_generator')
   voltmeter = nest.Create('voltmeter')

Modify properties of the device:

.. code-block:: python

    spikegenerator.set(spike_times=[10.0, 50.0])

Connect neurons to devices and specify synapse (connection) properties:

.. code-block:: python

   nest.Connect(spikegenerator, neuron, syn_spec={'weight': 1e3})
   nest.Connect(voltmeter, neuron)

Simulate the network for the given time in miliseconds:

.. code-block:: python

   nest.Simulate(100.0)

Display the voltage graph from the voltmeter:

.. code-block:: python

   nest.voltage_trace.from_device(voltmeter)
   plt.show()

You should see the following image as the output:

.. image:: static/img/output_getting_started.png
   :align: center

**And that's it! You have performed your first neuronal simulation in NEST!**

Need a quieter NEST?
--------------------

Take a look at the `set_verbosity <https://nest-simulator.readthedocs.io/en/latest/ref_material/pynest_apis.html#nest.lib.hl_api_info.set_verbosity>`_
documentation, which describes how to display fewer messages on the terminal.

Want to know more?
------------------

* Check out our :doc:`PyNEST tutorial <tutorials/index>`, which
  provides full explanations on how to build your first neural network
  simulation in NEST.

* We have a large collection of :doc:`Example networks
  <examples/index>` for you to explore.

* Regularly used terms and default physical units in NEST are
  explained in the :doc:`Glossary <glossary>`.
