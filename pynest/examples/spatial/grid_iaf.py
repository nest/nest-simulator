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

"""
Create a population of iaf_psc_alpha neurons on a 4x3 grid
-----------------------------------------------------------

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
"""

import nest
import matplotlib.pyplot as plt

nest.ResetKernel()

l1 = nest.Create('iaf_psc_alpha',
                 positions=nest.spatial.grid(shape=[4, 3], extent=[2., 1.5]))

nest.PrintNodes()

nest.PlotLayer(l1, nodesize=50)

# beautify
plt.axis([-1.0, 1.0, -0.75, 0.75])
plt.axes().set_aspect('equal', 'box')
plt.axes().set_xticks((-0.75, -0.25, 0.25, 0.75))
plt.axes().set_yticks((-0.5, 0, 0.5))
plt.grid(True)
plt.xlabel('4 Columns, Extent: 1.5')
plt.ylabel('2 Rows, Extent: 1.0')

plt.show()

# plt.savefig('grid_iaf.png')
