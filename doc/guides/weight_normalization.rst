Weight normalization
====================

Preliminaries
-------------

Suppose that the incoming synaptic weights of a neuron are given as :math:`\mathbf{w}=w_1, w_2, \ldots, w_n`. A plasticity rule might require that the vector norm :math:`|\mathbf{w}|` remains constant. For example, the L1-norm :math:`|\mathbf{w}|_1` is used in [1]_, [2]_:

.. math::

   |\mathbf{w}|_1 = \sum_i |w_i|

Keeping this norm constant at a desired target value, say :math:`w_{target}`, is typically done as an extra step after the main weights plasticity step (for example, after an STDP weight update). First, the norm is computed, and second, all weights :math:`w_1, \ldots, w_n` are updated according to:

.. math::

   w_i \leftarrow w_{target} \frac{w_i}{|\mathbf{w}|_1}


Implementation in NEST
----------------------

Because of the way that the data structures are arranged in NEST, normalizing the weights is a costly operation (in terms of time spent). One has to iterate over all the neurons, then for each neuron fetch all of its incoming connections, calculate the vector norm and perform the actual normalization, and finally to write back the new weights.

This would look something like:

.. code-block:: python

   def normalize_weights(neuron_gids_to_be_normalized, w_target=1):
       for neur in neuron_gids_to_be_normalized:
           conn = nest.GetConnections(target=[neur])
           w = np.array(conn.weight)
           w_normed = w / sum(abs(w))  # L1-norm
           conn.weight = w_target * w_normed

To apply normalization only to a certain synapse type, ``GetConnections()`` can be restricted to return only synapses of that type by specifying the model name, for example ``GetConnections(..., synapse_model="stdp_synapse")``.

To be formally correct, weight normalization should be done at each simulation timestep, but weights typically evolve on a much longer timescale than the timestep that the network is simulated at, so this would be very inefficient. Depending on how fast your weights change, you may want to perform a weight normalization, say, every 100 ms of simulated time, or every 1 s (or even less frequently). The duration of this interval can be chosen based on how far the norm is allowed to drift from :math:`w_{target}`: longer intervals allow for more drift. The magnitude of the drift can be calculated at the end of each interval, by subtracting the norm from its target, before writing back the normed vector to the NEST connection objects.

To summarize, the basic strategy is to divide the total simulated time into intervals of, say, 100 ms. You simulate for 100 ms, then pause the simulation and normalize the weights (using the code above), and then continue simulating the next interval.


References
----------

.. [1] Lazar, A. et al. (2009). SORN: a Self-organizing Recurrent Neural Network. Frontiers in Computational Neuroscience, 3. DOI: `10.3389/neuro.10.023.2009 <https://doi.org/10.3389/neuro.10.023.2009>`__

.. [2] Klos, C. et al. Bridging structure and function: A model of sequence learning and prediction in primary visual cortex. PLoS Computational Biology, 14(6). DOI: `10.1371/journal.pcbi.1006187 <https://doi.org/10.1371/journal.pcbi.1006187>`__

