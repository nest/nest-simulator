# -*- coding: utf-8 -*-
#
# receptors_and_current.py
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
    'C_m': 10.0,    # [pF] Capacitance
    'g_C': 0.0,     # soma has no parent
    'g_L': 1.,      # [nS] Leak conductance
    'e_L': -70.0    # [mV] leak reversal
}
dend_params = {
    'C_m': 0.1,     # [pF] Capacitance
    'g_C': 0.1,     # [nS] Coupling conductance to parent (soma here)
    'g_L': 0.1,     # [nS] Leak conductance
    'e_L': -70.0    # [mV] leak reversal
}

# create a model with three compartments
cm = nest.Create('cm_main')
nest.SetStatus(cm, {'compartments': {"comp_idx": 0, "parent_idx": -1, "params": soma_params}})
nest.SetStatus(cm, {'compartments': {"comp_idx": 1, "parent_idx":  0, "params": dend_params}})
nest.SetStatus(cm, {'compartments': {"comp_idx": 2, "parent_idx":  0, "params": dend_params}})

# spike threshold
nest.SetStatus(cm, {'V_th': -50.})

# add GABA receptor in compartment 0 (soma)
# syn_idx_GABA = nest.AddReceptor(cm, 0, "GABA", {})
nest.SetStatus(cm, {'receptors': {"comp_idx": 0, "receptor_type": "GABA"}})
syn_idx_GABA = 0
# add AMPA receptor in compartment 1
# syn_idx_AMPA = nest.AddReceptor(cm, 1, "AMPA", {})
nest.SetStatus(cm, {'receptors': {"comp_idx": 1, "receptor_type": "AMPA", "params": {}}})
syn_idx_AMPA = 1
# add AMPA+NMDA receptor in compartment 2
# syn_idx_NMDA = nest.AddReceptor(cm, 2, "AMPA_NMDA", {})
nest.SetStatus(cm, {'receptors': {"comp_idx": 2, "receptor_type": "AMPA_NMDA"}})
syn_idx_NMDA = 2

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

nest.Simulate(400.)
# nest.Simulate(.2)
res = nest.GetStatus(mm, 'events')[0]

plt.plot(res['times'], res['v_comp0'], c='b', label='v_comp0')
plt.plot(res['times'], res['v_comp1'], c='r', label='v_comp1')
plt.plot(res['times'], res['v_comp2'], c='g', label='v_comp2')
plt.legend()

plt.show()
