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

# create connectivity figures for topology manual

import nest
import nest.topology as tp
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.axes3d import Axes3D
import numpy as np

# seed NumPy RNG to ensure identical results for runs with random placement
np.random.seed(7654321)


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


def conn_figure(fig, layer, connd, targets=None, showmask=True, showkern=False,
                xticks=range(-5, 6), yticks=range(-5, 6),
                xlim=[-5.5, 5.5], ylim=[-5.5, 5.5]):
    if targets is None:
        targets = ((tp.FindCenterElement(layer), 'red'),)

    tp.PlotLayer(layer, fig=fig, nodesize=60)
    for src, clr in targets:
        if showmask:
            mask = connd['mask']
        else:
            mask = None
        if showkern:
            kern = connd['kernel']
        else:
            kern = None
        tp.PlotTargets(src, layer, fig=fig, mask=mask, kernel=kern,
                       src_size=250, tgt_color=clr, tgt_size=20,
                       kernel_color='green')

    beautify_layer(layer, fig,
                   xlim=xlim, ylim=ylim, xticks=xticks, yticks=yticks,
                   xlabel='', ylabel='')
    fig.gca().grid(False)


# -----------------------------------------------

# Simple connection

#{ conn1 #}
l = tp.CreateLayer({'rows': 11, 'columns': 11, 'extent': [11., 11.],
                    'elements': 'iaf_psc_alpha'})
conndict = {'connection_type': 'divergent',
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.]}}}
tp.ConnectLayers(l, l, conndict)
#{ end #}

fig = plt.figure()
fig.add_subplot(121)
conn_figure(fig, l, conndict,
            targets=((tp.FindCenterElement(l), 'red'),
                     (tp.FindNearestElement(l, [4., 5.]), 'yellow')))

# same another time, with periodic bcs
lpbc = tp.CreateLayer({'rows': 11, 'columns': 11, 'extent': [11., 11.],
                       'elements': 'iaf_psc_alpha', 'edge_wrap': True})
tp.ConnectLayers(lpbc, lpbc, conndict)
fig.add_subplot(122)
conn_figure(fig, lpbc, conndict, showmask=False,
            targets=((tp.FindCenterElement(lpbc), 'red'),
                     (tp.FindNearestElement(lpbc, [4., 5.]), 'yellow')))

plt.savefig('../user_manual_figures/conn1.png', bbox_inches='tight')


# -----------------------------------------------

# free masks

def free_mask_fig(fig, loc, cdict):
    nest.ResetKernel()
    l = tp.CreateLayer({'rows': 11, 'columns': 11, 'extent': [11., 11.],
                        'elements': 'iaf_psc_alpha'})
    tp.ConnectLayers(l, l, cdict)

    fig.add_subplot(loc)
    conn_figure(fig, l, cdict, xticks=range(-5, 6, 2), yticks=range(-5, 6, 2))

fig = plt.figure()

#{ conn2r #}
conndict = {'connection_type': 'divergent',
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.]}}}
#{ end #}
free_mask_fig(fig, 221, conndict)

#{ conn2c #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 2.0}}}
#{ end #}
free_mask_fig(fig, 222, conndict)

#{ conn2d #}
conndict = {'connection_type': 'divergent',
            'mask': {'doughnut': {'inner_radius': 1.5,
                                  'outer_radius': 3.}}}
#{ end #}
free_mask_fig(fig, 223, conndict)

#{ conn2e #}
conndict = {'connection_type': 'divergent',
            'mask': {'elliptical': {'major_axis': 7.,
                                    'minor_axis': 4.}}}
#{ end #}
free_mask_fig(fig, 224, conndict)

plt.savefig('../user_manual_figures/conn2_a.png', bbox_inches='tight')

#-----------------------------------------------------------------------------#

fig = plt.figure()

#{ conn2ro #}
conndict = {'connection_type': 'divergent',
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.]},
                     'anchor': [-1.5, -1.5]}}
#{ end #}
free_mask_fig(fig, 221, conndict)

