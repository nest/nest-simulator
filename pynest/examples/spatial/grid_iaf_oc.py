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

"""
Create three populations of iaf_psc_alpha neurons on a 4x3 grid, each with different center
-------------------------------------------------------------------------------------------

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
"""

import nest
import matplotlib.pyplot as plt
import numpy as np

for ctr in [(0.0, 0.0), (-2.0, 2.0), (0.5, 1.0)]:
    plt.figure()
    nest.ResetKernel()

    l1 = nest.Create('iaf_psc_alpha',
                     positions=nest.spatial.grid(shape=[4, 3], extent=[2., 1.5],
                                                 center=ctr))

    nest.PlotLayer(l1, nodesize=50, fig=plt.gcf())

    # beautify
    plt.axis([-3, 3, -3, 3])
    plt.axes().set_aspect('equal', 'box')
    plt.axes().set_xticks(np.arange(-3.0, 3.1, 1.0))
    plt.axes().set_yticks(np.arange(-3.0, 3.1, 1.0))
    plt.grid(True)
    plt.xlabel(f'4 Columns, Extent: 1.5, Center: {ctr[0]:.1f}')
    plt.ylabel(f'2 Rows, Extent: 1.0, Center: {ctr[1]:.1f}')

    plt.show()
    # plt.savefig('grid_iaf_oc_{}_{}.png'.format(ctr[0], ctr[1]))
