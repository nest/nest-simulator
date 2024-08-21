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

r"""Constructing and simulating compartmental models with different receptor types
------------------------------------------------------------------------------
This example demonstrates how to initialize a three-compartment model with
different receptor types. Each compartment receives a different receptor.

The output shows the voltage in each of the three compartments.

:Authors: WAM Wybo
"""

import matplotlib.pyplot as plt
import nest

nest.ResetKernel()

###############################################################################
# somatic and dendritic parameters
soma_params = {
    "C_m": 10.0,  # [pF] Capacitance
    "g_C": 0.0,  # soma has no parent
    "g_L": 1.0,  # [nS] Leak conductance
    "e_L": -70.0,  # [mV] leak reversal
    "v_comp": -70.0,  # [mV] voltage initialization
}
dend_params = {
    "C_m": 0.1,  # [pF] Capacitance
    "g_C": 0.1,  # [nS] Coupling conductance to parent (soma here)
    "g_L": 0.1,  # [nS] Leak conductance
    "e_L": -70.0,  # [mV] leak reversal
    "v_comp": -70.0,  # [mV] voltage initialization
}

###############################################################################
# create a model with three compartments
cm = nest.Create("cm_default")
cm.compartments = [
    {"parent_idx": -1, "params": soma_params},
    {"parent_idx": 0, "params": dend_params},
    {"parent_idx": 0, "params": dend_params},
]

###############################################################################
# spike threshold
nest.SetStatus(cm, {"V_th": -50.0})

###############################################################################
# - GABA receptor in compartment 0 (soma)
# - AMPA receptor in compartment 1
#   note that it is also possible to specify the receptor parameters, if we want
#   to overwrite the default values
# - AMPA+NMDA receptor in compartment 2
receptors = [
    {"comp_idx": 0, "receptor_type": "GABA"},
    {"comp_idx": 1, "receptor_type": "AMPA", "params": {"tau_r_AMPA": 0.2, "tau_d_AMPA": 3.0, "e_AMPA": 0.0}},
    {"comp_idx": 2, "receptor_type": "AMPA_NMDA"},
]
cm.receptors = receptors
###############################################################################
# receptors get assigned an index which corresponds to the order in which they
# are added. For clearer bookkeeping, we explicitly define these indices here.
syn_idx_GABA, syn_idx_AMPA, syn_idx_NMDA = 0, 1, 2

###############################################################################
# create three spike generators
sg1 = nest.Create("spike_generator", 1, {"spike_times": [101.0, 105.0, 106.0, 110.0, 150.0]})
sg2 = nest.Create(
    "spike_generator", 1, {"spike_times": [115.0, 155.0, 160.0, 162.0, 170.0, 254.0, 260.0, 272.0, 278.0]}
)
sg3 = nest.Create("spike_generator", 1, {"spike_times": [250.0, 255.0, 260.0, 262.0, 270.0]})

###############################################################################
# connect the spike generators to the receptors
nest.Connect(
    sg1, cm, syn_spec={"synapse_model": "static_synapse", "weight": 0.1, "delay": 0.5, "receptor_type": syn_idx_AMPA}
)
nest.Connect(
    sg2, cm, syn_spec={"synapse_model": "static_synapse", "weight": 0.2, "delay": 0.5, "receptor_type": syn_idx_NMDA}
)
nest.Connect(
    sg3, cm, syn_spec={"synapse_model": "static_synapse", "weight": 0.3, "delay": 0.5, "receptor_type": syn_idx_GABA}
)

###############################################################################
# create and connect a current generator to compartment 1
dcg = nest.Create("dc_generator", {"amplitude": 1.0})
nest.Connect(dcg, cm, syn_spec={"synapse_model": "static_synapse", "weight": 1.0, "delay": 0.1, "receptor_type": 1})

###############################################################################
# create and connect a multimeter to measure the three compartmental voltages
mm = nest.Create("multimeter", 1, {"record_from": ["v_comp0", "v_comp1", "v_comp2"], "interval": 0.1})
nest.Connect(mm, cm)

nest.Simulate(400.0)
res = nest.GetStatus(mm, "events")[0]

plt.plot(res["times"], res["v_comp0"], c="b", label="v_comp0")
plt.plot(res["times"], res["v_comp1"], c="r", label="v_comp1")
plt.plot(res["times"], res["v_comp2"], c="g", label="v_comp2")
plt.legend()

plt.show()
