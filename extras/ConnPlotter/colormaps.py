# -*- coding: utf-8 -*-
#
# colormaps.py
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
Colormaps for ConnPlotter.

Provides the following functions and colormaps:

 - make_colormap: based on color specification, create colormap
                  running from from white to fully saturated color
 - redblue: from fully saturated red to white to fully saturated blue
 - bluered: from fully saturated blue to white to fully saturated red

For all colormaps, "bad" values (NaN) are mapped to white.

Provides also ZeroCenterNorm, mapping negative values to 0..0.5,
positive to 0.5..1.
"""

# ----------------------------------------------------------------------------

import matplotlib.pyplot as plt
import matplotlib.colors as mc
import matplotlib.cbook as cbook
import numpy as np

__all__ = ['ZeroCenterNorm', 'make_colormap', 'redblue', 'bluered',
           'bad_color']

# ----------------------------------------------------------------------------

bad_color = (1.0, 1.0, 0.9)

# ----------------------------------------------------------------------------


class ZeroCenterNorm(mc.Normalize):
    """
    Normalize so that value 0 is always at 0.5.

    Code from matplotlib.colors.Normalize.
    Copyright (c) 2002-2009 John D. Hunter; All Rights Reserved
    http://matplotlib.sourceforge.net/users/license.html
    """

    # ------------------------------------------------------------------------

    def __call__(self, value, clip=None):
        """
        Normalize given values to [0,1].

        Returns data in same form as passed in.
        value can be scalar or array.
        """
        if clip is not None and clip is not False:
            assert (False)  # clip not supported

        if cbook.iterable(value):
            vtype = 'array'
            val = np.ma.asarray(value).astype(np.float)
        else:
            vtype = 'scalar'
            val = np.ma.array([value]).astype(np.float)

        self.autoscale_None(val)
        self.vmin = min(0, self.vmin)
        self.vmax = max(0, self.vmax)

        # imshow expects masked arrays
        # fill entire array with 0.5
        result = np.ma.array(0.5 * np.ma.asarray(np.ones(np.shape(val))),
                             dtype=np.float, mask=val.mask)

        # change values != 0
        result[val < 0] = 0.5 * (self.vmin - val[val < 0]) / self.vmin
        result[val > 0] = 0.5 + 0.5 * val[val > 0] / self.vmax

        if vtype == 'scalar':
            result = result[0]

        return result

    # ------------------------------------------------------------------------

    def inverse(self, value):
        """
        Invert color map. Required by colorbar().
        """

        if not self.scaled():
            raise ValueError("Not invertible until scaled")
        vmin, vmax = self.vmin, self.vmax

        if cbook.iterable(value):
            val = np.asarray(value)

            res = np.zeros(np.shape(val))
            res[val < 0.5] = vmin - 2 * vmin * val[val < 0.5]
            res[val > 0.5] = 2 * (val[val > 0.5] - 0.5) * vmax
            return res

        else:
            if value == 0.5:
                return 0
            elif value < 0.5:
                return vmin - 2 * vmin * value  # vmin < 0
            else:
                return 2 * (value - 0.5) * vmax


# ----------------------------------------------------------------------------

def make_colormap(color):
    """
    Create LinearSegmentedColormap ranging from white to the given color.
    Color can be given in any legal color format. Bad color is set to white.
    """

    try:
        r, g, b = mc.colorConverter.to_rgb(color)
    except:
        raise ValueError('Illegal color specification: %s' % color.__repr__)

    cm = mc.LinearSegmentedColormap(color.__str__(),
                                    {'red': [(0.0, 1.0, 1.0),
                                             (1.0, r, r)],
                                     'green': [(0.0, 1.0, 1.0),
                                               (1.0, g, g)],
                                     'blue': [(0.0, 1.0, 1.0),
                                              (1.0, b, b)]})
    cm.set_bad(color=bad_color)  # light yellow
    return cm


# ----------------------------------------------------------------------------

redblue = mc.LinearSegmentedColormap('redblue',
                                     {'red': [(0.0, 0.0, 1.0),
                                              (0.5, 1.0, 1.0),
                                              (1.0, 0.0, 0.0)],
                                      'green': [(0.0, 0.0, 0.0),
                                                (0.5, 1.0, 1.0),
                                                (1.0, 0.0, 0.0)],
                                      'blue': [(0.0, 0.0, 0.0),
                                               (0.5, 1.0, 1.0),
                                               (1.0, 1.0, 1.0)]})

redblue.set_bad(color=bad_color)

# ----------------------------------------------------------------------------

bluered = mc.LinearSegmentedColormap('bluered',
                                     {'red': [(0.0, 0.0, 0.0),
                                              (0.5, 1.0, 1.0),
                                              (1.0, 1.0, 1.0)],
                                      'green': [(0.0, 0.0, 0.0),
                                                (0.5, 1.0, 1.0),
                                                (1.0, 0.0, 0.0)],
                                      'blue': [(0.0, 1.0, 1.0),
                                               (0.5, 1.0, 1.0),
                                               (1.0, 0.0, 0.0)]})

bluered.set_bad(color=bad_color)

# ----------------------------------------------------------------------------

if __name__ == '__main__':

    # this should be proper unit tests
    n1 = ZeroCenterNorm()
    if (n1([-1, -0.5, 0.0, 0.5, 1.0]).data == np.array(
            [0, 0.25, 0.5, 0.75, 1.0])).all():
        print("n1 ok")
    else:
        print("n1 failed.")

    n2 = ZeroCenterNorm(-1, 2)
    if (n2([-1, -0.5, 0.0, 1.0, 2.0]).data == np.array(
            [0, 0.25, 0.5, 0.75, 1.0])).all():
        print("n2 ok")
    else:
        print("n2 failed.")
