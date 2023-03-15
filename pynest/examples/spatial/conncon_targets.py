# -*- coding: utf-8 -*-
#
# conncon_targets.py
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
Convergent projection and rectangular mask, from source perspective
--------------------------------------------------------------------

Create two populations of iaf_psc_alpha neurons on a 30x30 grid
Connect the two populations with convergent projection and rectangular mask, and
visualize connections from source perspective

BCCN Tutorial @ CNS*09
Hans Ekkehard Plesser, UMB
"""

import nest
import matplotlib.pyplot as plt
import numpy as np

nest.ResetKernel()

pos = nest.spatial.grid(shape=[30, 30], extent=[3., 3.], edge_wrap=True)

########################################################################
# create and connect two populations
a = nest.Create('iaf_psc_alpha', positions=pos)
b = nest.Create('iaf_psc_alpha', positions=pos)

cdict = {'rule': 'pairwise_bernoulli',
         'p': 0.5,
         'use_on_source': True,
         'mask': {'rectangular': {'lower_left': [-0.2, -0.5],
                                  'upper_right': [0.2, 0.5]}}}

nest.Connect(a, b,
             conn_spec=cdict,
             syn_spec={'weight': nest.random.uniform(0.5, 2.)})

#####################################################################
# first, clear existing figure, get current figure
plt.clf()
fig = plt.gcf()

# plot targets of two source neurons into same figure, with mask
for src_index in [30 * 15 + 15, 0]:
    # obtain node id for center
    src = a[src_index:src_index + 1]
    nest.PlotTargets(src, b, mask=cdict['mask'], fig=fig)

# beautify
plt.axes().set_xticks(np.arange(-1.5, 1.55, 0.5))
plt.axes().set_yticks(np.arange(-1.5, 1.55, 0.5))
plt.grid(True)
plt.axis([-2.0, 2.0, -2.0, 2.0])
plt.axes().set_aspect('equal', 'box')
plt.title('Connection targets')

plt.show()

# plt.savefig('conncon_targets.pdf')
