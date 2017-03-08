# -*- coding: utf-8 -*-
#
# connex_ew.py
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

Create two 30x30 layers of iaf_psc_alpha neurons with edge_wrap,
connect with circular mask, flat probability,
visualize.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import pylab
import nest
import nest.topology as topo

pylab.ion()

nest.ResetKernel()

# create two test layers
a = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_psc_alpha'})
b = topo.CreateLayer({'columns': 30, 'rows': 30, 'extent': [3.0, 3.0],
                      'elements': 'iaf_psc_alpha', 'edge_wrap': True})

conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 0.5}},
            'kernel': 0.5,
            'weights': {'uniform': {'min': 0.5, 'max': 2.0}},
            'delays': 1.0}
topo.ConnectLayers(a, b, conndict)

pylab.clf()

# plot targets of neurons in different grid locations

# first, clear existing figure, get current figure
pylab.clf()
fig = pylab.gcf()

# plot targets of two source neurons into same figure, with mask
for src_pos in [[15, 15], [0, 0]]:
    # obtain node id for center
    src = topo.GetElement(a, src_pos)
    topo.PlotTargets(src, b, mask=conndict['mask'], fig=fig)

# beautify
pylab.axes().set_xticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.axes().set_yticks(pylab.arange(-1.5, 1.55, 0.5))
pylab.grid(True)
pylab.axis([-2.0, 2.0, -2.0, 2.0])
pylab.axes().set_aspect('equal', 'box')
pylab.title('Connection targets')

# pylab.savefig('connex_ew.pdf')
