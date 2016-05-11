# -*- coding: utf-8 -*-
#
# ConnPlotter.py
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

# ConnPlotter --- A Tool to Generate Connectivity Pattern Matrices

"""
ConnPlotter is a tool to create connectivity pattern tables.

For background on ConnPlotter, please see

  Eilen Nordlie and Hans Ekkehard Plesser.
  Connection Pattern Tables: A new way to visualize connectivity
  in neuronal network models.
  Frontiers in Neuroinformatics 3:39 (2010)
  doi: 10.3389/neuro.11.039.2009

Example:
# code creating population and connection lists

from ConnPlotter import ConnectionPattern, SynType

# Case A: All connections have the same "synapse_model".
#
# Connections with weight < 0 are classified as excitatory,
#                  weight > 0 are classified as inhibitory.
# Each sender must make either excitatory or inhibitory connection,
# not both. When computing totals, excit/inhib connections are
# weighted with +-1.
pattern = ConnectionPattern(layerList, connList)

# Case B: All connections have the same "synapse_model", but violate Dale's law
#
# Connections with weight < 0 are classified as excitatory,
#                  weight > 0 are classified as inhibitory.
# A single sender may have excitatory and inhibitory connections.
# When computing totals, excit/inhib connections are
# weighted with +-1.
pattern = ConnectionPattern(layerList, connList,
                            synTypes=(((SynType('exc',  1.0, 'b'),
                                        SynType('inh', -1.0, 'r')),)))

# Case C: Synapse models are "AMPA", "NMDA", "GABA_A", "GABA_B".
#
# Connections are plotted by synapse model, with AMPA and NMDA
# on the top row, GABA_A and GABA_B in the bottom row when
# combining by layer. Senders must either have AMPA and NMDA or
# GABA_A and GABA_B synapses, but not both. When computing totals,
# AMPA and NMDA connections are weighted with +1, GABA_A and GABA_B
# with -1.
pattern = ConnectionPattern(layerList, connList)

# Case D: Explicit synapse types.
#
# If your network model uses other synapse types, or you want to use
# other weighting factors when computing totals, or you want different
# colormaps, you must specify synapse type information explicitly for
# ALL synase models in your network. For each synapse model, you create
# a
#
#        SynType(name, tweight, cmap)
#
# object, where "name" is the synapse model name, "tweight" the weight
# to be given to the type when computing totals (usually >0 for excit,
# <0 for inhib synapses), and "cmap" the "colormap": if may be a
# matplotlib.colors.Colormap instance or any valid matplotlib color
# specification; in the latter case, as colormap will be generated
# ranging from white to the given color.
# Synapse types are passed as a tuple of tuples. Synapses in a tuple form
# a group. ConnPlotter assumes that a sender may make synapses with all
# types in a single group, but never synapses with types from different
# groups (If you group by transmitter, this simply reflects Dale's law).
# When connections are aggregated by layer, each group is printed on one
# row.
pattern = ConnectionPattern(layerList, connList, synTypes = \
    ((SynType('Asyn',  1.0, 'orange'),
      SynType('Bsyn',  2.5, 'r'),
      SynType('Csyn',  0.5, (1.0, 0.5, 0.0))),  # end first group
     (SynType('Dsyn', -1.5, matplotlib.pylab.cm.jet),
      SynType('Esyn', -3.2, '0.95'))))
# See documentation of class ConnectionPattern for more options.

# plotting the pattern

# show connection kernels for all sender-target pairs and all synapse models
pattern.plot()

# combine synapses of all types for each sender-target pair
# always used red-blue (inhib-excit) color scale
pattern.plot(aggrSyns=True)

# for each pair of sender-target layer pair, show sums for each synapse type
pattern.plot(aggrGroups=True)

# As mode layer, but combine synapse types.
# always used red-blue (inhib-excit) color scale
pattern.plot(aggrSyns=True, aggrGroups=True)

# Show only synases of the selected type(s)
pattern.plot(mode=('AMPA',))
pattern.plot(mode=('AMPA', 'GABA_A'))

# use same color scales for all patches
pattern.plot(globalColors=True)

# manually specify limits for global color scale
pattern.plot(globalColors=True, colorLimits=[0, 2.5])

# save to file(s)
# NB: do not write to PDF directly, this seems to cause artifacts
pattern.plot(file='net.png')
pattern.plot(file=('net.eps','net.png'))

# You can adjust some properties of the figure by changing the
# default values in plotParams.

# Experimentally, you can dump the connection pattern into a LaTeX table
pattern.toLaTeX('pattern.tex', standalone=True)

# Figure layout can be modified by changing the global variable plotParams.
# Please see the documentation for class PlotParams for details.

# Changes 30 June 2010:
# - Singular layers (extent 0x0) are ignored as target layers.
#   The reason for this is so that single-generator "layers" can be
#   displayed as input.
#   Problems:
#   - singularity is not made clear visually
#   - This messes up the diagonal shading
#   - makes no sense to aggregate any longer
"""

# ----------------------------------------------------------------------------

from . import colormaps as cm

import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import warnings

__all__ = ['ConnectionPattern', 'SynType', 'plotParams', 'PlotParams']

# ----------------------------------------------------------------------------

# To do:
# - proper testsuite
# - layers of different sizes not handled properly
#   (find biggest layer extent in each direction, then center;
#    may run into problems with population label placement)
# - clean up main
# - color bars
# - "bad color" should be configurable
# - fix hack for colormaps import
# - use generators where possible (eg kernels?)

# ----------------------------------------------------------------------------


class SynType(object):
    """
    Provide information about how synapse types should be rendered.

    A singly nested list of SynType objects can be passed to the
    ConnectionPattern constructor to specify layout and rendering info.
    """

    def __init__(self, name, relweight, cmap):
        """
        Arguments:
        name         Name of synapse type (string, must be unique)
        relweight    Relative weight of synapse type when aggregating
                     across synapse types. Should be negative for inhibitory
                     connections.
        cmap         Either a matplotlib.colors.Colormap instance or a
                     color specification. In the latter case, the colormap
                     will be built from white to the color given. Thus,
                     the color should be fully saturated. Colormaps should
                     have "set_bad(color='white')".
        """

        self.name, self.relweight = name, relweight
        if isinstance(cmap, mpl.colors.Colormap):
            self.cmap = cmap
        else:
            self.cmap = cm.make_colormap(cmap)


# ----------------------------------------------------------------------------

