# -*- coding: utf-8 -*-
#
# layers.py
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

# Run as python layers.py > layers.log

import matplotlib.pyplot as plt
import nest
import numpy as np

# seed NumPy RNG to ensure identical results for runs with random placement
np.random.seed(1234567)


def beautify_layer(l, fig=plt.gcf(), xlabel=None, ylabel=None,
                   xlim=None, ylim=None, xticks=None, yticks=None, dx=0, dy=0):
    """Assume either x and ylims/ticks given or none"""

    top = nest.GetStatus(l)[0]['topology']
    ctr = top['center']
    ext = top['extent']

    if xticks is None:
        if 'rows' in top:
            dx = float(ext[0]) / top['columns']
            dy = float(ext[1]) / top['rows']
            xticks = ctr[0] - ext[0] / 2. + dx / 2. + dx * np.arange(
                top['columns'])
            yticks = ctr[1] - ext[1] / 2. + dy / 2. + dy * np.arange(
                top['rows'])

    if xlim is None:
        xlim = [ctr[0] - ext[0] / 2. - dx / 2., ctr[0] + ext[
            0] / 2. + dx / 2.]  # extra space so extent is visible
        ylim = [ctr[1] - ext[1] / 2. - dy / 2., ctr[1] + ext[1] / 2. + dy / 2.]
    else:
        ext = [xlim[1] - xlim[0], ylim[1] - ylim[0]]

    ax = fig.gca()
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    ax.set_aspect('equal', 'box')
    ax.set_xticks(xticks)
    ax.set_yticks(yticks)
    ax.grid(True)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    return


# --------------------------------------------------

nest.ResetKernel()

#{ layer1 #}
import nest.topology as tp

l = tp.CreateLayer({'rows': 5,
                    'columns': 5,
                    'elements': 'iaf_psc_alpha'})
#{ end #}

fig = tp.PlotLayer(l, nodesize=50)
beautify_layer(l, fig, xlabel='x-axis (columns)', ylabel='y-axis (rows)')
ax = fig.gca()
tx = []
for r in range(5):
    tx.append(ax.text(0.65, 0.4 - r * 0.2, str(r),
                      horizontalalignment='center',
                      verticalalignment='center'))
    tx.append(ax.text(-0.4 + r * 0.2, 0.65, str(r),
                      horizontalalignment='center',
                      verticalalignment='center'))

# For bbox_extra_artists, see
# https://github.com/matplotlib/matplotlib/issues/351
plt.savefig('../user_manual_figures/layer1.png', bbox_inches='tight',
            bbox_extra_artists=tx)

print("#{ layer1s.log #}")
#{ layer1s #}
print(nest.GetStatus(l)[0]['topology'])
#{ end #}
print("#{ end.log #}")

print("#{ layer1p.log #}")
#{ layer1p #}
nest.PrintNetwork(depth=3)
#{ end #}
print("#{ end.log #}")

# --------------------------------------------------

nest.ResetKernel()

#{ layer2 #}
l = tp.CreateLayer({'rows': 5,
                    'columns': 5,
                    'extent': [2.0, 0.5],
                    'elements': 'iaf_psc_alpha'})
#{ end #}

fig = tp.PlotLayer(l, nodesize=50)
beautify_layer(l, fig, xlabel='x-axis (columns)', ylabel='y-axis (rows)')
ax = fig.gca()
tx = []

for r in range(5):
    tx.append(fig.gca().text(1.25, 0.2 - r * 0.1, str(r),
                             horizontalalignment='center',
                             verticalalignment='center'))
    tx.append(fig.gca().text(-0.8 + r * 0.4, 0.35, str(r),
                             horizontalalignment='center',
                             verticalalignment='center'))

# See https://github.com/matplotlib/matplotlib/issues/351
plt.savefig('../user_manual_figures/layer2.png', bbox_inches='tight',
            bbox_extra_artists=tx)

# --------------------------------------------------

nest.ResetKernel()

