# -*- coding: utf-8 -*-
#
# test_stdp_dopa.py
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

# Begin Documentation
# Name: testsuite::test_stdp_dopa - script to test stdp_dopamine_synapse model implementing dopamine-dependent spike-timing dependent plasticity as defined in [1], based on [2].
# Two neurons, which fire poisson like, are connected by a stdp_dopamine_synapse. Dopamine is release by a third neuron, which also fires poisson like.
#
# author: Wiebke Potjans
# date: October 2010

import numpy as np
import nest

nest.ResetKernel()
nest.SetKernelStatus({'overwrite_files': True}) # set to True to permit overwriting

delay = 1.     # the delay in ms

w_ex = 45.
g = 3.83
w_in = -w_ex * g

K = 10000
f_ex = 0.8
K_ex = f_ex * K
K_in = (1.0 - f_ex) * K

nu_ex = 10.0#2.
nu_in = 10.0#2.

pg_ex = nest.Create("poisson_generator")
nest.SetStatus(pg_ex, {"rate": K_ex * nu_ex})

pg_in = nest.Create("poisson_generator")
nest.SetStatus(pg_in, {"rate": K_in * nu_in})

sd = nest.Create("spike_detector")
nest.SetStatus([sd], [ {
    "label": "spikes",
    "withtime": True,
    "withgid": True,
    "to_file": True,
    } ])

neuron1 = nest.Create("iaf_psc_alpha")
neuron2 = nest.Create("iaf_psc_alpha")
dopa_neuron = nest.Create("iaf_psc_alpha")
nest.SetStatus(neuron1, {"tau_syn_ex": 0.3, "tau_syn_in": 0.3, "tau_minus": 20.0})
nest.SetStatus(neuron2, {"tau_syn_ex": 0.3, "tau_syn_in": 0.3, "tau_minus": 20.0})

vt = nest.Create("volume_transmitter")

nest.Connect(pg_ex, neuron1, params=w_ex, delay=delay)
nest.Connect(pg_ex, neuron2, params=w_ex, delay=delay)
nest.Connect(pg_ex, dopa_neuron, params=w_ex, delay=delay)

nest.Connect(pg_in, neuron1, params=w_in, delay=delay)
nest.Connect(pg_in, neuron2, params=w_in, delay=delay)
nest.Connect(pg_in, dopa_neuron, params=w_in, delay=delay)

nest.Connect(neuron1, sd)
nest.Connect(neuron2, sd)
nest.Connect(dopa_neuron, sd)

nest.CopyModel("stdp_dopamine_synapse", "dopa", {"vt": vt[0], "weight": 35., "delay": delay})
nest.CopyModel("static_synapse", "static", {"delay": delay})

nest.Connect(dopa_neuron, vt, model="static")
nest.Connect(neuron1, neuron2, model="dopa")

if nest.GetStatus(neuron2)[0]['local']:
    filename = 'weight.gdf'
    fname = open(filename, 'w')
else:
    raise

T = 1000.0
dt = 10.0

weight = None
for t in np.arange(0, T + dt, dt):
    if nest.GetStatus(neuron2)[0]['local']:
        weight = nest.GetStatus(nest.FindConnections(neuron1, synapse_model="dopa"))[0]['weight']
        print(weight)
        weightstr = str(weight)
        timestr = str(t)
        data = timestr + ' ' + weightstr + '\n'
        fname.write(data)
        nest.Simulate(dt)

if nest.GetStatus(neuron2)[0]['local']:
    print("expected weight at T=1000 ms: 28.6125 pA")
    print("weight at last event: " + str(weight) + " pA")
    fname.close()


