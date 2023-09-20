# -*- coding: utf-8 -*-
#
# test_connect_pairwise_bernoulli_astro.py
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


import numpy as np
import scipy.stats
import pytest

import nest

# copied from connect_test_base.py
try:
    from mpi4py import MPI
    haveMPI4Py = True
except ImportError:
    haveMPI4Py = False

# adapted from connect_test_base.py
def setup_network(conn_dict, syn_dict, N1, N2, neuron_model="aeif_cond_alpha_astro", astrocyte_model="astrocyte_lr_1994"):
    pop1 = nest.Create(neuron_model, N1)
    pop2 = nest.Create(neuron_model, N2)
    pop_astro = nest.Create(astrocyte_model, N2)
    conn_dict["astrocyte"] = pop_astro
    nest.Connect(pop1, pop2, conn_dict, syn_dict)
    return pop1, pop2, pop_astro

# copied from connect_test_base.py
def get_expected_degrees_bernoulli(p, fan, len_source_pop, len_target_pop):
    """
    Calculate expected degree distribution.

    Degrees with expected number of observations below e_min are combined
    into larger bins.

    Return values
    -------------
        2D array. The four columns contain degree,
        expected number of observation, actual number observations, and
        the number of bins combined.
    """

    n = len_source_pop if fan == "in" else len_target_pop
    n_p = len_target_pop if fan == "in" else len_source_pop
    mid = int(round(n * p))
    e_min = 5

    # Combine from front.
    data_front = []
    cumexp = 0.0
    bins_combined = 0
    for degree in range(mid):
        cumexp += scipy.stats.binom.pmf(degree, n, p) * n_p
        bins_combined += 1
        if cumexp < e_min:
            if degree == mid - 1:
                if len(data_front) == 0:
                    raise RuntimeWarning("Not enough data")
                deg, exp, obs, num = data_front[-1]
                data_front[-1] = (deg, exp + cumexp, obs, num + bins_combined)
            else:
                continue
        else:
            data_front.append((degree - bins_combined + 1, cumexp, 0, bins_combined))
            cumexp = 0.0
            bins_combined = 0

    # Combine from back.
    data_back = []
    cumexp = 0.0
    bins_combined = 0
    for degree in reversed(range(mid, n + 1)):
        cumexp += scipy.stats.binom.pmf(degree, n, p) * n_p
        bins_combined += 1
        if cumexp < e_min:
            if degree == mid:
                if len(data_back) == 0:
                    raise RuntimeWarning("Not enough data")
                deg, exp, obs, num = data_back[-1]
                data_back[-1] = (degree, exp + cumexp, obs, num + bins_combined)
            else:
                continue
        else:
            data_back.append((degree, cumexp, 0, bins_combined))
            cumexp = 0.0
            bins_combined = 0
    data_back.reverse()

    expected = np.array(data_front + data_back)
    if fan == "out":
        assert sum(expected[:, 3]) == len_target_pop + 1
    else:  # , 'Something is wrong'
        assert sum(expected[:, 3]) == len_source_pop + 1

    # np.hstack((np.asarray(data_front)[0], np.asarray(data_back)[0]))
    return expected

# copied from connect_test_base.py
def get_degrees(fan, pop1, pop2):
    M = get_connectivity_matrix(pop1, pop2)
    if fan == "in":
        degrees = np.sum(M, axis=1)
    elif fan == "out":
        degrees = np.sum(M, axis=0)
    return degrees

# copied from connect_test_base.py
def gather_data(data_array):
    """
    Gathers data from all mpi processes by collecting all element in a list if
    data is a list and summing all elements to one numpy-array if data is one
    numpy-array. Returns gathered data if rank of current mpi node is zero and
    None otherwise.

    """
    if haveMPI4Py:
        data_array_list = MPI.COMM_WORLD.gather(data_array, root=0)
        if MPI.COMM_WORLD.Get_rank() == 0:
            if isinstance(data_array, list):
                gathered_data = [item for sublist in data_array_list for item in sublist]
            else:
                gathered_data = sum(data_array_list)
            return gathered_data
        else:
            return None
    else:
        return data_array

# copied from connect_test_base.py
def chi_squared_check(degrees, expected, distribution=None):
    """
    Create a single network and compare the resulting degree distribution
    with the expected distribution using Pearson's chi-squared GOF test.

    Parameters
    ----------
        seed   : PRNG seed value.
        control: Boolean value. If True, _generate_multinomial_degrees will
                 be used instead of _get_degrees.

    Return values
    -------------
        chi-squared statistic.
        p-value from chi-squared test.
    """

    if distribution in ("pairwise_bernoulli", "symmetric_pairwise_bernoulli"):
        observed = {}
        for degree in degrees:
            if degree not in observed:
                observed[degree] = 1
            else:
                observed[degree] += 1
        # Add observations to data structure, combining multiple observations
        # where necessary.
        expected[:, 2] = 0.0
        for row in expected:
            for i in range(int(row[3])):
                deg = int(row[0]) + i
                if deg in observed:
                    row[2] += observed[deg]

        # ddof: adjustment to the degrees of freedom. df = k-1-ddof
        return scipy.stats.chisquare(np.array(expected[:, 2]), np.array(expected[:, 1]))
    else:
        # ddof: adjustment to the degrees of freedom. df = k-1-ddof
        return scipy.stats.chisquare(np.array(degrees), np.array(expected))

