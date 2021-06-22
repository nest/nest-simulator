"""
Example of a passive three-compartment model that is connected to spike-
generators via three receptors types and is also connected to a current
generator.
"""

import nest
import matplotlib.pyplot as plt

nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=.1))

# somatic and dendritic parameters
soma_params = {
    'C_m': 10.0,
    'g_c': 0.0,
    'g_L': 1.,
    'e_L': -70.0
}
dend_params = {
    'C_m': 0.1,
    'g_c': 0.1,
    'g_L': 0.1,
    'e_L': -70.0
}

# create a model with three compartments
cm = nest.Create('cm_main')
nest.AddCompartment(cm, 0, -1, soma_params)
nest.AddCompartment(cm, 1, 0, dend_params)
nest.AddCompartment(cm, 2, 0, dend_params)

# spike threshold
nest.SetStatus(cm, {'V_th': -50.})

# add GABA receptor in compartment 0 (soma)
syn_idx_GABA = nest.AddReceptor(cm, 0, "GABA", {})
# add AMPA receptor in compartment 1
syn_idx_AMPA = nest.AddReceptor(cm, 1, "AMPA", {})
# add AMPA+NMDA receptor in compartment 2
syn_idx_NMDA = nest.AddReceptor(cm, 2, "AMPA_NMDA", {})

# create three spike generators
sg1 = nest.Create('spike_generator', 1, {'spike_times': [101., 105., 106.,110., 150.]})
sg2 = nest.Create('spike_generator', 1, {'spike_times': [115., 155., 160., 162., 170., 254., 260., 272., 278.]})
sg3 = nest.Create('spike_generator', 1, {'spike_times': [250., 255., 260., 262., 270.]})

# connect the spike generators to the receptors
nest.Connect(sg1, cm, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .1, 'delay': 0.5, 'receptor_type': syn_idx_AMPA})
nest.Connect(sg2, cm, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .2, 'delay': 0.5, 'receptor_type': syn_idx_NMDA})
nest.Connect(sg3, cm, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .3, 'delay': 0.5, 'receptor_type': syn_idx_GABA})

# # # create a current generator
# dcg = nest.Create('dc_generator', {'amplitude': 1.})
# # connect the current generator to compartment 1
# nest.Connect(dcg, cm, syn_spec={
#     'synapse_model': 'static_synapse', 'weight': 1., 'delay': 0.1, 'receptor_type': 1})

# create a multimeter to measure the three voltages
mm = nest.Create('multimeter', 1, {'record_from': ['v_comp0', 'v_comp1', 'v_comp2'], 'interval': .1})
# connect the multimeter to the compartmental model
nest.Connect(mm, cm)

# nest.Simulate(400.)
nest.Simulate(.2)
res = nest.GetStatus(mm, 'events')[0]

plt.plot(res['times'], res['v_comp0'], c='b', label='v_comp0')
plt.plot(res['times'], res['v_comp1'], c='r', label='v_comp1')
plt.plot(res['times'], res['v_comp2'], c='g', label='v_comp2')
plt.legend()

# plt.show()
