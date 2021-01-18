# import os
# import sys
# sys.path.insert(0, os.path.expanduser('~/opt/nest-simulator-neat-dend/install/lib/python3.7/site-packages/'))

import nest
from datarep.matplotlibsettings import *

import copy

dt = .1

nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=dt))

soma_params = {
    'C_m': 1.0,
    'g_c': 0.0,
    'g_L': 0.1,
    'E_L': -70.0,
}

dend_params = {
    'C_m': 0.1,
    'g_c': 0.05,
    'g_L': 0.01,
    'E_L': -70.0,
}



n_neat = nest.Create('iaf_neat')
nest.AddCompartment(n_neat, 0, -1, soma_params)
nest.AddCompartment(n_neat, 1, 0, dend_params)
nest.AddCompartment(n_neat, 2, 0, dend_params)

nest.SetStatus(n_neat, {'V_th': -40.})

syn_idx_AMPA = nest.AddReceptor(n_neat, 0, "AMPA")
syn_idx_GABA = nest.AddReceptor(n_neat, 0, "GABA")
syn_idx_NMDA = nest.AddReceptor(n_neat, 2, "AMPA+NMDA")


# sg = nest.Create('spike_generator', 1, {'spike_times': [1., 5., 6.,10., 50.]})
# sg2 = nest.Create('spike_generator', 1, {'spike_times': [15.,55., 60., 62., 70., 154., 160., 172., 178.]})
# sg3 = nest.Create('spike_generator', 1, {'spike_times': [150., 155., 160., 162., 170.]})

sg = nest.Create('spike_generator', 1, {'spike_times': [15.,18., 315., 318.]})
sg2 = nest.Create('spike_generator', 1, {'spike_times': [154., 156., 162., 304., 306., 312.]})
sg3 = nest.Create('spike_generator', 1, {'spike_times': []})

nest.Connect(sg, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .1, 'delay': 0.5, 'receptor_type': syn_idx_AMPA})
nest.Connect(sg2, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .2, 'delay': 0.5, 'receptor_type': syn_idx_NMDA})
nest.Connect(sg3, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .3, 'delay': 0.5, 'receptor_type': syn_idx_GABA})

# dcg = nest.Create('dc_generator', {'amplitude': 1.})
# nest.Connect(dcg, n_neat, syn_spec={
#     'synapse_model': 'static_synapse', 'weight': 1., 'delay': 0.1, 'receptor_type': 0})

m_neat = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1', 'V_m_2'], 'interval': .1})
nest.Connect(m_neat, n_neat)

nest.Simulate(400.)

events_neat = nest.GetStatus(m_neat, 'events')[0]

# plt.figure()
# plt.plot(events_neat['times'], events_neat['V_m_0'], c='b', label='V_m_0')
# plt.plot(events_neat['times'], events_neat['V_m_1'], c='r', ls='--', label='V_m_1')
# plt.plot(events_neat['times'], events_neat['V_m_2'], c='g', ls='--', label='V_m_2')
# plt.legend()



t0, t1 = 0., 100.
i0, i1 = int(t0/dt), int(t1/dt)
ylim = (-80., 0.)
size = (2.5,2.5)

pl.figure('v_dend_ampa', figsize=size)
ax = noFrameAx(pl.gca())

ax.plot(events_neat['times'][i0:i1], events_neat['V_m_1'][i0:i1], lw=1.5)
ax.set_ylim(ylim)

pl.figure('v_soma_ampa', figsize=size)
ax = noFrameAx(pl.gca())

ax.plot(events_neat['times'][i0:i1], events_neat['V_m_0'][i0:i1], lw=1.5)
ax.set_ylim(ylim)


t0, t1 = 150., 250.
i0, i1 = int(t0/dt), int(t1/dt)

pl.figure('v_dend_nmda', figsize=size)
ax = noFrameAx(pl.gca())

ax.plot(events_neat['times'][i0:i1], events_neat['V_m_2'][i0:i1], lw=1.5)
ax.set_ylim(ylim)


pl.figure('v_soma_nmda', figsize=size)
ax = noFrameAx(pl.gca())

ax.plot(events_neat['times'][i0:i1], events_neat['V_m_0'][i0:i1], lw=1.5)
ax.set_ylim(ylim)


t0, t1 = 300., 400.
i0, i1 = int(t0/dt), int(t1/dt)

pl.figure('v_dend_ampanmda', figsize=size)
ax = noFrameAx(pl.gca())

ax.plot(events_neat['times'][i0:i1], events_neat['V_m_2'][i0:i1], lw=1.5)
ax.set_ylim(ylim)


pl.figure('v_soma_ampanmda', figsize=size)
ax = noFrameAx(pl.gca())

ax.plot(events_neat['times'][i0:i1], events_neat['V_m_0'][i0:i1], lw=1.5)
ax.set_ylim(ylim)

pl.show()
