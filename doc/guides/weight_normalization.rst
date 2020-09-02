Weight normalization
====================

Preliminaries
-------------

Suppose that the incoming synaptic weights of a neuron are given as :math:`w_1, w_2, \ldots, w_n`. A plasticity rule might require that the vector norm :math:`|w|` remains constant. For example, the 1-norm is used in [1]_ [2]_:

.. math::

   |W|_1 = \sum_i W_i

Keeping this norm constant is typically done as an extra step after the main weights plasticity step (e.g. after an STDP weight update). First, the norm is computed, and second, all weights :math:`w_1, \ldots, w_n` are updated according to:

.. math::

   W_i &\leftarrow W_{total} \frac{W_i}{|W|_1}

where :math:`w_{total}` is the target total input weight [2]_.


Implementation in NEST
----------------------

Because of the way that the data structures are arranged in NEST, normalizing the weights is a costly operation (in terms of time spent). One has to iterate over all the neurons, then for each neuron fetch all of its incoming connections, calculate the vector norm and perform the actual normalization, and finally to write back the new weights.

This would look something like:

.. code-block:: python

   def normalize_weights(neuron_gids_to_be_normalized):
       for neur in neuron_gids_to_be_normalized:
           conn = nest.GetConnections(target=[neur], synapse_model="stdp_synapse")
           w = np.array(conn.weight)
           w_norm = w / sum(w)
           conn.weight = w_norm

To apply normalization only to a certain synapse type, restrict ``GetConnections()`` to that type. For example: ``GetConnections(..., synapse_model="stdp_synapse")``.

To be formally correct, this normalization should be done at each simulation timestep, but weights typically evolve on a much longer timescale than the timestep that the network is simulated at, so this would be very inefficient. Depending on how fast your weights change, you may want to perform this update, say, every 100 ms of simulated time, or every 1 s (or even less frequently).

To summarize, the basic strategy is to divide the total simulated time into chunks of, say, 100 ms. You simulate for 100 ms, then you pause the simulation and update the weights (using the code above), and then continue simulating the next chunk.


References
----------

.. [1] Lazar, A. et al. (2009). SORN: a Self-organizing Recurrent Neural Network. Frontiers in Computational Neuroscience, 3. https://doi.org/10.3389/neuro.10.023.2009

.. [2] Klos, C. et al. Bridging structure and function: A model of sequence learning and prediction in primary visual cortex. PLoS Computational Biology, 14(6). https://doi.org/10.1371/journal.pcbi.1006187

