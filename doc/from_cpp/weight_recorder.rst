

Recording weights from synapses
###############################

The change in synaptic weights over time is a key observable property in
studies of plasticity in neuronal network models. To access this information, the
``weight_recorder`` can be used. In contrast to other recording
devices, which are connected to a specific set of neurons, the weight
recorder is instead set as a parameter in the synapse model.

After assigning an instance of a weight recorder to the synapse model
by setting its ``weight_recorder`` property, the weight
recorder collects the global IDs of source and target neurons together
with the weight for each spike event that travels through the observed
synapses.

To only record from a subset of connected synapses, the
weight recorder accepts NodeCollections in the parameters ``senders`` and
``targets``. If set, they restrict the recording of data to only
synapses that fulfill the given criteria.

::

   >>> wr = nest.Create('weight_recorder')
   >>> nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr})

   >>> pre = nest.Create("iaf_psc_alpha", 10)
   >>> post = nest.Create("iaf_psc_alpha", 10)

   >>> nest.Connect(pre, post, syn_spec="stdp_synapse_rec")


