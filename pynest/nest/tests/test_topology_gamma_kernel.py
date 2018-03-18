# -*- coding: utf-8 -*-
#
# test_topology_gamma_kernel.py
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
Test of events
"""

import numpy as np
import scipy.stats as sts
import unittest
import nest
import nest.topology as tp


@nest.check_stack
class TopologyGammaKernelTestCase(unittest.TestCase):
    """Tests of gamma kernel in topology"""

    def test_connectivity_profile_match(self):
        """Match connectivity profile with PDF"""

        nest.ResetKernel()

        extent = [100., 100.]
        npop = 5000
        positions = np.random.uniform(-extent[0] / 2, extent[1] / 2, [npop, 2])
        ncon = 10000
        kappa = 3.
        theta = 4.
        radius = extent[0] / 2 * np.sqrt(2)
        dx = .5
        x = np.arange(0, radius + dx, dx)

        ly1 = tp.CreateLayer({
            'positions': [[0., 0.]],
            'elements': 'iaf_psc_alpha',
            'extent': extent,
            'edge_wrap': True
        })

        ly2 = tp.CreateLayer({
            'positions': positions.tolist(),
            'elements': 'iaf_psc_alpha',
            'extent': extent,
            'edge_wrap': True
        })

        tp.ConnectLayers(ly1, ly2, projections={
            'connection_type': 'divergent',
            'allow_oversized_mask': True,
            'mask': {
                'circular': {
                    'radius': radius
                }
            },
            'kernel': {
                'gamma': {
                    'kappa': kappa,
                    'theta': theta
                }
            },
            'number_of_connections': ncon,
            'allow_multapses': True,
            'allow_autapses': True,
        })

        gamma = sts.gamma.pdf(x, a=kappa + 1, scale=theta)
        gamma_normed = gamma / float(np.sum(gamma)) / dx

        center = tp.FindCenterElement(ly1)
        tgts = tp.GetTargetNodes(center, ly2)[0]
        d = tp.Distance(center, tgts)

        h = np.histogram(d, bins=x)[0]
        h_normed = h / float(np.sum(h)) / dx
        difference = np.diff([gamma_normed[:-1], h_normed], axis=0)
        error_score = np.abs(np.median(difference))
        self.assertLess(error_score, 1e-4)

        # Uncomment this section to visualize the comparision.
        '''
        print('Error: %s' % error_score)
        import pylab as pl
        fig, ax = pl.subplots(1)
        ax.step(x[:-1], h_normed, label='Constructed targets in NEST')
        ax.step(x, gamma_normed, label='Gamma PDF')
        ax.legend()
        pl.show()
        '''


def suite():
    suite = unittest.makeSuite(TopologyGammaKernelTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
