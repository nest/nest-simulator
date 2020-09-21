# -*- coding: utf-8 -*-
#
# urbanczik_synapse_example.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

'''
Weight adaptation according to the Urbanczik-Senn plasticity
------------------

This script demonstrates the learning in a compartmental neuron where the
dendritic synapses adapt their weight according to the plasticity rule by
Urbanczik and Senn [1]. In this simple setup, a spike pattern of 200 poisson
spike trains is repeatedly presented to a neuron that is composed of one
somatic and one dendritic compartment. At the same time, the somatic
conductances are activated to produce a time-varying matching potential.
After the learning, this signal is then reproreproduced by the membrane
potential of the neuron. This script produces Fig. 1B in [1] but uses standard
units instead of the unitless quantities used in the paper.

[1] R. Urbanczik, W. Senn (2014): Learning by the Dendritic Prediction of
    Somatic Spiking. Neuron, 81, 521-528.
'''
import numpy as np
from matplotlib import pyplot as plt
import nest


def g_inh(amplitude, t_start, t_end):
    '''
    returns weights for the spike generator that drives the inhibitory
    somatic conductance.
    '''
    return lambda t: np.piecewise(t, [(t >= t_start) & (t < t_end)],
                                  [amplitude, 0.0])


def g_exc(amplitude, freq, offset, t_start, t_end):
    '''
    returns weights for the spike generator that drives the excitatory
    somatic conductance.
    '''
    return lambda t: np.piecewise(t, [(t >= t_start) & (t < t_end)],
                                  [lambda t: amplitude*np.sin(freq*t) + offset, 0.0])


def matching_potential(g_E, g_I, nrn_params):
    '''
    returns the matching potential as a function of the somatic conductances.
    '''
    E_E = nrn_params['soma']['E_ex']
    E_I = nrn_params['soma']['E_in']
    return (g_E*E_E + g_I*E_I) / (g_E + g_I)


def V_w_star(V_w, nrn_params):
    '''
    returns the dendritic prediction of the somatic membrane potential.
    '''
    g_D = nrn_params['g_sp']
    g_L = nrn_params['soma']['g_L']
    E_L = nrn_params['soma']['E_L']
    return (g_L*E_L + g_D*V_w) / (g_L + g_D)


def phi(U, nrn_params):
    '''
    rate function of the soma
    '''
    phi_max = nrn_params['phi_max']
    k = nrn_params['rate_slope']
    beta = nrn_params['beta']
    theta = nrn_params['theta']
    return phi_max / (1.0 + k*np.exp(beta*(theta - U)))


def h(U, nrn_params):
    '''
    derivative of the rate function phi
    '''
    phi_max = nrn_params['phi_max']
    k = nrn_params['rate_slope']
    beta = nrn_params['beta']
    theta = nrn_params['theta']
    return 15.0*beta / (1.0 + np.exp(-beta*(theta - U)) / k)


'''
simulation params
'''
n_pattern_rep = 100         # number of repetitions of the spike pattern
pattern_duration = 200.0
t_start = 2.0*pattern_duration
t_end = n_pattern_rep*pattern_duration + t_start
simulation_time = t_end + 2.0*pattern_duration
n_rep_total = int(np.around(simulation_time / pattern_duration))
resolution = 0.1
nest.SetKernelStatus({'resolution': resolution})

'''
neuron parameters
'''
nrn_model = 'pp_cond_exp_mc_urbanczik'
nrn_params = {
    't_ref': 3.0,        # refractory period
    'g_sp': 600.0,       # soma-to-dendritic coupling conductance
    'soma': {
        'V_m': -70.0,    # initial value of V_m
        'C_m': 300.0,    # capacitance of membrane
        'E_L': -70.0,    # resting potential
        'g_L': 30.0,     # somatic leak conductance
        'E_ex': 0.0,     # resting potential for exc input
        'E_in': -75.0,   # resting potential for inh input
        'tau_syn_ex': 3.0,  # time constant of exc conductance
        'tau_syn_in': 3.0,  # time constant of inh conductance
    },
    'dendritic': {
        'V_m': -70.0,    # initial value of V_m
        'C_m': 300.0,    # capacitance of membrane
        'E_L': -70.0,    # resting potential
        'g_L': 30.0,     # dendritic leak conductance
        'tau_syn_ex': 3.0,  # time constant of exc input current
        'tau_syn_in': 3.0,  # time constant of inh input current
    },
    # parameters of rate function
    'phi_max': 0.15,     # max rate
    'rate_slope': 0.5,   # called 'k' in the paper
    'beta': 1.0 / 3.0,
    'theta': -55.0,
}

