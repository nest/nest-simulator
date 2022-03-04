import nest

# Populations
pre_neurons = nest.Create('aeif_cond_alpha_astro', 10)
post_neurons = nest.Create('aeif_cond_alpha_astro', 10)
astrocytes = nest.Create('astrocyte', 10)

# Connect
## When p_astro is given, astrocyte=>neuron is Bernoulli connection
## When p_astro not given, astrocyte=>neuron is evenly distributed
nest.Connect(
    pre_neurons, post_neurons,
    conn_spec=dict(
        rule='pairwise_bernoulli_astro', astrocyte=astrocytes,
        p=0.1, p_astro=0.1),
    syn_spec=dict(
        synapse_model='tsodyks_synapse', synapse_model_astro='sic_connection',
        weight=1., delay=2., weight_astro=3, c_spill=0.2)
)

# Check connections
print('astrocytes => post_neurons:')
print(nest.GetConnections(astrocytes, post_neurons))
print('pre_neurons => post_neurons:')
print(nest.GetConnections(pre_neurons, post_neurons))
print('pre_neurons => astrocytes:')
print(nest.GetConnections(pre_neurons, astrocytes))