#{ conn2co #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 2.0},
                     'anchor': [-2.0, 0.0]}}
#{ end #}
free_mask_fig(fig, 222, conndict)

#{ conn2do #}
conndict = {'connection_type': 'divergent',
            'mask': {'doughnut': {'inner_radius': 1.5,
                                  'outer_radius': 3.},
                     'anchor': [1.5, 1.5]}}
#{ end #}
free_mask_fig(fig, 223, conndict)

#{ conn2eo #}
conndict = {'connection_type': 'divergent',
            'mask': {'elliptical': {'major_axis': 7.,
                                    'minor_axis': 4.},
                     'anchor': [2.0, -1.0]}}
#{ end #}
free_mask_fig(fig, 224, conndict)

plt.savefig('../user_manual_figures/conn2_b.png', bbox_inches='tight')

#-----------------------------------------------------------------------------#

fig = plt.figure()

#{ conn2rr #}
conndict = {'connection_type': 'divergent',
            'mask': {'rectangular': {'lower_left': [-2., -1.],
                                     'upper_right': [2., 1.],
                                     'azimuth_angle': 120.}}}
#{ end #}
free_mask_fig(fig, 121, conndict)

#{ conn2er #}
conndict = {'connection_type': 'divergent',
            'mask': {'elliptical': {'major_axis': 7.,
                                    'minor_axis': 4.,
                                    'azimuth_angle': 45.}}}
#{ end #}
free_mask_fig(fig, 122, conndict)

plt.savefig('../user_manual_figures/conn2_c.png', bbox_inches='tight')

# -----------------------------------------------

# 3d masks


def conn_figure_3d(fig, layer, connd, targets=None, showmask=True,
                   showkern=False,
                   xticks=range(-5, 6), yticks=range(-5, 6),
                   xlim=[-5.5, 5.5], ylim=[-5.5, 5.5]):
    if targets is None:
        targets = ((tp.FindCenterElement(layer), 'red'),)

    tp.PlotLayer(layer, fig=fig, nodesize=20, nodecolor=(.5, .5, 1.))
    for src, clr in targets:
        if showmask:
            mask = connd['mask']
        else:
            mask = None
        if showkern:
            kern = connd['kernel']
        else:
            kern = None
        tp.PlotTargets(src, layer, fig=fig, mask=mask, kernel=kern,
                       src_size=250, tgt_color=clr, tgt_size=60,
                       kernel_color='green')

    ax = fig.gca()
    ax.set_aspect('equal', 'box')
    plt.draw()


def free_mask_3d_fig(fig, loc, cdict):
    nest.ResetKernel()
    l = tp.CreateLayer(
        {'rows': 11, 'columns': 11, 'layers': 11, 'extent': [11., 11., 11.],
         'elements': 'iaf_psc_alpha'})
    tp.ConnectLayers(l, l, cdict)

    fig.add_subplot(loc, projection='3d')
    conn_figure_3d(fig, l, cdict, xticks=range(-5, 6, 2),
                   yticks=range(-5, 6, 2))


fig = plt.figure()

#{ conn_3d_a #}
conndict = {'connection_type': 'divergent',
            'mask': {'box': {'lower_left': [-2., -1., -1.],
                             'upper_right': [2., 1., 1.]}}}
#{ end #}
free_mask_3d_fig(fig, 121, conndict)

#{ conn_3d_b #}
conndict = {'connection_type': 'divergent',
            'mask': {'spherical': {'radius': 2.5}}}
#{ end #}
free_mask_3d_fig(fig, 122, conndict)

#{ conn_3d_c #}
conndict = {'connection_type': 'divergent',
            'mask': {'ellipsoidal': {'major_axis': 7.,
                                     'minor_axis': 4.,
                                     'polar_axis': 4.5}}}
#{ end #}

plt.savefig('../user_manual_figures/conn_3d.png', bbox_inches='tight')


# -----------------------------------------------

# grid masks

