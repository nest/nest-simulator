# -*- coding: utf-8 -*-
#
# test_stdp_symmetric.py
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
# Name: testsuite::test_stdp_symmetric - script to test stdp_symmetric_ model
# implementing a symmetric stdp learning rule
# Two neurons, which fire poisson like, are connected by a
# stdp_symmetric_synapse model.
#
# author: Ankur Sinha
# date: January 2016

import numpy as np
import nest
import math
import matplotlib
matplotlib.use("Agg")
from matplotlib import pyplot as plt

output_graph_name = 'symmetric_stdp.png'

plus_deltaWs = []
minus_deltaWs = []
minus_postWs = []
plus_postWs = []
minus_deltaTs = [x for x in range(-100, 0)]
plus_deltaTs = [x for x in range(1, 101)]
weight_pre = 60.

# Run various tests over different deltaTs
for deltaT in minus_deltaTs:
    nest.ResetKernel()
    nest.SetKernelStatus(
        {
            'resolution': 0.01,
        }
    )
    nest.set_verbosity('M_INFO')
    #################################################################
    # To get the kplus value up
    first_spike_set = [(10. + 2. * math.fabs(deltaT) * i)
                       for i in range(0, 1000)]
    later_spike_set = [pre_spike + math.fabs(deltaT)
                       for pre_spike in first_spike_set]
    # an extra spike to equal the depression in second case
    # later_spike_set.append(later_spike_set[-1] + math.fabs(deltaT))
    pre_generator = nest.Create('spike_generator', 1,
                                {'spike_times': later_spike_set}
                                )
    post_generator = nest.Create('spike_generator', 1,
                                 {'spike_times': first_spike_set}
                                 )
    pre_neuron = nest.Create('parrot_neuron')
    post_neuron = nest.Create('parrot_neuron')

    nest.Connect(pre_generator, pre_neuron)
    nest.Connect(post_generator, post_neuron)

    # set up the synapse
    syn_spec_synapse = {'weight': weight_pre, 'Wmax': 100.,
                        'kappa': 0.1, 'eta': 0.001,
                        'tau': 50.,
                        'model': 'stdp_symmetric_synapse'}

    nest.Connect(pre_neuron, post_neuron, syn_spec=syn_spec_synapse)
    connections = nest.GetConnections(source=pre_neuron, target=post_neuron)

    sim_time = max(first_spike_set[-1], later_spike_set[-1])
    nest.Simulate(sim_time)
    # record the pre at this point and then simulate for one last iteration
    weight_post = nest.GetStatus(connections, "weight")

    # How many spike trains did we have?
    delta_w = (weight_post[0] - weight_pre)
    minus_deltaWs.append(100 * delta_w/weight_pre)
    minus_postWs.append(weight_post[0])

for deltaT in plus_deltaTs:
    nest.ResetKernel()
    nest.SetKernelStatus(
        {
            'resolution': 0.01,
        }
    )
    nest.set_verbosity('M_INFO')
    # To get the kplus value up
    first_spike_set = [(10. + 2. * math.fabs(deltaT) * i)
                       for i in range(0, 1000)]
    later_spike_set = [pre_spike + math.fabs(deltaT)
                       for pre_spike in first_spike_set]
    # an extra presynaptic spike
    first_spike_set.append(later_spike_set[-1] + math.fabs(deltaT))
    pre_generator = nest.Create('spike_generator', 1,
                                {'spike_times': first_spike_set}
                                )
    post_generator = nest.Create('spike_generator', 1,
                                 {'spike_times': later_spike_set}
                                 )

    pre_neuron = nest.Create('parrot_neuron')
    post_neuron = nest.Create('parrot_neuron')

    nest.Connect(pre_generator, pre_neuron)
    nest.Connect(post_generator, post_neuron)

    # set up the synapse
    syn_spec_synapse = {'weight': weight_pre, 'Wmax': 100.,
                        'kappa': 0.1, 'eta': 0.001,
                        'tau': 50.,
                        'model': 'stdp_symmetric_synapse'}

    nest.Connect(pre_neuron, post_neuron, syn_spec=syn_spec_synapse)
    connections = nest.GetConnections(source=pre_neuron, target=post_neuron)

    sim_time = max(first_spike_set[-1], later_spike_set[-1])
    nest.Simulate(sim_time)
    # record the pre at this point and then simulate for one last iteration
    weight_post = nest.GetStatus(connections, "weight")

    # How many spike trains did we have?
    delta_w = (weight_post[0] - weight_pre)
    plus_deltaWs.append(100 * delta_w/weight_pre)
    plus_postWs.append(weight_post[0])

deltaWs = minus_deltaWs + plus_deltaWs
deltaTs = minus_deltaTs + plus_deltaTs
postWs = minus_postWs + plus_postWs

# Print to files for inspection
if len(deltaWs) != len(deltaTs):
    print("Somethings wrong here.")
    print("Length of deltaWs: {}".format(len(deltaWs)))
    print("Length of deltaTs: {}".format(len(deltaTs)))
else:
    filename = 'minus_output.gdf'
    fname = open(filename, 'w')
    for index in range(0, len(minus_deltaWs)):
        printstatement = (str(minus_deltaTs[index]) + "\t" +
                          str(minus_postWs[index]) + "\t" +
                          str(minus_deltaWs[index]) + "\n")
        fname.write(printstatement)
    fname.close()
    filename = 'plus_output.gdf'
    fname = open(filename, 'w')
    for index in range(0, len(plus_deltaWs)):
        printstatement = (str(plus_deltaTs[index]) + "\t" +
                          str(plus_postWs[index]) + "\t" +
                          str(plus_deltaWs[index]) + "\n")
        fname.write(printstatement)
    fname.close()

    # Plot using matplotlib - why not?
    fig, ax1 = plt.subplots()
    plt.xlabel('delta T')
    plt.ylabel('delta W percentage')
    ax1.axhline(0, linewidth=2.0)
    ax1.plot(minus_deltaTs, minus_deltaWs, linewidth=2.0)
    ax1.plot(plus_deltaTs, plus_deltaWs, linewidth=2.0)
    plt.savefig(output_graph_name, format="png")
    plt.close()
