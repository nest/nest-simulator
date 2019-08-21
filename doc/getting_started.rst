Getting Started
================

Have you already :doc:`installed NEST <installation/index>`?

Let's look at how to create a neural network simulation!

NEST is a command line tool for simulating neural networks.
A NEST simulation tries to follow the logic of an electrophysiological
experiment - the difference being it takes place inside the computer
rather than in the physical world.

You can use NEST either within Python (PyNEST) or as a stand alone application
(``nest``).
You can use PyNEST interactively from the Python prompt, from within ipython or in a Jupyter Notebook.
This is helpful when you are exploring PyNEST, trying to learn a new
functionality or debugging a routine.

How does it work?
-------------------

Let's start with a basic script to simulate simple neural network.

* Run Python or a Jupyter Notebook and try out this example simulation in NEST:

Import required packages:

.. code-block:: python

   import nest
   import nest.voltage_trace
   nest.ResetKernel()

Create the models you want to simulate:

.. code-block:: python

   neuron = nest.Create('iaf_psc_exp')

Create the device to stimulate or measure the simulation:

.. code-block:: python

   spikegenerator = nest.Create('spike_generator')
   voltmeter = nest.Create('voltmeter')

Modify properties of the neuron and device:

.. code-block:: python

    nest.SetStatus(spikegenerator, {'spike_times': [10., 50.]})

Connect neurons to devices and specify synapse (connection) properties:

.. code-block:: python

   nest.Connect(spikegenerator, neuron, syn_spec={'weight': 1e3})
   nest.Connect(voltmeter, neuron)

Simulate  the network for a specific timeframe:

.. code-block:: python

   nest.Simulate(100.)

Display the voltage graph from device:

.. code-block:: python

   nest.voltage_trace.from_device(voltmeter)
   nest.voltage_trace.show()

You should see the following image as the output:

.. image:: _static/img/output_getting_started.png
   :align: center


**And that's it! You have simulated your first network in NEST!**

* To provide more detail and show what the commands did, we've broken down that example and included additional steps below:
You can also download the Jupyter Notebook version, right click and `save link as`: :download:`PyNEST_firststeps.ipynb <PyNEST_firststeps.ipynb>`

.. literalinclude:: 1_first_steps.py
    :language: python
    :lines: 22-146
    :linenos:


Want to know more?
---------------------

*  Check out our :doc:`PyNEST tutorial <tutorials/index>`,
   which provides full explanations on how to build your first neural network simulation in NEST.

*  We have a large collection of :doc:`Example networks <examples/index>` for you
   to explore.



Physical units in NEST
-----------------------

-   time - ms
-   voltage - mV
-   capacitance - pF
-   current - pA
-   conductance - nS
-   Spike rates (e.g., poisson\_generator) - spikes/s
-   modulation frequencies (e.g., ac\_generator) - Hz