def grid_mask_fig(fig, loc, cdict):
    nest.ResetKernel()
    l = tp.CreateLayer({'rows': 11, 'columns': 11, 'extent': [11., 11.],
                        'elements': 'iaf_psc_alpha'})
    tp.ConnectLayers(l, l, cdict)

    fig.add_subplot(loc)
    conn_figure(fig, l, cdict, xticks=range(-5, 6, 2), yticks=range(-5, 6, 2),
                showmask=False)


fig = plt.figure()

#{ conn3 #}
conndict = {'connection_type': 'divergent',
            'mask': {'grid': {'rows': 3, 'columns': 5}}}
#{ end #}
grid_mask_fig(fig, 131, conndict)

#{ conn3c #}
conndict = {'connection_type': 'divergent',
            'mask': {'grid': {'rows': 3, 'columns': 5},
                     'anchor': {'row': 1, 'column': 2}}}
#{ end #}
grid_mask_fig(fig, 132, conndict)

#{ conn3x #}
conndict = {'connection_type': 'divergent',
            'mask': {'grid': {'rows': 3, 'columns': 5},
                     'anchor': {'row': -1, 'column': 2}}}
#{ end #}
grid_mask_fig(fig, 133, conndict)

plt.savefig('../user_manual_figures/conn3.png', bbox_inches='tight')


# -----------------------------------------------

# free masks

def kernel_fig(fig, loc, cdict, showkern=True):
    nest.ResetKernel()
    l = tp.CreateLayer({'rows': 11, 'columns': 11, 'extent': [11., 11.],
                        'elements': 'iaf_psc_alpha'})
    tp.ConnectLayers(l, l, cdict)

    fig.add_subplot(loc)
    conn_figure(fig, l, cdict, xticks=range(-5, 6, 2), yticks=range(-5, 6, 2),
                showkern=showkern)


fig = plt.figure()

#{ conn4cp #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 4.}},
            'kernel': 0.5}
#{ end #}
kernel_fig(fig, 231, conndict)

#{ conn4g #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 4.}},
            'kernel': {'gaussian': {'p_center': 1.0,
                                    'sigma': 1.}}}
#{ end #}
kernel_fig(fig, 232, conndict)

#{ conn4gx #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 4.},
                     'anchor': [1.5, 1.5]},
            'kernel': {'gaussian': {'p_center': 1.0,
                                    'sigma': 1.,
                                    'anchor': [1.5, 1.5]}}}
#{ end #}
kernel_fig(fig, 233, conndict)
plt.draw()

#{ conn4cut #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 4.}},
            'kernel': {'gaussian': {'p_center': 1.0,
                                    'sigma': 1.,
                                    'cutoff': 0.5}}}
#{ end #}
kernel_fig(fig, 234, conndict)

#{ conn42d #}
conndict = {'connection_type': 'divergent',
            'mask': {'circular': {'radius': 4.}},
            'kernel': {'gaussian2D': {'p_center': 1.0,
                                      'sigma_x': 1.,
                                      'sigma_y': 3.}}}
#{ end #}
kernel_fig(fig, 235, conndict, showkern=False)

plt.savefig('../user_manual_figures/conn4.png', bbox_inches='tight')

# -----------------------------------------------


def wd_fig(fig, loc, ldict, cdict, what, rpos=None,
           xlim=[-1, 51], ylim=[0, 1], xticks=range(0, 51, 5),
           yticks=np.arange(0., 1.1, 0.2), clr='blue',
           label=''):
    nest.ResetKernel()
    l = tp.CreateLayer(ldict)
    tp.ConnectLayers(l, l, cdict)

    ax = fig.add_subplot(loc)

    if rpos is None:
        rn = nest.GetLeaves(l)[0][:1]  # first node
    else:
        rn = tp.FindNearestElement(l, rpos)

    conns = nest.GetConnections(rn)
    cstat = nest.GetStatus(conns)
    vals = np.array([sd[what] for sd in cstat])
    tgts = [sd['target'] for sd in cstat]
    locs = np.array(tp.GetPosition(tgts))
    ax.plot(locs[:, 0], vals, 'o', mec='none', mfc=clr, label=label)
    ax.set_xlim(xlim)
    ax.set_ylim(ylim)
    ax.set_xticks(xticks)
    ax.set_yticks(yticks)


