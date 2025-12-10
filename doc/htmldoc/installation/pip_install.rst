.. _pip_install:

Install NEST with ``pip``
=========================

Prerequisites
-------------

- **Python**: 3.9 to 3.13
- **Operating System**: Linux, macOS
- **Architecture**: x86_64 (64-bit)


Installation steps
------------------

First set up your virtual environent (recommended):

.. code-block:: bash

   python3 -m venv nest-env
   source nest-env/bin/activate
   pip install --upgrade pip

You can either install the basic package of NEST:

.. code-block:: bash

   pip install nest-simulator

Or install NEST with one of the following options:

.. code-block:: bash

   # Install with Sonata format support (HDF5-based network files)
   pip install nest-simulator[sonata]

   # Install with server capabilities (REST API)
   pip install nest-simulator[server]

   # Install with example dependencies
   pip install nest-simulator[examples]

   # Install all options
   pip install nest-simulator[full]

   # Combine multiple options
   pip install nest-simulator[sonata,server]
   pip install nest-simulator[examples,server]

Install other desired packages (e.g., you may also want to use NEST in a Jupyter instance):

.. code-block:: bash

   # Optional packages
   pip install jupyterlab

Verify that NEST installed successfully:

.. code-block:: bash

   python -c "import nest; print('NEST version:', nest.__version__)"

.. note::

   The package name is ``nest-simulator`` but you import it as ``nest`` in Python.

Package contents
----------------


**Core Features (always included):**

- NEST simulator with Python bindings
- Standard neuron and synapse models
- GSL (GNU Scientific Library)
- Boost libraries
- OpenMP parallelization
- Basic dependencies: numpy, matplotlib, cython

**Optional Features:**

- **Sonata support**: HDF5-based network format (macOS, Alpine Linux, Debian/Ubuntu)
- **Server mode**: REST API for remote access
- **Examples**: Additional tools needed by some examples (including `ipython`, `imageio`, `seaborn`, `networkX`, `cycler`)


A small test example
--------------------

Once installed, you can open your preferred platform for editing and running Python code (like Jupyter notebook, ipython etc.).
Then, run this example:


.. code-block:: python

   import nest
   import matplotlib.pyplot as plt

   # Reset and create network
   nest.ResetKernel()

   # Create neurons and devices
   neuron = nest.Create("iaf_psc_alpha", 2)
   spike_gen = nest.Create("spike_generator", params={"spike_times": [20.0, 80.0]})
   voltmeter = nest.Create("voltmeter")

   # Connect network
   nest.Connect(spike_gen, neuron[0])
   nest.Connect(voltmeter, neuron)

   # Simulate
   nest.Simulate(100.0)

   # Plot results
   events = voltmeter.events
   plt.plot(events["times"], events["V_m"])
   plt.xlabel("Time [ms]")
   plt.ylabel("Membrane potential [mV]")
   plt.show()

Next Steps
-----------

Now that you have NEST installed, you can

* Learn how to use NEST with our :ref:`tutorials_guides` or

* Explore our :ref:`example gallery <pynest_examples>`.

Troubleshooting
----------------


Common Issues
~~~~~~~~~~~~~~

**Import Error**

.. code-block:: bash

   # Verify installation
   pip list | grep nest-simulator
   python -c "import nest; print('OK')"

**Python Version Issues**

.. code-block:: bash

   # Check Python version
   python --version  # Must be 3.9-3.13

**Installation Failures**

.. code-block:: bash

   # Clean install
   pip install --upgrade pip
   python3 -m venv clean-env
   source clean-env/bin/activate
   pip install nest-simulator

Get Help
--------

- We have extensive :doc:`Documentation <index>` on various aspects of NEST.
- You can :ref:`contact us via the mailing list or create issues on GitHub <community>`