'''
synapse params
'''
syns = nest.GetDefaults(nrn_model)['receptor_types']
init_w = 0.3*nrn_params['dendritic']['C_m']
syn_params = {
    'synapse_model': 'urbanczik_synapse_wr',
    'receptor_type': syns['dendritic_exc'],
    'tau_Delta': 100.0,  # time constant of low pass filtering of the weight change
    'eta': 0.17,         # learning rate
    'weight': init_w,
    'Wmax': 4.5*nrn_params['dendritic']['C_m'],
    'delay': resolution,
}

'''
# in case you want to use the unitless quantities as in [1]:

# neuron params:

nrn_model = 'pp_cond_exp_mc_urbanczik'
nrn_params = {
    't_ref': 3.0,
    'g_sp': 2.0,
    'soma': {
        'V_m': 0.0,
        'C_m': 1.0,
        'E_L': 0.0,
        'g_L': 0.1,
        'E_ex': 14.0 / 3.0,
        'E_in': -1.0 / 3.0,
        'tau_syn_ex': 3.0,
        'tau_syn_in': 3.0,
    },
    'dendritic': {
        'V_m': 0.0,
        'C_m': 1.0,
        'E_L': 0.0,
        'g_L': 0.1,
        'tau_syn_ex': 3.0,
        'tau_syn_in': 3.0,
    },
    # parameters of rate function
    'phi_max': 0.15,
    'rate_slope': 0.5,
    'beta': 5.0,
    'theta': 1.0,
}

# synapse params:

syns = nest.GetDefaults(nrn_model)['receptor_types']
init_w = 0.2*nrn_params['dendritic']['g_L']
syn_params = {
    'synapse_model': 'urbanczik_synapse_wr',
    'receptor_type': syns['dendritic_exc'],
    'tau_Delta': 100.0,
    'eta': 0.0003 / (15.0*15.0*nrn_params['dendritic']['C_m']),
    'weight': init_w,
    'Wmax': 3.0*nrn_params['dendritic']['g_L'],
    'delay': resolution,
}
'''

'''
somatic input
'''
ampl_exc = 0.016*nrn_params['dendritic']['C_m']
offset = 0.018*nrn_params['dendritic']['C_m']
ampl_inh = 0.06*nrn_params['dendritic']['C_m']
freq = 2.0 / pattern_duration
soma_exc_inp = g_exc(ampl_exc, 2.0*np.pi*freq, offset, t_start, t_end)
soma_inh_inp = g_inh(ampl_inh, t_start, t_end)

'''
dendritic input
create spike pattern by recording the spikes of a simulation of n_pg
poisson generators. The recorded spike times are then given to spike
generators.
'''
n_pg = 200                 # number of poisson generators
p_rate = 10.0              # rate in Hz

pgs = nest.Create('poisson_generator', n=n_pg, params={'rate': p_rate})

prrt_nrns_pg = nest.Create('parrot_neuron', n_pg)

nest.Connect(pgs, prrt_nrns_pg, {'rule': 'one_to_one'})

sr = nest.Create('spike_recorder', n_pg)
nest.Connect(prrt_nrns_pg, sr, {'rule': 'one_to_one'})

nest.Simulate(pattern_duration)
t_srs = []
for i, ssr in enumerate(nest.GetStatus(sr)):
    t_sr = ssr['events']['times']
    t_srs.append(t_sr)

nest.ResetKernel()
nest.SetKernelStatus({'resolution': resolution})

'''
neuron and devices
'''
nest.SetDefaults(nrn_model, nrn_params)
nrn = nest.Create(nrn_model)

# poisson generators are connected to parrot neurons which are
# connected to the mc neuron
prrt_nrns = nest.Create('parrot_neuron', n_pg)

# excitatory input to the soma
spike_times_soma_inp = np.arange(resolution, simulation_time, resolution)
sg_soma_exc = nest.Create('spike_generator',
                          params={'spike_times': spike_times_soma_inp,
                                  'spike_weights': soma_exc_inp(spike_times_soma_inp)})
# inhibitory input to the soma
sg_soma_inh = nest.Create('spike_generator',
                          params={'spike_times': spike_times_soma_inp,
                                  'spike_weights': soma_inh_inp(spike_times_soma_inp)})

# excitatory input to the dendrite
sg_prox = nest.Create('spike_generator', n=n_pg)

# for recording all parameters of the Urbanczik neuron
rqs = nest.GetDefaults(nrn_model)['recordables']
mm = nest.Create('multimeter', params={'record_from': rqs, 'interval': 0.1})

# for recoding the synaptic weights of the Urbanczik synapses
wr = nest.Create('weight_recorder')

# for recording the spiking of the soma
sr_soma = nest.Create('spike_recorder')