fig = plt.figure()

#{ conn5lin #}
ldict = {'rows': 1, 'columns': 51,
         'extent': [51., 1.], 'center': [25., 0.],
         'elements': 'iaf_psc_alpha'}
cdict = {'connection_type': 'divergent',
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}},
         'weights': {'linear': {'c': 1.0,
                                'a': -0.05,
                                'cutoff': 0.0}},
         'delays': {'linear': {'c': 0.1, 'a': 0.02}}}
#{ end #}
wd_fig(fig, 311, ldict, cdict, 'weight', label='Weight')
wd_fig(fig, 311, ldict, cdict, 'delay', label='Delay', clr='red')
fig.gca().legend()

lpdict = {'rows': 1, 'columns': 51, 'extent': [51., 1.], 'center': [25., 0.],
          'elements': 'iaf_psc_alpha', 'edge_wrap': True}
#{ conn5linpbc #}
cdict = {'connection_type': 'divergent',
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}},
         'weights': {'linear': {'c': 1.0,
                                'a': -0.05,
                                'cutoff': 0.0}},
         'delays': {'linear': {'c': 0.1, 'a': 0.02}}}
#{ end #}
wd_fig(fig, 312, lpdict, cdict, 'weight', label='Weight')
wd_fig(fig, 312, lpdict, cdict, 'delay', label='Delay', clr='red')
fig.gca().legend(loc=1)

cdict = {'connection_type': 'divergent',
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}},
         'weights': {'linear': {'c': 1.0, 'a': -0.05, 'cutoff': 0.0}}}
wd_fig(fig, 313, ldict, cdict, 'weight', label='Linear',
       rpos=[25., 0.], clr='orange')

#{ conn5exp #}
cdict = {'connection_type': 'divergent',
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}},
         'weights': {'exponential': {'a': 1., 'tau': 5.}}}
#{ end #}
wd_fig(fig, 313, ldict, cdict, 'weight', label='Exponential',
       rpos=[25., 0.])

#{ conn5gauss #}
cdict = {'connection_type': 'divergent',
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}},
         'weights': {'gaussian': {'p_center': 1., 'sigma': 5.}}}
#{ end #}
wd_fig(fig, 313, ldict, cdict, 'weight', label='Gaussian', clr='green',
       rpos=[25., 0.])

#{ conn5uniform #}
cdict = {'connection_type': 'divergent',
         'mask': {'rectangular': {'lower_left': [-25.5, -0.5],
                                  'upper_right': [25.5, 0.5]}},
         'weights': {'uniform': {'min': 0.2, 'max': 0.8}}}
#{ end #}
wd_fig(fig, 313, ldict, cdict, 'weight', label='Uniform', clr='red',
       rpos=[25., 0.])

fig.gca().legend()

plt.savefig('../user_manual_figures/conn5.png', bbox_inches='tight')


# --------------------------------

def pn_fig(fig, loc, ldict, cdict,
           xlim=[0., .5], ylim=[0, 3.5], xticks=range(0, 51, 5),
           yticks=np.arange(0., 1.1, 0.2), clr='blue',
           label=''):
    nest.ResetKernel()
    l = tp.CreateLayer(ldict)
    tp.ConnectLayers(l, l, cdict)

    ax = fig.add_subplot(loc)

    rn = nest.GetLeaves(l)[0]
    conns = nest.GetConnections(rn)
    cstat = nest.GetStatus(conns)
    srcs = [sd['source'] for sd in cstat]
    tgts = [sd['target'] for sd in cstat]
    dist = np.array(tp.Distance(srcs, tgts))
    ax.hist(dist, bins=50, histtype='stepfilled', normed=True)
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

#{ conn6 #}
pos = [[np.random.uniform(-1., 1.), np.random.uniform(-1., 1.)]
       for j in range(1000)]
