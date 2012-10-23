#! /usr/bin/env python
#
# gaussex.py
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

Create two layers of 30x30 elements and connect
them using a Gaussian probabilistic kernel, visualize.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB/Simula
'''

import nest, pylab
import nest.topology as topo
nest.ResetKernel()

# create two test layers
a = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_neuron'})
b = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_neuron'})

topo.ConnectLayers(a, b, {'connection_type': 'divergent',
                         'mask': {'circular': {'radius': 3.0}},
                         'kernel': {'gaussian': {'p_center': 1.0, 'sigma': 0.5}},
                         'weights': 1.0,
                         'delays': 1.0})

pylab.clf()

# plot targets of neurons in different grid locations
for ctr in [[15,15], [0,0]]:
    
    # obtain node id for center
    ctr_id = topo.GetElement(a, ctr)

    # get positions of targets
    tpos = zip(*[topo.GetPosition([n])[0] for n in
                 nest.GetStatus(nest.FindConnections(ctr_id), 'target')])

    # scatter-plot
    pylab.scatter(tpos[0], tpos[1], 20, zorder = 10)

    # mark sender position with transparent red circle
    ctrpos = topo.GetPosition(ctr_id)[0]
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=0.15, zorder = 99,
                                       fc = 'r', alpha = 0.4, ec = 'none'))
 
    # mark mask position with open red circle
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=0.5, zorder = 2,
                                       fc = 'none', ec = 'r', lw=3, ls='dashed'))
    # mark mask position with open red circle
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=1.0, zorder = 2,
                                       fc = 'none', ec = 'r', lw=2, ls='dashed'))
    # mark mask position with open red circle

# mark layer edge
pylab.gca().add_patch(pylab.Rectangle((-1.5,-1.5), 3.0, 3.0, zorder = 1,
                                      fc = 'none', ec = 'k', lw=3))

# beautify
pylab.axes().set_xticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.axes().set_yticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.grid(True)
pylab.axis([-2.0, 2.0, -2.0, 2.0])
pylab.axes().set_aspect('equal', 'box')

#pylab.savefig('gaussex.pdf')
