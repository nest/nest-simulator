# -*- coding: utf-8 -*-
#
# test_connect_symmetric_pairwise_bernoulli.py
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


import collections
import unittest

import connect_test_base
import nest
import numpy as np
import scipy.stats

HAVE_THREADS = nest.build_info["have_threads"]


@unittest.skipIf(not HAVE_THREADS, "NEST was compiled without multi-threading")
class TestSymmetricPairwiseBernoulli(connect_test_base.ConnectTestBase):
    # sizes of source-, target-population and connection probability for
    # statistical test
    N_s = 60
    N_t = 60
    # specify connection pattern and specific params
    rule = "symmetric_pairwise_bernoulli"
    p = 0.5
    conn_dict = {"rule": rule, "p": p, "allow_multapses": True, "allow_autapses": False, "make_symmetric": True}
    # Critical values and number of iterations of two level test
    stat_dict = {"alpha2": 0.05, "n_runs": 300}

    def testStatistics(self):
        for fan in ["in", "out"]:
            expected = connect_test_base.get_expected_degrees_bernoulli(self.p, fan, self.N_s, self.N_t)

            pvalues = []
            for i in range(self.stat_dict["n_runs"]):
                connect_test_base.reset_seed(i + 1, self.nr_threads)
                self.setUpNetwork(conn_dict=self.conn_dict, N1=self.N_s, N2=self.N_t)
                degrees = connect_test_base.get_degrees(fan, self.pop1, self.pop2)
                degrees = connect_test_base.gather_data(degrees)
                # degrees = self.comm.gather(degrees, root=0)
                # if self.rank == 0:
                if degrees is not None:
                    chi, p = connect_test_base.chi_squared_check(degrees, expected, self.rule)
                    pvalues.append(p)
                connect_test_base.mpi_barrier()
            if degrees is not None:
                ks, p = scipy.stats.kstest(pvalues, "uniform")
                self.assertGreater(p, self.stat_dict["alpha2"])

    def testAutapsesTrue(self):
        conn_params = self.conn_dict.copy()
        conn_params["allow_autapses"] = True
        N = 10

        # test that autapses are not permitted
        pop = nest.Create("iaf_psc_alpha", N)
        with self.assertRaises(nest.NESTError):
            nest.Connect(pop, pop, conn_params)

    def testAutapsesFalse(self):
        conn_params = self.conn_dict.copy()
        N = 10

        # test that autapses were excluded
        conn_params["p"] = 1.0 - 1.0 / N
        conn_params["allow_autapses"] = False
        pop = nest.Create("iaf_psc_alpha", N)
        nest.Connect(pop, pop, conn_params)
        M = connect_test_base.get_connectivity_matrix(pop, pop)
        connect_test_base.mpi_assert(np.diag(M), np.zeros(N), self)

    def testMultapses(self):
        conn_params = self.conn_dict.copy()
        conn_params["allow_multapses"] = False
        N = 10

        # test that multapses must be permitted
        nest.ResetKernel()
        pop = nest.Create("iaf_psc_alpha", N)
        with self.assertRaises(nest.NESTError):
            nest.Connect(pop, pop, conn_params)

        # test that multapses can only arise from symmetric
        # connectivity
        conn_params["p"] = 1.0 - 1.0 / N
        conn_params["allow_multapses"] = True
        nest.ResetKernel()
        pop = nest.Create("iaf_psc_alpha", N)
        nest.Connect(pop, pop, conn_params)

        conn_dict = collections.defaultdict(int)
        conn = nest.GetConnections()
        for s_t_key in zip(conn.sources(), conn.targets()):
            conn_dict[s_t_key] += 1
            self.assertTrue(conn_dict[s_t_key] <= 2)

    def testCannotSetMSFalse(self):
        conn_params = self.conn_dict.copy()
        N = 100

        # test that make_symmetric must be enabled
        conn_params["make_symmetric"] = False
        nest.ResetKernel()
        pop = nest.Create("iaf_psc_alpha", N)
        with self.assertRaises(nest.NESTError):
            nest.Connect(pop, pop, conn_params)

    def testMakeSymmetric(self):
        conn_params = self.conn_dict.copy()
        N = 100

        # test that all connections are symmetric
        conn_params["make_symmetric"] = True
        nest.ResetKernel()
        pop = nest.Create("iaf_psc_alpha", N)
        nest.Connect(pop, pop, conn_params)

        M = connect_test_base.get_connectivity_matrix(pop, pop)
        print(M)
        M_all = connect_test_base.gather_data(M)
        print(M_all)
        if M_all is not None:
            self.assertTrue(np.array_equal(M_all, M_all.T))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSymmetricPairwiseBernoulli)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
