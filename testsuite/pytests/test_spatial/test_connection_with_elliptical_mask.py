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
        1  6  11  16  21            26  31  36  41  46
        2  7  12  17  22            27  32  37  42  47
        3  8  13  18  23            28  33  38  43  48
        4  9  14  19  24            29  34  39  44  49
        5  10 15  20  25            30  35  40  45  50

        some example connections will be:

        1  -> 26  31
              27

                  37
        13 -> 33  38  43
                  39
        """
        source = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(
                                 [5, 5], extent=[5., 5.]))

        target = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(
                                 [5, 5], extent=[5., 5.]))

        conndict = {'rule': 'pairwise_bernoulli',
                    'p': 1.,
                    'mask': {'elliptical': {'major_axis': 3.0,
                                            'minor_axis': 2.0}}}

        nest.Connect(source, target, conndict)

        ref = [[1, 26], [1, 27], [1, 31], [2, 26], [2, 27], [2, 28], [2, 32],
               [3, 27], [3, 28], [3, 29], [3, 33], [4, 28], [4, 29], [4, 30],
               [4, 34], [5, 29], [5, 30], [5, 35], [6, 26], [6, 31], [6, 32],
               [6, 36], [7, 27], [7, 31], [7, 32], [7, 33], [7, 37], [8, 28],
               [8, 32], [8, 33], [8, 34], [8, 38], [9, 29], [9, 33], [9, 34],
               [9, 35], [9, 39], [10, 30], [10, 34], [10, 35], [10, 40],
               [11, 31], [11, 36], [11, 37], [11, 41], [12, 32], [12, 36],
               [12, 37], [12, 38], [12, 42], [13, 33], [13, 37], [13, 38],
               [13, 39], [13, 43], [14, 34], [14, 38], [14, 39], [14, 40],
               [14, 44], [15, 35], [15, 39], [15, 40], [15, 45], [16, 36],
               [16, 41], [16, 42], [16, 46], [17, 37], [17, 41], [17, 42],
               [17, 43], [17, 47], [18, 38], [18, 42], [18, 43], [18, 44],
               [18, 48], [19, 39], [19, 43], [19, 44], [19, 45], [19, 49],
               [20, 40], [20, 44], [20, 45], [20, 50], [21, 41], [21, 46],
               [21, 47], [22, 42], [22, 46], [22, 47], [22, 48], [23, 43],
               [23, 47], [23, 48], [23, 49], [24, 44], [24, 48], [24, 49],
               [24, 50], [25, 45], [25, 49], [25, 50]]

        conns = nest.GetConnections()
        connections = [[s, t] for s, t in zip(conns.sources(), conns.targets())]

        for conn, conn_ref in zip(sorted(connections), ref):
            self.assertEqual(conn, conn_ref)

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
        1  6  11  16  21            26  31  36  41  46
        2  7  12  17  22            27  32  37  42  47
        3  8  13  18  23            28  33  38  43  48
        4  9  14  19  24            29  34  39  44  49
        5  10 15  20  25            30  35  40  45  50

        some example connections will be:

        1  -> 26

                      42
        13 ->     38
              34
        """
        source = nest.Create(
            'iaf_psc_alpha',
            positions=nest.spatial.grid([5, 5], extent=[5., 5.]))
        target = nest.Create(
            'iaf_psc_alpha',
            positions=nest.spatial.grid([5, 5], extent=[5., 5.]))

        conndict = {'rule': 'pairwise_bernoulli',
                    'p': 1.,
                    'mask': {'elliptical': {'major_axis': 3.0,
                                            'minor_axis': 1.0,
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
        connections = [[s, t] for s, t in zip(conns.sources(), conns.targets())]

        for conn, conn_ref in zip(sorted(connections), ref):
            self.assertEqual(conn, conn_ref)

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
        1  6  11  16  21            26  31  36  41  46
        2  7  12  17  22            27  32  37  42  47
        3  8  13  18  23            28  33  38  43  48
        4  9  14  19  24            29  34  39  44  49
        5  10 15  20  25            30  35  40  45  50

        some example connections will be:

        1  -> 26  31  36
                  32

                  42
        13 -> 38  43  48
                  44
        """
        source = nest.Create(
            'iaf_psc_alpha',
            positions=nest.spatial.grid([5, 5], extent=[5., 5.]))
        target = nest.Create(
            'iaf_psc_alpha',
            positions=nest.spatial.grid([5, 5], extent=[5., 5.]))

        conndict = {'rule': 'pairwise_bernoulli',
                    'p': 1.,
                    'mask': {'elliptical': {'major_axis': 3.,
                                            'minor_axis': 2.,
                                            'anchor': [1., 0.]}}}

        nest.Connect(source, target, conndict)

        ref = [[1, 26], [1, 31], [1, 32], [1, 36], [2, 27], [2, 31], [2, 32],
               [2, 33], [2, 37], [3, 28], [3, 32], [3, 33], [3, 34], [3, 38],
               [4, 29], [4, 33], [4, 34], [4, 35], [4, 39], [5, 30], [5, 34],
               [5, 35], [5, 40], [6, 31], [6, 36], [6, 37], [6, 41], [7, 32],
               [7, 36], [7, 37], [7, 38], [7, 42], [8, 33], [8, 37], [8, 38],
               [8, 39], [8, 43], [9, 34], [9, 38], [9, 39], [9, 40], [9, 44],
               [10, 35], [10, 39], [10, 40], [10, 45], [11, 36], [11, 41],
               [11, 42], [11, 46], [12, 37], [12, 41], [12, 42], [12, 43],
               [12, 47], [13, 38], [13, 42], [13, 43], [13, 44], [13, 48],
               [14, 39], [14, 43], [14, 44], [14, 45], [14, 49], [15, 40],
               [15, 44], [15, 45], [15, 50], [16, 41], [16, 46], [16, 47],
               [17, 42], [17, 46], [17, 47], [17, 48], [18, 43], [18, 47],
               [18, 48], [18, 49], [19, 44], [19, 48], [19, 49], [19, 50],
               [20, 45], [20, 49], [20, 50], [21, 46], [22, 47], [23, 48],
               [24, 49], [25, 50]]

        conns = nest.GetConnections()
        connections = [[s, t] for s, t in zip(conns.sources(), conns.targets())]

        for conn, conn_ref in zip(sorted(connections), ref):
            self.assertEqual(conn, conn_ref)

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
        1  6  11  16  21            26  31  36  41  46
        2  7  12  17  22            27  32  37  42  47
        3  8  13  18  23            28  33  38  43  48
        4  9  14  19  24            29  34  39  44  49
        5  10 15  20  25            30  35  40  45  50

        some example connections will be:

                  30
        1  -> 46  26  31
                  27

                  37
        13 -> 33  38  43
                  39
        """
        source = nest.Create(
            'iaf_psc_alpha',
            positions=nest.spatial.grid([5, 5], extent=[5., 5.], edge_wrap=True))
        target = nest.Create(
            'iaf_psc_alpha',
            positions=nest.spatial.grid([5, 5], extent=[5., 5.], edge_wrap=True))

        conndict = {'rule': 'pairwise_bernoulli',
                    'p': 1.,
                    'mask': {'elliptical': {'major_axis': 3.,
                                            'minor_axis': 2.}}}

        nest.Connect(source, target, conndict)

        ref = [[1, 26], [1, 27], [1, 30], [1, 31], [1, 46], [2, 26], [2, 27],
               [2, 28], [2, 32], [2, 47], [3, 27], [3, 28], [3, 29], [3, 33],
               [3, 48], [4, 28], [4, 29], [4, 30], [4, 34], [4, 49], [5, 26],
               [5, 29], [5, 30], [5, 35], [5, 50], [6, 26], [6, 31], [6, 32],
               [6, 35], [6, 36], [7, 27], [7, 31], [7, 32], [7, 33], [7, 37],
               [8, 28], [8, 32], [8, 33], [8, 34], [8, 38], [9, 29], [9, 33],
               [9, 34], [9, 35], [9, 39], [10, 30], [10, 31], [10, 34],
               [10, 35], [10, 40], [11, 31], [11, 36], [11, 37], [11, 40],
               [11, 41], [12, 32], [12, 36], [12, 37], [12, 38], [12, 42],
               [13, 33], [13, 37], [13, 38], [13, 39], [13, 43], [14, 34],
               [14, 38], [14, 39], [14, 40], [14, 44], [15, 35], [15, 36],
               [15, 39], [15, 40], [15, 45], [16, 36], [16, 41], [16, 42],
               [16, 45], [16, 46], [17, 37], [17, 41], [17, 42], [17, 43],
               [17, 47], [18, 38], [18, 42], [18, 43], [18, 44], [18, 48],
               [19, 39], [19, 43], [19, 44], [19, 45], [19, 49], [20, 40],
               [20, 41], [20, 44], [20, 45], [20, 50], [21, 26], [21, 41],
               [21, 46], [21, 47], [21, 50], [22, 27], [22, 42], [22, 46],
               [22, 47], [22, 48], [23, 28], [23, 43], [23, 47], [23, 48],
               [23, 49], [24, 29], [24, 44], [24, 48], [24, 49], [24, 50],
               [25, 30], [25, 45], [25, 46], [25, 49], [25, 50]]

        conns = nest.GetConnections()
        connections = [[s, t] for s, t in zip(conns.sources(), conns.targets())]

        for conn, conn_ref in zip(sorted(connections), ref):
            self.assertEqual(conn, conn_ref)


def suite():
    suite = unittest.makeSuite(ConnectWithEllipticalMask, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
