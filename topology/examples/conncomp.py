# -*- coding: utf-8 -*-
#
# conncomp.py
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

Create two 30x30 layers with nodes composed of one pyramidal cell
and one interneuron. Connect with two projections, one pyr->pyr, one
pyr->in, and visualize.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import nest
import nest.topology as topo
import pylab

pylab.ion()


nest.ResetKernel()
nest.set_verbosity('M_WARNING')

# create two test layers
nest.CopyModel('iaf_psc_alpha', 'pyr')
nest.CopyModel('iaf_psc_alpha', 'in')

a = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': ['pyr', 'in']})
b = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': ['pyr', 'in']})

topo.ConnectLayers(a, b, {'connection_type': 'divergent',
                          'sources': {'model': 'pyr'},
                          'targets': {'model': 'pyr'},
                          'mask': {'circular': {'radius': 0.5}},
                          'kernel': 0.5,
                          'weights': 1.0,
                          'delays': 1.0})
topo.ConnectLayers(a, b, {'connection_type': 'divergent',
                          'sources': {'model': 'pyr'},
                          'targets': {'model': 'in'},
                          'mask': {'circular': {'radius': 1.0}},
                          'kernel': 0.2,
                          'weights': 1.0,
                          'delays': 1.0})

pylab.clf()

# plot targets of neurons in different grid locations
for ctr in [[15, 15]]:
    # obtain node id for center: pick first node of composite
    ctr_id = topo.GetElement(a, ctr)

    # get all projection targets of center neuron
    tgts = [ci[1] for ci in nest.GetConnections(ctr_id)]

    # get positions of targets
    tpyr = pylab.array(tuple(zip(*[topo.GetPosition([n])[0] for n in tgts
                                   if
                                   nest.GetStatus([n], 'model')[0] == 'pyr'])))
    tin = pylab.array(tuple(zip(*[topo.GetPosition([n])[0] for n in tgts
                                  if
                                  nest.GetStatus([n], 'model')[0] == 'in'])))

    # scatter-plot
    pylab.scatter(tpyr[0] - 0.02, tpyr[1] - 0.02, 20, 'b', zorder=10)
    pylab.scatter(tin[0] + 0.02, tin[1] + 0.02, 20, 'r', zorder=10)

    # mark locations with background grey circle
    pylab.plot(tpyr[0], tpyr[1], 'o', markerfacecolor=(0.7, 0.7, 0.7),
               markersize=10, markeredgewidth=0, zorder=1, label='_nolegend_')
    pylab.plot(tin[0], tin[1], 'o', markerfacecolor=(0.7, 0.7, 0.7),
               markersize=10, markeredgewidth=0, zorder=1, label='_nolegend_')

    # mark sender position with transparent red circle
    ctrpos = topo.GetPosition(ctr_id)[0]
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=0.15, zorder=99,
                                       fc='r', alpha=0.4, ec='none'))

    # mark mask positions with open red/blue circles
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=0.5, zorder=2,
                                       fc='none', ec='b', lw=3))
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=1.0, zorder=2,
                                       fc='none', ec='r', lw=3))

# mark layer edge
pylab.gca().add_patch(pylab.Rectangle((-1.5, -1.5), 3.0, 3.0, zorder=1,
                                      fc='none', ec='k', lw=3))

# beautify
pylab.axes().set_xticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.axes().set_yticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.grid(True)
pylab.axis([-1.6, 1.6, -1.6, 1.6])
pylab.axes().set_aspect('equal', 'box')
