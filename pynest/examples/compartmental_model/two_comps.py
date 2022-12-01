# -*- coding: utf-8 -*-
#
# two_comps.py
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

r"""Constructing and simulating compartmental models with active and passive dendrites
---------------------------------------------------------------------------------------
This example demonstrates how to initialize compartmental models. It creates
two models with two compartments, once with active channels in the dendritic
compartment and once without. It then adds and excitatory receptors with AMPA
and NMDA components to both the somatic and dendritic compartment.

The output shows the evolution of the state variables present in the model.

:Authors: WAM Wybo
"""

import nest
import matplotlib.pyplot as plt

nest.ResetKernel()

soma_params = {
    # passive parameters
    'C_m': 89.245535,           # [pF] Capacitance
    'g_C': 0.0,                 # soma has no parent
    'g_L': 8.924572508,         # [nS] Leak conductance
    'e_L': -75.0,               # [mV] leak reversal
    # ion channel params
    'gbar_Na': 4608.698576715,  # [nS] Na maximal conductance
    'e_Na': 60.,                # [mV] Na reversal
    'gbar_K': 956.112772900,    # [nS] K maximal conductance
    'e_K': -90.                 # [mV] K reversal
}

###############################################################################
# by default, active conducances are set to zero, so we don't need to specify
# them explicitely
dend_params_passive = {
    # passive parameters
    'C_m': 1.929929,
    'g_C': 1.255439494,
    'g_L': 0.192992878,
    'e_L': -75.0,
}
dend_params_active = {
    # passive parameters
    'C_m': 1.929929,            # [pF] Capacitance
    'g_C': 1.255439494,         # [nS] Coupling conductance to parent (soma here)
    'g_L': 0.192992878,         # [nS] Leak conductance
    'e_L': -70.0,               # [mV] leak reversal
    # ion channel params
    'gbar_Na': 17.203212493,    # [nS] Na maximal conductance
    'e_Na': 60.,                # [mV] Na reversal
    'gbar_K': 11.887347450,     # [nS] K maximal conductance
    'e_K': -90.                 # [mV] K reversal
}

###############################################################################
# create a neuron model with a passive dendritic compartment
cm_pas = nest.Create('cm_default')
cm_pas.compartments = [
    {"parent_idx": -1, "params": soma_params},
    {"parent_idx":  0, "params": dend_params_passive}
]
###############################################################################
# create a neuron model with an active dendritic compartment
cm_act = nest.Create('cm_default')
cm_act.compartments = [
    {"parent_idx": -1, "params": soma_params},
    {"parent_idx":  0, "params": dend_params_active}
]

###############################################################################
# set spike thresholds
cm_pas.V_th = -50.
cm_act.V_th = -50.

###############################################################################
# add somatic and dendritic receptor to passive dendrite model
cm_pas.receptors = [
    {"comp_idx": 0, "receptor_type": "AMPA_NMDA"},
    {"comp_idx": 1, "receptor_type": "AMPA_NMDA"}
]
syn_idx_soma_pas = 0
syn_idx_dend_pas = 1
###############################################################################
# add somatic and dendritic receptor to active dendrite model
cm_act.receptors = [
    {"comp_idx": 0, "receptor_type": "AMPA_NMDA"},
    {"comp_idx": 1, "receptor_type": "AMPA_NMDA"}
]
syn_idx_soma_act = 0
syn_idx_dend_act = 1

###############################################################################
# create a two spike generators
sg_soma = nest.Create('spike_generator', 1, {'spike_times': [10., 13., 16.]})
sg_dend = nest.Create('spike_generator', 1, {'spike_times': [70., 73., 76.]})

###############################################################################
# connect spike generators to passive dendrite model (weight in nS)
nest.Connect(sg_soma, cm_pas, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 5., 'delay': 0.5, 'receptor_type': syn_idx_soma_pas})
nest.Connect(sg_dend, cm_pas, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 2., 'delay': 0.5, 'receptor_type': syn_idx_dend_pas})
###############################################################################
# connect spike generators to active dendrite model (weight in nS)
nest.Connect(sg_soma, cm_act, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 5., 'delay': 0.5, 'receptor_type': syn_idx_soma_act})
nest.Connect(sg_dend, cm_act, syn_spec={
    'synapse_model': 'static_synapse', 'weight': 2., 'delay': 0.5, 'receptor_type': syn_idx_dend_act})

###############################################################################
# create multimeters to record compartment voltages and various state variables
rec_list = ['v_comp0', 'v_comp1',
            'm_Na_0', 'h_Na_0', 'n_K_0', 'm_Na_1', 'h_Na_1', 'n_K_1',
            'g_r_AN_AMPA_1', 'g_d_AN_AMPA_1', 'g_r_AN_NMDA_1', 'g_d_AN_NMDA_1']
