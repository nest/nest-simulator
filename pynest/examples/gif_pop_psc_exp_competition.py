# -*- coding: utf-8 -*-
#
# gif_pop_psc_exp_competition.py
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

"""Simulate a Winner-Take-All network that switches stochastically.
Compares population model simulation to populations of single neurons.
Similar to figure 7B,E of the paper:
       Towards a theory of cortical columns: From spiking neurons to
       interacting neural populations of finite size
       Tilo Schwalger, Moritz Deger, Wulfram Gerstner
       PLoS Comput Biol 2017
       https://doi.org/10.1371/journal.pcbi.1005507"""



import numpy as np
import nest
import pylab
import time


### top-level switches ###

Adaptation = True
num_threads = 3
DoSingleNeurons = True


### parameter definitions ###

NE = 400
NI = 200
mu = 36.    # mV
Ja_in = 0.  # mV
tau_a = 1000. #ms
delay = 1.  # ms

dt_pop  = 0.2  # ms
dt_nrn  = 0.01 # ms
dt_plot = 250. # ms
T  = 100000.   # ms

if Adaptation:
    # with adaptation case
    Ja_ex = 0.1 # mV s
    muE = mu + 0.5   # mV
    wE =  0.096 # mV
    wI = -0.384 # mV
    w_fudge = 0.9385  # factor to multiply synaptic weights with to reproduce figure
else:
    # without adaptation case
    Ja_ex = 0.  # mV
    muE = mu    # mV
    wE = 0.0624 # mV
    wI =-0.2496 # mV
    w_fudge = 0.9385  # factor to multiply synaptic weights with to reproduce figure


### parameter translation ###
params_default = nest.GetDefaults('gif_pop_psc_exp')
params_default['Delta_V'] = 2.5
params_gif = {}
params_to_copy = ['V_T_star', 'tau_sfa', 'q_sfa', 'V_reset', 'I_e', 'V_m', \
    'E_L', 'tau_syn_ex', 'Delta_V', 't_ref', 'lambda_0', 'C_m', 'tau_syn_in']
for key in params_to_copy:
    params_gif[key] = params_default[key]
params_gif['g_L'] = params_default['C_m'] / params_default['tau_m']

# compute postsynaptic scaling in pA, as needed to connect NEST models
wE_I = params_default['C_m']   / params_default['tau_syn_ex'] * wE * w_fudge
wI_I = params_default['C_m']   / params_default['tau_syn_in'] * wI * w_fudge

# compute resistance R to map mu [mV] to input current I_e [pA]
R = params_default['tau_m'] / params_default['C_m']
params_nrnE = { 'I_e': muE / R, 'tau_sfa': [tau_a], 'q_sfa': [Ja_ex / (tau_a*1e-3)] }
params_nrnI = { 'I_e': mu  / R, 'tau_sfa': [tau_a], 'q_sfa': [Ja_in / (tau_a*1e-3)] }

# append population size to paramter dicts for populations
params_popE = params_nrnE.copy()
params_popE['N'] = NE
params_popI = params_nrnI.copy()
params_popI['N'] = NI



### simulation script ###

# set up the population model
nest.ResetKernel()
nest.SetKernelStatus({'resolution': dt_pop, 'local_num_threads': num_threads})

popE1, popE2, popI = nest.Create('gif_pop_psc_exp', 3)
nest.SetStatus( [popE1, popE2], params_popE )
nest.SetStatus( [popI], params_popI )

nest.Connect( [popE1], [popE1], syn_spec={'weight': wE_I, 'delay': delay}  )
nest.Connect( [popE1],  [popI], syn_spec={'weight': wE_I, 'delay': delay}  )
nest.Connect( [popE2], [popE2], syn_spec={'weight': wE_I, 'delay': delay}  )
nest.Connect( [popE2],  [popI], syn_spec={'weight': wE_I, 'delay': delay}  )
nest.Connect( [popI],   [popI], syn_spec={'weight': wI_I, 'delay': delay}  )
nest.Connect( [popI],  [popE1], syn_spec={'weight': wI_I, 'delay': delay}  )
nest.Connect( [popI],  [popE2], syn_spec={'weight': wI_I, 'delay': delay}  )

mmpop = nest.Create('multimeter')
nest.SetStatus(mmpop, {'record_from': ['n_events'], 'time_in_steps': True, 'interval': dt_pop})
nest.Connect( mmpop, [popE1, popE2, popI] )

# simulate
t0_pop = time.time()
nest.Simulate( T )
t1_pop = time.time()

