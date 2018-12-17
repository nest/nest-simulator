# -*- coding: utf-8 -*-
#
# test_spatial_kernels.py
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
Tests distribution of connections created with spatial kernels.

Original code by Daniel Hjertholm with Birgit Kriener.
Converted to automated test by Hans Ekkehard Plesser.

For theory, see
D. Hjertholm, Statistical tests for connection algorithms for
structured neural networks, MSc thesis, Norwegian University of
Life Science, 2013. http://hdl.handle.net/11250/189117.
"""

import math
import numpy as np
import numpy.random as rnd
import scipy.integrate
import scipy.stats
import scipy.special
import unittest

import nest
import nest.topology as topo


try:
    # for debugging
    from mpl_toolkits.mplot3d import Axes3D
    import matplotlib.pyplot as plt
    # make sure we can open a window; DISPLAY may not be set
    fig = plt.figure()
    plt.close(fig)
    PLOTTING_POSSIBLE = True
except:
    PLOTTING_POSSIBLE = False

# If False, tests will be run; otherwise, a single case will be plotted.
DEBUG_MODE = False

# Constant defining sensitivity of test (minimal p-value to pass)
P_MIN = 0.1

# Seed for all simulations
SEED = 1234567


class SpatialTester(object):
    '''Tests for spatially structured networks.'''

    def __init__(self, seed, dim, L, N, kernel_name, kernel_params=None,
                 open_bc=False, x0=0., y0=0.):
        '''
        Construct a test object.

        Parameters
        ----------
            seed         : Random seed for test
            dim          : Dimensions (2 or 3)
            L            : Side length of area / volume.
            N            : Number of nodes.
            kernel_name  : Name of kernel to use.
            kernel_params: Dict with params to update.
            open_bc      : Network with open boundary conditions
            x0, y0       : Location of source neuron; open_bc only

        Note
        ----
        For each new kernel to be added, the following needs to be
        defined:

        self._<kernel>  : lambda-function implementing kernel function
        kernels entry   : mapping kernel name to kernel function
        default_params  : default set of parameters for kernel
        '''

        if dim != 2 and open_bc:
            raise ValueError("open_bc only supported for 2D")

        self._seed = seed
        self._dimensions = dim
        self._L = float(L)
        self._N = N
        self._open_bc = open_bc
        self._x_d, self._y_d = x0, y0

        if (self._dimensions == 2):
            if (self._open_bc):
                self._max_dist = self._L * math.sqrt(2)
            else:
                self._max_dist = self._L / math.sqrt(2)
        elif (self._dimensions == 3):
            self._max_dist = self._L * math.sqrt(3) / 2

        self._target_dists = None
        self._all_dists = None

        # kernel functions
        self._constant = lambda D: self._params
        self._linear = lambda D: self._params['c'] + self._params['a'] * D
        self._exponential = lambda D: (self._params['c'] +
                                       self._params['a'] *
                                       math.exp(-D / self._params['tau']))
        self._gauss = lambda D: (self._params['c'] +
                                 self._params['p_center'] *
                                 math.exp(-(D - self._params['mean']) ** 2 /
                                          (2. * self._params['sigma'] ** 2)))
        self._gamma = lambda D: (D ** (self._params['kappa'] - 1) /
                                 (self._params['theta'] **
                                  self._params['kappa'] *
                                  scipy.special.gamma(self._params['kappa'])) *
                                 math.exp(-D / self._params['theta']))

        kernels = {
            'constant': self._constant,
            'linear': self._linear,
            'exponential': self._exponential,
            'gaussian': self._gauss,
            'gamma': self._gamma}
        self._kernel = kernels[kernel_name]

        default_params = {
            'constant': 1.,
            'linear': {'a': -math.sqrt(2) / self._L, 'c': 1.0},
            'exponential': {'a': 1.0, 'c': 0.0, 'tau': -self._L /
                            (math.sqrt(2) * math.log((.1 - 0) / 1))},
            'gaussian': {'p_center': 1., 'sigma': self._L / 4.,
                         'mean': 0., 'c': 0.},
            'gamma': {'kappa': 3., 'theta': self._L / 4.}}
        self._params = default_params[kernel_name]
        if kernel_params is not None:
            if kernel_name == 'constant':
                self._params = kernel_params
            else:
                self._params.update(kernel_params)

        if self._dimensions == 3:
            maskdict = {'box': {'lower_left': [-self._L / 2.] * 3,
                                'upper_right': [self._L / 2.] * 3}}
        elif self._dimensions == 2 and not self._open_bc:
            maskdict = {'rectangular': {'lower_left': [-self._L / 2.] * 2,
                                        'upper_right': [self._L / 2.] * 2}}
        elif self._dimensions == 2 and self._open_bc:
            maskdict = {'rectangular': {'lower_left': [-self._L] * 2,
                                        'upper_right': [self._L] * 2}}
        if kernel_name == 'constant':
            kerneldict = self._params
        else:
            kerneldict = {kernel_name: self._params}
        self._conndict = {'connection_type': 'divergent',
                          'mask': maskdict, 'kernel': kerneldict}

    def _create_distance_data(self):

        self._reset(self._seed)
        self._build()
        self._connect()

        self._target_dists = sorted(self._target_distances())
        self._all_dists = self._all_distances()

    def _reset(self, seed):
        '''
        Reset NEST and seed PRNGs.

        Parameters
        ----------
            seed: PRNG seed value.
        '''

        nest.ResetKernel()
        if seed is None:
            seed = rnd.randint(10 ** 10)
        seed = 3 * seed  # Reduces probability of overlapping seed values.
        rnd.seed(seed)
        nest.SetKernelStatus({'rng_seeds': [seed + 1],
                              'grng_seed': seed + 2})

    def _build(self):
        '''Create populations.'''
        if self._open_bc:
            ldict_s = {'elements': 'iaf_psc_alpha',
                       'positions': [[self._x_d, self._y_d]],
                       'extent': [self._L] * self._dimensions,
                       'edge_wrap': False}
            x = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            y = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            pos = list(zip(x, y))
            ldict_t = {'elements': 'iaf_psc_alpha', 'positions': pos,
                       'extent': [self._L] * self._dimensions,
                       'edge_wrap': False}
            self._ls = topo.CreateLayer(ldict_s)
            self._lt = topo.CreateLayer(ldict_t)
            self._driver = nest.GetLeaves(self._ls)[0]
        else:
            ldict_s = {'elements': 'iaf_psc_alpha',
                       'positions': [[0.] * self._dimensions],
                       'extent': [self._L] * self._dimensions,
                       'edge_wrap': True}
            x = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            y = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            if self._dimensions == 3:
                z = rnd.uniform(-self._L / 2., self._L / 2., self._N)
                pos = list(zip(x, y, z))
            else:
                pos = list(zip(x, y))
            ldict_t = {'elements': 'iaf_psc_alpha', 'positions': pos,
                       'extent': [self._L] * self._dimensions,
                       'edge_wrap': True}
            self._ls = topo.CreateLayer(ldict_s)
            self._lt = topo.CreateLayer(ldict_t)
            self._driver = topo.FindCenterElement(self._ls)

    def _connect(self):
        '''Connect populations.'''

        topo.ConnectLayers(self._ls, self._lt, self._conndict)

    def _all_distances(self):
        '''Return distances to all nodes in target population.'''

        return topo.Distance(self._driver, nest.GetLeaves(self._lt)[0])

    def _target_distances(self):
        '''Return distances from source node to connected nodes.'''

        connections = nest.GetConnections(source=self._driver)
        target_nodes = [conn[1] for conn in connections]
        return topo.Distance(self._driver, target_nodes)

    def _positions(self):
        '''Return positions of all nodes.'''

        return [tuple(pos) for pos in
                topo.GetPosition(nest.GetLeaves(self._lt)[0])]

    def _target_positions(self):
        '''Return positions of all connected target nodes.'''

        return [tuple(pos) for pos in
                topo.GetTargetPositions(self._driver, self._lt)[0]]

    def _roi_2d(self, x, y, L):
        '''
        Moves coordinates (x,y) to triangle area (x',y') in [0,L/2]X[0,x']
        without loss of generality
        '''
        x = -x if (x >= -self._L / 2.) and (x < 0) else x
        y = -y if (y >= -self._L / 2.) and (y < 0) else y
        return np.array([x, y]) if x > y else np.array([y, x])

    def _pdf(self, D):
        '''
        Unnormalized probability density function (PDF).

        Parameters
        ----------
            D: Distance in interval [0, max_dist], scalar.

        Return values
        -------------
            Not-normalized PDF at distance D.
        '''

        # TODO: The code below can most likely be optimized noticeably
        # by avoiding duplicate tests.
        if self._dimensions == 2:
            if self._open_bc:

                # move coordinates to right reference area:
                x0, y0 = self._roi_2d(self._x_d, self._y_d, self._L)

                # define a,b,c,d; alpha,beta,gamma,delta:
                a, b, c, d = self._L / 2. - np.array([x0, y0, -x0, -y0])
                alpha = math.acos(a / D) if a / D <= 1. else 0.
                beta = math.acos(b / D) if b / D <= 1. else 0.
                gamma = math.acos(c / D) if c / D <= 1. else 0.
                delta = math.acos(d / D) if d / D <= 1. else 0.
                kofD = max(0., min(1., self._kernel(D)))

                # cases:
                if (math.sqrt(a ** 2 + b ** 2) <= d):
                    if D < 0.:
                        return 0.
                    if (D >= 0.) and (D < a):
                        return 2 * math.pi * D * kofD
                    if (D >= a) and (D < b):
                        return 2 * D * (math.pi - alpha) * kofD
                    if (D >= b) and (D < math.sqrt(a ** 2 + b ** 2)):
                        return 2 * D * (math.pi - alpha - beta) * kofD
                    if (D >= math.sqrt(a ** 2 + b ** 2)) and (D < d):
                        return D * (3 * math.pi / 2. - alpha - beta) * kofD

                    if (c >= math.sqrt(a ** 2 + d ** 2)):
                        if (D >= d) and (D < math.sqrt(a ** 2 + d ** 2)):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * delta) * kofD
                        if (D >= math.sqrt(a ** 2 + d ** 2)) and (D < c):
                            return D * (math.pi - beta - delta) * kofD
                        if (D >= c) and (D < math.sqrt(b ** 2 + c ** 2)):
                            return D * (math.pi - beta - delta -
                                        2 * gamma) * kofD
                        if ((D >= math.sqrt(b ** 2 + c ** 2)) and
                           (D < math.sqrt(d ** 2 + c ** 2))):
                            return D * (math.pi / 2. - delta - gamma) * kofD

                    if (c < math.sqrt(a ** 2 + d ** 2)):
                        if (D >= d) and (D < c):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * delta) * kofD
                        if (D >= c) and (D < math.sqrt(a ** 2 + d ** 2)):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * (delta + gamma)) * kofD
                        if ((D >= math.sqrt(a ** 2 + d ** 2)) and
                           (D < math.sqrt(b ** 2 + c ** 2))):
                            return D * (math.pi - beta - delta -
                                        2 * gamma) * kofD
                        if ((D >= math.sqrt(b ** 2 + c ** 2)) and
                           (D < math.sqrt(d ** 2 + c ** 2))):
                            return D * (math.pi / 2. - delta - gamma) * kofD
                    if (D >= math.sqrt(d ** 2 + c ** 2)):
                        return 0.

                else:
                    if D < 0:
                        return 0.
                    if (D >= 0) and (D < a):
                        return 2 * math.pi * D * kofD
                    if (D >= a) and (D < b):
                        return 2 * D * (math.pi - alpha) * kofD
                    if (D >= b) and (D < d):
                        return 2 * D * (math.pi - alpha - beta) * kofD

                    if (((math.sqrt(a ** 2 + b ** 2) > c) and
                         (math.sqrt(a ** 2 + d ** 2) > c))):
                        if (D >= d) and (D < c):
                            return 2 * D * (math.pi - alpha - beta -
                                            delta) * kofD
                        if (D >= c) and (D < math.sqrt(a ** 2 + b ** 2)):
                            return 2 * D * (math.pi - alpha - beta -
                                            delta - gamma) * kofD
                        if ((D >= math.sqrt(a ** 2 + b ** 2)) and
                           (D < math.sqrt(a ** 2 + d ** 2))):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * (delta + gamma)) * kofD
                        if ((D >= math.sqrt(a ** 2 + d ** 2)) and
                           (D < math.sqrt(b ** 2 + c ** 2))):
                            return D * (math.pi - beta - delta -
                                        2 * gamma) * kofD
                        if ((D >= math.sqrt(b ** 2 + c ** 2)) and
                           (D < math.sqrt(d ** 2 + c ** 2))):
                            return D * (math.pi / 2. - delta - gamma) * kofD

                    if (((math.sqrt(a ** 2 + b ** 2) <= c) and
                         (math.sqrt(a ** 2 + d ** 2) <= c))):
                        if (D >= d) and (D < math.sqrt(a ** 2 + b ** 2)):
                            return 2 * D * (math.pi -
                                            (alpha + beta + delta)) * kofD
                        if ((D >= math.sqrt(a ** 2 + b ** 2)) and
                           (D < math.sqrt(a ** 2 + d ** 2))):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * delta) * kofD
                        if (D < c) and (D >= math.sqrt(a ** 2 + d ** 2)):
                            return D * (math.pi - beta - delta) * kofD
                        if (D >= c) and (D < math.sqrt(b ** 2 + c ** 2)):
                            return D * (math.pi - beta - delta -
                                        2 * gamma) * kofD
                        if ((D >= math.sqrt(b ** 2 + c ** 2)) and
                           (D < math.sqrt(d ** 2 + c ** 2))):
                            return D * (math.pi / 2. - delta - gamma) * kofD

                    if (((math.sqrt(a ** 2 + b ** 2) < c) and
                         (math.sqrt(a ** 2 + d ** 2) > c))):
                        if (D >= d) and (D < math.sqrt(a ** 2 + b ** 2)):
                            return 2 * D * (math.pi - alpha - beta -
                                            delta) * kofD
                        if (D >= math.sqrt(a ** 2 + b ** 2)) and (D < c):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * delta) * kofD
                        if (D >= c) and (D < math.sqrt(a ** 2 + d ** 2)):
                            return D * (3 * math.pi / 2. - alpha - beta -
                                        2 * (delta + gamma)) * kofD
                        if ((D >= math.sqrt(a ** 2 + d ** 2)) and
                           (D < math.sqrt(b ** 2 + c ** 2))):
                            return D * (math.pi - beta - delta -
                                        2 * gamma) * kofD
                        if ((D >= math.sqrt(b ** 2 + c ** 2)) and
                           (D < math.sqrt(d ** 2 + c ** 2))):
                            return D * (math.pi / 2. - delta - gamma) * kofD
                    if (D >= math.sqrt(d ** 2 + c ** 2)):
                        return 0.

            else:
                if D <= self._L / 2.:
                    return (max(0., min(1., self._kernel(D))) * math.pi * D)
                elif self._L / 2. < D <= self._max_dist:
                    return ((max(0., min(1., self._kernel(D))) * D *
                             (math.pi - 4. * math.acos(self._L / (D * 2.)))))
                else:
                    return 0.

        elif self._dimensions == 3:
            if D <= self._L / 2.:
                return (max(0., min(1., self._kernel(D))) *
                        4. * math.pi * D ** 2.)
            elif self._L / 2. < D <= self._L / math.sqrt(2):
                return (max(0., min(1., self._kernel(D))) *
                        2. * math.pi * D * (3. * self._L - 4. * D))
            elif self._L / math.sqrt(2) < D <= self._max_dist:
                A = 4. * math.pi * D ** 2.
                C = 2. * math.pi * D * (D - self._L / 2.)
                alpha = math.asin(1. / math.sqrt(2. - self._L ** 2. /
                                                 (2. * D ** 2.)))
                beta = math.pi / 2.
                gamma = math.asin(math.sqrt((1. - .5 * (self._L / D) ** 2.) /
                                            (1. - .25 * (self._L / D) ** 2.)))
                T = D ** 2. * (alpha + beta + gamma - math.pi)
                return (max(0., min(1., self._kernel(D))) *
                        (A + 6. * C * (-1. + 4. * gamma / math.pi) - 48. * T))
            else:
                return 0.

    def _cdf(self, D):
        '''
        Normalized cumulative distribution function (CDF).

        Parameters
        ----------
            D: Iterable of distances in interval [0, max_dist].

        Return values
        -------------
            List of CDF(d) for each distance d in D.
        '''
        cdf = []
        last_d = 0.
        for d in D:
            cdf.append(scipy.integrate.quad(self._pdf, last_d, d)[0])
            last_d = d

        cdf = np.cumsum(cdf)
        top = scipy.integrate.quad(self._pdf, 0, self._max_dist)[0]
        normed_cdf = cdf / top

        return normed_cdf

    def ks_test(self):
        '''
        Perform a Kolmogorov-Smirnov GOF test on the distribution
        of distances to connected nodes.

        Return values
        -------------
            KS statistic.
            p-value from KS test.
        '''

        if self._target_dists is None:
            self._create_distance_data()

        ks, p = scipy.stats.kstest(self._target_dists, self._cdf,
                                   alternative='two_sided')

        return ks, p

    def z_test(self):
        '''
        Perform a Z-test on the total number of connections.

        Return values
        -------------
            Standard score (z-score).
            Two-sided p-value.
        '''

        if self._target_dists is None or self._all_dists is None:
            self._create_distance_data()

        num_targets = len(self._target_dists)

        ps = ([max(0., min(1., self._kernel(D)))
               for D in self._all_dists])
        expected_num_targets = sum(ps)
        variance_num_targets = sum([p * (1. - p) for p in ps])

        if variance_num_targets == 0:
            return np.nan, 1.0
        else:
            sd = math.sqrt(variance_num_targets)
            z = abs((num_targets - expected_num_targets) / sd)
            p = 2. * (1. - scipy.stats.norm.cdf(z))

        return z, p


if PLOTTING_POSSIBLE:
    class PlottingSpatialTester(SpatialTester):
        '''Add plotting capability to SpatialTester.'''

        def __init__(self, seed, dim, L, N, kernel_name, kernel_params=None,
                     open_bc=False, x0=0., y0=0.):
            SpatialTester.__init__(self, seed, dim, L, N, kernel_name,
                                   kernel_params, open_bc, x0, y0)

        def show_network(self):
            '''Plot nodes in the network.'''

            # Adjust size of nodes in plot based on number of nodes.
            nodesize = max(0.01, round(111. / 11 - self._N / 1100.))

            figsize = (8, 6) if self._dimensions == 3 else (6, 6)
            fig = plt.figure(figsize=figsize)
            positions = self._positions()
            connected = self._target_positions()
            not_connected = set(positions) - set(connected)

            x1 = [pos[0] for pos in not_connected]
            y1 = [pos[1] for pos in not_connected]
            x2 = [pos[0] for pos in connected]
            y2 = [pos[1] for pos in connected]

            if self._dimensions == 2:
                plt.scatter(x1, y1, s=nodesize, marker='o', color='grey')
                plt.scatter(x2, y2, s=nodesize, marker='o', color='red')
            if self._dimensions == 3:
                ax = fig.add_subplot(111, projection='3d')
                z1 = [pos[2] for pos in not_connected]
                z2 = [pos[2] for pos in connected]
                ax.scatter(x1, y1, z1, s=nodesize, marker='o', color='grey')
                ax.scatter(x2, y2, z2, s=nodesize, marker='o', color='red')

            plt.xlabel(r'$x$', size=24)
            plt.ylabel(r'$y$', size=24)
            plt.xticks(size=16)
            plt.yticks(size=16)
            plt.xlim(-0.505, 0.505)
            plt.ylim(-0.505, 0.505)
            plt.subplots_adjust(bottom=0.15, left=0.17)

        def show_CDF(self):
            '''
            Plot the cumulative distribution function (CDF) of
            source-target distances.
            '''

            plt.figure()
            x = np.linspace(0, self._max_dist, 1000)
            cdf = self._cdf(x)
            plt.plot(x, cdf, '-', color='black', linewidth=3,
                     label='Theory', zorder=1)
            y = [(i + 1.) / len(self._target_dists)
                 for i in range(len(self._target_dists))]
            plt.step([0.0] + self._target_dists, [0.0] + y, color='red',
                     linewidth=1, label='Empirical', zorder=2)
            plt.ylim(0, 1)

            plt.xlabel('Distance')
            plt.ylabel('CDF')
            plt.legend(loc='center right')

        def show_PDF(self, bins=100):
            '''
            Plot the probability density function of source-target distances.

            Parameters
            ----------
                bins: Number of histogram bins for PDF plot.
            '''

            plt.figure()
            x = np.linspace(0, self._max_dist, 1000)
            area = scipy.integrate.quad(self._pdf, 0, self._max_dist)[0]
            y = np.array([self._pdf(D) for D in x]) / area
            plt.plot(x, y, color='black', linewidth=3, label='Theory',
                     zorder=1)
            plt.hist(self._target_dists, bins=bins, histtype='step',
                     linewidth=1, normed=True, color='red',
                     label='Empirical', zorder=2)
            plt.ylim(ymin=0.)

            plt.xlabel('Distance')
            plt.ylabel('PDF')
            plt.legend(loc='center right')


class TestSpatial2D(unittest.TestCase):
    """
    Test for 2D kernels.
    """

    def test_constant(self):
        kernel = 'constant'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_linear(self):
        kernel = 'linear'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_exponential(self):
        kernel = 'exponential'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_gaussian(self):
        kernel = 'gaussian'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_gamma(self):
        kernel = 'gamma'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))


class TestSpatial2DOBC(unittest.TestCase):
    """
    Test for 2D kernels with open boundary conditions.
    """

    def test_constant(self):
        kernel = 'constant'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_linear(self):
        kernel = 'linear'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_exponential(self):
        kernel = 'exponential'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_gaussian(self):
        kernel = 'gaussian'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_gamma(self):
        kernel = 'gamma'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             kernel_name=kernel, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))


class TestSpatial3D(unittest.TestCase):
    """
    Test for 3D kernels.
    """

    def test_constant(self):
        kernel = 'constant'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_linear(self):
        kernel = 'linear'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_exponential(self):
        kernel = 'exponential'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_gaussian(self):
        kernel = 'gaussian'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))

    def test_gamma(self):
        kernel = 'gamma'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             kernel_name=kernel)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(kernel))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(kernel))


def suite():
    suite = unittest.TestSuite([
        unittest.TestLoader().loadTestsFromTestCase(TestSpatial2D),
        unittest.TestLoader().loadTestsFromTestCase(TestSpatial2DOBC),
        unittest.TestLoader().loadTestsFromTestCase(TestSpatial3D),
        ])
    return suite


if __name__ == '__main__':

    if not DEBUG_MODE:
        runner = unittest.TextTestRunner(verbosity=2)
        runner.run(suite())
    elif PLOTTING_POSSIBLE:
        test = PlottingSpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                                     kernel_name='gaussian')
        ks, p = test.ks_test()
        print('p-value of KS-test:', p)
        z, p = test.z_test()
        print('p-value of Z-test:', p)
        test.show_network()
        test.show_PDF()
        test.show_CDF()
        plt.show()
    else:
        assert False, "DEBUG_MODE makes sense only if PLOTTING_POSSIBLE"
