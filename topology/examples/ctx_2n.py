# -*- coding: utf-8 -*-
#
# ctx_2n.py
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

Create layer of 4x3 elements compose of one
pyramidal cell and one interneuron, visualize

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB

This example uses the function GetLeaves, which is deprecated. A deprecation
warning is therefore issued. For details about deprecated functions, see
documentation.
'''

import nest
import nest.topology as topo
import pylab
import random

pylab.ion()

nest.ResetKernel()

nest.CopyModel('iaf_psc_alpha', 'pyr')
nest.CopyModel('iaf_psc_alpha', 'in')
ctx = topo.CreateLayer({'columns': 4, 'rows': 3,
                        'extent': [2.0, 1.5],
                        'elements': ['pyr', 'in']})

nest.hl_api.PrintNetwork()

nest.hl_api.PrintNetwork(2)

nest.hl_api.PrintNetwork(2, ctx)

# ctx_leaves is a work-around until NEST 3.0 is released
ctx_leaves = nest.hl_api.GetLeaves(ctx)[0]

# extract position information
ppyr = pylab.array(
    tuple(zip(*[topo.GetPosition([n])[0] for n in ctx_leaves
                if nest.GetStatus([n], 'model')[0] == 'pyr'])))
pin = pylab.array(
    tuple(zip(*[topo.GetPosition([n])[0] for n in ctx_leaves
                if nest.GetStatus([n], 'model')[0] == 'in'])))
# plot
pylab.clf()
pylab.plot(ppyr[0] - 0.05, ppyr[1] - 0.05, 'bo', markersize=20,
           label='Pyramidal', zorder=2)
pylab.plot(pin[0] + 0.05, pin[1] + 0.05, 'ro', markersize=20,
           label='Interneuron', zorder=2)
pylab.plot(ppyr[0], ppyr[1], 'o', markerfacecolor=(0.7, 0.7, 0.7),
           markersize=60, markeredgewidth=0, zorder=1, label='_nolegend_')

# beautify
pylab.axis([-1.0, 1.0, -0.75, 0.75])
pylab.axes().set_aspect('equal', 'box')
pylab.axes().set_xticks((-0.75, -0.25, 0.25, 0.75))
pylab.axes().set_yticks((-0.5, 0, 0.5))
pylab.grid(True)
pylab.xlabel('4 Columns, Extent: 1.5')
pylab.ylabel('3 Rows, Extent: 1.0')
pylab.legend(numpoints=1)

# pylab.savefig('ctx_2n.png')