# things for processing the data
dbin = int( dt_plot / dt_pop )
bins = np.arange( 0, int(  T / dt_pop ) +1 , dbin )
times = bins * dt_pop * 1e-3

# process the output #
mmpopdata = nest.GetStatus(mmpop)[0]['events']
poprate = []
for gid in [popE1, popE2, popI]:
    N_ = nest.GetStatus([gid])[0]['N']
    gid_idx = mmpopdata['senders'] == gid
    rates_idx = mmpopdata['n_events'][gid_idx].astype(float) / N_ / (dt_pop*1e-3)
    poprate.append( [ rates_idx[i:i+dbin].mean() for i in bins[:-1] ] )
poprate = np.array(poprate)



if DoSingleNeurons:
    # set up the single neuron model
    nest.ResetKernel()
    nest.SetKernelStatus({'resolution': dt_nrn, 'local_num_threads': num_threads})

    nest.SetDefaults('gif_psc_exp', params_gif)
    nrnE1 = nest.Create('gif_psc_exp', NE)
    nrnE2 = nest.Create('gif_psc_exp', NE)
    nrnI  = nest.Create('gif_psc_exp', NI)
    nest.SetStatus( nrnE1 + nrnE2, params_nrnE )
    nest.SetStatus( nrnI, params_nrnI )

    nest.Connect( nrnE1, nrnE1, syn_spec={'weight': wE_I, 'delay': delay}  )
    nest.Connect( nrnE1, nrnI , syn_spec={'weight': wE_I, 'delay': delay}  )
    nest.Connect( nrnE2, nrnE2, syn_spec={'weight': wE_I, 'delay': delay}  )
    nest.Connect( nrnE2, nrnI , syn_spec={'weight': wE_I, 'delay': delay}  )
    nest.Connect( nrnI , nrnI , syn_spec={'weight': wI_I, 'delay': delay}  )
    nest.Connect( nrnI , nrnE1, syn_spec={'weight': wI_I, 'delay': delay}  )
    nest.Connect( nrnI , nrnE2, syn_spec={'weight': wI_I, 'delay': delay}  )

    sdnrnE1, sdnrnE2, sdnrnI = nest.Create('spike_detector', 3)
    nest.Connect( nrnE1, [sdnrnE1] )
    nest.Connect( nrnE2, [sdnrnE2] )
    nest.Connect( nrnI,  [sdnrnI]  )
    nest.SetStatus([sdnrnE1, sdnrnE2, sdnrnI], {'withgid': False, 'time_in_steps': True})

    # simulate
    t0_nrn = time.time()
    nest.Simulate( T )
    t1_nrn = time.time()

    # things for processing the data
    dbin = int( dt_plot / dt_nrn)
    bins = np.arange( 0, int(  T / dt_nrn ) +1 , dbin )
    times = bins * dt_nrn * 1e-3

    # process the output #
    nrnrate = []
    for sd, N_ in zip( [sdnrnE1, sdnrnE2, sdnrnI], [NE, NE, NI] ):
        spike_times = nest.GetStatus([sd])[0]['events']['times']
        nspikes = np.histogram( spike_times, bins=bins )[0]
        nrnrate.append( nspikes.astype(float) / N_ / (dt_nrn*1e-3*dbin) )
    nrnrate = np.array(nrnrate)



# print basic info

print 'mean rates pop:', poprate.mean(axis=1), '+/-', poprate.std(axis=1)
print 'simtime pop [s]:', t1_pop - t0_pop
if DoSingleNeurons:
    print 'mean rates nrn:', nrnrate.mean(axis=1), '+/-', nrnrate.std(axis=1)
    print 'simtime nrn [s]:', t1_nrn - t0_nrn

# visualize

pylab.figure()
pylab.title('pop E')
pylab.plot(times[:-1], poprate[0])
pylab.plot(times[:-1], poprate[1])
pylab.xlabel('time [s]')
pylab.ylabel('rate [1/s]')

pylab.figure()
pylab.title('pop I')
pylab.plot(times[:-1], poprate[2])
pylab.xlabel('time [s]')
pylab.ylabel('rate [1/s]')

if DoSingleNeurons:
    pylab.figure()
    pylab.title('nrn E')
    pylab.plot(times[:-1], nrnrate[0])
    pylab.plot(times[:-1], nrnrate[1])
    pylab.xlabel('time [s]')
    pylab.ylabel('rate [1/s]')

    pylab.figure()
    pylab.title('nrn I')
    pylab.plot(times[:-1], nrnrate[2])
    pylab.xlabel('time [s]')
    pylab.ylabel('rate [1/s]')

pylab.show()
