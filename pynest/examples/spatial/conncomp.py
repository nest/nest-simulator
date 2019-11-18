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
NEST Spatial Example

Create two populations of pyramidal cells and two populations of interneurons
on a 30x30 grid. Connect with two projections, one pyr->pyr, one pyr->in, and
visualize.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import nest
import pylab

nest.ResetKernel()
nest.set_verbosity('M_WARNING')

nest.CopyModel('iaf_psc_alpha', 'pyr')
nest.CopyModel('iaf_psc_alpha', 'in')

# same positions for all populations
pos = nest.spatial.grid(shape=[30, 30], extent=[3., 3.])

a_pyr = nest.Create('pyr', positions=pos)
a_in = nest.Create('in', positions=pos)

b_pyr = nest.Create('pyr', positions=pos)
b_in = nest.Create('in', positions=pos)

nest.Connect(a_pyr, b_pyr, {'rule': 'pairwise_bernoulli',
                            'p': 0.5,
                            'mask': {'circular': {'radius': 0.5}}})

nest.Connect(a_pyr, b_in, {'rule': 'pairwise_bernoulli',
                           'p': 0.2,
                           'mask': {'circular': {'radius': 1.}}})

pylab.clf()

# plot targets of neurons in different grid locations

# obtain node id for center: pick first node of composite
ctr_index = 30 * 15 + 15
ctr_id = a_pyr[ctr_index:ctr_index + 1]

# get all projection targets of center neuron
conn = nest.GetConnections(ctr_id)
tgts = conn.get('target')

tpyr = nest.GetTargetPositions(ctr_id, b_pyr)[0]
tin = nest.GetTargetPositions(ctr_id, b_in)[0]

tpyr_x = pylab.array([x for x, y in tpyr])
tpyr_y = pylab.array([y for x, y in tpyr])
tin_x = pylab.array([x for x, y in tin])
tin_y = pylab.array([y for x, y in tin])

# scatter-plot
pylab.scatter(tpyr_x - 0.02, tpyr_y - 0.02, 20, 'b', zorder=10)
pylab.scatter(tin_x + 0.02, tin_y + 0.02, 20, 'r', zorder=10)

# mark locations with background grey circle
pylab.plot(tpyr_x, tpyr_y, 'o', markerfacecolor=(0.7, 0.7, 0.7),
           markersize=10, markeredgewidth=0, zorder=1, label='_nolegend_')
pylab.plot(tin_x, tin_y, 'o', markerfacecolor=(0.7, 0.7, 0.7),
           markersize=10, markeredgewidth=0, zorder=1, label='_nolegend_')

# mark sender position with transparent red circle
ctrpos = nest.GetPosition(ctr_id)
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
pylab.show()
