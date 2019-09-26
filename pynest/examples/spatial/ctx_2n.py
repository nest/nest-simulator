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
NEST Spatial Example

Create a 4x3 grid with one pyramidal cell and one interneuron at each position,
visualize.

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
'''

import nest
import pylab

nest.ResetKernel()

nest.CopyModel('iaf_psc_alpha', 'pyr')
nest.CopyModel('iaf_psc_alpha', 'in')

pos = nest.spatial.grid(shape=[4,3], extent=[2., 1.5])

ctx_pyr = nest.Create('pyr', positions=pos)
ctx_in = nest.Create('in', positions=pos)

nest.PrintNodes()

# extract position information
ppyr = nest.GetPosition(ctx_pyr)
pin = nest.GetPosition(ctx_in)

ppyr_x = pylab.array([x for x, y in ppyr])
ppyr_y = pylab.array([y for x, y in ppyr])

pin_x = pylab.array([x for x, y in pin])
pin_y = pylab.array([y for x, y in pin])

# plot
pylab.clf()
pylab.plot(pin_x - 0.05, ppyr_y - 0.05, 'bo', markersize=20,
           label='Pyramidal', zorder=2)
pylab.plot(pin_x + 0.05, pin_y + 0.05, 'ro', markersize=20,
           label='Interneuron', zorder=2)
pylab.plot(pin_x, ppyr_y, 'o', markerfacecolor=(0.7, 0.7, 0.7),
           markersize=60, markeredgewidth=0, zorder=1, label='_nolegend_')

# beautify
pylab.axis([-1.0, 1.0, -1.0, 1.0])
pylab.axes().set_aspect('equal', 'box')
pylab.axes().set_xticks((-0.75, -0.25, 0.25, 0.75))
pylab.axes().set_yticks((-0.5, 0, 0.5))
pylab.grid(True)
pylab.xlabel('4 Columns, Extent: 1.5')
pylab.ylabel('3 Rows, Extent: 1.0')
pylab.legend(numpoints=1)

pylab.show()

# pylab.savefig('ctx_2n.png')