class PlotParams(object):
    """
    Collects parameters governing plotting.
    Implemented using properties to ensure they are read-only.
    """

    class Margins(object):
        """Width of outer margins, in mm."""

        def __init__(self):
            """Set default values."""
            self._left = 15.0
            self._right = 10.0
            self._top = 10.0
            self._bottom = 10.0
            self._colbar = 10.0

        @property
        def left(self):
            return self._left

        @left.setter
        def left(self, l):
            self._left = float(l)

        @property
        def right(self):
            return self._right

        @right.setter
        def right(self, r):
            self._right = float(r)

        @property
        def top(self):
            return self._top

        @top.setter
        def top(self, t):
            self._top = float(t)

        @property
        def bottom(self):
            return self._bottom

        @bottom.setter
        def bottom(self, b):
            self._bottom = float(b)

        @property
        def colbar(self):
            return self._colbar

        @colbar.setter
        def colbar(self, b):
            self._colbar = float(b)

    def __init__(self):
        """Set default values"""
        self._n_kern = 100
        self._patch_size = 20.0  # 20 mm
        self._layer_bg = {'super': '0.9', 'diag': '0.8', 'sub': '0.9'}
        self._layer_font = mpl.font_manager.FontProperties(size='large')
        self._layer_orient = {'sender': 'horizontal', 'target': 'horizontal'}
        self._pop_font = mpl.font_manager.FontProperties(size='small')
        self._pop_orient = {'sender': 'horizontal', 'target': 'horizontal'}
        self._lgd_tick_font = mpl.font_manager.FontProperties(size='x-small')
        self._lgd_title_font = mpl.font_manager.FontProperties(size='xx-small')
        self._lgd_ticks = None
        self._lgd_tick_fmt = None
        self._lgd_location = None
        self._cbwidth = None
        self._cbspace = None
        self._cbheight = None
        self._cboffset = None
        self._z_layer = 25
        self._z_pop = 50
        self._z_conn = 100
        self.margins = self.Margins()

    def reset(self):
        """
        Reset to default values.
        """
        self.__init__()

    @property
    def n_kern(self):
        """Sample long kernel dimension at N_kernel points."""
        return self._n_kern

    @n_kern.setter
    def n_kern(self, n):
        if n <= 0:
            raise ValueError('n_kern > 0 required')
        self._n_kern = n

    @property
    def patch_size(self):
        """Length of the longest edge of the largest patch, in mm."""
        return self._patch_size

    @patch_size.setter
    def patch_size(self, sz):
        if sz <= 0:
            raise ValueError('patch_size > 0 required')
        self._patch_size = sz

    @property
    def layer_bg(self):
        """
        Dictionary of colors for layer background.
        Entries "super", "diag", "sub". Each entry
        can be set to any valid color specification.
        If just a color is given, create dict by
        brightening/dimming.
        """
        return self._layer_bg

    @layer_bg.setter
    def layer_bg(self, bg):
        if isinstance(bg, dict):
            if set(bg.keys()) != set(('super', 'diag', 'sub')):
                raise ValueError(
                    'Background dict must have keys "super", "diag", "sub"')
            for bgc in bg.values():
                if not mpl.colors.is_color_like(bgc):
                    raise ValueError('Entries in background dict must be ' +
                                     'valid color specifications.')
            self._layer_bg = bg
        elif not mpl.colors.is_color_like(bg):
            raise ValueError(
                'layer_bg must be dict or valid color specification.')
        else:  # is color like
            rgb = mpl.colors.colorConverter.to_rgb(bg)
            self._layer_bg = {'super': [1.1 * c for c in rgb],
                              'diag': rgb,
                              'sub': [0.9 * c for c in rgb]}

    @property
    def layer_font(self):
        """
        Font to use for layer labels.
        Can be set to a matplotlib.font_manager.FontProperties instance.
        """
        return self._layer_font

    @layer_font.setter
    def layer_font(self, font):
        if not isinstance(font, mpl.font_manager.FontProperties):
            raise ValueError('layer_font must be a ' +
                             'matplotlib.font_manager.FontProperties instance')
        self._layer_font = font

    @property
    def layer_orientation(self):
        """
        Orientation of layer labels.
        Dictionary with orientation of sender and target labels. Orientation
        is either 'horizontal', 'vertial', or a value in degrees.  When set
        to a single string or number, this value is used for both sender and
        target labels.
        """
        return self._layer_orient

    @layer_orientation.setter
    def layer_orientation(self, orient):

        if isinstance(orient, (str, float, int)):
            tmp = {'sender': orient, 'target': orient}
        elif isinstance(orient, dict):
            tmp = self._layer_orient
            tmp.update(orient)
        else:
            raise ValueError(
                'Orientation ust be set to dict, string or number.')

        if len(tmp) > 2:
            raise ValueError('Orientation dictionary can only contain keys ' +
                             '"sender" and "target".')

        self._layer_orient = tmp

    @property
    def pop_font(self):
        """
        Font to use for population labels.
        Can be set to a matplotlib.font_manager.FontProperties instance.
        """
        return self._pop_font

    @pop_font.setter
    def pop_font(self, font):
        if not isinstance(font, mpl.font_manager.FontProperties):
            raise ValueError('pop_font must be a ' +
                             'matplotlib.font_manager.FontProperties instance')
        self._pop_font = font

    @property
    def pop_orientation(self):
        """
        Orientation of population labels.
        Dictionary with orientation of sender and target labels. Orientation
        is either 'horizontal', 'vertial', or a value in degrees.  When set
        to a single string or number, this value is used for both sender and
        target labels.
        """
        return self._pop_orient

    @pop_orientation.setter
    def pop_orientation(self, orient):

        if isinstance(orient, (str, float, int)):
            tmp = {'sender': orient, 'target': orient}
        elif isinstance(orient, dict):
            tmp = self._pop_orient
            tmp.update(orient)
        else:
            raise ValueError(
                'Orientation ust be set to dict, string or number.')

        if len(tmp) > 2:
            raise ValueError('Orientation dictionary can only contain keys ' +
                             '"sender" and "target".')

        self._pop_orient = tmp

    @property
    def legend_tick_font(self):
        """
        FontProperties for legend (colorbar) ticks.
        """
        return self._lgd_tick_font

    @legend_tick_font.setter
    def legend_tick_font(self, font):
        if not isinstance(font, mpl.font_manager.FontProperties):
            raise ValueError('legend_tick_font must be a ' +
                             'matplotlib.font_manager.FontProperties instance')
        self._lgd_tick_font = font

    @property
    def legend_title_font(self):
        """
        FontProperties for legend (colorbar) titles.
        """
        return self._lgd_title_font

    @legend_title_font.setter
    def legend_title_font(self, font):
        if not isinstance(font, mpl.font_manager.FontProperties):
            raise ValueError('legend_title_font must be a ' +
                             'matplotlib.font_manager.FontProperties instance')
        self._lgd_title_font = font

    @property
    def legend_ticks(self):
        """
        Ordered list of values at which legend (colorbar) ticks shall be set.
        """
        return self._lgd_ticks

    @legend_ticks.setter
    def legend_ticks(self, ticks):
        self._lgd_ticks = ticks

    @property
    def legend_tick_format(self):
        """
        C-style format string for legend (colorbar) tick marks.
        """
        return self._lgd_tick_fmt

    @legend_tick_format.setter
    def legend_tick_format(self, tickfmt):
        self._lgd_tick_fmt = tickfmt

    @property
    def legend_location(self):
        """
        If set to 'top', place legend label above colorbar,
        if None, to the left.
        """
        return self._lgd_location

    @legend_location.setter
    def legend_location(self, loc):
        self._lgd_location = loc

    @property
    def cbwidth(self):
        """
        Width of single colorbar, relative to figure width.
        """
        return self._cbwidth

    @cbwidth.setter
    def cbwidth(self, cbw):
        self._cbwidth = cbw

    @property
    def cbheight(self):
        """
        Height of colorbar, relative to margins.colbar
        """
        return self._cbheight

    @cbheight.setter
    def cbheight(self, cbh):
        self._cbheight = cbh

    @property
    def cbspace(self):
        """
        Spacing between colorbars, relative to figure width.
        """
        return self._cbspace

    @cbspace.setter
    def cbspace(self, cbs):
        self._cbspace = cbs

    @property
    def cboffset(self):
        """
        Left offset of colorbar, relative to figure width.
        """
        return self._cboffset

    @cboffset.setter
    def cboffset(self, cbo):
        self._cboffset = cbo

    @property
    def z_layer(self):
        """Z-value for layer label axes."""
        return self._z_layer

    @property
    def z_pop(self):
        """Z-value for population label axes."""
        return self._z_pop

    @property
    def z_conn(self):
        """Z-value for connection kernel axes."""
        return self._z_conn


# ----------------------------------------------------------------------------

# plotting settings, default values
plotParams = PlotParams()


# ----------------------------------------------------------------------------

