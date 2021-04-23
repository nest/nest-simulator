"""
Example comparison of a two-compartment model with an active dendritic
compartment and a two-compartment model with a passive dendritic compartment.
"""

import nest
import matplotlib.pyplot as plt

nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=.1))

soma_params = {
    # passive parameters
    'C_m': 89.245535, # pF
    'g_c': 0.0, # soma has no parameters
    'g_L': 8.924572508, # nS
    'e_L': -75.0,
    # E-type specific
    'g_Na': 4608.698576715, # nS
    'e_Na': 60.,
    'g_K': 956.112772900, # nS
    'e_K': -90.
}
dend_params_passive = {
    # passive parameters
    'C_m': 1.929929,
    'g_c': 1.255439494,
    'g_L': 0.192992878,
    'e_L': -75.0,
    # by default, active conducances are set to zero, so we don't need to specify
    # them explicitely
}
dend_params_active = {
    # passive parameters
    'C_m': 1.929929, # pF
    'g_c': 1.255439494, # nS
    'g_L': 0.192992878, # nS
    'e_L': -70.0, # mV
    # E-type specific
    'g_Na': 17.203212493, # nS
    'e_Na': 60., # mV
    'g_K': 11.887347450, # nS
    'e_K': -90. # mV
}

# create a neuron model with a passive dendritic compartment
cm_pas = nest.Create('cm_main')
nest.AddCompartment(cm_pas, 0, -1, soma_params)
nest.AddCompartment(cm_pas, 1, 0, dend_params_passive)
# create a neuron model with an active dendritic compartment
cm_act = nest.Create('cm_main')
nest.AddCompartment(cm_act, 0, -1, soma_params)
nest.AddCompartment(cm_act, 1, 0, dend_params_active)

# set spike thresholds
nest.SetStatus(cm_pas, {'V_th': -50.})
nest.SetStatus(cm_act, {'V_th': -50.})

# add somatic and dendritic receptor to passive dendrite model
syn_idx_soma_pas = nest.AddReceptor(cm_pas, 0, "AMPA_NMDA", {})
syn_idx_dend_pas = nest.AddReceptor(cm_pas, 1, "AMPA_NMDA", {})
# add somatic and dendritic receptor to active dendrite model
syn_idx_soma_act = nest.AddReceptor(cm_act, 0, "AMPA_NMDA", {})
syn_idx_dend_act = nest.AddReceptor(cm_act, 1, "AMPA_NMDA", {})

# create a two spike generators
sg_soma = nest.Create('spike_generator', 1, {'spike_times': [10.,13.,16.]})
sg_dend = nest.Create('spike_generator', 1, {'spike_times': [70.,73.,76.]})

# connect spike generators to passive dendrite model (weight in nS)
nest.Connect(sg_soma, cm_pas, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 5., 'delay': 0.5, 'receptor_type': syn_idx_soma_pas})
nest.Connect(sg_dend, cm_pas, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 2., 'delay': 0.5, 'receptor_type': syn_idx_dend_pas})
# connect spike generators to active dendrite model (weight in nS)
nest.Connect(sg_soma, cm_act, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 5., 'delay': 0.5, 'receptor_type': syn_idx_soma_act})
nest.Connect(sg_dend, cm_act, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 2., 'delay': 0.5, 'receptor_type': syn_idx_dend_act})

# create multimeters to record compartment voltages
mm_pas = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1'], 'interval': .1})
mm_act = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1'], 'interval': .1})
# connect the multimeters to the respective neurons
nest.Connect(mm_pas, cm_pas)
nest.Connect(mm_act, cm_act)

# simulate the models
nest.Simulate(160.)
res_pas = nest.GetStatus(mm_pas, 'events')[0]
res_act = nest.GetStatus(mm_act, 'events')[0]

plt.figure('voltage')
# plot voltage for somatic compartment
ax_soma = plt.subplot(121)
ax_soma.plot(res_pas['times'], res_pas['V_m_0'], c='b', label='passive dend')
ax_soma.plot(res_act['times'], res_act['V_m_0'], c='r', label='active dend')
ax_soma.set_xlabel(r'$t$ (ms)')
ax_soma.set_ylabel(r'$v_{soma}$ (mV)')
ax_soma.set_ylim((-90.,40.))
ax_soma.legend(loc=0)
# plot voltage for dendritic compartment
ax_dend = plt.subplot(122)
ax_dend.plot(res_pas['times'], res_pas['V_m_1'], c='b', label='passive dend')
ax_dend.plot(res_act['times'], res_act['V_m_1'], c='r', label='active dend')
ax_dend.set_xlabel(r'$t$ (ms)')
ax_dend.set_ylabel(r'$v_{dend}$ (mV)')
ax_dend.set_ylim((-90.,40.))
ax_dend.legend(loc=0)

plt.tight_layout()
plt.show()

