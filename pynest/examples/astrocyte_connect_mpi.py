import numpy as np
import nest
from mpi4py import MPI

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

# Populations
pre_neurons = nest.Create('aeif_cond_alpha_astro', 10)
post_neurons = nest.Create('aeif_cond_alpha_astro', 10)
astrocytes = nest.Create('astrocyte', 5)

# Connect
nest.Connect(
    pre_neurons, post_neurons,
    conn_spec={
        'rule':'pairwise_bernoulli_astro',
        'astrocyte':astrocytes,
        'p':0.5,
        'p_astro':0.5,
        },
    syn_spec=dict(
        synapse_model='tsodyks_synapse', synapse_model_astro='sic_connection',
        weight=1., delay=2., weight_sic=3, c_spill=0.2)
)

pre_loc = np.array(nest.GetLocalNodeCollection(pre_neurons))
print('pre nodes on rank {}:\n{}'.format(rank, pre_loc))
post_loc = np.array(nest.GetLocalNodeCollection(post_neurons))
print('post nodes on rank {}:\n{}'.format(rank, post_loc))
astrocytes_loc = np.array(nest.GetLocalNodeCollection(astrocytes))
print('astro nodes on rank {}:\n{}'.format(rank, astrocytes_loc))

# Check connections
print('astrocytes => post_neurons:')
print(nest.GetConnections(astrocytes, post_neurons))
print('pre_neurons => post_neurons:')
print(nest.GetConnections(pre_neurons, post_neurons))
print('pre_neurons => astrocytes:')
print(nest.GetConnections(pre_neurons, astrocytes))