class ConnectionPattern(object):
    """
    Connection pattern representation for plotting.

    When a ConnectionPattern is instantiated, all connection kernels
    are pre-computed. They can later be plotted in various forms by
    calling the plot() method.

    The constructor requires layer and connection lists:

       ConnectionPattern(layerList, connList, synTypes, **kwargs)

    The layerList is used to:
       - determine the size of patches
       - determine the block structure

    All other information is taken from the connList. Information
    about synapses is inferred from the connList.

    The following keyword arguments can also be given:

    poporder :  Population order. A dictionary mapping population names
                to numbers; populations will be sorted in diagram in order
                of increasing numbers. Otherwise, they are sorted
                alphabetically.

    intensity:  'wp'  -  use weight * probability (default)
                'p'   -  use probability alone
                'tcd' -  use total charge deposited * probability
                         requires mList and Vmem; per v 0.7 only supported
                         for ht_neuron.

    mList    :  model list; required for 'tcd'
    Vmem     :  membrane potential; required for 'tcd'
    """

    # ------------------------------------------------------------------------

    class _LayerProps(object):
        """
        Information about layer.
        """

        def __init__(self, name, extent):
            """
            name  : name of layer
            extent: spatial extent of the layer
            """
            self.name = name
            self.ext = extent
            self.singular = extent[0] == 0.0 and extent[1] == 0.0

    # ------------------------------------------------------------------------

    class _SynProps(object):
        """
        Information on how to plot patches for a synapse type.
        """

        def __init__(self, row, col, tweight, cmap, idx):
            """
            row, col: Position of synapse in grid of synapse patches, from 0,0
            tweight : weight used when adding kernels for different synapses
            cmap    : colormap for synapse type (matplotlib.colors.Colormap)
            idx     : linear index, used to order colorbars in figure
            """
            self.r, self.c = row, col
            self.tw = tweight
            self.cmap = cmap
            self.index = idx

    # --------------------------------------------------------------------

    class _PlotKern(object):
        """
        Representing object ready for plotting.
        """

        def __init__(self, sl, sn, tl, tn, syn, kern):
            """
            sl  : sender layer
            sn  : sender neuron/population
            tl  : target layer
            tn  : target neuron/population
            syn : synapse model
            kern: kernel values (numpy masked array)
            All arguments but kern are strings.
            """
            self.sl = sl
            self.sn = sn
            self.tl = tl
            self.tn = tn
            self.syn = syn
            self.kern = kern

    # ------------------------------------------------------------------------

    class _Connection(object):

        def __init__(self, conninfo, layers, synapses, intensity, tcd, Vmem):
            """
            Arguments:
            conninfo: list of connection info entries:
                      (sender,target,conn_dict)
            layers  : list of _LayerProps objects
            synapses: list of _SynProps objects
            intensity: 'wp', 'p', 'tcd'
            tcd      : tcd object
            Vmem     : reference membrane potential for tcd calculations
            """

            self._intensity = intensity

            # get source and target layer
            self.slayer, self.tlayer = conninfo[:2]
            lnames = [l.name for l in layers]

            if self.slayer not in lnames:
                raise Exception('Unknown source layer "%s".' % self.slayer)
            if self.tlayer not in lnames:
                raise Exception('Unknown target layer "%s".' % self.tlayer)

            # if target layer is singular (extent==(0,0)),
            # we do not create a full object
            self.singular = False
            for l in layers:
                if l.name == self.tlayer and l.singular:
                    self.singular = True
                    return

            # see if we connect to/from specific neuron types
            cdict = conninfo[2]

            if 'sources' in cdict:
                if tuple(cdict['sources'].keys()) == ('model',):
                    self.snrn = cdict['sources']['model']
                else:
                    raise ValueError(
                        'Can only handle sources in form {"model": ...}')
            else:
                self.snrn = None

            if 'targets' in cdict:
                if tuple(cdict['targets'].keys()) == ('model',):
                    self.tnrn = cdict['targets']['model']
                else:
                    raise ValueError(
                        'Can only handle targets in form {"model": ...}')
            else:
                self.tnrn = None

            # now get (mean) weight, we need this if we classify
            # connections by sign of weight only
            try:
                self._mean_wght = _weighteval(cdict['weights'])
            except:
                raise ValueError('No or corrupt weight information.')

            # synapse model
            if sorted(synapses.keys()) == ['exc', 'inh']:
                # implicit synapse type, we ignore value of
                # 'synapse_model', it is for use by NEST only
                if self._mean_wght >= 0:
                    self.synmodel = 'exc'
                else:
                    self.synmodel = 'inh'
            else:
                try:
                    self.synmodel = cdict['synapse_model']
                    if self.synmodel not in synapses:
                        raise Exception('Unknown synapse model "%s".'
                                        % self.synmodel)
                except:
                    raise Exception('Explicit synapse model info required.')

            # store information about connection
            try:
                self._mask = cdict['mask']
                self._kern = cdict['kernel']
                self._wght = cdict['weights']
                # next line presumes only one layer name will match
                self._textent = [tl.ext for tl in layers
                                 if tl.name == self.tlayer][0]
                if intensity == 'tcd':
                    self._tcd = tcd(self.synmodel, self.tnrn, Vmem)
                else:
                    self._tcd = None
            except:
                raise Exception('Corrupt connection dictionary')

            # prepare for lazy evaluation
            self._kernel = None

        # --------------------------------------------------------------------

        @property
        def keyval(self):
            """
            Return key and _Connection as tuple.
            Useful to create dictionary via list comprehension.
            """
            if self.singular:
                return (None, self)
            else:
                return ((self.slayer, self.snrn, self.tlayer,
                         self.tnrn, self.synmodel),
                        self)

        # --------------------------------------------------------------------

        @property
        def kernval(self):
            """Kernel value, as masked array."""
            if self._kernel is None:
                self._kernel = _evalkernel(self._mask, self._kern,
                                           self._mean_wght,
                                           self._textent, self._intensity,
                                           self._tcd)
            return self._kernel

        # --------------------------------------------------------------------

        @property
        def mask(self):
            """Dictionary describing the mask."""
            return self._mask

        # --------------------------------------------------------------------

        @property
        def kernel(self):
            """Dictionary describing the kernel."""
            return self._kern

        # --------------------------------------------------------------------

        @property
        def weight(self):
            """Dictionary describing weight distribution."""
            return self._wght

        # --------------------------------------------------------------------

        def matches(self, sl=None, sn=None, tl=None, tn=None, syn=None):
            """
            Return True if all non-None arguments match.
            Arguments:
            sl : sender layer
            sn : sender neuron type
            tl : target layer
            tn : target neuron type
            syn: synapse type
            """

            return ((sl is None or sl == self.slayer) and
                    (sn is None or sn == self.snrn) and
                    (tl is None or tl == self.tlayer) and
                    (tn is None or tn == self.tnrn) and
                    (syn is None or syn == self.synmodel))

    # ------------------------------------------------------------------------

    class _Patch(object):
        """
        Represents a patch, i.e., an axes that will actually contain an
        imshow graphic of a connection kernel.
        The patch object contains the physical coordinates of the patch,
        as well as a reference to the actual Axes object once it is created.
        Also contains strings to be used as sender/target labels.

        Everything is based on a coordinate system looking from the top left
        corner down.
        """

        # --------------------------------------------------------------------

        def __init__(self, left, top, row, col, width, height,
                     slabel=None, tlabel=None, parent=None):
            """
            Arguments:
            left, top     : Location of top-left corner
            row, col      : row, column location in parent block
            width, height : Width and height of patch
            slabel, tlabel: Values for sender/target label
            parent        : _Block to which _Patch/_Block belongs
            """
            self.l, self.t, self.r, self.c = left, top, row, col
            self.w, self.h = width, height
            self.slbl, self.tlbl = slabel, tlabel
            self.ax = None
            self._parent = parent

        # --------------------------------------------------------------------

        def _update_size(self, new_lr):
            """Update patch size by inspecting all children."""
            if new_lr[0] < self.l:
                raise ValueError(
                    "new_lr[0] = %f < l = %f" % (new_lr[0], self.l))
            if new_lr[1] < self.t:
                raise ValueError(
                    "new_lr[1] = %f < t = %f" % (new_lr[1], self.t))
            self.w, self.h = new_lr[0] - self.l, new_lr[1] - self.t
            if self._parent:
                self._parent._update_size(new_lr)

        # --------------------------------------------------------------------

        @property
        def tl(self):
            """Top left corner of the patch."""
            return (self.l, self.t)

        # --------------------------------------------------------------------

        @property
        def lr(self):
            """Lower right corner of the patch."""
            return (self.l + self.w, self.t + self.h)

        # --------------------------------------------------------------------

        @property
        def l_patches(self):
            """Left edge of leftmost _Patch in _Block."""
            if isinstance(self, ConnectionPattern._Block):
                return min([e.l_patches for e in _flattened(self.elements)])
            else:
                return self.l

        # --------------------------------------------------------------------

        @property
        def t_patches(self):
            """Top edge of topmost _Patch in _Block."""
            if isinstance(self, ConnectionPattern._Block):
                return min([e.t_patches for e in _flattened(self.elements)])
            else:
                return self.t

        # --------------------------------------------------------------------

        @property
        def r_patches(self):
            """Right edge of rightmost _Patch in _Block."""
            if isinstance(self, ConnectionPattern._Block):
                return max([e.r_patches for e in _flattened(self.elements)])
            else:
                return self.l + self.w

        # --------------------------------------------------------------------

        @property
        def b_patches(self):
            """Bottom edge of lowest _Patch in _Block."""

            if isinstance(self, ConnectionPattern._Block):
                return max([e.b_patches for e in _flattened(self.elements)])
            else:
                return self.t + self.h

        # --------------------------------------------------------------------

        @property
        def location(self):
            if self.r < self.c:
                return 'super'
            elif self.r == self.c:
                return 'diag'
            else:
                return 'sub'

    # ------------------------------------------------------------------------

    class _Block(_Patch):
        """
        Represents a block of patches.

        A block is initialized with its top left corner and is then built
        row-wise downward and column-wise to the right. Rows are added by

        block.newRow(2.0, 1.5)

        where 2.0 is the space between rows, 1.5 the space between the
        first row. Elements are added to a row by

        el = block.newElement(1.0, 0.6, 's', 't')
        el = block.newElement(1.0, 0.6, 's', 't', size=[2.0, 3.0])

        The first example adds a new _Block to the row. 1.0 is space between
        blocks, 0.6 space before the first block in a row. 's' and 't' are
        stored as slbl and tlbl (optional). If size is given, a _Patch with
        the given size is created. _Patch is atomic. newElement() returns the
        _Block or _Patch created.
        """

        # --------------------------------------------------------------------

        def __init__(self, left, top, row, col, slabel=None, tlabel=None,
                     parent=None):
            ConnectionPattern._Patch.__init__(self, left, top, row, col, 0, 0,
                                              slabel, tlabel, parent)
            self.elements = []
            self._row_top = None  # top of current row
            self._row = 0
            self._col = 0

        # --------------------------------------------------------------------

        def newRow(self, dy=0.0, dynew=0.0):
            """
            Open new row of elements.
            Arguments:
            dy   : vertical skip before new row
            dynew: vertical skip if new row is first row
            """

            if self.elements:
                # top of row is bottom of block so far + dy
                self._row_top = self.lr[1] + dy
            else:
                # place relative to top edge of parent
                self._row_top = self.tl[1] + dynew
            self._row += 1
            self._col = 0

            self.elements.append([])

        # --------------------------------------------------------------------

        def newElement(self, dx=0.0, dxnew=0.0, slabel=None, tlabel=None,
                       size=None):
            """
            Append new element to last row.
            Creates _Block instance if size is not given, otherwise _Patch.

            Arguments:
            dx    : horizontal skip before new element
            dxnew : horizontal skip if new element is first
            slabel: sender label (on y-axis)
            tlabel: target label (on x-axis)
            size  : size of _Patch to create

            Returns:
            Created _Block or _Patch.
            """

            assert (self.elements)

            if self.elements[-1]:
                # left edge is right edge of block so far + dx
                col_left = self.lr[0] + dx
            else:
                # place relative to left edge of parent
                col_left = self.tl[0] + dxnew

            self._col += 1

            if size is not None:
                elem = ConnectionPattern._Patch(col_left, self._row_top,
                                                self._row, self._col,
                                                size[0], size[1], slabel,
                                                tlabel, self)
            else:
                elem = ConnectionPattern._Block(col_left, self._row_top,
                                                self._row, self._col,
                                                slabel, tlabel, self)

            self.elements[-1].append(elem)
            self._update_size(elem.lr)

            return elem

        # --------------------------------------------------------------------

        def addMargin(self, rmarg=0.0, bmarg=0.0):
            """Extend block by margin to right and bottom."""
            if rmarg < 0.0:
                raise ValueError('rmarg must not be negative!')
            if bmarg < 0.0:
                raise ValueError('bmarg must not be negative!')

            lr = self.lr
            self._update_size((lr[0] + rmarg, lr[1] + bmarg))

    # ------------------------------------------------------------------------

    def _prepareAxes(self, mode, showLegend):
        """
        Prepare information for all axes, but do not create the actual axes
        yet.
        mode: one of 'detailed', 'by layer', 'totals'
        """

        # parameters for figure, all quantities are in mm
        patchmax = plotParams.patch_size  # length of largest patch dimension

        # actual parameters scaled from default patchmax = 20mm
        lmargin = plotParams.margins.left
        tmargin = plotParams.margins.top
        rmargin = plotParams.margins.right
        bmargin = plotParams.margins.bottom
        cbmargin = plotParams.margins.colbar

        blksep = 3. / 20. * patchmax  # distance between blocks
        popsep = 2. / 20. * patchmax  # distance between populations
        synsep = 0.5 / 20. * patchmax  # distance between synapse types

        # find maximal extents of individual patches, horizontal and vertical
        maxext = max(_flattened([l.ext for l in self._layers]))

        patchscale = patchmax / float(maxext)  # determines patch size

        # obtain number of synaptic patches per population pair
        # maximum column across all synapse types, same for rows
        nsyncols = max([s.c for s in self._synAttr.values()]) + 1
        nsynrows = max([s.r for s in self._synAttr.values()]) + 1

        # dictionary mapping into patch-axes, to they can be found later
        self._patchTable = {}

        # set to store all created patches to avoid multiple
        # creation of patches at same location
        axset = set()

        # create entire setup, top-down
        self._axes = self._Block(lmargin, tmargin, 1, 1)
        for sl in self._layers:

            # get sorted list of populations for sender layer
            spops = sorted([p[1] for p in self._pops if p[0] == sl.name],
                           key=lambda pn: self._poporder[pn])

            self._axes.newRow(blksep, 0.0)

            for tl in self._layers:

                # ignore singular target layers
                if tl.singular:
                    continue

                # get sorted list of populations for target layer
                tpops = sorted([p[1] for p in self._pops if p[0] == tl.name],
                               key=lambda pn: self._poporder[pn])

                # compute size for patches
                patchsize = patchscale * np.array(tl.ext)

                block = self._axes.newElement(blksep, 0.0, sl.name, tl.name)

                if mode == 'totals':
                    # single patch
                    block.newRow(popsep, popsep / 2.)
                    p = block.newElement(popsep, popsep / 2., size=patchsize)
                    self._patchTable[(sl.name, None, tl.name, None, None)] = p

                elif mode == 'layer':
                    # We loop over all rows and columns in the synapse patch
                    # grid. For each (r,c), we find the pertaining synapse name
                    # by reverse lookup in the _synAttr dictionary. This is
                    # inefficient, but should not be too costly overall. But we
                    # must create the patches in the order they are placed.
                    # NB: We must create also those block.newElement() that are
                    #     not registered later, since block would otherwise not
                    #     skip over the unused location.
                    for r in range(nsynrows):
                        block.newRow(synsep, popsep / 2.)
                        for c in range(nsyncols):
                            p = block.newElement(synsep, popsep / 2.,
                                                 size=patchsize)
                            smod = [k for k, s in self._synAttr.items()
                                    if s.r == r and s.c == c]
                            if smod:
                                assert (len(smod) == 1)
                                self._patchTable[(sl.name, None, tl.name,
                                                  None, smod[0])] = p

                elif mode == 'population':
                    # one patch per population pair
                    for sp in spops:
                        block.newRow(popsep, popsep / 2.)
                        for tp in tpops:
                            pblk = block.newElement(popsep, popsep / 2.,
                                                    sp, tp)
                            pblk.newRow(synsep, synsep / 2.)
                            self._patchTable[(sl.name, sp,
                                              tl.name, tp, None)] = \
                                pblk.newElement(synsep, blksep / 2.,
                                                size=patchsize)

                else:
                    # detailed presentation of all pops
                    for sp in spops:
                        block.newRow(popsep, popsep / 2.)
                        for tp in tpops:
                            pblk = block.newElement(popsep, popsep / 2.,
                                                    sp, tp)
                            pblk.newRow(synsep, synsep / 2.)

                            # Find all connections with matching properties
                            # all information we need here is synapse model.
                            # We store this in a dictionary mapping synapse
                            # patch column to synapse model, for use below.
                            syns = dict(
                                [(self._synAttr[c.synmodel].c, c.synmodel)
                                 for c in _flattened(self._cTable.values())
                                 if c.matches(sl.name, sp, tl.name, tp)])

                            # create all synapse patches
                            for n in range(nsyncols):

                                # Do not duplicate existing axes.
                                if (sl.name, sp, tl.name, tp, n) in axset:
                                    continue

                                # Create patch. We must create also such
                                # patches that do not have synapses, since
                                # spacing would go wrong otherwise.
                                p = pblk.newElement(synsep, 0.0,
                                                    size=patchsize)

                                # if patch represents existing synapse,
                                # register
                                if n in syns:
                                    self._patchTable[(sl.name, sp, tl.name,
                                                      tp, syns[n])] = p

                block.addMargin(popsep / 2., popsep / 2.)

        self._axes.addMargin(rmargin, bmargin)
        if showLegend:
            self._axes.addMargin(0, cbmargin)  # add color bar at bottom
            figwidth = self._axes.lr[0] - self._axes.tl[
                0] - rmargin  # keep right marg out of calc

            if mode == 'totals' or mode == 'population':

                # single patch at right edge, 20% of figure
                if plotParams.cbwidth:
                    lwidth = plotParams.cbwidth * figwidth
                else:
                    lwidth = 0.2 * figwidth
                    if lwidth > 100.0:  # colorbar shouldn't be wider than 10cm
                        lwidth = 100.0
                lheight = (plotParams.cbheight * cbmargin
                           if plotParams.cbheight else 0.3 * cbmargin)

                if plotParams.legend_location is None:
                    cblift = 0.9 * cbmargin
                else:
                    cblift = 0.7 * cbmargin

                self._cbPatches = self._Patch(self._axes.tl[0],
                                              self._axes.lr[1] - cblift,
                                              None, None,
                                              lwidth,
                                              lheight)
            else:
                # one patch per synapse type, 20% of figure or less
                # we need to get the synapse names in ascending order
                # of synapse indices
                snames = [s[0] for s in
                          sorted([(k, v) for k, v in self._synAttr.items()],
                                 key=lambda kv: kv[1].index)
                          ]
                snum = len(snames)

                if plotParams.cbwidth:
                    lwidth = plotParams.cbwidth * figwidth
                    if plotParams.cbspace:
                        lstep = plotParams.cbspace * figwidth
                    else:
                        lstep = 0.5 * lwidth
                else:
                    if snum < 5:
                        lwidth = 0.15 * figwidth
                        lstep = 0.1 * figwidth
                    else:
                        lwidth = figwidth / (snum + 1.0)
                        lstep = (figwidth - snum * lwidth) / (snum - 1.0)
                    if lwidth > 100.0:  # colorbar shouldn't be wider than 10cm
                        lwidth = 100.0
                        lstep = 30.0
                lheight = (plotParams.cbheight * cbmargin
                           if plotParams.cbheight else 0.3 * cbmargin)

                if plotParams.cboffset is not None:
                    offset = plotParams.cboffset
                else:
                    offset = lstep

                if plotParams.legend_location is None:
                    cblift = 0.9 * cbmargin
                else:
                    cblift = 0.7 * cbmargin

                self._cbPatches = {}
                for j in range(snum):
                    self._cbPatches[snames[j]] = \
                        self._Patch(
                            self._axes.tl[0] + offset + j * (lstep + lwidth),
                            self._axes.lr[1] - cblift,
                            None, None,
                            lwidth,
                            lheight)

    # ------------------------------------------------------------------------

    def _scaledBox(self, p):
        """Scaled axes rectangle for patch, reverses y-direction."""
        xsc, ysc = self._axes.lr
        return self._figscale * np.array(
            [p.l / xsc, 1 - (p.t + p.h) / ysc, p.w / xsc, p.h / ysc])

    # ------------------------------------------------------------------------

    def _scaledBoxNR(self, p):
        """Scaled axes rectangle for patch, does not reverse y-direction."""
        xsc, ysc = self._axes.lr
        return self._figscale * np.array(
            [p.l / xsc, p.t / ysc, p.w / xsc, p.h / ysc])

    # ------------------------------------------------------------------------

    def _configSynapses(self, cList, synTypes):
        """Configure synapse information based on connections and user info."""

        # compile information on synapse types and weights
        synnames = set(c[2]['synapse_model'] for c in cList)
        synweights = set(_weighteval(c[2]['weights']) for c in cList)

        # set up synTypes for all pre-defined cases
        if synTypes:
            # check if there is info for all synapse types
            stnames = _flattened([[s.name for s in r] for r in synTypes])
            if len(stnames) != len(set(stnames)):
                raise ValueError(
                    'Names of synapse types in synTypes must be unique!')
            if len(synnames) > 1 and not synnames.issubset(set(stnames)):
                raise ValueError('synTypes must provide information about' +
                                 'all synapse types.')

        elif len(synnames) == 1:
            # only one synapse type used
            if min(synweights) >= 0:
                # all weights positive
                synTypes = ((SynType('exc', 1.0, 'red'),),)
            elif max(synweights) <= 0:
                # all weights negative
                synTypes = ((SynType('inh', -1.0, 'blue'),),)
            else:
                # positive and negative weights, assume Dale holds
                synTypes = ((SynType('exc', 1.0, 'red'),),
                            (SynType('inh', -1.0, 'blue'),))

        elif synnames == set(['AMPA', 'GABA_A']):
            # only AMPA and GABA_A
            synTypes = ((SynType('AMPA', 1.0, 'red'),),
                        (SynType('GABA_A', -1.0, 'blue'),))

        elif synnames.issubset(set(['AMPA', 'NMDA', 'GABA_A', 'GABA_B'])):
            synTypes = ((SynType('AMPA', 1.0, 'red'),
                         SynType('NMDA', 1.0, 'orange'),),
                        (SynType('GABA_A', -1.0, 'blue'),
                         SynType('GABA_B', -1.0, 'purple'),))

        else:
            raise ValueError('Connection list contains unknown synapse ' +
                             'models; synTypes required.')

        # now build _synAttr by assigning blocks to rows
        self._synAttr = {}
        row = 0
        ctr = 0
        for sgroup in synTypes:
            col = 0
            for stype in sgroup:
                self._synAttr[stype.name] = self._SynProps(row, col,
                                                           stype.relweight,
                                                           stype.cmap, ctr)
                col += 1
                ctr += 1
            row += 1

    # ------------------------------------------------------------------------

    def __init__(self, lList, cList, synTypes=None, intensity='wp',
                 mList=None, Vmem=None, poporder=None):
        """
        lList    : layer list
        cList    : connection list
        synTypes : nested list of synapse types
        intensity: 'wp'  - weight * probability
                   'p'   - probability
                   'tcd' - |total charge deposited| * probability
                           requires mList; currently only for ht_model
                           proper results only if Vmem within reversal
                           potentials
        mList    : model list; only needed with 'tcd'
        Vmem     : reference membrane potential for 'tcd'
        poporder : dictionary mapping population names to numbers; populations
                   will be sorted in diagram in order of increasing numbers.
        """
        # extract layers to dict mapping name to extent
        self._layers = [self._LayerProps(l[0], l[1]['extent']) for l in lList]

        # ensure layer names are unique
        lnames = [l.name for l in self._layers]
        if len(lnames) != len(set(lnames)):
            raise ValueError('Layer names must be unique.')

        # set up synapse attributes
        self._configSynapses(cList, synTypes)

        # if tcd mode, build tcd representation
        if intensity != 'tcd':
            tcd = None
        else:
            assert (mList)
            from . import tcd_nest
            tcd = tcd_nest.TCD(mList)

        # Build internal representation of connections.
        # This representation contains one entry for each sender pop,
        # target pop, synapse type tuple. Creating the connection object
        # implies computation of the kernel.
        # Several connection may agree in all properties, these need to be
        # added here. Therefore, we need to build iteratively and store
        # everything in a dictionary, so we can find early instances.
        self._cTable = {}
        for conn in cList:
            key, val = self._Connection(conn, self._layers, self._synAttr,
                                        intensity, tcd, Vmem).keyval
            if key:
                if key in self._cTable:
                    self._cTable[key].append(val)
                else:
                    self._cTable[key] = [val]

        # number of layers
        self._nlyr = len(self._layers)

        # compile list of populations, list(set()) makes list unique
        self._pops = list(
            set(_flattened([[(c.slayer, c.snrn), (c.tlayer, c.tnrn)]
                            for c in _flattened(self._cTable.values())])))
        self._npop = len(self._pops)

        # store population ordering; if not given, use alphabetical ordering
        # also add any missing populations alphabetically at end
        # layers are ignored
        # create alphabetically sorted list of unique population names
        popnames = sorted(list(set([p[1] for p in self._pops])),
                          key=lambda x: x if x is not None else "")
        if poporder:
            self._poporder = poporder
            next = max(self._poporder.values()) + 1  # next free sorting index
        else:
            self._poporder = {}
            next = 0
        for pname in popnames:
            if pname not in self._poporder:
                self._poporder[pname] = next
                next += 1

        # compile list of synapse types
        self._synTypes = list(
            set([c.synmodel for c in _flattened(self._cTable.values())]))

    # ------------------------------------------------------------------------

    def plot(self, aggrGroups=False, aggrSyns=False, globalColors=False,
             colorLimits=None, showLegend=True,
             selectSyns=None, file=None, fixedWidth=None):
        """
        Plot connection pattern.
        By default, connections between any pair of populations
        are plotted on the screen, with separate color scales for
        all patches.

        Arguments:
        aggrGroups  If True, aggregate projections with the same synapse type
                    and the same source and target groups (default: False)

        aggrSyns    If True, aggregate projections with the same synapse model
                    (default: False)

        globalColors  If True, use global color scale, otherwise local
                      (default: False)

        colorLimits   If given, must be two element vector for lower and
                      upper limits of color scale. Implies globalColors
                      (default: None)

        showLegend   If True, show legend below CPT (default: True).

        selectSyns    If tuple of synapse models, show only connections of the
                      give types. Cannot be combined with aggregation.

        file     If given, save plot to given file name; file may also be a
                 tuple of file names, the figure will then be saved to all
                 files. This may be useful if you want to save the same figure
                 in several formats.

        fixedWidth Figure will be scaled to this width in mm by changing
                   patch size.

        Returns:
        kern_min, kern_max   Minimal and maximal values of kernels,
                             with kern_min <= 0, kern_max >= 0.

        Output:
        figure created
        """

        # translate new to old paramter names (per v 0.5)
        normalize = globalColors
        if colorLimits:
            normalize = True

        if selectSyns:
            if aggrPops or aggrSyns:
                raise ValueError(
                    'selectSyns cannot be combined with aggregation.')
            selected = selectSyns
            mode = 'select'
        elif aggrGroups and aggrSyns:
            mode = 'totals'
        elif aggrGroups and not aggrSyns:
            mode = 'layer'
        elif aggrSyns and not aggrGroups:
            mode = 'population'
        else:
            mode = None

        if mode == 'layer':
            # reduce to dimensions sender layer, target layer, synapse type
            # add all kernels agreeing on these three attributes
            plotKerns = []
            for slayer in self._layers:
                for tlayer in self._layers:
                    for synmodel in self._synTypes:
                        kerns = [c.kernval for c in
                                 _flattened(self._cTable.values())
                                 if c.matches(sl=slayer.name, tl=tlayer.name,
                                              syn=synmodel)]
                        if len(kerns) > 0:
                            plotKerns.append(
                                self._PlotKern(slayer.name, None, tlayer.name,
                                               None, synmodel,
                                               _addKernels(kerns)))
        elif mode == 'population':
            # reduce to dimensions sender layer, target layer
            # all all kernels, weighting according to synapse type
            plotKerns = []
            for spop in self._pops:
                for tpop in self._pops:
                    kerns = [self._synAttr[c.synmodel].tw * c.kernval for c in
                             _flattened(self._cTable.values())
                             if c.matches(sl=spop[0], sn=spop[1], tl=tpop[0],
                                          tn=tpop[1])]
                    if len(kerns) > 0:
                        plotKerns.append(
                            self._PlotKern(spop[0], spop[1], tpop[0], tpop[1],
                                           None,
                                           _addKernels(kerns)))
        elif mode == 'totals':
            # reduce to dimensions sender layer, target layer
            # all all kernels, weighting according to synapse type
            plotKerns = []
            for slayer in self._layers:
                for tlayer in self._layers:
                    kerns = [self._synAttr[c.synmodel].tw * c.kernval for c in
                             _flattened(self._cTable.values())
                             if c.matches(sl=slayer.name, tl=tlayer.name)]
                    if len(kerns) > 0:
                        plotKerns.append(
                            self._PlotKern(slayer.name, None, tlayer.name,
                                           None, None, _addKernels(kerns)))
        elif mode == 'select':
            # copy only those kernels that have the requested synapse type,
            # no dimension reduction
            # We need to sum all kernels in the list for a set of attributes
            plotKerns = [
                self._PlotKern(clist[0].slayer, clist[0].snrn, clist[0].tlayer,
                               clist[0].tnrn,
                               clist[0].synmodel,
                               _addKernels([c.kernval for c in clist]))
                for clist in self._cTable.values() if
                clist[0].synmodel in selected]
        else:
            # copy all
            # We need to sum all kernels in the list for a set of attributes
            plotKerns = [
                self._PlotKern(clist[0].slayer, clist[0].snrn, clist[0].tlayer,
                               clist[0].tnrn,
                               clist[0].synmodel,
                               _addKernels([c.kernval for c in clist]))
                for clist in self._cTable.values()]

        self._prepareAxes(mode, showLegend)
        if fixedWidth:

            margs = plotParams.margins.left + plotParams.margins.right

            if fixedWidth <= margs:
                raise ValueError('Requested width must be less than ' +
                                 'width of margins (%g mm)' % margs)

            currWidth = self._axes.lr[0]
            currPatchMax = plotParams.patch_size  # store

            # compute required patch size
            plotParams.patch_size = ((fixedWidth - margs) /
                                     (currWidth - margs) * currPatchMax)

            # build new axes
            del self._axes
            self._prepareAxes(mode, showLegend)

            # restore patch size
            plotParams.patch_size = currPatchMax

        # create figure with desired size
        fsize = np.array(self._axes.lr) / 25.4  # convert mm to inches
        f = plt.figure(figsize=fsize, facecolor='w')

        # size will be rounded according to DPI setting, adjust fsize
        dpi = f.get_dpi()
        fsize = np.floor(fsize * dpi) / dpi

        # check that we got the correct size
        actsize = np.array([f.get_figwidth(), f.get_figheight()], dtype=float)
        if all(actsize == fsize):
            self._figscale = 1.0  # no scaling
        else:
            warnings.warn("""
                  WARNING: Figure shrunk on screen!
                  The figure is shrunk to fit onto the screen.

                  Please specify a different backend using the -d
                  option to obtain full-size figures. Your current
                  backend is: %s
                  """ % mpl.get_backend())
            plt.close(f)

            # determine scale: most shrunk dimension
            self._figscale = np.min(actsize / fsize)

            # create shrunk on-screen figure
            f = plt.figure(figsize=self._figscale * fsize, facecolor='w')

            # just ensure all is well now
            actsize = np.array([f.get_figwidth(), f.get_figheight()],
                               dtype=float)

        # add decoration
        for block in _flattened(self._axes.elements):

            ax = f.add_axes(self._scaledBox(block),
                            axisbg=plotParams.layer_bg[block.location],
                            xticks=[], yticks=[],
                            zorder=plotParams.z_layer)
            if hasattr(ax, 'frame'):
                ax.frame.set_visible(False)
            else:
                for sp in ax.spines.values():
                    # turn off axis lines, make room for frame edge
                    sp.set_color('none')
            if block.l <= self._axes.l_patches and block.slbl:
                ax.set_ylabel(block.slbl,
                              rotation=plotParams.layer_orientation['sender'],
                              fontproperties=plotParams.layer_font)
            if block.t <= self._axes.t_patches and block.tlbl:
                ax.set_xlabel(block.tlbl,
                              rotation=plotParams.layer_orientation['target'],
                              fontproperties=plotParams.layer_font)
                ax.xaxis.set_label_position('top')

            # inner blocks for population labels
            if mode not in ('totals', 'layer'):
                for pb in _flattened(block.elements):
                    if not isinstance(pb, self._Block):
                        continue  # should not happen

                    ax = f.add_axes(self._scaledBox(pb),
                                    axisbg='none', xticks=[], yticks=[],
                                    zorder=plotParams.z_pop)
                    if hasattr(ax, 'frame'):
                        ax.frame.set_visible(False)
                    else:
                        for sp in ax.spines.values():
                            # turn off axis lines, make room for frame edge
                            sp.set_color('none')
                    if pb.l + pb.w >= self._axes.r_patches and pb.slbl:
                        ax.set_ylabel(pb.slbl,
                                      rotation=plotParams.pop_orientation[
                                          'sender'],
                                      fontproperties=plotParams.pop_font)
                        ax.yaxis.set_label_position('right')
                    if pb.t + pb.h >= self._axes.b_patches and pb.tlbl:
                        ax.set_xlabel(pb.tlbl,
                                      rotation=plotParams.pop_orientation[
                                          'target'],
                                      fontproperties=plotParams.pop_font)

        # determine minimum and maximum values across all kernels,
        # but set min <= 0, max >= 0
        kern_max = max(0.0, max([np.max(kern.kern) for kern in plotKerns]))
        kern_min = min(0.0, min([np.min(kern.kern) for kern in plotKerns]))

        # determine color limits for plots
        if colorLimits:
            c_min, c_max = colorLimits  # explicit values

        else:
            # default values for color limits
            # always 0 as lower limit so anything > 0 is non-white,
            # except when totals or populations
            c_min = None if mode in ('totals', 'population') else 0.0
            c_max = None  # use patch maximum as upper limit

            if normalize:
                # use overall maximum, at least 0
                c_max = kern_max
                if aggrSyns:
                    # use overall minimum, if negative, otherwise 0
                    c_min = kern_min
                    # for c_max, use the larger of the two absolute values
                    c_max = kern_max
                    # if c_min is non-zero, use same color scale for neg values
                    if c_min < 0:
                        c_min = -c_max

        # Initialize dict storing sample patches for each synapse type for use
        # in creating color bars. We will store the last patch of any given
        # synapse type for reference. When aggrSyns, we have only one patch
        #  type and store that.
        if not aggrSyns:
            samplePatches = dict(
                [(sname, None) for sname in self._synAttr.keys()])
        else:
            # only single type of patches
            samplePatches = None

        for kern in plotKerns:
            p = self._patchTable[(kern.sl, kern.sn, kern.tl,
                                  kern.tn, kern.syn)]

            p.ax = f.add_axes(self._scaledBox(p), aspect='equal',
                              xticks=[], yticks=[], zorder=plotParams.z_conn)
            p.ax.patch.set_edgecolor('none')
            if hasattr(p.ax, 'frame'):
                p.ax.frame.set_visible(False)
            else:
                for sp in p.ax.spines.values():
                    # turn off axis lines, make room for frame edge
                    sp.set_color('none')

            if not aggrSyns:
                # we have synapse information -> not totals, a vals positive
                assert (kern.syn)
                assert (np.min(kern.kern) >= 0.0)

                # we may overwrite here, but this does not matter, we only need
                # some reference patch
                samplePatches[kern.syn] = p.ax.imshow(kern.kern,
                                                      vmin=c_min, vmax=c_max,
                                                      cmap=self._synAttr[
                                                          kern.syn].cmap)  # ,
                # interpolation='nearest')

            else:
                # we have totals, special color table and normalization
                # we may overwrite here, but this does not matter, we only need
                # some reference patch
                samplePatches = p.ax.imshow(kern.kern,
                                            vmin=c_min, vmax=c_max,
                                            cmap=cm.bluered,
                                            norm=cm.ZeroCenterNorm())
                # interpolation='nearest')

        # Create colorbars at bottom of figure
        if showLegend:

            # FIXME: rewrite the function to avoid comparisons with None!
            f_min = float("-inf") if c_min is None else c_min
            f_max = float("-inf") if c_max is None else c_max

            # Do we have kernel values exceeding the color limits?
            if f_min <= kern_min and kern_max <= f_max:
                extmode = 'neither'
            elif f_min > kern_min and kern_max <= f_max:
                extmode = 'min'
            elif f_min <= kern_min and kern_max > f_max:
                extmode = 'max'
            else:
                extmode = 'both'

            if aggrSyns:
                cbax = f.add_axes(self._scaledBox(self._cbPatches))

                # by default, use 4 ticks to avoid clogging
                # according to docu, we need a separate Locator object
                # for each axis.
                if plotParams.legend_ticks:
                    tcks = plotParams.legend_ticks
                else:
                    tcks = mpl.ticker.MaxNLocator(nbins=4)

                if normalize:
                    # colorbar with freely settable ticks
                    cb = f.colorbar(samplePatches, cax=cbax,
                                    orientation='horizontal',
                                    ticks=tcks,
                                    format=plotParams.legend_tick_format,
                                    extend=extmode)
                else:
                    # colorbar with tick labels 'Exc', 'Inh'
                    # we add the color bare here explicitly, so we get no
                    # problems if the sample patch includes only pos or
                    # only neg values
                    cb = mpl.colorbar.ColorbarBase(cbax, cmap=cm.bluered,
                                                   orientation='horizontal')
                    cbax.set_xticks([0, 1])
                    cbax.set_xticklabels(['Inh', 'Exc'])

                cb.outline.set_linewidth(0.5)  # narrower line around colorbar

                # fix font for ticks
                plt.setp(cbax.get_xticklabels(),
                         fontproperties=plotParams.legend_tick_font)

                # no title in this case

            else:
                # loop over synapse types
                for syn in self._synAttr.keys():
                    cbax = f.add_axes(self._scaledBox(self._cbPatches[syn]))
                    if plotParams.legend_location is None:
                        cbax.set_ylabel(
                            syn,
                            fontproperties=plotParams.legend_title_font,
                            rotation='horizontal')
                    else:
                        cbax.set_title(
                            syn,
                            fontproperties=plotParams.legend_title_font,
                            rotation='horizontal')

                    if normalize:

                        # by default, use 4 ticks to avoid clogging
                        # according to docu, we need a separate Locator object
                        # for each axis.
                        if plotParams.legend_ticks:
                            tcks = plotParams.legend_ticks
                        else:
                            tcks = mpl.ticker.MaxNLocator(nbins=4)

                        # proper colorbar
                        cb = f.colorbar(samplePatches[syn], cax=cbax,
                                        orientation='horizontal',
                                        ticks=tcks,
                                        format=plotParams.legend_tick_format,
                                        extend=extmode)
                        cb.outline.set_linewidth(
                            0.5)  # narrower line around colorbar

                        # fix font for ticks
                        plt.setp(cbax.get_xticklabels(),
                                 fontproperties=plotParams.legend_tick_font)

                    else:
                        # just a solid color bar with no ticks
                        cbax.set_xticks([])
                        cbax.set_yticks([])

                        # full-intensity color from color map
                        cbax.set_axis_bgcolor(self._synAttr[syn].cmap(1.0))

                        # narrower border
                        if hasattr(cbax, 'frame'):
                            cbax.frame.set_linewidth(0.5)
                        else:
                            for sp in cbax.spines.values():
                                sp.set_linewidth(0.5)

        # save to file(s), use full size
        f.set_size_inches(fsize)
        if isinstance(file, (list, tuple)):
            for fn in file:
                f.savefig(fn)
        elif isinstance(file, str):
            f.savefig(file)
        f.set_size_inches(actsize)  # reset size for further interactive work

        return kern_min, kern_max

    # ------------------------------------------------------------------------

    def toLaTeX(self, file, standalone=False, enumerate=False, legend=True):
        """
        Write connection table to file.

        Arguments:
        file        output file name
        standalone  create complete LaTeX file (default: False)
        enumerate   enumerate connections (default: False)
        legend      add explanation of functions used (default: True)
        """

        lfile = open(file, 'w')
        if not lfile:
            raise Exception('Could not open file "%s"' % file)

        if standalone:
            lfile.write(
                r"""
                \documentclass[a4paper,american]{article}
                \usepackage[pdftex,margin=1in,centering,
                            noheadfoot,a4paper]{geometry}
                \usepackage[T1]{fontenc}
                \usepackage[utf8]{inputenc}

                \usepackage{color}
                \usepackage{calc}
                \usepackage{tabularx}  % autom. adjusts column width in tables
                \usepackage{multirow}  % allows entries spanning several rows
                \usepackage{colortbl}  % allows coloring tables
                \usepackage[fleqn]{amsmath}
                \setlength{\mathindent}{0em}

                \usepackage{mathpazo}
                \usepackage[scaled=.95]{helvet}
                \renewcommand\familydefault{\sfdefault}

                \renewcommand\arraystretch{1.2}
                \pagestyle{empty}

                % \hdr{ncols}{label}{title}
                %
                % Typeset header bar across table with ncols columns
                % with label at left margin and centered title
                %
                \newcommand{\hdr}[3]{%
                  \multicolumn{#1}{|l|}{%
                    \color{white}\cellcolor[gray]{0.0}%
                    \textbf{\makebox[0pt]{#2}\hspace{0.5\linewidth}%
                            \makebox[0pt][c]{#3}}%
                  }%
                }

                \begin{document}
                """)
        lfile.write(
            r"""
            \noindent\begin{tabularx}{\linewidth}{%s|l|l|l|c|c|X|}\hline
            \hdr{%d}{}{Connectivity}\\\hline
            %s \textbf{Src} & \textbf{Tgt} & \textbf{Syn} &
            \textbf{Wght} & \textbf{Mask} & \textbf{Kernel} \\\hline
            """ % (('|r', 7, '&') if enumerate else ('', 6, '')))
        # ensure sorting according to keys, gives some alphabetic sorting
        haveU, haveG = False, False

        cctr = 0  # connection counter
        for ckey in sorted(self._cTable.keys()):
            for conn in self._cTable[ckey]:

                cctr += 1
                if enumerate:
                    lfile.write('%d &' % cctr)

                # take care to escape _ in names such as GABA_A
                # also remove any pending '/None'
                lfile.write((r'%s/%s & %s/%s & %s' %
                             (conn.slayer, conn.snrn, conn.tlayer, conn.tnrn,
                              conn.synmodel)).replace('_', r'\_').replace(
                    '/None', ''))
                lfile.write(' & \n')

                if isinstance(conn.weight, (int, float)):
                    lfile.write(r'%g' % conn.weight)
                elif 'uniform' in conn.weight:
                    cw = conn.weight['uniform']
                    lfile.write(
                        r'$\mathcal{U}[%g, %g)$' % (cw['min'], cw['max']))
                    haveU = True
                else:
                    raise ValueError(
                        'Unkown weight type "%s"' % conn.weight.__str__)
                lfile.write(' & \n')

                if 'circular' in conn.mask:
                    lfile.write(r'$\leq %g$' % conn.mask['circular']['radius'])
                elif 'rectangular' in conn.mask:
                    cmr = conn.mask['rectangular']
                    lfile.write(
                        r"""$[(%+g, %+g), (%+g, %+g)]$"""
                        % (cmr['lower_left'][0], cmr['lower_left'][1],
                           cmr['upper_right'][0], cmr['upper_right'][1]))
                else:
                    raise ValueError(
                        'Unknown mask type "%s"' % conn.mask.__str__)
                lfile.write(' & \n')

                if isinstance(conn.kernel, (int, float)):
                    lfile.write(r'$%g$' % conn.kernel)
                elif 'gaussian' in conn.kernel:
                    ckg = conn.kernel['gaussian']
                    lfile.write(r'$\mathcal{G}(p_0 = %g, \sigma = %g)$' %
                                (ckg['p_center'], ckg['sigma']))
                    haveG = True
                else:
                    raise ValueError(
                        'Unkown kernel type "%s"' % conn.kernel.__str__)
                lfile.write('\n')

                lfile.write(r'\\\hline' '\n')

        if legend and (haveU or haveG):
            # add bottom line with legend
            lfile.write(r'\hline' '\n')
            lfile.write(r'\multicolumn{%d}{|l|}{\footnotesize ' %
                        (7 if enumerate else 6))
            if haveG:
                lfile.write(r'$\mathcal{G}(p_0, \sigma)$: ' +
                            r'$p(\mathbf{x})=p_0 e^{-\mathbf{x}^2/2\sigma^2}$')
            if haveG and haveU:
                lfile.write(r', ')
            if haveU:
                lfile.write(
                    r'$\mathcal{U}[a, b)$: uniform distribution on $[a, b)$')
            lfile.write(r'}\\\hline' '\n')

        lfile.write(r'\end{tabularx}' '\n\n')

        if standalone:
            lfile.write(r'\end{document}''\n')

        lfile.close()


