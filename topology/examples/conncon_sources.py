# -*- coding: utf-8 -*-
#
# conncon_sources.py
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

Create two 30x30 layers of iaf_psc_alpha neurons,
connect with convergent projection and rectangular mask,
visualize connection from target perspective.

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
a = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_psc_alpha', 'edge_wrap': True})
b = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_psc_alpha', 'edge_wrap': True})

topo.ConnectLayers(a, b, {'connection_type': 'convergent',
                          'mask': {'rectangular': {'lower_left': [-0.2, -0.5],
                                                   'upper_right': [0.2, 0.5]}},
                          'kernel': 0.5,
                          'weights': {'uniform': {'min': 0.5, 'max': 2.0}},
                          'delays': 1.0})

pylab.clf()

# plot sources of neurons in different grid locations
for tgt_pos in [[15, 15], [0, 0]]:
    # obtain node id for center
    tgt = topo.GetElement(b, tgt_pos)

    # obtain list of outgoing connections for ctr
    # int() required to cast numpy.int64
    spos = tuple(zip(*[topo.GetPosition([int(conn[0])])[0] for conn in
                       nest.GetConnections(target=tgt)]))

    # scatter-plot
    pylab.scatter(spos[0], spos[1], 20, zorder=10)

    # mark sender position with transparent red circle
    ctrpos = pylab.array(topo.GetPosition(tgt)[0])
    pylab.gca().add_patch(pylab.Circle(ctrpos, radius=0.1, zorder=99,
                                       fc='r', alpha=0.4, ec='none'))

    # mark mask position with open red rectangle
    pylab.gca().add_patch(
        pylab.Rectangle(ctrpos - (0.2, 0.5), 0.4, 1.0, zorder=1,
                        fc='none', ec='r', lw=3))

# mark layer edge
pylab.gca().add_patch(pylab.Rectangle((-1.5, -1.5), 3.0, 3.0, zorder=1,
                                      fc='none', ec='k', lw=3))

# beautify
pylab.axes().set_xticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.axes().set_yticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.grid(True)
pylab.axis([-2.0, 2.0, -2.0, 2.0])
pylab.axes().set_aspect('equal', 'box')
pylab.title('Connection sources')
