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
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[5, 5],
                                                        extent=[5., 5.]))

        # First test without rotation.
        maskdict = {'lower_left': [-1., -0.5], 'upper_right': [1., 0.5]}
        mask = nest.CreateMask('rectangular', maskdict)
        cntr = [0., 0.]
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((8, 13, 18,)))

        # Test if we get correct GIDs when rotating 90 degrees.
        maskdict = {'lower_left': [-1., -0.5],
                    'upper_right': [1., 0.5],
                    'azimuth_angle': 90.0}
        mask = nest.CreateMask('rectangular', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((12, 13, 14,)))

        # Test rotation with an azimuth angle of 45 degrees.
        maskdict = {'lower_left': [-1.5, -0.5],
                    'upper_right': [1.5, 0.5],
                    'azimuth_angle': 45.0}
        mask = nest.CreateMask('rectangular', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((9, 13, 17,)))

        # Test rotation with an azimuth angle of 135 degrees.
        maskdict = {'lower_left': [-1.5, -0.5],
                    'upper_right': [1.5, 0.5],
                    'azimuth_angle': 135.0}
        mask = nest.CreateMask('rectangular', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((7, 13, 19,)))

        # Test that an error is raised if we send in a polar angle to a 2D
        # mask.
        maskdict = {'lower_left': [-1.5, -0.5],
                    'upper_right': [1.5, 0.5],
                    'polar_angle': 45.0}
        with self.assertRaises(nest.kernel.NESTError):
            mask = nest.CreateMask('rectangular', maskdict)

    def test_RotatedBoxMaskByAzimuthAngle(self):
        """Test rotated box mask with azimuth angle."""
        # Test a 3D layer.
        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        # First test that we get correct GIDs with box mask that is not
        # rotated.
        maskdict = {'lower_left': [-1., -0.5, -0.5],
                    'upper_right': [1., 0.5, 0.5]}
        mask = nest.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((38, 63, 88,)))

        # Test with a larger box mask.
        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.]}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([37, 38, 39, 62, 63, 64, 87, 88, 89]))

        # Test the smaller box mask with a rotation of 90 degrees. Only test
        # the azimuth angle, not the polar angle.
        maskdict = {'lower_left': [-1., -0.5, -0.5],
                    'upper_right': [1., 0.5, 0.5],
                    'azimuth_angle': 90.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((58, 63, 68,)))

        # Test rotation of the larger box with an azimuth angle of 90 degrees.
        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.],
                    'azimuth_angle': 90.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([57, 58, 59, 62, 63, 64, 67, 68, 69]))

    def test_RotatedBoxMaskByPolarAngle(self):
        """Test rotated box mask with polar angle."""

        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        # First test without rotation
        maskdict = {'lower_left': [-0.5, -1.0, -1.0],
                    'upper_right': [0.5, 1.0, 1.0]}
        mask = nest.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([57, 58, 59, 62, 63, 64, 67, 68, 69]))

        # Test with a polar angle of 90 degrees.
        maskdict = {'lower_left': [-0.5, -1.0, -1.0],
                    'upper_right': [0.5, 1.0, 1.0],
                    'polar_angle': 90.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([33, 38, 43, 58, 63, 68, 83, 88, 93]))

        # Test with a polar angle of 180 degrees, should be the same as the
        # one without a polar angle.
        maskdict = {'lower_left': [-0.5, -1.0, -1.0],
                    'upper_right': [0.5, 1.0, 1.0],
                    'polar_angle': 180.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([57, 58, 59, 62, 63, 64, 67, 68, 69]))

        # Test with a polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5],
                    'upper_right': [0.5, 1.5, 1.5],
                    'polar_angle': 45.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([32, 37, 42, 58, 63, 68, 84, 89, 94]))

        # Test with a polar angle of 135 degrees. The GIDs should be
        # perpendicular to the ones obtained by a polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5],
                    'upper_right': [0.5, 1.5, 1.5],
                    'polar_angle': 135.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection([34, 39, 44, 58, 63, 68, 82, 87, 92]))

        # Test two symmetric masks in x and z direction. One with no polar
        # angle and one with a polar angle of 90 degrees. As the masks are
        # symmetrical in  x and z, a polar angle of 90 degrees should give the
        # same GIDs as the one without a polar angle.
        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.]}
        mask = nest.CreateMask('box', maskdict)
        gids_2 = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids_2, nest.GIDCollection([37, 38, 39, 62, 63, 64, 87, 88, 89]))

        maskdict = {'lower_left': [-1., -0.5, -1.],
                    'upper_right': [1., 0.5, 1.],
                    'polar_angle': 90.}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)
        self.assertEqual(gids, nest.GIDCollection([37, 38, 39, 62, 63, 64, 87, 88, 89]))

        self.assertEqual(gids_2, gids)

    def test_RotatedBoxMaskByAzimuthAndPolarAngle(self):
        """Test rotated box mask with azimuth and polar angle."""

        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        # Test with a azimuth angle and polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5],
                    'upper_right': [0.5, 1.5, 1.5],
                    'azimuth_angle': 45.,
                    'polar_angle': 45.}
        mask = nest.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection([37, 38, 43, 57, 58, 63, 68, 69, 83, 88, 89]))

    def test_RotatedRectangleOutsideOrigin(self):
        """
        Test rotated rectangle where the mask does not contain the origin.
        """

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[11, 11],
                                                        extent=[11., 11.]))

        # First test that we get the correct GIDs when our mask does not
        # contain the origin.
        maskdict = {'lower_left': [1., 1.], 'upper_right': [4., 2.]}
        mask = nest.CreateMask('rectangular', maskdict)
        cntr = [0., 0.]
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((70, 71, 81, 82, 92, 93, 103, 104,)))

        # Then test that we get the correct GIDs with a azimuth rotation angle
        # of 45 degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [0.5, 0.5],
                    'upper_right': [4.5, 2.5],
                    'azimuth_angle': 45.0}
        mask = nest.CreateMask('rectangular', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((71, 81, 82, 83, 91, 92, 93, 103,)))

        # Test that we get the correct GIDs with a azimuth rotation angle
        # of 90 degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [1.0, 1.0],
                    'upper_right': [4.0, 2.0],
                    'azimuth_angle': 90.0}
        mask = nest.CreateMask('rectangular', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((80, 81, 82, 83, 91, 92, 93, 94,)))

    def test_RotatedBoxOutsideOrigin(self):
        """Test rotated box where the mask does not contain the origin."""

        pos = [[x * 1., y * 1., z * 1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        # First test that we get the correct GIDs when our mask does not
        # contain the origin.
        maskdict = {'lower_left': [-2.0, -1.0, 0.5],
                    'upper_right': [-0.5, -0.5, 2.0]}
        mask = nest.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((9, 10, 34, 35,)))

        # Test that we get the correct GIDs with a azimuth rotation angle of 45
        # degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [-2.5, -1.0, 0.5],
                    'upper_right': [-0.5, -0.5, 2.5],
                    'azimuth_angle': 45.0}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((9, 10, 39, 40,)))

        # Test that we get the correct GIDs with a polar rotation angle of 45
        # degrees when the mask does not contain the origin.
        maskdict = {'lower_left': [-1.5, -2.5, 0.5],
                    'upper_right': [-1.0, -0.5, 2.5],
                    'polar_angle': 45.0}
        mask = nest.CreateMask('box', maskdict)
        gids = nest.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gids, nest.GIDCollection((4, 9, 30, 35,)))

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

        source = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=[5, 5],
                                                         extent=[5., 5.]))
        target = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=[5, 5],
                                                         extent=[5., 5.]))

        conndict = {'rule': 'pairwise_bernoulli',
                    'p': 1.,
                    'mask': {'rectangular': {'lower_left': [-1.5, -0.5],
                                             'upper_right': [1.5, 0.5],
                                             'azimuth_angle': 45.}}}

        nest.Connect(source, target, conndict)

        ref = [[1, 26], [2, 27], [2, 31], [3, 28], [3, 32], [4, 29], [4, 33],
               [5, 30], [5, 34], [6, 27], [6, 31], [7, 28], [7, 32], [7, 36],
               [8, 29], [8, 33], [8, 37], [9, 30], [9, 34], [9, 38], [10, 35],
               [10, 39], [11, 32], [11, 36], [12, 33], [12, 37], [12, 41],
               [13, 34], [13, 38], [13, 42], [14, 35], [14, 39], [14, 43],
               [15, 40], [15, 44], [16, 37], [16, 41], [17, 38], [17, 42],
               [17, 46], [18, 39], [18, 43], [18, 47], [19, 40], [19, 44],
               [19, 48], [20, 45], [20, 49], [21, 42], [21, 46], [22, 43],
               [22, 47], [23, 44], [23, 48], [24, 45], [24, 49], [25, 50]]

        conns = nest.GetConnections()
        connections = [[s, t] for s, t in zip(conns.source(), conns.target())]

        for conn, conn_ref in zip(sorted(connections), ref):
            self.assertEqual(conn, conn_ref)


def suite():
    suite = unittest.makeSuite(RotatedRectangularMask, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