ldict = {'positions': pos, 'extent': [2., 2.],
         'elements': 'iaf_psc_alpha', 'edge_wrap': True}
cdict = {'connection_type': 'divergent',
         'mask': {'circular': {'radius': 1.0}},
         'kernel': {'linear': {'c': 1., 'a': -2., 'cutoff': 0.0}},
         'number_of_connections': 50,
         'allow_multapses': True, 'allow_autapses': False}
#{ end #}
pn_fig(fig, 111, ldict, cdict)

plt.savefig('../user_manual_figures/conn6.png', bbox_inches='tight')

# -----------------------------

#{ conn7 #}
nest.ResetKernel()
nest.CopyModel('iaf_psc_alpha', 'pyr')
nest.CopyModel('iaf_psc_alpha', 'in')
ldict = {'rows': 10, 'columns': 10, 'elements': ['pyr', 'in']}
cdict_p2i = {'connection_type': 'divergent',
             'mask': {'circular': {'radius': 0.5}},
             'kernel': 0.8,
             'sources': {'model': 'pyr'},
             'targets': {'model': 'in'}}
cdict_i2p = {'connection_type': 'divergent',
             'mask': {'rectangular': {'lower_left': [-0.2, -0.2],
                                      'upper_right': [0.2, 0.2]}},
             'sources': {'model': 'in'},
             'targets': {'model': 'pyr'}}
l = tp.CreateLayer(ldict)
tp.ConnectLayers(l, l, cdict_p2i)
tp.ConnectLayers(l, l, cdict_i2p)
#{ end #}


# ----------------------------

#{ conn8 #}
nest.ResetKernel()
nest.CopyModel('iaf_psc_alpha', 'pyr')
nest.CopyModel('iaf_psc_alpha', 'in')
nest.CopyModel('static_synapse', 'exc', {'weight': 2.0})
nest.CopyModel('static_synapse', 'inh', {'weight': -8.0})
ldict = {'rows': 10, 'columns': 10, 'elements': ['pyr', 'in']}
cdict_p2i = {'connection_type': 'divergent',
             'mask': {'circular': {'radius': 0.5}},
             'kernel': 0.8,
             'sources': {'model': 'pyr'},
             'targets': {'model': 'in'},
             'synapse_model': 'exc'}
cdict_i2p = {'connection_type': 'divergent',
             'mask': {'rectangular': {'lower_left': [-0.2, -0.2],
                                      'upper_right': [0.2, 0.2]}},
             'sources': {'model': 'in'},
             'targets': {'model': 'pyr'},
             'synapse_model': 'inh'}
l = tp.CreateLayer(ldict)
tp.ConnectLayers(l, l, cdict_p2i)
tp.ConnectLayers(l, l, cdict_i2p)
#{ end #}


# ----------------------------

#{ conn9 #}
nrn_layer = tp.CreateLayer({'rows': 20,
                            'columns': 20,
                            'elements': 'iaf_psc_alpha'})

stim = tp.CreateLayer({'rows': 1,
                       'columns': 1,
                       'elements': 'poisson_generator'})

cdict_stim = {'connection_type': 'divergent',
              'mask': {'circular': {'radius': 0.1},
                       'anchor': [0.2, 0.2]}}

tp.ConnectLayers(stim, nrn_layer, cdict_stim)
#{ end #}


# ----------------------------

#{ conn10 #}
rec = tp.CreateLayer({'rows': 1,
                      'columns': 1,
                      'elements': 'spike_detector'})

cdict_rec = {'connection_type': 'convergent',
             'mask': {'circular': {'radius': 0.1},
                      'anchor': [-0.2, 0.2]}}

tp.ConnectLayers(nrn_layer, rec, cdict_rec)
#{ end #}

# ----------------------------

#{ conn11 #}
rec = nest.Create('spike_detector')
nrns = nest.GetLeaves(nrn_layer, local_only=True)[0]
nest.Connect(nrns, rec)
#{ end #}
