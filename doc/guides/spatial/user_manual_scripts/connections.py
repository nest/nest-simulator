# -*- coding: utf-8 -*-
#
# connections.py
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

# create connectivity figures for spatial manual

import nest
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.axes3d import Axes3D
import numpy as np

# seed NumPy RNG to ensure identical results for runs with random placement
np.random.seed(7654321)


def beautify_layer(layer, fig=plt.gcf(), xlabel=None, ylabel=None,
                   xlim=None, ylim=None, xticks=None, yticks=None, dx=0, dy=0):
    """Assume either x and ylims/ticks given or none"""
    ctr = layer.spatial['center']
    ext = layer.spatial['extent']

    if xticks is None:
        if 'shape' in layer.spatial:
            dx = float(ext[0]) / layer.spatial['shape'][0]
            dy = float(ext[1]) / layer.spatial['shape'][1]
            xticks = ctr[0] - ext[0] / 2. + dx / 2. + dx * np.arange(
                layer.spatial['shape'][0])
            yticks = ctr[1] - ext[1] / 2. + dy / 2. + dy * np.arange(
                layer.spatial['shape'][1])

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
    ax.set_axisbelow(True)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    return


def conn_figure(fig, layer, connd, targets=None, showmask=True, kern=None,
                xticks=range(-5, 6), yticks=range(-5, 6),
                xlim=[-5.5, 5.5], ylim=[-5.5, 5.5]):
    if targets is None:
        targets = ((nest.FindCenterElement(layer), 'red'),)

    nest.PlotLayer(layer, fig=fig, nodesize=60)
    for src, clr in targets:
        if showmask:
            mask = connd['mask']
        else:
            mask = None
        nest.PlotTargets(src, layer, fig=fig, mask=mask, probability_parameter=kern,
                         src_size=250, tgt_color=clr, tgt_size=20, mask_color='red',
                         probability_cmap='Greens')

    beautify_layer(layer, fig,
                   xlim=xlim, ylim=ylim, xticks=xticks, yticks=yticks,
                   xlabel='', ylabel='')
    fig.gca().grid(False)


# -----------------------------------------------

# Simple connection

#{ conn1 #}
spatial_nodes = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11], extent=[11., 11.]))
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.]}}}
nest.Connect(spatial_nodes, spatial_nodes, conndict)
#{ end #}

fig = plt.figure()
fig.add_subplot(121)
conn_figure(fig, spatial_nodes, conndict,
            targets=((nest.FindCenterElement(spatial_nodes), 'red'),
                     (nest.FindNearestElement(spatial_nodes, [4., 5.])[0], 'yellow')))

# same another time, with periodic bcs
lpbc = nest.Create('iaf_psc_alpha',
                   positions=nest.spatial.grid(
                       shape=[11, 11], extent=[11., 11.],
                       edge_wrap=True))
nest.Connect(lpbc, lpbc, conndict)
fig.add_subplot(122)
conn_figure(fig, lpbc, conndict, showmask=False,
            targets=((nest.FindCenterElement(lpbc), 'red'),
                     (nest.FindNearestElement(lpbc, [4., 5.])[0], 'yellow')))

plt.savefig('../user_manual_figures/conn1.png', bbox_inches='tight')


# -----------------------------------------------

# free masks

def free_mask_fig(fig, loc, cdict):
    nest.ResetKernel()
    spatial_nodes = nest.Create('iaf_psc_alpha',
                                positions=nest.spatial.grid(shape=[11, 11], extent=[11., 11.]))
    nest.Connect(spatial_nodes, spatial_nodes, cdict)

    fig.add_subplot(loc)
    conn_figure(fig, spatial_nodes, cdict, xticks=range(-5, 6, 2), yticks=range(-5, 6, 2))


fig = plt.figure()

#{ conn2r #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.]}}}
#{ end #}
free_mask_fig(fig, 221, conndict)

#{ conn2c #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'circular': {'radius': 2.0}}}
#{ end #}
free_mask_fig(fig, 222, conndict)

