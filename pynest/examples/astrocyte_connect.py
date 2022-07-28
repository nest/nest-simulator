import numpy as np
import nest

# Populations
pre_neurons = nest.Create('aeif_cond_alpha_astro', 10)
post_neurons = nest.Create('aeif_cond_alpha_astro', 10)
astrocytes = nest.Create('astrocyte', 10)

# Connect
nest.Connect(
    pre_neurons, post_neurons,
    conn_spec={
        'rule':'pairwise_bernoulli_astro',
        'astrocyte': astrocytes,
        'p': 0.5,
        'p_syn_astro': 0.5,
        'max_astro_per_target': 2,
        'astro_pool_per_target_det': True,
    },
    syn_spec={
        'synapse_model': 'tsodyks_synapse', 'synapse_model_astro': 'sic_connection',
        'weight': 1., 'delay': 2., 'weight_sic' :3, 'c_spill': 0.2
    }
)

# Check connections
print('astrocytes => post_neurons:')
print(nest.GetConnections(astrocytes, post_neurons))
print('pre_neurons => post_neurons:')
print(nest.GetConnections(pre_neurons, post_neurons))
print('pre_neurons => astrocytes:')
print(nest.GetConnections(pre_neurons, astrocytes))
