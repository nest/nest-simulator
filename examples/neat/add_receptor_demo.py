import nest
import matplotlib.pyplot as pl

nest.ResetKernel()
nest.SetKernelStatus(dict(resolution=.1))

# define soma and dendrite compartment parameters
params_soma = {'C_m': 1.0, 'g_L': .1, 'g_c': 0., 'E_L': -70.}
params_dend = {'C_m': .1, 'g_L': .01, 'g_c': 0.1, 'E_L': -65.}

# create neuron model
n_neat = nest.Create('iaf_neat')

# add compartments
nest.AddCompartment(n_neat, 0, -1, params_soma)
nest.AddCompartment(n_neat, 1, 0, params_dend)
nest.AddCompartment(n_neat, 2, 0, params_dend)

# set the spike threshold
nest.SetStatus(n_neat, {'V_th': -50.})

# create spike generators
sg_ampa = nest.Create('spike_generator', 1, {'spike_times': [1., 5., 6.,10., 50.]})
sg_gaba = nest.Create('spike_generator', 1, {'spike_times': [51., 54., 58., 67., 70.]})
sg_nmda = nest.Create('spike_generator', 1, {'spike_times': [1., 8., 10.,11., 15., 50., 55., 57., 62.,63.]})


# add receptors
ridx_ampa = nest.AddReceptor(n_neat, 1, "AMPA")
ridx_gaba = nest.AddReceptor(n_neat, 0, "GABA")
ridx_nmda = nest.AddReceptor(n_neat, 2, "AMPA+NMDA")


# connect spike generators
nest.Connect(sg_ampa, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .1, 'delay': 0.5, 'receptor_type': ridx_ampa})
nest.Connect(sg_gaba, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .2, 'delay': 0.5, 'receptor_type': ridx_gaba})
nest.Connect(sg_nmda, n_neat, syn_spec={
    'synapse_model': 'static_synapse', 'weight': .3, 'delay': 0.5, 'receptor_type': ridx_nmda})

# create and connect multimeter
m_neat = nest.Create('multimeter', 1, {'record_from': ['V_m_0', 'V_m_1', 'V_m_2'], 'interval': .1})
nest.Connect(m_neat, n_neat)

# simulate
nest.Simulate(1000.)
events_neat = nest.GetStatus(m_neat, 'events')[0]

# plot
pl.plot(events_neat['times'], events_neat['V_m_0'], c='b', label='V_m_0')
pl.plot(events_neat['times'], events_neat['V_m_1'], c='r', ls='--', label='V_m_1')
pl.plot(events_neat['times'], events_neat['V_m_2'], c='g', ls='--', label='V_m_2')
pl.legend()
pl.show()