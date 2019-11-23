# -*- coding: utf-8 -*-
#
# test_selection_function_and_elliptical_mask.py
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
Tests selection function and elliptical mask.
"""

import unittest
import nest


class SelectionFunctionAndEllipticalMask(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_SelectNodesByMaskIn2D(self):
        """Test SelectNodesByMask for rectangular mask in 2D layer"""

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11],
                                                        extent=[11., 11.]))
        maskdict = {'lower_left': [-2., -1.], 'upper_right': [2., 1.]}
        mask = nest.CreateMask('rectangular', maskdict)

        cntr = [0.0, 0.0]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids,
                         nest.NodeCollection([38, 39, 40, 49, 50, 51, 60, 61, 62, 71, 72, 73, 82, 83, 84]))

        nest.ResetKernel()

        cntr = [3., 3.]

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[5, 5],
                                                        extent=[11., 11.],
                                                        center=cntr))
        maskdict = {'lower_left': [1., 1.], 'upper_right': [5., 5.]}
        mask = nest.CreateMask('rectangular', maskdict)

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((16, 17, 21, 22,)))

    def test_SelectNodesByMaskIn3D(self):
        """Test SelectNodesByMask for rectangular mask in 3D layer"""

        pos = [[x*1., y*1., z*1.] for x in range(-5, 6)
               for y in range(-5, 6)
               for z in range(-5, 6)]
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'lower_left': [-6., -6., -6.],
                    'upper_right': [-4., -4., -4.]}
        mask = nest.CreateMask('box', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((1, 2, 12, 13, 122, 123, 133, 134,)))

    def test_CreateEllipticalMask2D(self):
        """Creates simple elliptical mask"""
        mask_dict = {'major_axis': 6.0, 'minor_axis': 3.0}
        mask = nest.CreateMask('elliptical', mask_dict)

        self.assertTrue(mask.Inside([0.0, 0.0]))

    def test_EllipticalMask2D(self):
        """Simple elliptical mask contains the correct node IDs"""

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11],
                                                        extent=[11., 11.]))
        maskdict = {'major_axis': 2.0, 'minor_axis': 1.0}
        mask = nest.CreateMask('elliptical', maskdict)

        cntr = [0.0, 0.0]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((50, 61, 72,)))

        maskdict = {'major_axis': 6.0, 'minor_axis': 3.0}
        mask = nest.CreateMask('elliptical', maskdict)

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids,
                         nest.NodeCollection([28, 38, 39, 40, 49, 50, 51, 60, 61, 62, 71, 72, 73,
                                             82, 83, 84, 94]))

    def test_EllipticalMask2DWithAnchor(self):
        """Anchored elliptical mask contains the correct node IDs"""

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11],
                                                        extent=[11., 11.]))
        maskdict = {'major_axis': 6.0, 'minor_axis': 3.0, 'anchor': [-2., -2.]}
        mask = nest.CreateMask('elliptical', maskdict)

        cntr = [0.0, 0.0]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids,
                         nest.NodeCollection((8, 18, 19, 20, 29, 30, 31, 40, 41, 42, 51, 52, 53,
                                             62, 63, 64, 74,)))

    def test_TiltedEllipticalMask2DWithAnchor(self):
        """Tilted and anchored elliptical mask contains the correct node IDs"""

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11],
                                                        extent=[11., 11.]))
        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'anchor': [3., 3.], 'azimuth_angle': 45.}
        mask = nest.CreateMask('elliptical', maskdict)

        cntr = [0.0, 0.0]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((81, 91, 101,)))

        maskdict = {'major_axis': 6.0, 'minor_axis': 3.0,
                    'anchor': [-1.5, 1.], 'azimuth_angle': 135.}
        mask = nest.CreateMask('elliptical', maskdict)

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids,
                         nest.NodeCollection([25, 26, 27, 36, 37, 38, 39, 48, 49, 50, 51, 60, 61, 62]))

        maskdict = {'major_axis': 8.0, 'minor_axis': 3.0,
                    'anchor': [0., 1.], 'azimuth_angle': 90.}
        mask = nest.CreateMask('elliptical', maskdict)

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids,
                         nest.NodeCollection([47, 48, 49, 50, 51, 56, 57, 58, 59, 60,
                                             61, 62, 63, 64, 69, 70, 71, 72, 73]))

    def test_EllipticalMask2DwithAnchorAndCenteredLayer(self):
        """Anchored elliptical mask contains correct node IDs when layer is not
        centered around origo"""

        cntr = [5.0, 5.0]

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[5, 5],
                                                        extent=[5., 5.],
                                                        center=cntr))
        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0}
        mask = nest.CreateMask('elliptical', maskdict)

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((8, 13, 18,)))

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0, 'anchor': [1., 1.]}
        mask = nest.CreateMask('elliptical', maskdict)

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((12, 17, 22,)))

    def test_EllipsoidalMask3D(self):
        """Simple ellipsoidal mask contains the correct node IDs"""

        pos = [[x*1., y*1., z*1.] for x in range(-5, 6)
               for y in range(-5, 6)
               for z in range(-5, 6)]
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((545, 666, 787,)))

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0, 'azimuth_angle': 90.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((655, 666, 677,)))

    def test_TiltedEllipsoidalMask(self):
        """Ellipsoidal mask contains correct node IDs when tilted with respect to
        x-axis and z-axis"""

        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0,
                    'polar_angle': 90.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((62, 63, 64,)))

        nest.ResetKernel()

        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 4.0, 'minor_axis': 1.,
                    'polar_axis': 1.5,
                    'azimuth_angle': 45.,
                    'polar_angle': 45.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection([34, 63, 92]))

        nest.ResetKernel()

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 3.0, 'minor_axis': 2.,
                    'polar_axis': 1.0,
                    'polar_angle': 45.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection([39, 58, 63, 68, 87]))

        nest.ResetKernel()

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 4.0, 'minor_axis': 1.,
                    'polar_axis': 1.5,
                    'polar_angle': 30.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection([38, 39, 63, 87, 88]))

        nest.ResetKernel()

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 4.0, 'minor_axis': 2.5,
                    'polar_axis': 1.0,
                    'azimuth_angle': 45.,
                    'polar_angle': 30.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection([34, 38, 58, 63, 68, 88, 92]))

    def test_TiltedEllipsoidalMask3DWithAnchor(self):
        """Tilted and anchored ellipsoidal mask contains the correct node IDs"""

        pos = [[x*1., y*1., z*1.] for x in range(-5, 6)
               for y in range(-5, 6)
               for z in range(-5, 6)]
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        maskdict = {'major_axis': 4.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0, 'anchor': [-5., -5., -4.]}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((2, 123, 244,)))

        maskdict = {'major_axis': 4., 'minor_axis': 1.,
                    'polar_axis': 1., 'anchor': [-4., -4., -4.],
                    'azimuth_angle': 45.}
        mask = nest.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        node_ids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(node_ids, nest.NodeCollection((2, 134, 266,)))


def suite():
    suite = unittest.makeSuite(SelectionFunctionAndEllipticalMask, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
