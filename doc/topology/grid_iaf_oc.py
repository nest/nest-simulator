# -*- coding: utf-8 -*-
#
# grid_iaf_oc.py
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

Create three layers of 4x3 iaf_psc_alpha neurons, each with different center.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import pylab
import time
import nest
import nest.topology as topo

pylab.ion()

for ctr in [(0.0, 0.0), (-2.0, 2.0), (0.5, 1.0)]:
    nest.ResetKernel()
    pylab.clf()
    l1 = topo.CreateLayer({'columns': 4, 'rows': 3,
                           'extent': [2.0, 1.5],
                           'center': ctr,
                           'elements': 'iaf_psc_alpha'})

    topo.PlotLayer(l1, nodesize=50, fig=pylab.gcf())

    # beautify
    pylab.axis([-3, 3, -3, 3])
    pylab.axes().set_aspect('equal', 'box')
    pylab.axes().set_xticks(pylab.arange(-3.0, 3.1, 1.0))
    pylab.axes().set_yticks(pylab.arange(-3.0, 3.1, 1.0))
    pylab.grid(True)
    pylab.xlabel('4 Columns, Extent: 1.5, Center: %.1f' % ctr[0])
    pylab.ylabel('2 Rows, Extent: 1.0, Center: %.1f' % ctr[1])

    pylab.draw()