'''
create connections
'''
nest.Connect(sg_prox, prrt_nrns, {'rule': 'one_to_one'})
nest.CopyModel('urbanczik_synapse', 'urbanczik_synapse_wr',
               {'weight_recorder': wr[0]})
nest.Connect(prrt_nrns, nrn, syn_spec=syn_params)
nest.Connect(mm, nrn, syn_spec={'delay': 0.1})
nest.Connect(sg_soma_exc, nrn,
             syn_spec={'receptor_type': syns['soma_exc'], 'weight': 10.0*resolution, 'delay': resolution})
nest.Connect(sg_soma_inh, nrn,
             syn_spec={'receptor_type': syns['soma_inh'], 'weight': 10.0*resolution, 'delay': resolution})
nest.Connect(nrn, sr_soma)

'''
simulation divided into intervals of the pattern duration
'''
for i in np.arange(n_rep_total):
    # Set the spike times of the pattern for each spike generator
    for (sg, t_sp) in zip(sg_prox, t_srs):
        nest.SetStatus(
            sg, {'spike_times': np.array(t_sp) + i*pattern_duration})

    nest.Simulate(pattern_duration)

'''
read out devices
'''
# multimeter
rec = nest.GetStatus(mm)[0]['events']
t = rec['times']
V_s = rec['V_m.s']
V_d = rec['V_m.p']
V_d_star = V_w_star(V_d, nrn_params)
g_in = rec['g_in.s']
g_ex = rec['g_ex.s']
I_ex = rec['I_ex.p']
I_in = rec['I_in.p']
U_M = matching_potential(g_ex, g_in, nrn_params)

# weight recorder
data = nest.GetStatus(wr)
senders = data[0]['events']['senders']
targets = data[0]['events']['targets']
weights = data[0]['events']['weights']
times = data[0]['events']['times']

# spike recorder
data = nest.GetStatus(sr_soma)[0]['events']
spike_times_soma = data['times']

'''
plot results
'''
fs = 22
lw = 2.5
fig1, (axA, axB, axC, axD) = plt.subplots(4, 1, sharex=True)

# membrane potentials and matching potential
axA.plot(t, V_s, lw=lw, label=r'$U$ (soma)', color='darkblue')
axA.plot(t, V_d, lw=lw, label=r'$V_W$ (dendrit)', color='deepskyblue')
axA.plot(t, V_d_star, lw=lw, label=r'$V_W^\ast$ (dendrit)', color='b', ls='--')
axA.plot(t, U_M, lw=lw, label=r'$U_M$ (soma)', color='r', ls='-')
axA.set_ylabel('membrane pot [mV]', fontsize=fs)
axA.legend(fontsize=fs)

# somatic conductances
axB.plot(t, g_in, lw=lw, label=r'$g_I$', color='r')
axB.plot(t, g_ex, lw=lw, label=r'$g_E$', color='coral')
axB.set_ylabel('somatic cond', fontsize=fs)
axB.legend(fontsize=fs)

# dendritic currents
axC.plot(t, I_ex, lw=lw, label=r'$I_ex$', color='r')
axC.plot(t, I_in, lw=lw, label=r'$I_in$', color='coral')
axC.set_ylabel('dend current', fontsize=fs)
axC.legend(fontsize=fs)

# rates
axD.plot(t, phi(V_s, nrn_params), lw=lw, label=r'$\phi(U)$', color='darkblue')
axD.plot(t, phi(V_d, nrn_params), lw=lw,
         label=r'$\phi(V_W)$', color='deepskyblue')
axD.plot(t, phi(V_d_star, nrn_params), lw=lw,
         label=r'$\phi(V_W^\ast)$', color='b', ls='--')
axD.plot(t, h(V_d_star, nrn_params), lw=lw,
         label=r'$h(V_W^\ast)$', color='g', ls='--')
axD.plot(t, phi(V_s, nrn_params) - phi(V_d_star, nrn_params), lw=lw,
         label=r'$\phi(U) - \phi(V_W^\ast)$', color='r', ls='-')
axD.plot(spike_times_soma, 0.0*np.ones(len(spike_times_soma)),
         's', color='k', markersize=2)
axD.legend(fontsize=fs)

# synaptic weights
fig2, axA = plt.subplots(1, 1)
for i in np.arange(2, 200, 10):
    index = np.intersect1d(np.where(senders == i), np.where(targets == 1))
    if not len(index) == 0:
        axA.step(times[index], weights[index], label='pg_{}'.format(i - 2),
                 lw=lw)

axA.set_title('Synaptic weights of Urbanczik synapses')
axA.set_xlabel('time [ms]', fontsize=fs)
axA.set_ylabel('weight', fontsize=fs)
axA.legend(fontsize=fs - 4)
plt.show()