# ----------------------------------------------------------------------------

def _evalkernel(mask, kernel, weight, extent, intensity, tcd):
    """
    Plot kernel within extent.

    Kernel values are multiplied with abs(weight). If weight is a
    distribution, the mean value is used.

    Result is a masked array, in which the values outside the mask are
    masked.
    """

    # determine resolution, number of data points
    dx = max(extent) / plotParams.n_kern
    nx = np.ceil(extent[0] / dx)
    ny = np.ceil(extent[1] / dx)

    x = np.linspace(-0.5 * extent[0], 0.5 * extent[0], nx)
    y = np.linspace(-0.5 * extent[1], 0.5 * extent[1], ny)
    X, Y = np.meshgrid(x, y)

    if intensity == 'wp':
        return np.ma.masked_array(abs(weight) * _kerneval(X, Y, kernel),
                                  np.logical_not(_maskeval(X, Y, mask)))
    elif intensity == 'p':
        return np.ma.masked_array(_kerneval(X, Y, kernel),
                                  np.logical_not(_maskeval(X, Y, mask)))
    elif intensity == 'tcd':
        return np.ma.masked_array(
            abs(tcd) * abs(weight) * _kerneval(X, Y, kernel),
            np.logical_not(_maskeval(X, Y, mask)))


# ----------------------------------------------------------------------------

