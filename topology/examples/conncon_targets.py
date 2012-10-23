#! /usr/bin/env python
#
# conncon_targets.py
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
NEST Topology Module Example

Create two 30x30 layers of iaf_neurons, 
connect with convergent projection and rectangular mask,
visualize connections from source perspective. 

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB/Simula
'''

import nest, pylab
import nest.topology as topo
nest.ResetKernel()

# create two test layers
a = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_neuron', 'edge_wrap': True})
b = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_neuron', 'edge_wrap': True})

topo.ConnectLayers(a, b, {'connection_type': 'convergent',
                         'mask': {'rectangular': {'lower_left': [-0.2, -0.5], 'upper_right': [0.2, 0.5]}},
                         'kernel': 0.5,
                         'weights': {'uniform': {'min': 0.5, 'max': 2.0}},
                         'delays': 1.0})

pylab.clf()

# plot sources of neurons in different grid locations
for ctr in [[15,15],[0,0]]:
    
    # obtain node id for center
    ctr_id = topo.GetElement(a, ctr)

    # get positions of targets
    tpos = zip(*[topo.GetPosition([n])[0] for n in 
                 nest.GetStatus(nest.FindConnections(ctr_id), 'target')])

    # scatter-plot
    pylab.scatter(tpos[0], tpos[1], 20, zorder = 10)

    # mark sender position with transparent red circle
    ctrpos = pylab.array(topo.GetPosition(ctr_id)[0])
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=0.1, zorder = 99,
                                       fc = 'r', alpha = 0.4, ec = 'none'))
 
    # mark mask position with open red rectangle
    pylab.gca().add_patch(pylab.Rectangle(ctrpos-(0.2,0.5), 0.4, 1.0, zorder = 1,
                                       fc = 'none', ec = 'r', lw=3))

# mark layer edge
pylab.gca().add_patch(pylab.Rectangle((-1.5,-1.5), 3.0, 3.0, zorder = 1,
                                      fc = 'none', ec = 'k', lw=3))

# beautify
pylab.axes().set_xticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.axes().set_yticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.grid(True)
pylab.axis([-2.0, 2.0, -2.0, 2.0])
pylab.axes().set_aspect('equal', 'box')
pylab.title('Connection targets')

# pylab.savefig('conncon_targets.pdf')
