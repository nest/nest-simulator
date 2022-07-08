
import nest



nest.CopyModel('static_synapse', 'synapse_ex')
nest.SetDefaults('synapse_ex', {'weight': 1, 'delay': 1.0})
nest.SetKernelStatus({
    'structural_plasticity_synapses': {
        'synapse_ex': {
            'synapse_model': 'synapse_ex',
            'post_synaptic_element': 'Den_ex',
            'pre_synaptic_element': 'Axon_ex',
            'allow_autapses': False,
        },
    }
})