#{ layer3 #}
l1 = tp.CreateLayer({'rows': 5, 'columns': 5, 'elements': 'iaf_psc_alpha'})
l2 = tp.CreateLayer({'rows': 5, 'columns': 5, 'elements': 'iaf_psc_alpha',
                     'center': [-1., 1.]})
l3 = tp.CreateLayer({'rows': 5, 'columns': 5, 'elements': 'iaf_psc_alpha',
                     'center': [1.5, 0.5]})
#{ end #}

fig = tp.PlotLayer(l1, nodesize=50)
tp.PlotLayer(l2, nodesize=50, nodecolor='g', fig=fig)
tp.PlotLayer(l3, nodesize=50, nodecolor='r', fig=fig)
beautify_layer(l1, fig, xlabel='x-axis (columns)', ylabel='y-axis (rows)',
               xlim=[-1.6, 2.1], ylim=[-0.6, 1.6],
               xticks=np.arange(-1.4, 2.05, 0.2),
               yticks=np.arange(-0.4, 1.45, 0.2))

plt.savefig('../user_manual_figures/layer3.png', bbox_inches='tight')

# --------------------------------------------------

nest.ResetKernel()

#{ layer3a #}
nc, nr = 5, 3
d = 0.1
l = tp.CreateLayer({'columns': nc, 'rows': nr, 'elements': 'iaf_psc_alpha',
                    'extent': [nc * d, nr * d], 'center': [nc * d / 2., 0.]})

#{ end #}

fig = tp.PlotLayer(l, nodesize=100)
plt.plot(0, 0, 'x', markersize=20, c='k', mew=3)
plt.plot(nc * d / 2, 0, 'o', markersize=20, c='k', mew=3, mfc='none',
         zorder=100)
beautify_layer(l, fig, xlabel='x-axis (columns)', ylabel='y-axis (rows)',
               xticks=np.arange(0., 0.501, 0.05),
               yticks=np.arange(-0.15, 0.151, 0.05),
               xlim=[-0.05, 0.55], ylim=[-0.2, 0.2])

plt.savefig('../user_manual_figures/layer3a.png', bbox_inches='tight')

# --------------------------------------------------

nest.ResetKernel()

#{ layer4 #}
import numpy as np

pos = [[np.random.uniform(-0.5, 0.5), np.random.uniform(-0.5, 0.5)]
       for j in range(50)]
l = tp.CreateLayer({'positions': pos,
                    'elements': 'iaf_psc_alpha'})
#{ end #}

fig = tp.PlotLayer(l, nodesize=50)
beautify_layer(l, fig, xlabel='x-axis (columns)', ylabel='y-axis (rows)',
               xlim=[-0.55, 0.55], ylim=[-0.55, 0.55],
               xticks=[-0.5, 0., 0.5], yticks=[-0.5, 0., 0.5])

plt.savefig('../user_manual_figures/layer4.png', bbox_inches='tight')

# --------------------------------------------------

nest.ResetKernel()

#{ layer4_3d #}
import numpy as np

pos = [[np.random.uniform(-0.5, 0.5), np.random.uniform(-0.5, 0.5),
        np.random.uniform(-0.5, 0.5)] for j in range(200)]
l = tp.CreateLayer({'positions': pos,
                    'elements': 'iaf_psc_alpha'})
#{ end #}

fig = tp.PlotLayer(l, nodesize=50)

plt.savefig('../user_manual_figures/layer4_3d.png', bbox_inches='tight')

# --------------------------------------------------

nest.ResetKernel()

#{ player #}
lp = tp.CreateLayer({'rows': 1, 'columns': 5, 'extent': [5., 1.],
                     'elements': 'iaf_psc_alpha',
                     'edge_wrap': True})
#{ end #}

# fake plot with layer on line and circle
clist = [(0, 0, 1), (0.35, 0, 1), (0.6, 0, 1), (0.8, 0, 1), (1.0, 0, 1)]
fig = plt.figure()
ax1 = fig.add_subplot(221)
ax1.plot([0.5, 5.5], [0, 0], 'k-', lw=2)
ax1.scatter(range(1, 6), [0] * 5, s=200, c=clist)
ax1.set_xlim([0, 6])
ax1.set_ylim([-0.5, 1.25])
ax1.set_aspect('equal', 'box')
ax1.set_xticks([])
ax1.set_yticks([])
for j in range(1, 6):
    ax1.text(j, 0.5, str('(%d,0)' % (j - 3)),
             horizontalalignment='center', verticalalignment='bottom')