#{ conn2d #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'doughnut': {'inner_radius': 1.5,
                                  'outer_radius': 3.}}}
#{ end #}
free_mask_fig(fig, 223, conndict)

#{ conn2e #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'elliptical': {'major_axis': 7.,
                                    'minor_axis': 4.}}}
#{ end #}
free_mask_fig(fig, 224, conndict)

plt.savefig('../user_manual_figures/conn2_a.png', bbox_inches='tight')

#-----------------------------------------------------------------------------#

fig = plt.figure()

#{ conn2ro #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.]},
                     'anchor': [-1.5, -1.5]}}
#{ end #}
free_mask_fig(fig, 221, conndict)

#{ conn2co #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'circular': {'radius': 2.0},
                     'anchor': [-2.0, 0.0]}}
#{ end #}
free_mask_fig(fig, 222, conndict)

#{ conn2do #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'doughnut': {'inner_radius': 1.5,
                                  'outer_radius': 3.},
                     'anchor': [1.5, 1.5]}}
#{ end #}
free_mask_fig(fig, 223, conndict)

#{ conn2eo #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'elliptical': {'major_axis': 7.,
                                    'minor_axis': 4.},
                     'anchor': [2.0, -1.0]}}
#{ end #}
free_mask_fig(fig, 224, conndict)

plt.savefig('../user_manual_figures/conn2_b.png', bbox_inches='tight')

#-----------------------------------------------------------------------------#

fig = plt.figure()

#{ conn2rr #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.],
                                     'azimuth_angle': 120.}}}
#{ end #}
free_mask_fig(fig, 121, conndict)

#{ conn2er #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'elliptical': {'major_axis': 7.,
                                    'minor_axis': 4.,
                                    'azimuth_angle': 45.}}}
#{ end #}
free_mask_fig(fig, 122, conndict)

plt.savefig('../user_manual_figures/conn2_c.png', bbox_inches='tight')

# -----------------------------------------------

# 3d masks


def conn_figure_3d(fig, layer, connd, targets=None, showmask=True,
                   xticks=range(-5, 6), yticks=range(-5, 6),
                   xlim=[-5.5, 5.5], ylim=[-5.5, 5.5]):
    if targets is None:
        targets = ((nest.FindCenterElement(layer), 'red'),)

    nest.PlotLayer(layer, fig=fig, nodesize=20, nodecolor=(.5, .5, 1.))
    for src, clr in targets:
        if showmask:
            mask = connd['mask']
        else:
            mask = None
        nest.PlotTargets(src, layer, fig=fig, mask=mask, probability_parameter=None,
                         src_size=250, tgt_color=clr, tgt_size=60,
                         probability_cmap='Greens')

    ax = fig.gca()
    # ax.set_aspect('equal', 'box')
    plt.draw()


def free_mask_3d_fig(fig, loc, cdict):
    nest.ResetKernel()
    spatial_nodes = nest.Create('iaf_psc_alpha',
                                positions=nest.spatial.grid(
                                    shape=[11, 11, 11],
                                    extent=[11., 11., 11.]))
    nest.Connect(spatial_nodes, spatial_nodes, cdict)

    fig.add_subplot(loc, projection='3d')
    conn_figure_3d(fig, spatial_nodes, cdict, xticks=range(-5, 6, 2),
                   yticks=range(-5, 6, 2))


fig = plt.figure()

#{ conn_3d_a #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'box': {'lower_left': [-2., -1., -1.],
                             'upper_right': [2., 1., 1.]}}}
#{ end #}
# free_mask_3d_fig(fig, 121, conndict)

#{ conn_3d_b #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'spherical': {'radius': 2.5}}}
#{ end #}
# free_mask_3d_fig(fig, 122, conndict)

#{ conn_3d_c #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'ellipsoidal': {'major_axis': 7.,
                                     'minor_axis': 4.,
                                     'polar_axis': 4.5}}}
#{ end #}

# plt.savefig('../user_manual_figures/conn_3d.png', bbox_inches='tight')


