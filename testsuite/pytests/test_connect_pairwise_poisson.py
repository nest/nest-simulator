# -*- coding: utf-8 -*-
#
# test_connect_pairwise_poisson.py
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


import unittest

import connect_test_base
import nest
import numpy as np
import scipy.stats
from connect_test_base import get_connectivity_matrix

HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@unittest.skipIf(not HAVE_OPENMP, "NEST was compiled without multi-threading")
@nest.ll_api.check_stack
class TestPairwisePoisson(connect_test_base.ConnectTestBase):
    # specify connection pattern and specific params
    rule = "pairwise_poisson"
    pairwise_avg_num_conns = 0.5
    conn_dict = {"rule": rule, "pairwise_avg_num_conns": pairwise_avg_num_conns}
    # sizes of source-, target-population and connection probability for
    # statistical test
    N_s = 50
    N_t = 50
    # Critical values and number of iterations of two level test
    stat_dict = {"alpha2": 0.8, "n_runs": 20}

    def testErrorMessages(self):
        got_error = False
        conn_params = self.conn_dict.copy()
        conn_params["allow_multapses"] = False
        try:
            self.setUpNetwork(conn_params)
        except nest.kernel.NESTError:
            got_error = True
        self.assertTrue(got_error)

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()

        # test that autapses exist
        conn_params["allow_autapses"] = True
        pop1 = nest.Create("iaf_psc_alpha", self.N1)
        nest.Connect(pop1, pop1, conn_params)
        # make sure all connections do exist
        M = connect_test_base.get_connectivity_matrix(pop1, pop1)
        np.testing.assert_allclose(np.diag(M).sum(), self.N1 * self.pairwise_avg_num_conns, atol=2)

    def testExpInOutdegree(self):
        connect_test_base.reset_seed(1, self.nr_threads)
        self.setUpNetwork(conn_dict=self.conn_dict, N1=self.N_s, N2=self.N_t)
        for fan in ["in", "out"]:
            expected = connect_test_base.get_expected_degrees_poisson(
                self.pairwise_avg_num_conns, fan, self.N_s, self.N_t
            )

            mean_degrees = connect_test_base.get_degrees(fan, self.pop1, self.pop2).mean()

            np.testing.assert_allclose(expected, mean_degrees, atol=2)

    def testStatistics(self):
        multapses = []

        for i in range(self.stat_dict["n_runs"]):
            connect_test_base.reset_seed(i + 1, self.nr_threads)
            self.setUpNetwork(conn_dict=self.conn_dict, N1=1, N2=1)
            multapses.append(connect_test_base.get_degrees("out", self.pop1, self.pop2))

        # Create x values for binning multapses
        x = np.arange(0, 10, 1) - 0.5
        multapse_distribution, auto_bins = np.histogram(multapses, bins=x, density=True)
        bins = (auto_bins[:-1] + auto_bins[1:]) / 2

        expected_distribution = scipy.stats.poisson.pmf(bins, self.pairwise_avg_num_conns)
        _, p = connect_test_base.chi_squared_check(multapse_distribution, expected_distribution, self.rule)

        self.assertTrue(p > self.stat_dict["alpha2"])


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPairwisePoisson)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    unittest.main()
