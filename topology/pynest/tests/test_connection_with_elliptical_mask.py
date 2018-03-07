# -*- coding: utf-8 -*-
#
# test_connection_with_elliptical_mask.py
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
Tests ConnectLayer with elliptical mask.
"""

import unittest
import nest
import nest.topology as topo


class ConnectWithEllipticalMask(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_ConnectEllipticalMask(self):
        """Test connection with simple elliptical mask.

        We have: major_axis = 3.0
                 minor_axis = 2.0

        Each source node should then connect to:
            - The node in the same position in target layer
            - The node to the left and right of that position
            - The nodes above and below.

        So, if we have

        sources:                    targets:
        2  7  12  17  22            28  33  38  43  48
        3  8  13  18  23            29  34  39  44  49
        4  9  14  19  24            30  35  40  45  50
        5  10 15  20  25            31  36  41  46  51
        6  11 16  21  26            32  37  42  47  52

        some example connections will be:

        2  -> 28  33
              29

                  39
        14 -> 35  40  45
                  41
        """
        source = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})
        target = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})

        conndict = {'connection_type': 'divergent',
                    'mask': {'elliptical': {'major_axis': 3.0,
                                            'minor_axis': 2.0}}}

        topo.ConnectLayers(source, target, conndict)

        ref = [[2, 28], [2, 29], [2, 33], [3, 28], [3, 29], [3, 30], [3, 34],
               [4, 29], [4, 30], [4, 31], [4, 35], [5, 30], [5, 31], [5, 32],
               [5, 36], [6, 31], [6, 32], [6, 37], [7, 28], [7, 33], [7, 34],
               [7, 38], [8, 29], [8, 33], [8, 34], [8, 35], [8, 39], [9, 30],
               [9, 34], [9, 35], [9, 36], [9, 40], [10, 31], [10, 35],
               [10, 36], [10, 37], [10, 41], [11, 32], [11, 36], [11, 37],
               [11, 42], [12, 33], [12, 38], [12, 39], [12, 43], [13, 34],
               [13, 38], [13, 39], [13, 40], [13, 44], [14, 35], [14, 39],
               [14, 40], [14, 41], [14, 45], [15, 36], [15, 40], [15, 41],
               [15, 42], [15, 46], [16, 37], [16, 41], [16, 42], [16, 47],
               [17, 38], [17, 43], [17, 44], [17, 48], [18, 39], [18, 43],
               [18, 44], [18, 45], [18, 49], [19, 40], [19, 44], [19, 45],
               [19, 46], [19, 50], [20, 41], [20, 45], [20, 46], [20, 47],
               [20, 51], [21, 42], [21, 46], [21, 47], [21, 52], [22, 43],
               [22, 48], [22, 49], [23, 44], [23, 48], [23, 49], [23, 50],
               [24, 45], [24, 49], [24, 50], [24, 51], [25, 46], [25, 50],
               [25, 51], [25, 52], [26, 47], [26, 51], [26, 52]]

        connections = nest.GetConnections()

        for conn, conn_ref in zip(connections, ref):
            conn_list = [conn[0], conn[1]]
            self.assertEqual(conn_list, conn_ref)

    def test_ConnectTiltedEllipticalMask(self):
        """Test connection with tilted elliptical mask.

        We have: major_axis = 3.0
                 minor_axis = 1.0
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

        2  -> 28

                      44
        14 ->     40
              36
        """
        source = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})
        target = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})

        conndict = {'connection_type': 'divergent',
                    'mask': {'elliptical': {'major_axis': 3.0,
                                            'minor_axis': 1.0,
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

        connections = nest.GetConnections()

        for conn, conn_ref in zip(connections, ref):
            conn_list = [conn[0], conn[1]]
            self.assertEqual(conn_list, conn_ref)

    def test_ConnectAnchoredEllipticalMask(self):
        """Test connection with anchored elliptical mask.

        We have: major_axis = 3.0
                 minor_axis = 2.0
                 anchor = [1.0, 0.0] (right edge of mask aligned with source)

        Each source node should then connect to:
            - The node in the same position in target layer
            - The two nodes to the right of that position
            - The nodes above and below the node to the right of the position.

        So, if we have

        sources:                    targets:
        2  7  12  17  22            28  33  38  43  48
        3  8  13  18  23            29  34  39  44  49
        4  9  14  19  24            30  35  40  45  50
        5  10 15  20  25            31  36  41  46  51
        6  11 16  21  26            32  37  42  47  52

        some example connections will be:

        2  -> 28  33  38
                  34

                  44
        14 -> 40  45  50
                  46
        """
        source = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})
        target = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha'})

        conndict = {'connection_type': 'divergent',
                    'mask': {'elliptical': {'major_axis': 3.,
                                            'minor_axis': 2.,
                                            'anchor': [1., 0.]}}}

        topo.ConnectLayers(source, target, conndict)

        ref = [[2, 28], [2, 33], [2, 34], [2, 38], [3, 29], [3, 33], [3, 34],
               [3, 35], [3, 39], [4, 30], [4, 34], [4, 35], [4, 36], [4, 40],
               [5, 31], [5, 35], [5, 36], [5, 37], [5, 41], [6, 32], [6, 36],
               [6, 37], [6, 42], [7, 33], [7, 38], [7, 39], [7, 43], [8, 34],
               [8, 38], [8, 39], [8, 40], [8, 44], [9, 35], [9, 39], [9, 40],
               [9, 41], [9, 45], [10, 36], [10, 40], [10, 41], [10, 42],
               [10, 46], [11, 37], [11, 41], [11, 42], [11, 47], [12, 38],
               [12, 43], [12, 44], [12, 48], [13, 39], [13, 43], [13, 44],
               [13, 45], [13, 49], [14, 40], [14, 44], [14, 45], [14, 46],
               [14, 50], [15, 41], [15, 45], [15, 46], [15, 47], [15, 51],
               [16, 42], [16, 46], [16, 47], [16, 52], [17, 43], [17, 48],
               [17, 49], [18, 44], [18, 48], [18, 49], [18, 50], [19, 45],
               [19, 49], [19, 50], [19, 51], [20, 46], [20, 50], [20, 51],
               [20, 52], [21, 47], [21, 51], [21, 52], [22, 48], [23, 49],
               [24, 50], [25, 51], [26, 52]]

        connections = nest.GetConnections()

        for conn, conn_ref in zip(connections, ref):
            conn_list = [conn[0], conn[1]]
            self.assertEqual(conn_list, conn_ref)

    def test_ConnectEllipticalMaskWithPeriodicBoundary(self):
        """Test connection with simple elliptical mask.

        We have: major_axis = 3.0
                 minor_axis = 2.0

        Each source node should then connect to:
            - The node in the same position in target layer
            - The node to the left and right of that position
            - The nodes above and below
            - Nodes on the edges have connections on the other, corresponding
              edges.

        So, if we have

        sources:                    targets:
        2  7  12  17  22            28  33  38  43  48
        3  8  13  18  23            29  34  39  44  49
        4  9  14  19  24            30  35  40  45  50
        5  10 15  20  25            31  36  41  46  51
        6  11 16  21  26            32  37  42  47  52

        some example connections will be:

                  6
        2  -> 48  28  33
                  29

                  39
        14 -> 35  40  45
                  41
        """
        source = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha',
                                   'edge_wrap': True})
        target = topo.CreateLayer({'rows': 5, 'columns': 5,
                                   'extent': [5., 5.],
                                   'elements': 'iaf_psc_alpha',
                                   'edge_wrap': True})

        conndict = {'connection_type': 'divergent',
                    'mask': {'elliptical': {'major_axis': 3.,
                                            'minor_axis': 2.}}}

        topo.ConnectLayers(source, target, conndict)

        ref = [[2, 28], [2, 29], [2, 32], [2, 33], [2, 48], [3, 28], [3, 29],
               [3, 30], [3, 34], [3, 49], [4, 29], [4, 30], [4, 31], [4, 35],
               [4, 50], [5, 30], [5, 31], [5, 32], [5, 36], [5, 51], [6, 28],
               [6, 31], [6, 32], [6, 37], [6, 52], [7, 28], [7, 33], [7, 34],
               [7, 37], [7, 38], [8, 29], [8, 33], [8, 34], [8, 35], [8, 39],
               [9, 30], [9, 34], [9, 35], [9, 36], [9, 40], [10, 31], [10, 35],
               [10, 36], [10, 37], [10, 41], [11, 32], [11, 33], [11, 36],
               [11, 37], [11, 42], [12, 33], [12, 38], [12, 39], [12, 42],
               [12, 43], [13, 34], [13, 38], [13, 39], [13, 40], [13, 44],
               [14, 35], [14, 39], [14, 40], [14, 41], [14, 45], [15, 36],
               [15, 40], [15, 41], [15, 42], [15, 46], [16, 37], [16, 38],
               [16, 41], [16, 42], [16, 47], [17, 38], [17, 43], [17, 44],
               [17, 47], [17, 48], [18, 39], [18, 43], [18, 44], [18, 45],
               [18, 49], [19, 40], [19, 44], [19, 45], [19, 46], [19, 50],
               [20, 41], [20, 45], [20, 46], [20, 47], [20, 51], [21, 42],
               [21, 43], [21, 46], [21, 47], [21, 52], [22, 28], [22, 43],
               [22, 48], [22, 49], [22, 52], [23, 29], [23, 44], [23, 48],
               [23, 49], [23, 50], [24, 30], [24, 45], [24, 49], [24, 50],
               [24, 51], [25, 31], [25, 46], [25, 50], [25, 51], [25, 52],
               [26, 32], [26, 47], [26, 48], [26, 51], [26, 52]]

        connections = nest.GetConnections()

        for conn, conn_ref in zip(connections, ref):
            conn_list = [conn[0], conn[1]]
            self.assertEqual(conn_list, conn_ref)


def suite():
    suite = unittest.makeSuite(ConnectWithEllipticalMask, 'test')
    return suite

if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