# -----------------------------------------------

# grid masks

def grid_mask_fig(fig, loc, cdict):
    nest.ResetKernel()
    spatial_nodes = nest.Create('iaf_psc_alpha',
                                positions=nest.spatial.grid(shape=[11, 11],
                                                            extent=[11., 11.]))
    nest.Connect(spatial_nodes, spatial_nodes, cdict)

    fig.add_subplot(loc)
    conn_figure(fig, spatial_nodes, cdict, xticks=range(-5, 6, 2), yticks=range(-5, 6, 2),
                showmask=False)


fig = plt.figure()

#{ conn3 #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'grid': {'shape': [5, 3]}}}
#{ end #}
grid_mask_fig(fig, 131, conndict)

#{ conn3c #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'grid': {'shape': [5, 3]},
                     'anchor': [2, 1]}}
#{ end #}
grid_mask_fig(fig, 132, conndict)

#{ conn3x #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {'grid': {'shape': [3, 5]},
                     'anchor': [2, -1]}}
#{ end #}
grid_mask_fig(fig, 133, conndict)

plt.savefig('../user_manual_figures/conn3.png', bbox_inches='tight')


# -----------------------------------------------

# free masks

def kernel_fig(fig, loc, cdict, kern=None):
    nest.ResetKernel()
    spatial_nodes = nest.Create('iaf_psc_alpha',
                                positions=nest.spatial.grid(
                                    shape=[11, 11],
                                    extent=[11., 11.]))
    nest.Connect(spatial_nodes, spatial_nodes, cdict)

    fig.add_subplot(loc)
    conn_figure(fig, spatial_nodes, cdict, xticks=range(-5, 6, 2), yticks=range(-5, 6, 2),
                kern=kern)


fig = plt.figure()

#{ conn4cp #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': 0.5,
            'mask': {'circular': {'radius': 4.}}}
#{ end #}
kernel_fig(fig, 231, conndict)

#{ conn4g #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': nest.spatial_distributions.gaussian(nest.spatial.distance, std=1.0),
            'mask': {'circular': {'radius': 4.}}}
#{ end #}
kernel_fig(fig, 232, conndict, kern=nest.spatial_distributions.gaussian(nest.spatial.distance, std=1.0))

#{ conn4cut #}
distribution = nest.spatial_distributions.gaussian(nest.spatial.distance, std=1.0)
conndict = {'rule': 'pairwise_bernoulli',
            'p': nest.logic.conditional(distribution > 0.5,
                                        distribution,
                                        0),
            'mask': {'circular': {'radius': 4.}}}
#{ end #}
kernel_fig(fig, 234, conndict)

#{ conn42d #}
conndict = {'rule': 'pairwise_bernoulli',
            'p': nest.spatial_distributions.gaussian2D(nest.spatial.distance.x,
                                                       nest.spatial.distance.y,
                                                       std_x=1.,
                                                       std_y=3.),
            'mask': {'circular': {'radius': 4.}}}
#{ end #}
kernel_fig(fig, 235, conndict)

plt.savefig('../user_manual_figures/conn4.png', bbox_inches='tight')

# -----------------------------------------------


def wd_fig(fig, loc, pos, cdict, sdict, what, rpos=None,
           xlim=[-1, 51], ylim=[0, 1], xticks=range(0, 51, 5),
           yticks=np.arange(0., 1.1, 0.2), clr='blue',
           label=''):
    nest.ResetKernel()
    spatial_nodes = nest.Create('iaf_psc_alpha', positions=pos)
    nest.Connect(spatial_nodes, spatial_nodes, cdict, sdict)

    ax = fig.add_subplot(loc)

    if rpos is None:
        rn = spatial_nodes[0]  # first node
    else:
        rn = nest.FindNearestElement(spatial_nodes, rpos)

    conns = nest.GetConnections(rn)
    vals = np.array([c.get(what) for c in conns])
    tgts = [c.get('target') for c in conns]
    locs = np.array([nest.GetPosition(spatial_nodes[spatial_nodes.index(t)]) for t in tgts])
    ax.plot(locs[:, 0], vals, 'o', mec='none', mfc=clr, label=label)
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    ax.set_xticks(xticks)
    ax.set_yticks(yticks)


