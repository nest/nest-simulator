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
import nest.topology as topo


class SelectionFunctionAndEllipticalMask(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_SelectNodesByMaskIn2D(self):
        """Test SelectNodesByMask for rectangular mask in 2D layer"""

        layer = topo.CreateLayer({'rows': 11, 'columns': 11,
                                  'extent': [11., 11.],
                                  'elements': 'iaf_psc_alpha'})
        maskdict = {'lower_left': [-2., -1.], 'upper_right': [2., 1.]}
        mask = topo.CreateMask('rectangular', maskdict)

        cntr = [0.0, 0.0]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        gid_list_sort = sorted(gid_list)

        self.assertEqual(gid_list_sort,
                         [39, 40, 41, 50, 51, 52, 61, 62, 63, 72, 73, 74, 83,
                          84, 85])

        nest.ResetKernel()

        cntr = [3., 3.]

        layer = topo.CreateLayer({'rows': 5, 'columns': 5,
                                  'extent': [11., 11.], 'center': cntr,
                                  'elements': 'iaf_psc_alpha'})
        maskdict = {'lower_left': [1., 1.], 'upper_right': [5., 5.]}
        mask = topo.CreateMask('rectangular', maskdict)

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (17, 18, 22, 23,))

    def test_SelectNodesByMaskIn3D(self):
        """Test SelectNodesByMask for rectangular mask in 3D layer"""

        pos = [[x*1., y*1., z*1.] for x in range(-5, 6)
               for y in range(-5, 6)
               for z in range(-5, 6)]
        layer = topo.CreateLayer({'positions': pos, 'extent': [11., 11., 11.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'lower_left': [-6., -6., -6.],
                    'upper_right': [-4., -4., -4.]}
        mask = topo.CreateMask('box', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (2, 3, 13, 14, 123, 124, 134, 135,))

    def test_CreateEllipticalMask2D(self):
        """Creates simple elliptical mask"""
        mask_dict = {'major_axis': 6.0, 'minor_axis': 3.0}
        mask = topo.CreateMask('elliptical', mask_dict)

        self.assertTrue(mask.Inside([0.0, 0.0]))

    def test_EllipticalMask2D(self):
        """Simple elliptical mask contains the correct GIDs"""

        layer = topo.CreateLayer({'rows': 11, 'columns': 11,
                                  'extent': [11., 11.],
                                  'elements': 'iaf_psc_alpha'})
        maskdict = {'major_axis': 2.0, 'minor_axis': 1.0}
        mask = topo.CreateMask('elliptical', maskdict)

        cntr = [0.0, 0.0]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (51, 62, 73,))

        maskdict = {'major_axis': 6.0, 'minor_axis': 3.0}
        mask = topo.CreateMask('elliptical', maskdict)

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        gid_list_sort = sorted(gid_list)

        self.assertEqual(gid_list_sort,
                         [29, 39, 40, 41, 50, 51, 52, 61, 62, 63, 72, 73, 74,
                          83, 84, 85, 95])

    def test_EllipticalMask2DWithAnchor(self):
        """Anchored elliptical mask contains the correct GIDs"""

        layer = topo.CreateLayer({'rows': 11, 'columns': 11,
                                  'extent': [11., 11.],
                                  'elements': 'iaf_psc_alpha'})
        maskdict = {'major_axis': 6.0, 'minor_axis': 3.0, 'anchor': [-2., -2.]}
        mask = topo.CreateMask('elliptical', maskdict)

        cntr = [0.0, 0.0]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list,
                         (9, 19, 20, 21, 30, 31, 32, 41, 42, 43, 52, 53, 54,
                          63, 64, 65, 75,))

    def test_TiltedEllipticalMask2DWithAnchor(self):
        """Tilted and anchored elliptical mask contains the correct GIDs"""

        layer = topo.CreateLayer({'rows': 11, 'columns': 11,
                                  'extent': [11., 11.],
                                  'elements': 'iaf_psc_alpha'})
        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'anchor': [3., 3.], 'azimuth_angle': 45.}
        mask = topo.CreateMask('elliptical', maskdict)

        cntr = [0.0, 0.0]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (82, 92, 102,))

        maskdict = {'major_axis': 6.0, 'minor_axis': 3.0,
                    'anchor': [-1.5, 1.], 'azimuth_angle': 135.}
        mask = topo.CreateMask('elliptical', maskdict)

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        gid_list_sort = sorted(gid_list)

        self.assertEqual(gid_list_sort,
                         [26, 27, 28, 37, 38, 39, 40, 49, 50, 51, 52, 61, 62,
                          63])

        maskdict = {'major_axis': 8.0, 'minor_axis': 3.0,
                    'anchor': [0., 1.], 'azimuth_angle': 90.}
        mask = topo.CreateMask('elliptical', maskdict)

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        gid_list_sort = sorted(gid_list)

        self.assertEqual(gid_list_sort,
                         [48, 49, 50, 51, 52, 57, 58, 59, 60, 61, 62, 63, 64,
                          65, 70, 71, 72, 73, 74])

    def test_EllipticalMask2DwithAnchorAndCenteredLayer(self):
        """Anchored elliptical mask contains correct GIDs when layer is not
        centered around origo"""

        cntr = [5.0, 5.0]

        layer = topo.CreateLayer({'rows': 5, 'columns': 5,
                                  'extent': [5., 5.], 'center': cntr,
                                  'elements': 'iaf_psc_alpha'})
        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0}
        mask = topo.CreateMask('elliptical', maskdict)

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (9, 14, 19,))

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0, 'anchor': [1., 1.]}
        mask = topo.CreateMask('elliptical', maskdict)

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (13, 18, 23,))

    def test_EllipsoidalMask3D(self):
        """Simple ellipsoidal mask contains the correct GIDs"""

        pos = [[x*1., y*1., z*1.] for x in range(-5, 6)
               for y in range(-5, 6)
               for z in range(-5, 6)]
        layer = topo.CreateLayer({'positions': pos, 'extent': [11., 11., 11.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (546, 667, 788,))

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0, 'azimuth_angle': 90.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (656, 667, 678,))

    def test_TiltedEllipsoidalMask(self):
        """Ellipsoidal mask contains correct GIDs when tilted with respect to
        x-axis and z-axis"""

        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 3.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0,
                    'polar_angle': 90.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (63, 64, 65,))

        nest.ResetKernel()

        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 4.0, 'minor_axis': 1.,
                    'polar_axis': 1.5,
                    'azimuth_angle': 45.,
                    'polar_angle': 45.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [35, 64, 93])

        nest.ResetKernel()

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 3.0, 'minor_axis': 2.,
                    'polar_axis': 1.0,
                    'polar_angle': 45.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [40, 59, 64, 69, 88])

        nest.ResetKernel()

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 4.0, 'minor_axis': 1.,
                    'polar_axis': 1.5,
                    'polar_angle': 30.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [39, 40, 64, 88, 89])

        nest.ResetKernel()

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 4.0, 'minor_axis': 2.5,
                    'polar_axis': 1.0,
                    'azimuth_angle': 45.,
                    'polar_angle': 30.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [35, 39, 59, 64, 69, 89, 93])

    def test_TiltedEllipsoidalMask3DWithAnchor(self):
        """Tilted and anchored ellipsoidal mask contains the correct GIDs"""

        pos = [[x*1., y*1., z*1.] for x in range(-5, 6)
               for y in range(-5, 6)
               for z in range(-5, 6)]
        layer = topo.CreateLayer({'positions': pos, 'extent': [11., 11., 11.],
                                  'elements': 'iaf_psc_alpha'})

        maskdict = {'major_axis': 4.0, 'minor_axis': 1.0,
                    'polar_axis': 1.0, 'anchor': [-5., -5., -4.]}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (3, 124, 245,))

        maskdict = {'major_axis': 4., 'minor_axis': 1.,
                    'polar_axis': 1., 'anchor': [-4., -4., -4.],
                    'azimuth_angle': 45.}
        mask = topo.CreateMask('ellipsoidal', maskdict)

        cntr = [0., 0., 0.]

        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (3, 135, 267,))


def suite():
    suite = unittest.makeSuite(SelectionFunctionAndEllipticalMask, 'test')
    return suite

if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