ax1a = fig.add_subplot(223)
ax1a.plot([0.5, 5.5], [0, 0], 'k-', lw=2)
ax1a.scatter(range(1, 6), [0] * 5, s=200,
             c=[clist[0], clist[1], clist[2], clist[2], clist[1]])
ax1a.set_xlim([0, 6])
ax1a.set_ylim([-0.5, 1.25])
ax1a.set_aspect('equal', 'box')
ax1a.set_xticks([])
ax1a.set_yticks([])
for j in range(1, 6):
    ax1a.text(j, 0.5, str('(%d,0)' % (j - 3)),
              horizontalalignment='center', verticalalignment='bottom')

ax2 = fig.add_subplot(122)
phic = np.arange(0., 2 * np.pi + 0.5, 0.1)
r = 5. / (2 * np.pi)
ax2.plot(r * np.cos(phic), r * np.sin(phic), 'k-', lw=2)
phin = np.arange(0., 4.1, 1.) * 2 * np.pi / 5
ax2.scatter(r * np.sin(phin), r * np.cos(phin), s=200,
            c=[clist[0], clist[1], clist[2], clist[2], clist[1]])
ax2.set_xlim([-1.3, 1.3])
ax2.set_ylim([-1.2, 1.2])
ax2.set_aspect('equal', 'box')
ax2.set_xticks([])
ax2.set_yticks([])
for j in range(5):
    ax2.text(1.4 * r * np.sin(phin[j]), 1.4 * r * np.cos(phin[j]),
             str('(%d,0)' % (j + 1 - 3)),
             horizontalalignment='center', verticalalignment='center')

plt.savefig('../user_manual_figures/player.png', bbox_inches='tight')

# --------------------------------------------------

nest.ResetKernel()

#{ layer6 #}
l = tp.CreateLayer({'rows': 1, 'columns': 2,
                    'elements': ['iaf_cond_alpha', 'poisson_generator']})
#{ end #}

print("#{ layer6 #}")
nest.PrintNetwork(depth=3)
print("#{ end #}")

# --------------------------------------------------

nest.ResetKernel()

#{ layer7 #}
l = tp.CreateLayer({'rows': 1, 'columns': 2,
                    'elements': ['iaf_cond_alpha', 10, 'poisson_generator',
                                 'noise_generator', 2]})
#{ end #}

print("#{ layer7 #}")
nest.PrintNetwork(depth=3)
print("#{ end #}")

# --------------------------------------------------

nest.ResetKernel()

#{ layer10 #}
for lyr in ['L23', 'L4', 'L56']:
    nest.CopyModel('iaf_psc_alpha', lyr + 'pyr')
    nest.CopyModel('iaf_psc_alpha', lyr + 'in', {'V_th': -52.})
l = tp.CreateLayer({'rows': 20, 'columns': 20, 'extent': [0.5, 0.5],
                    'elements': ['L23pyr', 3, 'L23in',
                                 'L4pyr', 3, 'L4in',
                                 'L56pyr', 3, 'L56in']})
#{ end #}

# --------------------------------------------------

nest.ResetKernel()

#{ vislayer #}
l = tp.CreateLayer({'rows': 21, 'columns': 21,
                    'elements': 'iaf_psc_alpha'})
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 0.4}},
            'kernel': {'gaussian': {'p_center': 1.0, 'sigma': 0.15}}}
tp.ConnectLayers(l, l, conndict)
fig = tp.PlotLayer(l, nodesize=80)
ctr = tp.FindCenterElement(l)
tp.PlotTargets(ctr, l, fig=fig,
               mask=conndict['mask'], kernel=conndict['kernel'],
               src_size=250, tgt_color='red', tgt_size=20,
               kernel_color='green')
#{ end #}
plt.savefig('../user_manual_figures/vislayer.png', bbox_inches='tight')