# copied from connect_test_base.py
def mpi_barrier():
    if haveMPI4Py:
        MPI.COMM_WORLD.Barrier()

# copied from connect_test_base.py
def get_connectivity_matrix(pop1, pop2):
    """
    Returns a connectivity matrix describing all connections from pop1 to pop2
    such that M_ij describes the connection between the jth neuron in pop1 to
    the ith neuron in pop2.
    """

    M = np.zeros((len(pop2), len(pop1)))
    connections = nest.GetConnections(pop1, pop2)
    index_dic = {}
    for count, node in enumerate(pop1):
        index_dic[node.get("global_id")] = count
    for count, node in enumerate(pop2):
        index_dic[node.get("global_id")] = count
    for source, target in zip(connections.sources(), connections.targets()):
        M[index_dic[target]][index_dic[source]] += 1
    return M

# adapted from connect_test_base.py
def mpi_assert(data_original, data_test):
    """
    Compares data_original and data_test.
    """

    data_original = gather_data(data_original)
    # only test if on rank 0
    if data_original is not None:
        if isinstance(data_original, (np.ndarray, np.generic)) and isinstance(data_test, (np.ndarray, np.generic)):
            assert data_original == pytest.approx(data_test)
        else:
            TestCase.assertTrue(data_original == data_test)

# adapted from test_connect_pairwise_bernoulli.py
# a test for parameters "p" and "max_astro_per_target"
# run for three levels of neuron-neuron connection probabilities
@pytest.mark.parametrize("p_n2n", [0.1, 0.3, 0.5])
def test_statistics(p_n2n):
    # set connection parameters
    N1 = 50
    N2 = 50
    max_astro_per_target = 5
    conn_dict = {
        "rule": "pairwise_bernoulli_astro",
        "p": p_n2n,
        "max_astro_per_target": max_astro_per_target
    }

    # set test parameters
    stat_dict = {"alpha2": 0.05, "n_runs": 20}
    nr_threads = 2

    # set NEST verbosity
    nest.set_verbosity("M_FATAL")

    # here we test
    # 1. p yields the correct indegree and outdegree
    # 2. max_astro_per_target limits the number of astrocytes connected to each target neuron
    for fan in ["in", "out"]:
        expected = get_expected_degrees_bernoulli(conn_dict["p"], fan, N1, N2)
        pvalues = []
        n_astrocytes = []
        for i in range(stat_dict["n_runs"]):
            # setup network and connect
            nest.ResetKernel()
            nest.local_num_threads = nr_threads
            nest.rng_seed = i + 1
            pop1, pop2, pop_astro = setup_network(conn_dict, None, N1, N2)
            # get indegree or outdegree
            degrees = get_degrees(fan, pop1, pop2)
            # gather data from MPI processes
            degrees = gather_data(degrees)
            # do chi-square test for indegree or outdegree
            if degrees is not None:
                chi, p_degrees = chi_squared_check(degrees, expected, "pairwise_bernoulli")
                pvalues.append(p_degrees)
            mpi_barrier()
            # get number of astrocytes connected to each target neuron
            conns_n2n = nest.GetConnections(pop1, pop2).get()
            conns_a2n = nest.GetConnections(pop_astro, pop2).get()
            for id in list(set(conns_n2n["target"])):
                astrocytes = np.array(conns_a2n["source"])
                targets = np.array(conns_a2n["target"])
                n_astrocytes.append(len(set(astrocytes[targets == id])))
        # assert that the p-values are uniformly distributed
        if degrees is not None:
            ks, p_uniform = scipy.stats.kstest(pvalues, "uniform")
            assert p_uniform > stat_dict["alpha2"]
        # assert that for each target neuron, number of astrocytes is smaller than max_astro_per_target
        assert all(n <= max_astro_per_target for n in n_astrocytes)

# adapted from test_connect_pairwise_bernoulli
def test_autapses_true():
    # set connection parameters
    N = 50
    conn_dict = {
        "rule": "pairwise_bernoulli_astro",
        "p": 1.0,
        "allow_autapses": True,
    }

    # set NEST verbosity
    nest.set_verbosity("M_FATAL")

    # test that autapses exist
    pop = nest.Create("aeif_cond_alpha_astro", N)
    pop_astro = nest.Create("astrocyte_lr_1994", N)
    conn_dict["astrocyte"] = pop_astro
    nest.Connect(pop, pop, conn_dict)
    # make sure all connections do exist
    M = get_connectivity_matrix(pop, pop)
    mpi_assert(np.diag(M), np.ones(N))

# adapted from test_connect_pairwise_bernoulli
def test_autapses_false():
    # set connection parameters
    N = 50
    conn_dict = {
        "rule": "pairwise_bernoulli_astro",
        "p": 1.0,
        "allow_autapses": False,
    }

    # set NEST verbosity
    nest.set_verbosity("M_FATAL")

    # test that autapses were excluded
    pop = nest.Create("aeif_cond_alpha_astro", N)
    pop_astro = nest.Create("astrocyte_lr_1994", N)
    conn_dict["astrocyte"] = pop_astro
    nest.Connect(pop, pop, conn_dict)
    # make sure all connections do exist
    M = get_connectivity_matrix(pop, pop)
    mpi_assert(np.diag(M), np.zeros(N))
