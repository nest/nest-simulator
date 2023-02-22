# -*- coding: utf-8 -*-
#
# static_injector_neuron.py
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

import nest

spike_times = [1.0, 2.0, 3.34]
syn_weight = 1.0
syn_delay = 0.5
dt = 0.1

nest.set(resolution=dt)

parrot = nest.Create("parrot_neuron", n=1)
#parrot = nest.Create("parrot_neuron_ps", n=1)

injector = nest.Create("spike_generator",
                       n=1,
                       params={"spike_times": spike_times,
                               "precise_times": True,
                               "allow_offgrid_times": False}
                       )

nest.CopyModel("static_synapse",
               "syn_static",
               {"weight": syn_weight, "delay": syn_delay}
               )

s_rec = nest.Create("spike_recorder")

nest.Connect(injector, parrot, syn_spec='syn_static')
nest.Connect(parrot, s_rec)

nest.Simulate(5.0)

kernel_status = nest.GetKernelStatus()
print(f"number of nodes: {kernel_status['network_size']:,}")
print(f"number of connections: {kernel_status['num_connections']:,}")
print(f'local spike counter: {nest.GetKernelStatus("local_spike_counter"):,}')
print(f"spike recorder: {s_rec.events['times']}")