fig = plt.figure()

#{ conn5lin #}
pos = nest.spatial.grid(shape=[51, 1], extent=[51., 1.], center=[25., 0.])
spatial_nodes = nest.Create('iaf_psc_alpha', positions=pos)

cdict = {'rule': 'pairwise_bernoulli',
         'p': 1.0,
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}}}
sdict = {'weight': nest.math.max(1.0 - 0.05 * nest.spatial.distance, 0.),
         'delay': 0.1 + 0.02 * nest.spatial.distance}

nest.Connect(spatial_nodes, spatial_nodes, cdict, sdict)
#{ end #}
wd_fig(fig, 311, pos, cdict, sdict, 'weight', label='Weight')
wd_fig(fig, 311, pos, cdict, sdict, 'delay', label='Delay', clr='red')
fig.gca().legend()

ppos = nest.spatial.grid(shape=[51, 1],
                         extent=[51., 1.],
                         center=[25., 0.],
                         edge_wrap=True)
#{ conn5linpbc #}
cdict = {'rule': 'pairwise_bernoulli',
         'p': 1.0,
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}}}
sdict = {'weight': nest.math.max(1.0 - 0.05 * nest.spatial.distance, 0.),
         'delay': 0.1 + 0.02 * nest.spatial.distance}
#{ end #}
wd_fig(fig, 312, ppos, cdict, sdict, 'weight', label='Weight')
wd_fig(fig, 312, ppos, cdict, sdict, 'delay', label='Delay', clr='red')
fig.gca().legend(loc=1)

cdict = {'rule': 'pairwise_bernoulli',
         'p': 1.0,
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}}}
sdict = {'weight': nest.math.max(1.0 - 0.05 * nest.spatial.distance, 0.)}
wd_fig(fig, 313, pos, cdict, sdict, 'weight', label='Linear',
       rpos=[25., 0.], clr='orange')

#{ conn5exp #}
cdict = {'rule': 'pairwise_bernoulli',
         'p': 1.0,
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}}}
sdict = {'weight': nest.spatial_distributions.exponential(nest.spatial.distance, beta=5.)}
#{ end #}
wd_fig(fig, 313, pos, cdict, sdict, 'weight', label='Exponential',
       rpos=[25., 0.])

#{ conn5gauss #}
cdict = {'rule': 'pairwise_bernoulli',
         'p': 1.0,
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}}}
sdict = {'weight': nest.spatial_distributions.gaussian(nest.spatial.distance, std=5.)}
#{ end #}
wd_fig(fig, 313, pos, cdict, sdict, 'weight', label='Gaussian', clr='green',
       rpos=[25., 0.])

#{ conn5uniform #}
cdict = {'rule': 'pairwise_bernoulli',
         'p': 1.0,
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}}}
sdict = {'weight': nest.random.uniform(min=0.2, max=0.8)}
#{ end #}
wd_fig(fig, 313, pos, cdict, sdict, 'weight', label='Uniform', clr='red',
       rpos=[25., 0.])

fig.gca().legend()

plt.savefig('../user_manual_figures/conn5.png', bbox_inches='tight')


# --------------------------------
#{ conn_param_design #}
parameter = 0.5 + nest.spatial.distance.x + 2. * nest.spatial.distance.y
#{ end #}

#{ conn_param_design_ex #}
spatial_nodes = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11],
                                                        extent=[1., 1.]))
nest.Connect(spatial_nodes, spatial_nodes, {'rule': 'pairwise_bernoulli',
                                            'p': parameter,
                                            'mask': {'circular': {'radius': 0.5}}})
#{ end #}

# --------------------------------