mm_pas = nest.Create('multimeter', 1, {'record_from': rec_list, 'interval': .1})
mm_act = nest.Create('multimeter', 1, {'record_from': rec_list, 'interval': .1})
###############################################################################
# connect the multimeters to the respective neurons
nest.Connect(mm_pas, cm_pas)
nest.Connect(mm_act, cm_act)

###############################################################################
# simulate the models
nest.Simulate(160.)
res_pas = nest.GetStatus(mm_pas, 'events')[0]
res_act = nest.GetStatus(mm_act, 'events')[0]

plt.figure('voltage')
###############################################################################
# plot voltage for somatic compartment
ax_soma = plt.subplot(121)
ax_soma.plot(res_pas['times'], res_pas['v_comp0'], c='b', label='passive dend')
ax_soma.plot(res_act['times'], res_act['v_comp0'], c='r', label='active dend')
ax_soma.set_xlabel(r'$t$ (ms)')
ax_soma.set_ylabel(r'$v_{soma}$ (mV)')
ax_soma.set_ylim((-90., 40.))
ax_soma.legend(loc=0)
###############################################################################
# plot voltage for dendritic compartment
ax_dend = plt.subplot(122)
ax_dend.plot(res_pas['times'], res_pas['v_comp1'], c='b', label='passive dend')
ax_dend.plot(res_act['times'], res_act['v_comp1'], c='r', label='active dend')
ax_dend.set_xlabel(r'$t$ (ms)')
ax_dend.set_ylabel(r'$v_{dend}$ (mV)')
ax_dend.set_ylim((-90., 40.))
ax_dend.legend(loc=0)

plt.figure('channel state variables')
###############################################################################
# plot ion channel state variables for somatic compartment
ax_soma = plt.subplot(121)
ax_soma.plot(res_pas['times'], res_pas['m_Na_0'], c='b', label='m_Na passive dend')
ax_soma.plot(res_pas['times'], res_pas['h_Na_0'], c='r', label='h_Na passive dend')
ax_soma.plot(res_pas['times'], res_pas['n_K_0'], c='g', label='n_K passive dend')
ax_soma.plot(res_act['times'], res_act['m_Na_0'], c='b', ls='--', lw=2., label='m_Na active dend')
ax_soma.plot(res_act['times'], res_act['h_Na_0'], c='r', ls='--', lw=2., label='h_Na active dend')
ax_soma.plot(res_act['times'], res_act['n_K_0'], c='g', ls='--', lw=2., label='n_K active dend')
ax_soma.set_xlabel(r'$t$ (ms)')
ax_soma.set_ylabel(r'svar')
ax_soma.set_ylim((0., 1.))
ax_soma.legend(loc=0)
###############################################################################
# plot ion channel state variables for dendritic compartment
ax_dend = plt.subplot(122)
ax_dend.plot(res_pas['times'], res_pas['m_Na_1'], c='b', label='m_Na passive dend')
ax_dend.plot(res_pas['times'], res_pas['h_Na_1'], c='r', label='h_Na passive dend')
ax_dend.plot(res_pas['times'], res_pas['n_K_1'], c='g', label='n_K passive dend')
ax_dend.plot(res_act['times'], res_act['m_Na_1'], c='b', ls='--', lw=2., label='m_Na active dend')
ax_dend.plot(res_act['times'], res_act['h_Na_1'], c='r', ls='--', lw=2., label='h_Na active dend')
ax_dend.plot(res_act['times'], res_act['n_K_1'], c='g', ls='--', lw=2., label='n_K active dend')
ax_dend.set_xlabel(r'$t$ (ms)')
ax_dend.set_ylabel(r'svar')
ax_dend.set_ylim((0., 1.))
ax_dend.legend(loc=0)

plt.figure('dendritic synapse conductances')
###############################################################################
# plot synapse state variables for dendritic compartment
ax_dend = plt.gca()
ax_dend.plot(res_pas['times'], res_pas['g_r_AN_AMPA_1'] + res_pas['g_d_AN_AMPA_1'], c='b', label='AMPA passive dend')
ax_dend.plot(res_pas['times'], res_pas['g_r_AN_NMDA_1'] + res_pas['g_d_AN_NMDA_1'], c='r', label='NMDA passive dend')
ax_dend.plot(res_act['times'], res_act['g_r_AN_AMPA_1'] + res_act['g_d_AN_AMPA_1'],
             c='b', ls='--', lw=2., label='AMPA active dend')
ax_dend.plot(res_act['times'], res_act['g_r_AN_NMDA_1'] + res_act['g_d_AN_NMDA_1'],
             c='r', ls='--', lw=2., label='NMDA active dend')
ax_dend.legend(loc=0)

plt.tight_layout()
plt.show()
