# -*- coding: utf-8 -*-
#
# test_connect_tripartite_bernoulli.py
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


import nest
import numpy as np
import pytest
import scipy.stats

try:
    from mpi4py import MPI

    haveMPI4Py = True
except ImportError:
    haveMPI4Py = False


def setup_network(conn_dict, syn_dict, N1, N2, primary_model="aeif_cond_alpha_astro", third_model="astrocyte_lr_1994"):
    """Setup the network for the statistical test.

    A three-population network is created for a statictical test of the
    "tripartite_bernoulli_with_pool" rule.

    Parameters
    ---------
    conn_dict
        Dictionary for the connectivity specifications (conn_spec).
    syn_dict
        Dictionary for the synapse specifications (syn_spec).
    N1
        Number of nodes in the source population.
    N2
        Number of nodes in the target population.
    primary_model
        Model name for the nodes of the primary (source and target) populations.
    third_model
        Model name for the nodes of the third population.

    Return values
    -------------
        Nodes of the three populations created.

    """
    pop1 = nest.Create(primary_model, N1)
    pop2 = nest.Create(primary_model, N2)
    pop3 = nest.Create(third_model, N2)
    nest.TripartiteConnect(pop1, pop2, pop3, conn_spec=conn_dict, syn_specs=syn_dict)
    return pop1, pop2, pop3


def get_expected_degrees_bernoulli(p, fan, len_source_pop, len_target_pop):
    """Calculate expected degree distribution.

    Degrees with expected number of observations below e_min are combined
    into larger bins.

    Parameters
    ---------
    p
        Connection probability between the source and target populations.
    fan
        Type of degree to be calculated.
    len_source_pop
        Number of nodes in the source population.
    len_source_pop
        Number of nodes in the target population.

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

    data_front = []  # data from degree=0 to degree=mid-1 (mid is the expected degree in average)
    cumexp = 0.0  # expected number of observations
    bins_combined = 0
    # iterate from degree=0 to degree=mid-1
    for degree in range(mid):
        # for each degree, generate expected number of observations, with binomial distribution
        cumexp += scipy.stats.binom.pmf(degree, n, p) * n_p
        bins_combined += 1
        # if cumexp is < e_min, keep it and combine it into another bin later
        if cumexp < e_min:
            if degree == mid - 1:
                if len(data_front) == 0:
                    raise RuntimeWarning("Not enough data")
                deg, exp, obs, num = data_front[-1]
                data_front[-1] = (deg, exp + cumexp, obs, num + bins_combined)
            else:
                continue
        # if cumexp is >= e_min, append the data (along with previous data to be combined)
        else:
            data_front.append((degree - bins_combined + 1, cumexp, 0, bins_combined))
            # reset number of observations and move on to the next degree
            cumexp = 0.0
            bins_combined = 0

    data_back = []  # data from degree=mid to degree=n
    cumexp = 0.0
    bins_combined = 0
    # do the same iteration but from degree=n to degree=mid
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

    # combine to obtain the expected degree distribution
    expected = np.array(data_front + data_back)
    if fan == "out":
        assert sum(expected[:, 3]) == len_target_pop + 1
    else:
        assert sum(expected[:, 3]) == len_source_pop + 1

    return expected


def get_degrees(fan, pop1, pop2):
    """Get total indegree or outdegree in a connectivity matrix.

    The total indegree or outdegree in the connectivity matrix of two
    connected populations is calculated.

    Parameters
    ---------
    p
        Connection probability between the source and target populations.
    fan
        Type of degree to be calculated.
    pop1
        Source population.
    pop2
        Target population.

    Return values
    -------------
        Total indegree or outdegree.

    """
    M = get_connectivity_matrix(pop1, pop2)
    if fan == "in":
        degrees = np.sum(M, axis=1)
    elif fan == "out":
        degrees = np.sum(M, axis=0)
    return degrees


def gather_data(data_array):
    """Gather data of a given array from all MPI processes.

    Gathers data from all MPI processes by collecting all element in a list if
    data is a list and summing all elements to one numpy-array if data is one
    numpy-array. Returns gathered data if rank of current MPI node is zero and
    None otherwise.

    Parameters
    ---------
    data_array
        The array for which data are to be obtained.

    Return values
    -------------
        If MPI is available, the array with data from all MPI processes
        (MPI node is zero) or None (otherwise) is returned. If MPI is not
        available, the original array is returned.

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


def chi_squared_check(degrees, expected):
    """
    Compare the actual degree distribution with the expected distribution using
    Pearson's chi-squared GOF test.

    Parameters
    ----------
    degrees
        The actual degree distribution to be evaluated.
    expected
        The expected degree distribution.

    Return values
    -------------
        chi-squared statistic.
        p-value from chi-squared test.

    """
    observed = {}
    for degree in degrees:
        if degree not in observed:
            observed[degree] = 1
        else:
            observed[degree] += 1

    # add observations to data structure
    # combine multiple observations where necessary
    expected[:, 2] = 0.0
    for row in expected:
        for i in range(int(row[3])):
            deg = int(row[0]) + i
            if deg in observed:
                row[2] += observed[deg]

    # adjustment to the degrees of freedom
    return scipy.stats.chisquare(np.array(expected[:, 2]), np.array(expected[:, 1]))


