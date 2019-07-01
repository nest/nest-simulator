# -*- coding: utf-8 -*-
#
# test_3d_exp.py
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
NEST Topology Module

EXPERIMENTAL example of 3d layer.

3d layers are currently not supported, use at your own risk!

Hans Ekkehard Plesser, UMB

This example uses the function GetChildren, which is deprecated. A deprecation
warning is therefore issued. For details about deprecated functions, see
documentation.
'''

import nest
import random
import nest.topology as topo
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

nest.ResetKernel()

# generate list of 1000 (x,y,z) triplets
pos = [[random.uniform(-0.5, 0.5), random.uniform(-0.5, 0.5),
        random.uniform(-0.5, 0.5)]
       for j in range(1000)]

l1 = topo.CreateLayer(
    {'extent': [1.5, 1.5, 1.5],  # must specify 3d extent AND center
     'center': [0., 0., 0.],
     'positions': pos,
     'elements': 'iaf_psc_alpha'})

# visualize

# extract position information, transpose to list of x, y and z positions
xpos, ypos, zpos = zip(*topo.GetPosition(l1))
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(xpos, ypos, zpos, s=15, facecolor='b', edgecolor='none')

# Gaussian connections in full volume [-0.75,0.75]**3
topo.ConnectLayers(l1, l1,
                   {'connection_type': 'divergent', 'allow_autapses': False,
                    'mask': {'volume': {'lower_left': [-0.75, -0.75, -0.75],
                                        'upper_right': [0.75, 0.75, 0.75]}},
                    'kernel': {'exponential':
                               {'c': 0., 'a': 1., 'tau': 0.25}}})

# show connections from center element
# sender shown in red, targets in green
ctr_gid = topo.FindCenterElement(l1)
ctr_index = ctr_gid - 1
ctr = l1[ctr_index:ctr_index + 1]
xtgt, ytgt, ztgt = zip(*topo.GetTargetPositions(ctr, l1)[0])
xctr, yctr, zctr = topo.GetPosition(ctr)
ax.scatter([xctr], [yctr], [zctr], s=40, facecolor='r', edgecolor='none')
ax.scatter(xtgt, ytgt, ztgt, s=40, facecolor='g', edgecolor='g')

tgts = topo.GetTargetNodes(ctr, l1)[0]
distances = topo.Distance(ctr, l1)
tgt_distances = [d for i, d in enumerate(distances) if i + 1 in tgts]

plt.figure()
plt.hist(tgt_distances, 25)
plt.show()