def pn_fig(fig, loc, spatial_nodes, cdict,
           xlim=[0., .5], ylim=[0, 3.5], xticks=range(0, 51, 5),
           yticks=np.arange(0., 1.1, 0.2), clr='blue',
           label=''):
    nest.Connect(spatial_nodes, spatial_nodes, cdict)

    ax = fig.add_subplot(loc)

    conns = nest.GetConnections(spatial_nodes)
    dist = np.array([nest.Distance(spatial_nodes[spatial_nodes.index(s)],
                                   spatial_nodes[spatial_nodes.index(t)])
                     for s, t in zip(conns.sources(), conns.targets())])
    ax.hist(dist, bins=50, histtype='stepfilled', density=True)
    r = np.arange(0., 0.51, 0.01)

    plt.plot(r, 2 * np.pi * r * (1 - 2 * r) * 12 / np.pi, 'r-', lw=3,
             zorder=-10)

    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    """ax.set_xticks(xticks)
    ax.set_yticks(yticks)"""
    # ax.set_aspect(100, 'box')
    ax.set_xlabel('Source-target distance d')
    ax.set_ylabel('Connection probability pconn(d)')


fig = plt.figure()

nest.ResetKernel()


#{ conn6 #}
pos = nest.spatial.free(nest.random.uniform(-1., 1.),
                        extent=[2., 2.], edge_wrap=True)
spatial_nodes = nest.Create('iaf_psc_alpha', 1000, positions=pos)

cdict = {'rule': 'fixed_outdegree',
         'p': nest.math.max(1. - 2 * nest.spatial.distance, 0.),
         'mask': {'circular': {'radius': 1.0}},
         'outdegree': 50,
         'allow_multapses': True, 'allow_autapses': False}
nest.Connect(spatial_nodes, spatial_nodes, cdict)
#{ end #}
pn_fig(fig, 111, spatial_nodes, cdict)

plt.savefig('../user_manual_figures/conn6.png', bbox_inches='tight')

# ----------------------------

#{ conn8 #}
nest.ResetKernel()
nest.CopyModel('static_synapse', 'exc', {'weight': 2.0})
nest.CopyModel('static_synapse', 'inh', {'weight': -8.0})

pos = nest.spatial.grid(shape=[10, 10])
ex_nodes = nest.Create('iaf_psc_alpha', positions=pos)
in_nodes = nest.Create('iaf_psc_alpha', positions=pos)

nest.Connect(ex_nodes, in_nodes, {'rule': 'pairwise_bernoulli',
                                  'p': 0.8,
                                  'mask': {'circular': {'radius': 0.5}}},
             {'synapse_model': 'exc'})
nest.Connect(in_nodes, ex_nodes, {'rule': 'pairwise_bernoulli',
                                  'p': 1.0,
                                  'mask': {'rectangular': {'lower_left': [-0.2, -0.2],
                                                           'upper_right': [0.2, 0.2]}}},
             {'synapse_model': 'inh'})
#{ end #}


# ----------------------------

#{ conn9 #}
nrn_layer = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(shape=[20, 20]))

stim = nest.Create('poisson_generator',
                   positions=nest.spatial.grid(shape=[1, 1]))

cdict_stim = {'rule': 'pairwise_bernoulli',
              'p': 1.0,
              'mask': {'circular': {'radius': 0.1},
                       'anchor': [0.2, 0.2]}}

nest.Connect(stim, nrn_layer, cdict_stim)
#{ end #}


# ----------------------------

#{ conn10 #}
rec = nest.Create('spike_recorder',
                  positions=nest.spatial.grid(shape=[1, 1]))

cdict_rec = {'rule': 'pairwise_bernoulli',
             'p': 1.0,
             'use_on_source': True,
             'mask': {'circular': {'radius': 0.1},
                      'anchor': [-0.2, 0.2]}}

nest.Connect(nrn_layer, rec, cdict_rec)
#{ end #}

# ----------------------------

#{ conn11 #}
rec = nest.Create('spike_recorder')
nest.Connect(nrn_layer, rec)
#{ end #}