def _weighteval(weight):
    """Returns weight, or mean of distribution, signed."""

    w = None
    if isinstance(weight, (float, int)):
        w = weight
    elif isinstance(weight, dict):
        assert (len(weight) == 1)
        if 'uniform' in weight:
            w = 0.5 * (weight['uniform']['min'] + weight['uniform']['max'])
        elif 'gaussian' in weight:
            w = weight['gaussian']['mean']
        else:
            raise Exception(
                'Unknown weight type "%s"' % tuple(weight.keys())[0])

    if not w:
        raise Exception('Cannot handle weight.')

    return float(w)


# ----------------------------------------------------------------------------

def _maskeval(x, y, mask):
    """
    Evaluate mask given as topology style dict at
    (x,y). Assume x,y are 2d numpy matrices.
    """

    assert (len(mask) == 1)
    if 'circular' in mask:
        r = mask['circular']['radius']
        m = x ** 2 + y ** 2 <= r ** 2
    elif 'doughnut' in mask:
        ri = mask['doughnut']['inner_radius']
        ro = mask['doughnut']['outer_radius']
        d = x ** 2 + y ** 2
        m = np.logical_and(ri <= d, d <= ro)
    elif 'rectangular' in mask:
        ll = mask['rectangular']['lower_left']
        ur = mask['rectangular']['upper_right']
        m = np.logical_and(np.logical_and(ll[0] <= x, x <= ur[0]),
                           np.logical_and(ll[1] <= y, y <= ur[1]))
    else:
        raise Exception('Unknown mask type "%s"' % tuple(mask.keys())[0])

    return m


