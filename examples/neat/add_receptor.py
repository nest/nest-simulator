# import os
# import sys
# sys.path.insert(0, os.path.expanduser('~/opt/nest-simulator-neat-dend/install/lib/python3.7/site-packages/'))

import nest
import matplotlib.pyplot as plt

import copy

nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=.1))

soma_params = {
    'C_m': 1.0,
    'g_c': 0.1,
    'g_L': 0.1,
    'E_L': -70.0,
}

dend_params = {
    'C_m': 0.1,
    'g_c': 0.1,
    'g_L': 0.01,
    'E_L': -70.0,
}

sg = nest.Create('spike_generator', 1, {'spike_times': [1., 5., 6.,10., 50., 500.]})
sg2 = nest.Create('spike_generator', 2, {'spike_times': [15.,55., 550.]})

n_neat = nest.Create('iaf_neat')
nest.AddCompartment(n_neat, 0, -1, soma_params)
nest.AddCompartment(n_neat, 1, 0, dend_params)
nest.AddCompartment(n_neat, 2, 0, copy.deepcopy(dend_params))

nest.SetStatus(n_neat, {'V_th': -50.})

syn_idx_AMPA = nest.AddReceptor(n_neat, 1, "AMPA")
syn_idx_GABA = nest.AddReceptor(n_neat, 0, "GABA")
syn_idx_AMPA2 = nest.AddReceptor(n_neat, 2, "AMPA")

nest.Connect(sg, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .1, 'delay': 0.5, 'receptor_type': syn_idx_AMPA})
nest.Connect(sg2, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .1, 'delay': 0.5, 'receptor_type': syn_idx_AMPA2})

m_neat = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1', 'V_m_2'], 'interval': .1})
nest.Connect(m_neat, n_neat)

nest.Simulate(1000.)

events_neat = nest.GetStatus(m_neat, 'events')[0]

plt.plot(events_neat['times'], events_neat['V_m_0'], c='b', label='V_m_0')
plt.plot(events_neat['times'], events_neat['V_m_1'], c='r', ls='--', label='V_m_1')
plt.plot(events_neat['times'], events_neat['V_m_2'], c='g', ls='--', label='V_m_2')
plt.legend()

plt.show()
