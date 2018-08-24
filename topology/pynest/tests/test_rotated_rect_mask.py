# -*- coding: utf-8 -*-
#
# test_rotated_rect_mask.py
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
Tests rotated rectangular and box masks.
"""

import unittest
import nest
import nest.topology as topo


class RotatedRectangularMask(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_RotatedRectangularMask(self):
        """Test rotated rectangular mask.

            We have:
                lower_left:  [-1., -0.5]
                upper_right: [ 1.,  0.5]

            So, if we have:

            layer:
            2  7  12  17  22
            3  8  13  18  23
            4  9  14  19  24
            5  10 15  20  25
            6  11 16  21  26

            and have azimuth_angle = 0, we should get gids 9, 14, 19 if we
            select GIDs by mask.
            If we have azimuth_angle = 90, we should get gids 13, 14, 15.
        """

        # Test 2D layer
        layer = topo.CreateLayer({'rows': 5, 'columns': 5,
                                  'extent': [5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        # First test without rotation.
        maskdict = {'lower_left': [-1., -0.5], 'upper_right': [1., 0.5]}
        mask = topo.CreateMask('rectangular', maskdict)
        cntr = [0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (9, 14, 19,))

        # Test if we get correct GIDs when rotating 90 degrees.
        maskdict = {'lower_left': [-1., -0.5],
                    'upper_right': [1., 0.5],
                    'azimuth_angle': 90.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (13, 14, 15,))

        # Test rotation with an azimuth angle of 45 degrees.
        maskdict = {'lower_left': [-1.5, -0.5],
                    'upper_right': [1.5, 0.5],
                    'azimuth_angle': 45.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (10, 14, 18,))

        # Test rotation with an azimuth angle of 135 degrees.
        maskdict = {'lower_left': [-1.5, -0.5],
                    'upper_right': [1.5, 0.5],
                    'azimuth_angle': 135.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (8, 14, 20,))

        # Test that an error is raised if we send in a polar angle to a 2D
        # mask.
        maskdict = {'lower_left': [-1.5, -0.5],
                    'upper_right': [1.5, 0.5],
                    'polar_angle': 45.0}
        with self.assertRaises(nest.NESTError):
            mask = topo.CreateMask('rectangular', maskdict)

    def test_RotatedBoxMaskByAzimuthAngle(self):
        """Test rotated box mask with azimuth angle."""
        # Test a 3D layer.
        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        # First test that we get correct GIDs with box mask that is not
        # rotated.
        maskdict = {'lower_left': [-1., -0.5, -0.5],
                    'upper_right': [1., 0.5, 0.5]}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (39, 64, 89,))

        # Test with a larger box mask.
        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.]}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [38, 39, 40, 63, 64, 65, 88, 89, 90])

        # Test the smaller box mask with a rotation of 90 degrees. Only test
        # the azimuth angle, not the polar angle.
        maskdict = {'lower_left': [-1., -0.5, -0.5],
                    'upper_right': [1., 0.5, 0.5],
                    'azimuth_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (59, 64, 69,))

        # Test rotation of the larger box with an azimuth angle of 90 degrees.
        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.],
                    'azimuth_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [58, 59, 60, 63, 64, 65, 68, 69, 70])

    def test_RotatedBoxMaskByPolarAngle(self):
        """Test rotated box mask with polar angle."""

        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        # First test without rotation
        maskdict = {'lower_left': [-0.5, -1.0, -1.0],
                    'upper_right': [0.5, 1.0, 1.0]}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [58, 59, 60, 63, 64, 65, 68, 69, 70])

        # Test with a polar angle of 90 degrees.
        maskdict = {'lower_left': [-0.5, -1.0, -1.0],
                    'upper_right': [0.5, 1.0, 1.0],
                    'polar_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [34, 39, 44, 59, 64, 69, 84, 89, 94])

        # Test with a polar angle of 180 degrees, should be the same as the
        # one without a polar angle.
        maskdict = {'lower_left': [-0.5, -1.0, -1.0],
                    'upper_right': [0.5, 1.0, 1.0],
                    'polar_angle': 180.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [58, 59, 60, 63, 64, 65, 68, 69, 70])

        # Test with a polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5],
                    'upper_right': [0.5, 1.5, 1.5],
                    'polar_angle': 45.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [33, 38, 43, 59, 64, 69, 85, 90, 95])

        # Test with a polar angle of 135 degrees. The GIDs should be
        # perpendicular to the ones obtained by a polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5],
                    'upper_right': [0.5, 1.5, 1.5],
                    'polar_angle': 135.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [35, 40, 45, 59, 64, 69, 83, 88, 93])

        # Test two symmetric masks in x and z direction. One with no polar
        # angle and one with a polar angle of 90 degrees. As the masks are
        # symmetrical in  x and z, a polar angle of 90 degrees should give the
        # same GIDs as the one without a polar angle.
        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.]}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list_1 = sorted(gid_list)

        self.assertEqual(sorted_gid_list_1,
                         [38, 39, 40, 63, 64, 65, 88, 89, 90])

        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.],
                    'polar_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [38, 39, 40, 63, 64, 65, 88, 89, 90])

        self.assertEqual(sorted_gid_list_1, sorted_gid_list)

    def test_RotatedBoxMaskByAzimuthAndPolarAngle(self):
        """Test rotated box mask with azimuth and polar angle."""

        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        # Test with a azimuth angle and polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5],
                    'upper_right': [0.5, 1.5, 1.5],
                    'azimuth_angle': 45.,
                    'polar_angle': 45.}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list,
                         [38, 39, 44, 58, 59, 64, 69, 70, 84, 89, 90])

    def test_RotatedRectangleOutsideOrigin(self):
        """
        Test rotated rectangle where the mask does not contain the origin.
        """

        layer = topo.CreateLayer({'rows': 11, 'columns': 11,
                                  'extent': [11., 11.],
                                  'elements': 'iaf_psc_alpha'})

        # First test that we get the correct GIDs when our mask does not
        # contain the origin.
        maskdict = {'lower_left': [1., 1.], 'upper_right': [4., 2.]}
        mask = topo.CreateMask('rectangular', maskdict)
        cntr = [0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (71, 72, 82, 83, 93, 94, 104, 105,))

        # Then test that we get the correct GIDs with a azimuth rotation angle
        # of 45 degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [0.5, 0.5],
                    'upper_right': [4.5, 2.5],
                    'azimuth_angle': 45.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (72, 82, 83, 84, 92, 93, 94, 104,))

        # Test that we get the correct GIDs with a azimuth rotation angle
        # of 90 degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [1.0, 1.0],
                    'upper_right': [4.0, 2.0],
                    'azimuth_angle': 90.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (81, 82, 83, 84, 92, 93, 94, 95,))

    def test_RotatedBoxOutsideOrigin(self):
        """Test rotated box where the mask does not contain the origin."""

        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        # First test that we get the correct GIDs when our mask does not
        # contain the origin.
        maskdict = {'lower_left': [-2.0, -1.0, 0.5],
                    'upper_right': [-0.5, -0.5, 2.0]}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (10, 11, 35, 36,))

        # Test that we get the correct GIDs with a azimuth rotation angle of 45
        # degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [-2.5, -1.0, 0.5],
                    'upper_right': [-0.5, -0.5, 2.5],
                    'azimuth_angle': 45.0}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (10, 11, 40, 41,))

        # Test that we get the correct GIDs with a polar rotation angle of 45
        # degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [-1.5, -2.5, 0.5],
                    'upper_right': [-1.0, -0.5, 2.5],
                    'polar_angle': 45.0}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (5, 10, 31, 36,))

    def test_ConnectWithRotatedRectangleMask(self):
        """Test connection with rotated rectangle mask.

            We have: lower_left  = [-1.5, -0.5]
                     upper_right = [ 1.5,  0.5]
                     azimuth_angle = 45 degrees

            Each source node should then connect to:
                - The node in the same position in target layer
                - The node above the node to the right of that position
                - The node below the node to the left of the position.

            So, if we have

            sources:                    targets:
            2  7  12  17  22            28  33  38  43  48
            3  8  13  18  23            29  34  39  44  49
            4  9  14  19  24            30  35  40  45  50
            5  10 15  20  25            31  36  41  46  51
            6  11 16  21  26            32  37  42  47  52

            some example connections will be:
                            ______
                          /       /
            2  ->      /  28    /
                    /        /
                  /_______ /
                            _______
                          /     44 /
            14 ->      /    40  /
                    /   36   /
                  /_______ /
        """

        source = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})
        target = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})

        conndict = {'connection_type': 'divergent',
                    'mask': {'rectangular': {'lower_left': [-1.5, -0.5],
                                             'upper_right': [1.5, 0.5],
                                             'azimuth_angle': 45.}}}

        topo.ConnectLayers(source, target, conndict)

        ref = [[2, 28], [3, 29], [3, 33], [4, 30], [4, 34], [5, 31], [5, 35],
               [6, 32], [6, 36], [7, 29], [7, 33], [8, 30], [8, 34], [8, 38],
               [9, 31], [9, 35], [9, 39], [10, 32], [10, 36], [10, 40],
               [11, 37], [11, 41], [12, 34], [12, 38], [13, 35], [13, 39],
               [13, 43], [14, 36], [14, 40], [14, 44], [15, 37], [15, 41],
               [15, 45], [16, 42], [16, 46], [17, 39], [17, 43], [18, 40],
               [18, 44], [18, 48], [19, 41], [19, 45], [19, 49], [20, 42],
               [20, 46], [20, 50], [21, 47], [21, 51], [22, 44], [22, 48],
               [23, 45], [23, 49], [24, 46], [24, 50], [25, 47], [25, 51],
               [26, 52]]

        connections = list(sorted(nest.GetConnections()))

        for conn, conn_ref in zip(connections, ref):
            conn_list = [conn[0], conn[1]]
            self.assertEqual(conn_list, conn_ref)


def suite():
    suite = unittest.makeSuite(RotatedRectangularMask, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