# ----------------------------------------------------------------------------

def _kerneval(x, y, fun):
    """
    Evaluate function given as topology style dict at
    (x,y). Assume x,y are 2d numpy matrices
    """

    if isinstance(fun, (float, int)):
        return float(fun) * np.ones(np.shape(x))
    elif isinstance(fun, dict):
        assert (len(fun) == 1)

    if 'gaussian' in fun:
        g = fun['gaussian']
        p0 = g['p_center']
        sig = g['sigma']
        return p0 * np.exp(-0.5 * (x ** 2 + y ** 2) / sig ** 2)
    else:
        raise Exception('Unknown kernel "%s"', tuple(fun.keys())[0])

    # something very wrong
    raise Exception('Cannot handle kernel.')


# ----------------------------------------------------------------------------

def _addKernels(kList):
    """
    Add a list of kernels.

    Arguments:
    kList:  List of masked arrays of equal size.

    Returns:
    Masked array of same size as input. All values are added,
    setting masked values to 0. The mask for the sum is the
    logical AND of all individual masks, so that only such
    values are masked that are masked in all kernels.
    _addKernels always returns a new array object, even if
    kList has only a single element.
    """

    assert (len(kList) > 0)

    if len(kList) < 2:
        return kList[0].copy()

    d = np.ma.filled(kList[0], fill_value=0).copy()
    m = kList[0].mask.copy()
    for k in kList[1:]:
        d += np.ma.filled(k, fill_value=0)
        m = np.logical_and(m, k.mask)

    return np.ma.masked_array(d, m)


