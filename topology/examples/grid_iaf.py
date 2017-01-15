# -*- coding: utf-8 -*-
#
# grid_iaf.py
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

Create layer of 4x3 iaf_psc_alpha neurons, visualize

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import nest
import pylab
import nest.topology as topo

pylab.ion()

nest.ResetKernel()

l1 = topo.CreateLayer({'columns': 4, 'rows': 3,
                       'extent': [2.0, 1.5],
                       'elements': 'iaf_psc_alpha'})

nest.PrintNetwork()
nest.PrintNetwork(2)
nest.PrintNetwork(2, l1)

topo.PlotLayer(l1, nodesize=50)

# beautify
pylab.axis([-1.0, 1.0, -0.75, 0.75])
pylab.axes().set_aspect('equal', 'box')
pylab.axes().set_xticks((-0.75, -0.25, 0.25, 0.75))
pylab.axes().set_yticks((-0.5, 0, 0.5))
pylab.grid(True)
pylab.xlabel('4 Columns, Extent: 1.5')
pylab.ylabel('2 Rows, Extent: 1.0')

# pylab.savefig('grid_iaf.png')
