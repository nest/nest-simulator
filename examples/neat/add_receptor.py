import os
import sys
sys.path.insert(0, os.path.expanduser('~/opt/nest-simulator-neat-dend/install/lib/python3.7/site-packages/'))

import nest
import matplotlib.pyplot as plt

nest.ResetKernel()

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

sg = nest.Create('spike_generator', 1, {'spike_times': [1., 5., 50., 500.]})

n_neat = nest.Create('iaf_neat')
nest.AddCompartment(n_neat, 0, -1, soma_params)
nest.AddCompartment(n_neat, 1, 0, dend_params)

syn_idx_AMPA = nest.AddReceptor(n_neat, 0, "AMPA")
syn_idx_GABA = nest.AddReceptor(n_neat, 0, "GABA")
syn_idx_NMDA = nest.AddReceptor(n_neat, 0, "NMDA")

nest.Connect(sg, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 1.0, 'delay': 0.5, 'receptor_type': syn_idx_NMDA})

m_neat = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1']})
nest.Connect(m_neat, n_neat)

nest.Simulate(1000)

events_neat = nest.GetStatus(m_neat, 'events')[0]

plt.plot(events_neat['times'], events_neat['V_m_0'], c='b', label='V_m_0')
plt.plot(events_neat['times'], events_neat['V_m_1'], c='r', ls='--', label='V_m_1')
plt.legend()

plt.show()
