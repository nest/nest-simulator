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
                lower_left: [-1., -0.5]
                upper_right: [1., 0.5]
            
            So, if we have:

            layer:
            2  7  12  17  22
            3  8  13  18  23
            4  9  14  19  24
            5  10 15  20  25
            6  11 16  21  26
            
            and have azimuth_angle = 0, we should get gids 9, 14, 19.
            If we have azimuth_angle = 90, we should get gids 13, 14, 15.
        """
        
        # First test 2D layer
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
        maskdict = {'lower_left': [-1., -0.5], 'upper_right': [1., 0.5], 'azimuth_angle': 90.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        
        self.assertEqual(gid_list, (13, 14, 15,))
        
        # Test rotation with an azimuth angle of 45 degrees.
        maskdict = {'lower_left': [-1.5, -0.5], 'upper_right': [1.5, 0.5], 'azimuth_angle': 45.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (10, 14, 18,))
        
        # Test rotation with an azimuth angle of 135 degrees.
        maskdict = {'lower_left': [-1.5, -0.5], 'upper_right': [1.5, 0.5], 'azimuth_angle': 135.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (8, 14, 20,))
        
        # Test that an error is rased if we send in a polar angle to a 2D mask.
        maskdict = {'lower_left': [-1.5, -0.5], 'upper_right': [1.5, 0.5], 'polar_angle': 45.0}
        with self.assertRaises(nest.NESTError):
            mask = topo.CreateMask('rectangular', maskdict)
        
    def test_RotatedBoxMask(self):
        """Test rotated box mask with azimuth angle."""
        # Test a 3D layer.
        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})

        # First test that we get correct GIDs with box mask that is not rotated.
        maskdict = {'lower_left': [-1., -0.5, -0.5], 'upper_right': [1., 0.5, 0.5]}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (39, 64, 89,))
        
        # Test with a larger box mask.
        maskdict = {'lower_left': [-1., -0.5, -1.], 'upper_right': [1., 0.5, 1.]}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [38, 39, 40, 63, 64, 65, 88, 89, 90])
        
        # Test the smaller box mask with a rotation of 90 degrees. Only test
        # the azimuth angle, not the polar angle.
        maskdict = {'lower_left': [-1., -0.5, -0.5], 'upper_right': [1., 0.5, 0.5], 'azimuth_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (59, 64, 69,))
        
        # Test rotation of the larger box with an azimuth angle of 90 degrees.
        maskdict = {'lower_left': [-1., -0.5, -1.], 'upper_right': [1., 0.5, 1.], 'azimuth_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [58, 59, 60, 63, 64, 65, 68, 69, 70])
        
    def test_RotatedBoxMaskByPolarAngle(self):
        """Test rotated box mask with polar angle."""
        # Må i denne gjøre y og x like store for å få noe utslag, ellers blir gidene de samme
        # men test dette også
        
        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})
        
        # First test without rotation
        maskdict = {'lower_left': [-0.5, -1.0, -1.0], 'upper_right': [0.5, 1.0, 1.0]}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [58, 59, 60, 63, 64, 65, 68, 69, 70])
        
        # Test with a polar angle of 90 degrees.
        maskdict = {'lower_left': [-0.5, -1.0, -1.0], 'upper_right': [0.5, 1.0, 1.0], 'polar_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [34, 39, 44, 59, 64, 69, 84, 89, 94])
        
        # Test with a polar angle of 180 degrees, should be the same as the
        # one without a polar angle.
        maskdict = {'lower_left': [-0.5, -1.0, -1.0], 'upper_right': [0.5, 1.0, 1.0], 'polar_angle': 180.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [58, 59, 60, 63, 64, 65, 68, 69, 70])
        
        # Test with a polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5], 'upper_right': [0.5, 1.5, 1.5], 'polar_angle': 45.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [33, 38, 43, 59, 64, 69, 85, 90, 95])
        
        # Test with a polar angle of 135 degrees. This should be opposite of
        # the ones obtained by a polar angle of 45 degrees.
        maskdict = {'lower_left': [-0.5, -1.5, -1.5], 'upper_right': [0.5, 1.5, 1.5], 'polar_angle': 135.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [35, 40, 45, 59, 64, 69, 83, 88, 93])
        
        # Test two symmetric masks in x and z direction. One with no polar angle
        # and one with a polar angle of 90 degrees. As the masks are symmetrical
        # in  x and z, a polar angle of 90 degrees should give the same GIDs as
        # the one without a polar angle.
        maskdict = {'lower_left': [-1., -0.5, -1.], 'upper_right': [1., 0.5, 1.]}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [38, 39, 40, 63, 64, 65, 88, 89, 90])
        
        maskdict = {'lower_left': [-1., -0.5, -1.], 'upper_right': [1., 0.5, 1.], 'polar_angle': 90.}
        mask = topo.CreateMask('box', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        sorted_gid_list = sorted(gid_list)

        self.assertEqual(sorted_gid_list, [38, 39, 40, 63, 64, 65, 88, 89, 90])
        
    def test_RotatedBoxMaskByAzimuthAndPolarAngle(self):
        """Test rotated box mask with azimuth and polar angle."""
        pass
    
    def test_RotatedRectangleOutsideOrigo(self):
        """Test rotated rectangle where mask is outside origo."""
        
        layer = topo.CreateLayer({'rows': 11, 'columns': 11,
                                  'extent': [11., 11.],
                                  'elements': 'iaf_psc_alpha'})

        # First test that we get the correct GIDs when our mask does not
        # contain origo.
        maskdict = {'lower_left': [1., 1.], 'upper_right': [4., 2.]}
        mask = topo.CreateMask('rectangular', maskdict)
        cntr = [0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)

        self.assertEqual(gid_list, (71, 72, 82, 83, 93, 94, 104, 105,))
        
        # Then test that we get the correct GIDs with a azimuth rotation angle
        # of 45 degrees when the mask does not contain origo.
        maskdict = {'lower_left': [0.5, 0.5], 'upper_right': [4.5, 2.5], 'azimuth_angle': 45.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        
        self.assertEqual(gid_list, (72, 82, 83, 84, 92, 93, 94, 104,))

        # Test that we get the correct GIDs with a azimuth rotation angle
        # of 90 degrees when the mask does not contain origo.
        maskdict = {'lower_left': [1.0, 1.0], 'upper_right': [4.0, 2.0], 'azimuth_angle': 90.0}
        mask = topo.CreateMask('rectangular', maskdict)
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        
        self.assertEqual(gid_list, (81, 82, 83, 84, 92, 93, 94, 95,))
        
    def test_RotatedBoxOutsideOrigo(self):
        """Test rotated box where mask is outside origo."""
        
        pos = [[x*1., y*1., z*1.] for x in range(-2, 3)
               for y in range(-2, 3)
               for z in range(-2, 3)]

        layer = topo.CreateLayer({'positions': pos, 'extent': [5., 5., 5.],
                                  'elements': 'iaf_psc_alpha'})
        
        # First test that we get the correct GIDs when our mask does not
        # contain origo.
        maskdict = {'lower_left': [-2.0, -1.0, 0.5], 'upper_right': [-0.5, -0.5, 2.0]}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        
        self.assertEqual(gid_list, (10, 11, 35, 36,))
        
        # Test that we get the correct GIDs with a azimuth rotation angle of 45
        # degrees when the masj does not contain origo
        maskdict = {'lower_left': [-2.5, -1.0, 0.5], 'upper_right': [-0.5, -0.5, 2.5], 'azimuth_angle': 45.0}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        
        self.assertEqual(gid_list, (10, 11, 40, 41,)) 
        
        # Test that we get the correct GIDs with a polar rotation angle of 45
        # degrees when the mask does not contain origo
        maskdict = {'lower_left': [-1.5, -2.5, 0.5], 'upper_right': [-1.0, -0.5, 2.5], 'polar_angle': 45.0}
        mask = topo.CreateMask('box', maskdict)
        cntr = [0., 0., 0.]
        gid_list = topo.SelectNodesByMask(layer, cntr, mask)
        
        self.assertEqual(gid_list, (5, 10, 31, 36,))
    
    # Få inn en test hvor vi roterer både med azimuth og polar angle.
    # Få inn en test hvor vi ikke sentrerer om null.
    # Få inn en test med connektivitet også?
    # Kommenter også alt i mask.h, mask_impl.h
        
        
def suite():
    suite = unittest.makeSuite(RotatedRectangularMask, 'test')
    return suite

if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())