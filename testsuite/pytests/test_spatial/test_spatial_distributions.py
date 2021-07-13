# -*- coding: utf-8 -*-
#
# test_spatial_distributions.py
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
Tests distribution of connections created with spatial distributions.

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


try:
    # for debugging
    from mpl_toolkits.mplot3d import Axes3D
    import matplotlib.pyplot as plt
    # make sure we can open a window; DISPLAY may not be set
    fig = plt.figure()
    plt.close(fig)
    PLOTTING_POSSIBLE = True
except Exception:
    PLOTTING_POSSIBLE = False

# If False, tests will be run; otherwise, a single case will be plotted.
DEBUG_MODE = False

# Constant defining sensitivity of test (minimal p-value to pass)
P_MIN = 0.1

# Seed for all simulations
SEED = 1234567


class SpatialTester(object):
    """Tests for spatially structured networks."""

    def __init__(self, seed, dim, L, N, spatial_distribution, distribution_params=None,
                 open_bc=False, x0=0., y0=0.):
        """
        Construct a test object.

        Parameters
        ----------
            seed                  : Random seed for test
            dim                   : Dimensions (2 or 3)
            L                     : Side length of area / volume.
            N                     : Number of nodes.
            spatial_distribution  : Name of spatial distribution to use.
            distribution_params   : Dict with params to update.
            open_bc               : Network with open boundary conditions
            x0, y0                : Location of source neuron; open_bc only

        Note
        ----
        For each new distribution to be added, the following needs to be
        defined:

        self._<distribution>        : function implementing distribution function
        spatial_distributions entry : mapping distribution name to distribution function
        default_params              : default set of parameters for distribution
        """

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
                self._pdf = self._pdf_2d_obc
            else:
                self._max_dist = self._L / math.sqrt(2)
                self._pdf = self._pdf_2d
        elif (self._dimensions == 3):
            self._max_dist = self._L * math.sqrt(3) / 2
            self._pdf = self._pdf_3d

        self._target_dists = None
        self._all_dists = None

        spatial_distributions = {
            'constant': self._constant,
            'linear': self._linear,
            'exponential': self._exponential,
            'gaussian': self._gauss,
            'gaussian2d': self._gauss2d,
            'gamma': self._gamma}
        self._distribution = spatial_distributions[spatial_distribution]

        default_params = {
            'constant': 1.,
            'linear': {'a': -math.sqrt(2) / self._L, 'c': 1.0},
            'exponential': {'a': 1.0, 'c': 0.0, 'tau': -self._L /
                            (math.sqrt(2) * math.log((.1 - 0) / 1))},
            'gaussian': {'p_center': 1., 'sigma': self._L / 4.,
                         'mean': 0., 'c': 0.},
            'gaussian2d': {'p_center': 1., 'sigma_x': self._L / 4., 'sigma_y': self._L / 4.,
                           'mean_x': 0.5, 'mean_y': 0.7, 'rho': 0.5, 'c': 0.},
            'gamma': {'kappa': 3., 'theta': self._L / 4.}}
        self._params = default_params[spatial_distribution]
        if distribution_params is not None:
            if spatial_distribution == 'constant':
                self._params = distribution_params
            else:
                self._params.update(distribution_params)

        # Pre-calculate constant variables for efficiency
        self._calculate_constants(spatial_distribution)

        if self._dimensions == 3:
            maskdict = {'box': {'lower_left': [-self._L / 2.] * 3,
                                'upper_right': [self._L / 2.] * 3}}
        elif self._dimensions == 2 and not self._open_bc:
            maskdict = {'rectangular': {'lower_left': [-self._L / 2.] * 2,
                                        'upper_right': [self._L / 2.] * 2}}
        elif self._dimensions == 2 and self._open_bc:
            maskdict = {'rectangular': {'lower_left': [-self._L] * 2,
                                        'upper_right': [self._L] * 2}}

        if spatial_distribution == 'constant':
            distribution = nest.CreateParameter('constant', {'value': self._params})
        elif spatial_distribution == 'linear':
            distribution = self._params['c'] + self._params['a'] * nest.spatial.distance
        elif spatial_distribution == 'exponential':
            distribution = self._params['c'] + nest.spatial_distributions.exponential(
                nest.spatial.distance,
                beta=self._params['tau'])
        elif spatial_distribution == 'gaussian':
            distribution = self._params['c'] + nest.spatial_distributions.gaussian(
                nest.spatial.distance,
                mean=self._params['mean'],
                std=self._params['sigma'])
        elif spatial_distribution == 'gaussian2d':
            distribution = self._params['c'] + nest.spatial_distributions.gaussian2D(
                nest.spatial.distance,
                nest.spatial.distance,
                mean_x=self._params['mean_x'],
                mean_y=self._params['mean_y'],
                std_x=self._params['sigma_x'],
                std_y=self._params['sigma_y'],
                rho=self._params['rho'])
        elif spatial_distribution == 'gamma':
            distribution = nest.spatial_distributions.gamma(
                nest.spatial.distance,
                kappa=self._params['kappa'],
                theta=self._params['theta'])

        self._conndict = {'rule': 'pairwise_bernoulli',
                          'p': distribution,
                          'mask': maskdict}

    def _calculate_constants(self, spatial_distribution):
        """Calculate constant variables used when calculating distributions and probability density functions

        Variables that can be pre-calculated are calculated here to make the calculation of distributions
        and probability density functions more efficient.
        """
        # Constants for spatial distribution functions
        if spatial_distribution == 'gaussian':
            self.two_sigma2 = 2. * self._params['sigma']**2
        if spatial_distribution == 'gaussian2d':
            self.sigmax2 = self._params['sigma_x']**2
            self.sigmay2 = self._params['sigma_y']**2
        elif spatial_distribution == 'gamma':
            self.kappa_m_1 = self._params['kappa'] - 1
            self.gamma_kappa_mul_theta_pow_kappa = (scipy.special.gamma(self._params['kappa']) *
                                                    self._params['theta']**self._params['kappa'])
        # Constants for pdfs
        x0, y0 = self._roi_2d()  # move coordinates to the right reference area
        # Constants used when using open boundary conditions
        self.a = self._L / 2. - x0  # used to calculate alpha
        self.b = self._L / 2. - y0  # used to calculate beta
        self.c = self._L / 2. + x0  # used to calculate gamma
        self.d = self._L / 2. + y0  # used to calculate delta

        self.sqrt_a2_b2 = math.sqrt(self.a ** 2 + self.b ** 2)
        self.sqrt_a2_d2 = math.sqrt(self.a ** 2 + self.d ** 2)
        self.sqrt_d2_c2 = math.sqrt(self.d ** 2 + self.c ** 2)

        self._L_half = self._L / 2.
        self.pi_half = math.pi / 2.
        self.two_pi = 2 * math.pi
        self.four_pi = 4 * math.pi

    def _constant(self, _):
        """Constant spatial distribution"""
        return self._params

    def _linear(self, D):
        """Linear spatial distribution"""
        return self._params['c'] + self._params['a'] * D

    def _exponential(self, D):
        """Exponential spatial distribution"""
        return self._params['c'] + self._params['a'] * math.exp(-D / self._params['tau'])

    def _gauss(self, D):
        """Gaussian spatial distribution"""
        return (self._params['c'] +
                self._params['p_center'] *
                math.exp(-(D - self._params['mean']) ** 2 / self.two_sigma2))

    def _gauss2d(self, D):
        """Gaussian2D spatial distribution"""
        x_term = (D - self._params['mean_x'])**2 / self.sigmax2
        y_term = (D - self._params['mean_y'])**2 / self.sigmay2
        xy_term = (D - self._params['mean_x']) * (D - self._params['mean_y']) / \
            (self._params['sigma_x']*self._params['sigma_y'])
        return (self._params['c'] +
                self._params['p_center'] *
                math.exp(- (x_term + y_term - 2*self._params['rho']*xy_term)/(2*(1-self._params['rho']**2))))

    def _gamma(self, D):
        """Gamma spatial distribution"""
        return (D**self.kappa_m_1 /
                self.gamma_kappa_mul_theta_pow_kappa *
                math.exp(-D / self._params['theta']))

    def _create_distance_data(self):

        self._reset(self._seed)
        self._build()
        self._connect()

        self._target_dists = sorted(self._target_distances())
        self._all_dists = self._all_distances()

    def _reset(self, seed):
        """
        Reset NEST and seed PRNGs.

        Parameters
        ----------
            seed: PRNG seed value.
        """

        nest.ResetKernel()
        if seed is None:
            seed = rnd.randint(10 ** 10)
        seed = 3 * seed  # Reduces probability of overlapping seed values.
        rnd.seed(seed)
        nest.SetKernelStatus({'rng_seed': seed})

    def _build(self):
        """Create populations."""
        if self._open_bc:
            x = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            y = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            pos = list(zip(x, y))
            self._ls = nest.Create('iaf_psc_alpha',
                                   positions=nest.spatial.free(
                                       [[self._x_d, self._y_d]],
                                       edge_wrap=False))
            self._lt = nest.Create('iaf_psc_alpha',
                                   positions=nest.spatial.free(
                                       pos,
                                       edge_wrap=False))
            self._driver = self._ls
        else:
            x = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            y = rnd.uniform(-self._L / 2., self._L / 2., self._N)
            if self._dimensions == 3:
                z = rnd.uniform(-self._L / 2., self._L / 2., self._N)
                pos = list(zip(x, y, z))
            else:
                pos = list(zip(x, y))
            self._ls = nest.Create('iaf_psc_alpha',
                                   positions=nest.spatial.free(
                                       [[0.] * self._dimensions],
                                       [self._L] * self._dimensions,
                                       edge_wrap=True))
            self._lt = nest.Create('iaf_psc_alpha',
                                   positions=nest.spatial.free(
                                       pos, [self._L] * self._dimensions,
                                       edge_wrap=True))
            self._driver = nest.FindCenterElement(self._ls)

    def _connect(self):
        """Connect populations."""

        nest.Connect(self._ls, self._lt, self._conndict)

    def _all_distances(self):
        """Return distances to all nodes in target population."""

        return nest.Distance(self._driver, self._lt)

    def _target_distances(self):
        """Return distances from source node to connected nodes."""

        # Distance from source node to all nodes in target population
        dist = np.array(nest.Distance(self._driver, self._lt))

        # Target nodes
        connections = nest.GetConnections(source=self._driver)
        target_array = np.array(connections.target)

        # Convert lt node IDs to a NumPy array
        lt_array = np.array(self._lt.tolist())

        # Pick distance values of connected targets only
        target_dist = dist[np.isin(lt_array, target_array)]

        return target_dist

    def _positions(self):
        """Return positions of all nodes."""
        return [tuple(pos) for pos in
                nest.GetPosition(self._lt)]

    def _target_positions(self):
        """Return positions of all connected target nodes."""

        return [tuple(pos) for pos in
                nest.GetTargetPositions(self._driver, self._lt)[0]]

    def _roi_2d(self):
        """
        Moves coordinates (x,y) to triangle area (x',y') in [0,L/2]X[0,x']
        without loss of generality
        """
        self._x_d = -self._x_d if (self._x_d >= -self._L / 2.) and (self._x_d < 0) else self._x_d
        self._y_d = -self._y_d if (self._y_d >= -self._L / 2.) and (self._y_d < 0) else self._y_d
        return np.array([self._x_d, self._y_d]) if self._x_d > self._y_d else np.array([self._y_d, self._x_d])

    def _pdf_2d(self, D):
        """Calculate the probability density function in 2D, at the distance D"""
        if D <= self._L_half:
            return max(0., min(1., self._distribution(D))) * math.pi * D
        elif self._L_half < D <= self._max_dist:
            return max(0., min(1., self._distribution(D))) * D * (math.pi - 4. * math.acos(self._L / (D * 2.)))
        else:
            return 0.

    def _pdf_2d_obc(self, D):
        """Calculate the probability density function in 2D with open boundary conditions, at the distance D"""
        # calculate alpha, beta, gamma, delta:
        alpha = math.acos(self.a / D) if self.a / D <= 1. else 0.
        beta = math.acos(self.b / D) if self.b / D <= 1. else 0.
        gamma = math.acos(self.c / D) if self.c / D <= 1. else 0.
        delta = math.acos(self.d / D) if self.d / D <= 1. else 0.
        kofD = max(0., min(1., self._distribution(D)))

        if (D >= 0) and (D < self.a):
            return self.two_pi * D * kofD
        if (self.sqrt_a2_b2 > self.c and self.sqrt_a2_d2 > self.c and
                D >= self.c and D < self.sqrt_a2_b2):
            return 2 * D * (math.pi - alpha - beta - delta - gamma) * kofD
        if D >= self.sqrt_d2_c2:
            return 0.

    def _pdf_3d(self, D):
        """Calculate the probability density function in 3D, at the distance D"""
        if D <= self._L_half:
            return max(0., min(1., self._distribution(D))) * self.four_pi * D ** 2.
        elif self._L_half < D <= self._L / math.sqrt(2):
            return max(0., min(1., self._distribution(D))) * self.two_pi * D * (3. * self._L - 4. * D)
        elif self._L / math.sqrt(2) < D <= self._max_dist:
            A = self.four_pi * D ** 2.
            C = self.two_pi * D * (D - self._L_half)
            alpha = math.asin(1. / math.sqrt(2. - self._L ** 2. / (2. * D ** 2.)))
            beta = self.pi_half
            gamma = math.asin(math.sqrt((1. - .5 * (self._L / D) ** 2.) /
                                        (1. - .25 * (self._L / D) ** 2.)))
            T = D ** 2. * (alpha + beta + gamma - math.pi)
            return (max(0., min(1., self._distribution(D))) *
                    (A + 6. * C * (-1. + 4. * gamma / math.pi) - 48. * T))
        else:
            return 0.

    def _cdf(self, D):
        """
        Normalized cumulative distribution function (CDF).

        Parameters
        ----------
            D: Iterable of distances in interval [0, max_dist].

        Return values
        -------------
            List of CDF(d) for each distance d in D.
        """
        cdf = np.zeros(len(D))
        last_d = 0.
        for i, d in enumerate(D):
            cdf[i] = scipy.integrate.quad(self._pdf, last_d, d)[0]
            last_d = d

        cdf = np.cumsum(cdf)
        top = scipy.integrate.quad(self._pdf, 0, self._max_dist)[0]
        normed_cdf = cdf / top

        return normed_cdf

    def ks_test(self):
        """
        Perform a Kolmogorov-Smirnov GOF test on the distribution
        of distances to connected nodes.

        Return values
        -------------
            KS statistic.
            p-value from KS test.
        """

        if self._target_dists is None:
            self._create_distance_data()

        ks, p = scipy.stats.kstest(self._target_dists, self._cdf,
                                   alternative='two_sided')

        return ks, p

    def z_test(self):
        """
        Perform a Z-test on the total number of connections.

        Return values
        -------------
            Standard score (z-score).
            Two-sided p-value.
        """

        if self._target_dists is None or self._all_dists is None:
            self._create_distance_data()

        num_targets = len(self._target_dists)

        ps = ([max(0., min(1., self._distribution(D)))
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
        """Add plotting capability to SpatialTester."""

        def __init__(self, seed, dim, L, N, spatial_distribution, distribution_params=None,
                     open_bc=False, x0=0., y0=0.):
            SpatialTester.__init__(self, seed, dim, L, N, spatial_distribution,
                                   distribution_params, open_bc, x0, y0)

        def show_network(self):
            """Plot nodes in the network."""

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
            """
            Plot the cumulative distribution function (CDF) of
            source-target distances.
            """

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
            """
            Plot the probability density function of source-target distances.

            Parameters
            ----------
                bins: Number of histogram bins for PDF plot.
            """

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
    Test for 2D distributions.
    """

    def test_constant(self):
        distribution = 'constant'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_linear(self):
        distribution = 'linear'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_exponential(self):
        distribution = 'exponential'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gaussian(self):
        distribution = 'gaussian'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gaussian2d(self):
        distribution = 'gaussian2d'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gamma(self):
        distribution = 'gamma'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))


class TestSpatial2DOBC(unittest.TestCase):
    """
    Test for 2D distributions with open boundary conditions.
    """

    def test_constant(self):
        distribution = 'constant'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_linear(self):
        distribution = 'linear'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_exponential(self):
        distribution = 'exponential'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gaussian(self):
        distribution = 'gaussian'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gaussian2d(self):
        distribution = 'gaussian2d'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gamma(self):
        distribution = 'gamma'
        test = SpatialTester(seed=SEED, dim=2, L=1.0, N=10000,
                             spatial_distribution=distribution, open_bc=True)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))


class TestSpatial3D(unittest.TestCase):
    """
    Test for 3D distributions.
    """

    def test_constant(self):
        distribution = 'constant'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_linear(self):
        distribution = 'linear'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_exponential(self):
        distribution = 'exponential'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gaussian(self):
        distribution = 'gaussian'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))

    def test_gamma(self):
        distribution = 'gamma'
        test = SpatialTester(seed=SEED, dim=3, L=1.0, N=10000,
                             spatial_distribution=distribution)
        _, p_ks = test.ks_test()
        _, p_Z = test.z_test()
        self.assertGreater(p_ks, P_MIN, '{} failed KS-test'.format(distribution))
        self.assertGreater(p_Z, P_MIN, '{} failed Z-test'.format(distribution))


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
                                     spatial_distribution='gaussian')
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
