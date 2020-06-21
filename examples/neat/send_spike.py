import numpy as np
import os
import sys
sys.path.insert(0, os.path.expanduser('~/opt/nest-simulator-neat-dend/install/lib/python3.7/site-packages/'))

import nest
import matplotlib.pyplot as plt

dt = 0.1

nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=dt))

soma_params = {
    'C_m': 1.0,
    'g_c': 0.1,
    'g_L': 0.1,
    'E_L': -70.0,
}

n_neat_0 = nest.Create('iaf_neat')
nest.AddCompartment(n_neat_0, 0, -1, soma_params)

n_neat_1 = nest.Create('iaf_neat')
nest.AddCompartment(n_neat_1, 0, -1, soma_params)
syn_idx = nest.AddReceptor(n_neat_1, 0, "AMPA")

nest.Connect(n_neat_0, n_neat_1, syn_spec={'synapse_model': 'static_synapse', 'weight': .1,
                                           'receptor_type': syn_idx})

dc = nest.Create('dc_generator', {'amplitude': 2.0})
nest.Connect(dc, n_neat_0, syn_spec={'synapse_model': 'static_synapse', 'weight': 1.,
                                     'receptor_type': 0})

m_neat_0 = nest.Create('multimeter', 1, {'record_from': ['V_m_0'], 'interval': dt})
nest.Connect(m_neat_0, n_neat_0)

m_neat_1 = nest.Create('multimeter', 1, {'record_from': ['V_m_0'], 'interval': dt})
nest.Connect(m_neat_1, n_neat_1)

nest.Simulate(100.)

events_neat_0 = nest.GetStatus(m_neat_0, 'events')[0]
events_neat_1 = nest.GetStatus(m_neat_1, 'events')[0]

plt.plot(events_neat_0['times'], events_neat_0['V_m_0'], c='C0', label='0: V_m_0')
plt.plot(events_neat_1['times'], events_neat_1['V_m_0'], c='C1', label='1: V_m_0')
plt.legend()

plt.show()
