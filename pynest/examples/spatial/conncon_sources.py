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
NEST Spatial Example

Create two populations of iaf_psc_alpha neurons on a 30x30 grid,
connect with convergent projection and rectangular mask,
visualize connection from target perspective.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import nest
import pylab

nest.ResetKernel()
nest.set_verbosity('M_WARNING')

pos = nest.spatial.grid(shape=[30, 30], extent=[3., 3.], edge_wrap=True)

# create and connect two populations
a = nest.Create('iaf_psc_alpha', positions=pos)
b = nest.Create('iaf_psc_alpha', positions=pos)

nest.Connect(a, b,
             conn_spec={'rule': 'pairwise_bernoulli',
                        'p': 0.5,
                        'use_on_source': True,
                        'mask': {'rectangular': {'lower_left': [-0.2, -0.5],
                                                 'upper_right': [0.2, 0.5]}}},
             syn_spec={'weight': nest.random.uniform(0.5, 2.)})
pylab.clf()

# plot sources of neurons in different grid locations
for tgt_index in [30 * 15 + 15, 0]:
    # obtain node id for center
    tgt = a[tgt_index:tgt_index + 1]

    # obtain list of outgoing connections for ctr
    spos = nest.GetTargetPositions(tgt, b)[0]

    spos_x = pylab.array([x for x, y in spos])
    spos_y = pylab.array([y for x, y in spos])

    print(spos_x)
    print(spos_y)

    # scatter-plot
    pylab.scatter(spos_x, spos_y, 20, zorder=10)

    # mark sender position with transparent red circle
    ctrpos = pylab.array(nest.GetPosition(tgt))
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
pylab.show()
