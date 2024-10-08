# -*- coding: utf-8 -*-
#
# test_connect_fixed_outdegree.py
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

HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@unittest.skipIf(not HAVE_OPENMP, "NEST was compiled without multi-threading")
@nest.ll_api.check_stack
class TestFixedOutDegree(connect_test_base.ConnectTestBase):
    # sizes of source-, target-population and outdegree for connection test
    N1 = 50
    N2 = 70
    Nout = 10

    # specify connection pattern and specific params
    conn_dict = nest.FixedOutdegree(source=None, target=None, outdegree=Nout)

    # sizes of source-, target-population and outdegree for statistical test
    N_s = 10
    N_t = 10
    C = 10
    # Critical values and number of iterations of two level test
    stat_dict = {"alpha2": 0.05, "n_runs": 400}

    # tested on each mpi process separately
    def testErrorMessages(self):
        got_error = False
        conn_params = nest.FixedOutdegree(
            source=None, target=None, outdegree=self.N2 + 1, allow_autapses=True, allow_multapses=False
        )

        try:
            self.setUpNetwork(conn_params)
        except nest.kernel.NESTError:
            got_error = True
        self.assertTrue(got_error)

    def testOutDegree(self):
        conn_params = nest.FixedOutdegree(
            source=None, target=None, outdegree=self.Nout, allow_autapses=False, allow_multapses=False
        )
        self.setUpNetwork(conn_params)
        # make sure the outdegree is right
        M = connect_test_base.get_connectivity_matrix(self.pop1, self.pop2)
        outds = np.sum(M, axis=0)
        conns = nest.GetConnections()
        connect_test_base.mpi_assert(outds, self.Nout * np.ones(self.N1), self)
        # make sure no connections were drawn from the target to the source
        # population
        M = connect_test_base.get_connectivity_matrix(self.pop2, self.pop1)
        M_none = np.zeros((len(self.pop1), len(self.pop2)))
        connect_test_base.mpi_assert(M, M_none, self)

    def testStatistics(self):
        conn_params = nest.FixedOutdegree(
            source=None, target=None, outdegree=self.C, allow_autapses=True, allow_multapses=True
        )

        expected = connect_test_base.get_expected_degrees_fixedDegrees(self.C, "out", self.N_s, self.N_t)
        pvalues = []
        for i in range(self.stat_dict["n_runs"]):
            connect_test_base.reset_seed(i + 1, self.nr_threads)
            self.setUpNetwork(projections=conn_params, N1=self.N_s, N2=self.N_t)
            degrees = connect_test_base.get_degrees("in", self.pop1, self.pop2)
            degrees = connect_test_base.gather_data(degrees)
            if degrees is not None:
                chi, p = connect_test_base.chi_squared_check(degrees, expected)
                pvalues.append(p)
            connect_test_base.mpi_barrier()
        if degrees is not None:
            ks, p = scipy.stats.kstest(pvalues, "uniform")
            self.assertGreater(p, self.stat_dict["alpha2"])

    def testAutapsesTrue(self):
        N = 10

        # test that autapses exist
        pop = nest.Create("iaf_psc_alpha", N)
        conn_params = nest.FixedOutdegree(
            source=pop, target=pop, outdegree=N, allow_multapses=False, allow_autapses=True
        )
        nest.Connect(conn_params)

        # make sure all connections do exist
        M = connect_test_base.get_connectivity_matrix(pop, pop)
        connect_test_base.mpi_assert(np.diag(M), np.ones(N), self)

    def testAutapsesFalse(self):
        N = 10

        # test that autapses were excluded
        pop = nest.Create("iaf_psc_alpha", N)
        conn_params = nest.FixedOutdegree(
            source=pop, target=pop, outdegree=N - 1, allow_multapses=False, allow_autapses=False
        )
        nest.Connect(conn_params)

        # make sure all connections do exist
        M = connect_test_base.get_connectivity_matrix(pop, pop)
        connect_test_base.mpi_assert(np.diag(M), np.zeros(N), self)

    def testMultapsesTrue(self):
        N = 3

        # test that multapses were drawn
        pop = nest.Create("iaf_psc_alpha", N)
        conn_params = nest.FixedOutdegree(
            source=pop, target=pop, outdegree=N + 1, allow_multapses=True, allow_autapses=True
        )
        nest.Connect(conn_params)
        nr_conns = len(nest.GetConnections(pop, pop))
        connect_test_base.mpi_assert(nr_conns, conn_params.outdegree * N, self)

    def testMultapsesFalse(self):
        N = 3

        # test that no multapses exist
        pop = nest.Create("iaf_psc_alpha", N)
        conn_params = nest.FixedOutdegree(
            source=pop, target=pop, outdegree=N, allow_multapses=False, allow_autapses=True
        )
        nest.Connect(conn_params)
        M = connect_test_base.get_connectivity_matrix(pop, pop)
        M = connect_test_base.gather_data(M)

        if M is not None:
            self.assertTrue(M.flatten, np.ones(N * N))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestFixedOutDegree)
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
