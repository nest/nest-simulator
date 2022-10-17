# -*- coding: utf-8 -*-
#
# nodes_source_target.py
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
Showcase of PlotTargets, PlotSources, GetTargetNodes, GetSourceNodes
--------------------------------------------------------------------

Anno Christopher Kurth, INM-6
"""

import nest
from matplotlib import pyplot as plt

# create a spatial population
s_nodes = nest.Create('iaf_psc_alpha',
                      positions=nest.spatial.grid(shape=[11, 11],
                                                  extent=[11., 11.]))
# connectivity specifications with a mask
conndict = {'rule': 'pairwise_bernoulli', 'p': 1.,
            'mask': {'rectangular': {'lower_left': [-1.0, -1.0],
                                     'upper_right': [1.0, 1.0]},
                     'anchor': [3., 3.]}
            }

# get center element
center_neuron = nest.FindCenterElement(s_nodes)

# connect population s_nodes with itself according to the given
# specifications
nest.Connect(s_nodes, s_nodes, conndict)

# Plot target neurons of center neuron
fig = nest.PlotLayer(s_nodes, nodesize=80, nodecolor='coral')
nest.PlotTargets(center_neuron, s_nodes, fig=fig)
plt.title('Target neurons of center neuron')
plt.show()

# Plot source neurons of center neuron
fig = nest.PlotLayer(s_nodes, nodesize=80, nodecolor='coral')
nest.PlotSources(s_nodes, center_neuron, fig=fig)
plt.title('Source neurons of center neuron')
plt.show()

print('Target neurons of center neuron')
print(nest.GetTargetNodes(center_neuron, s_nodes))

print('Source neurons of center neuron')
print(nest.GetSourceNodes(s_nodes, center_neuron))

print('Positions of target neurons of center neuron')
print(nest.GetTargetPositions(center_neuron, s_nodes))

print('Positions of source neurons of center neuron')
print(nest.GetSourcePositions(s_nodes, center_neuron))
