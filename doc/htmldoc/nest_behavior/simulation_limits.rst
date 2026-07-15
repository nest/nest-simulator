.. _simulation_limits:

Network and simulation limits
==============================

NEST's kernel encodes target neuron identifiers, synapse types, thread IDs, and
MPI ranks into a compact 64-bit integer.  The number of bits allocated to each
field sets hard limits on the size of a simulation.  Some limits are fixed; others
depend on the :ref:`target-bits-split compile option <target_bits_split>`.

.. _target_bits_split:

Configuring limits at compile time
-----------------------------------

The ``-Dtarget-bits-split`` CMake option controls how the 64-bit target
identifier is partitioned between synapse types, threads, and MPI ranks.  Two
presets are available:

.. list-table::
   :header-rows: 1
   :widths: 30 15 15 25

   * - CMake flag
     - Preset name
     - Use case
     - Description
   * - ``-Dtarget-bits-split='standard'``
     - ``standard``
     - Default
     - More synapse types and threads; fewer MPI ranks
   * - ``-Dtarget-bits-split='hpc'``
     - ``hpc``
     - Large HPC clusters
     - More MPI ranks and threads; fewer synapse types

See :ref:`cmake_options` for the full list of build options.

.. _limit_synapse_types:

Maximum number of synapse types
---------------------------------

The ``syn_id`` field width controls how many distinct synapse models a single
simulation can use simultaneously.

.. list-table::
   :header-rows: 1
   :widths: 30 20 25

   * - Preset
     - Bits for ``syn_id``
     - Max synapse types
   * - ``standard``
     - 9
     - 511
   * - ``hpc``
     - 6
     - 63

The limit applies to the number of *distinct models* registered with the kernel,
not the total number of synaptic connections.

.. _limit_threads_ranks:

Maximum threads and MPI ranks
-------------------------------

Thread IDs (``tid``) and MPI ranks each occupy their own bitfield.

.. list-table::
   :header-rows: 1
   :widths: 30 20 25

   * - Preset
     - Bits for threads (``tid``)
     - Max threads per MPI process
   * - ``standard``
     - 9
     - 511
   * - ``hpc``
     - 10
     - 1 023

.. list-table::
   :header-rows: 1
   :widths: 30 20 25

   * - Preset
     - Bits for ranks
     - Max MPI processes
   * - ``standard``
     - 18
     - 262 143
   * - ``hpc``
     - 20
     - 1 048 575

.. note::

   Use the ``hpc`` preset when you need more than 262 143 MPI ranks or more
   than 511 threads.  The trade-off is that the ``hpc`` preset supports only
   63 synapse types instead of 511.

.. _limit_nodes:

Maximum number of nodes
-------------------------

The node ID field is 61 bits wide regardless of the target-bits-split preset.
One value is reserved as a *disabled* sentinel, so the maximum node ID
accessible in a simulation is :math:`2^{61} - 2` (approximately
2.3 × 10\ :sup:`18`).

.. _limit_delays:

Delay constraints
------------------

Delays in NEST are stored as integer multiples of the simulation resolution
:math:`h`.  Two bitfields impose hard upper bounds:

**Minimum delay (``min_delay``)**

The *lag* field is 14 bits wide.  The minimum delay across all connections in
the network must satisfy:

.. math::

   d_{\min} < 2^{14} \cdot h = 16\,384 \cdot h

For example, with the default resolution :math:`h = 0.1\,\text{ms}`, the
minimum delay must be less than 1 638.4 ms.

**Maximum delay (``max_delay``)**

The *delay* field is 21 bits wide.  The maximum delay across all connections
must satisfy:

.. math::

   d_{\max} < 2^{21} \cdot h = 2\,097\,152 \cdot h

For example, with :math:`h = 0.1\,\text{ms}`, the maximum delay must be less
than 209 715.2 ms (≈ 209 s).

These constraints are enforced at simulation time.  NEST raises an error if a
connection delay violates either bound.

.. note::

   These delay limits are rarely a practical constraint at typical resolutions,
   but become relevant when using very small :math:`h` values or very long
   axonal delays.

Summary table
--------------

The table below collects all kernel limits for quick reference.

.. list-table::
   :header-rows: 1
   :widths: 35 30 30

   * - Quantity
     - Limit (``standard``)
     - Limit (``hpc``)
   * - Synapse types
     - 511
     - 63
   * - Threads per MPI process
     - 511
     - 1 023
   * - MPI processes (ranks)
     - 262 143
     - 1 048 575
   * - Nodes (neurons + devices)
     - :math:`2^{61} - 2` (both presets)
     - :math:`2^{61} - 2` (both presets)
   * - Min delay
     - :math:`< 16\,384 \cdot h` (both presets)
     - :math:`< 16\,384 \cdot h` (both presets)
   * - Max delay
     - :math:`< 2\,097\,152 \cdot h` (both presets)
     - :math:`< 2\,097\,152 \cdot h` (both presets)