def mpi_barrier():
    """Forms a barrier for MPI processes until all of them call the function."""
    if haveMPI4Py:
        MPI.COMM_WORLD.Barrier()


def get_connectivity_matrix(pop1, pop2):
    """
    Returns a connectivity matrix describing all connections from pop1 to pop2
    such that M_ij describes the connection between the jth node in pop1 to
    the ith node in pop2.

    Parameters
    ---------
    pop1
        Source population.
    pop2
        Target population.

    Return values
    -------------
        A connectivity matrix describing all connections from pop1 to pop2.

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


def mpi_assert(data_original, data_test):
    """
    Compares two arrays with assert and pytest.

    Parameters
    ---------
    data_original
        The original data.
    data_test
        The data to be compared with.
    """
    data_original = gather_data(data_original)
    if data_original is not None:
        if isinstance(data_original, (np.ndarray, np.generic)) and isinstance(data_test, (np.ndarray, np.generic)):
            assert data_original == pytest.approx(data_test)
        else:
            assert data_original == data_test


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("p_primary", [0.1, 0.3, 0.5])
def test_statistics(p_primary):
    """
    A test for the parameters "p_primary" and "pool_size" in the
    "tripartite_bernoulli_with_pool" rule.

    Parameters
    ---------
    p_primary
        Connection probability between the primary populations.

    """
    # set network and connectivity parameters
    N1 = 50
    N2 = 50
    pool_size = 5
    conn_dict = {"rule": "tripartite_bernoulli_with_pool", "p_primary": p_primary, "pool_size": pool_size}

    # set test parameters
    stat_dict = {"alpha2": 0.05, "n_runs": 20}
    nr_threads = 2

    # set NEST verbosity
    nest.set_verbosity("M_FATAL")

    # here we test
    # 1. p_primary yields the correct indegree and outdegree
    # 2. pool_size limits the number of third-population nodes connected to each target nodes
    for fan in ["in", "out"]:
        expected = get_expected_degrees_bernoulli(conn_dict["p_primary"], fan, N1, N2)
        pvalues = []
        n_third_nodes = []
        for i in range(stat_dict["n_runs"]):
            # setup network and connect
            nest.ResetKernel()
            nest.local_num_threads = nr_threads
            nest.rng_seed = i + 1
            pop1, pop2, pop3 = setup_network(conn_dict, {"third_out": {"synapse_model": "sic_connection"}}, N1, N2)
            # get indegree or outdegree
            degrees = get_degrees(fan, pop1, pop2)
            # gather data from MPI processes
            degrees = gather_data(degrees)
            # do chi-square test for indegree or outdegree
            if degrees is not None:
                chi, p_degrees = chi_squared_check(degrees, expected)
                pvalues.append(p_degrees)
            mpi_barrier()
            # get number of third_nodes connected to each target nodes
            conns_n2n = nest.GetConnections(pop1, pop2).get()
            conns_a2n = nest.GetConnections(pop3, pop2).get()
            for id in list(set(conns_n2n["target"])):
                third_nodes = np.array(conns_a2n["source"])
                targets = np.array(conns_a2n["target"])
                n_third_nodes.append(len(set(third_nodes[targets == id])))
        # assert that the p-values are uniformly distributed
        if degrees is not None:
            ks, p_uniform = scipy.stats.kstest(pvalues, "uniform")
            assert p_uniform > stat_dict["alpha2"]
        # assert that for each target nodes, number of third_nodes is smaller than pool_size
        assert all(n <= pool_size for n in n_third_nodes)


@pytest.mark.parametrize("autapses", [True, False])
def test_autapses_true(autapses):
    """
    A test for autapses in the "tripartite_bernoulli_with_pool" rule.

    Parameters
    ---------
    autapses
        If True, autapses are allowed. If False, not allowed.

    """
    # set network and connectivity parameters
    N = 50
    conn_dict = {
        "rule": "tripartite_bernoulli_with_pool",
        "p_primary": 1.0,
        "allow_autapses": autapses,
    }

    # set NEST verbosity
    nest.set_verbosity("M_FATAL")

    # create the network
    pop_primay = nest.Create("aeif_cond_alpha_astro", N)
    pop_third = nest.Create("astrocyte_lr_1994", N)
    nest.TripartiteConnect(
        pop_primay,
        pop_primay,
        pop_third,
        conn_spec=conn_dict,
        syn_specs={"third_out": {"synapse_model": "sic_connection"}},
    )

    # make sure autapses do (or do not) exist
    M = get_connectivity_matrix(pop_primay, pop_primay)
    mpi_assert(np.diag(M), autapses * np.ones(N))
    mpi_assert(np.sum(M), N**2 - (1 - autapses) * N)
