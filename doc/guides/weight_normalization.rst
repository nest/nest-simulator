Weight normalization
====================

Let's say that you have a neuron with incoming synaptic weights :math:`w_1, w_2, \ldots, w_n`. A plasticity rule might require that the vector norm :math:`|w|` remains constant.

Because of the way that the data structures are arranged in NEST, normalizing the weights per neuron is a costly operation (in terms of time spent). One has to iterate over all the neurons, then for each neuron fetch all of its connections, calculate the vector norm and perform the actual normalisation, and finally to write back the new weights.

This would look something like:

.. code-block:: python

   for neur in neurons_to_be_normalised:
       connect = nest.GetConnections(target=[neur], synapse_model="stdp_synapse")
       w = nest.GetStatus(connect, "weight")
       nest.SetStatus(connect, "weight", np.array(w) / np.sum(w))

The next question is then, how often should this normalisation be carried out? To be formally correct, it should be done at each simulation timestep, but weights typically evolve on a much longer timescale than the network is simulated at, so this would be very inefficient. Depending on how fast your weights change, you may want to perform this update, say, every 100 ms of simulated time, or every 1 s (or even less frequently).

So, the basic strategy is to divide your total simulation time into chunks of, say, 100 ms. You simulate for 100 ms, then you stop simulating and update the weights (using the code above), and then continue simulating the next chunk.