# ----------------------------------------------------------------------------

def _flattened(lst):
    """Returned list flattend at first level."""
    return sum(lst, [])


# ----------------------------------------------------------------------------

"""
if __name__ == "__main__":

    import sys
    sys.path += ['./examples']

#    import simple
#    reload(simple)
    cp = ConnectionPattern(simple.layerList, simple.connectList)

    import simple2
    reload(simple2)
    cp2 = ConnectionPattern(simple2.layerList, simple2.connectList)

    st3 = ((SynType('GABA_B', -5.0, 'orange'),
            SynType('GABA_A', -1.0, 'm')),
           (SynType('NMDA', 5.0, 'b'),
            SynType('FOO', 1.0, 'aqua'),
            SynType('AMPA', 3.0, 'g')))
    cp3s = ConnectionPattern(simple2.layerList, simple2.connectList,
                             synTypes=st3)

    import simple3
    reload(simple3)
    cp3 = ConnectionPattern(simple3.layerList, simple3.connectList)

#    cp._prepareAxes('by layer')
#    cp2._prepareAxes('by layer')
#    cp3._prepareAxes('detailed')

    cp2.plot()
    cp2.plot(mode='layer')
    cp2.plot(mode='population')
    cp2.plot(mode='totals')
    cp2.plot(mode=('AMPA',))
    cp2.plot(mode=('AMPA','GABA_B'))
#    cp3.plot()
#    cp3.plot(mode='population')
#    cp3.plot(mode='layer')
#    cp3.plot(mode='totals')
#    cp.plot(normalize=True)
#    cp.plot(totals=True, normalize=True)
#    cp2.plot()
#    cp2.plot(file=('cp3.eps'))
#    cp2.plot(byLayer=True)
#    cp2.plot(totals=True)

"""
